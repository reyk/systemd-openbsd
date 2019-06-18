/*
 * This file is part of the satirical systemd-openbsd.
 *
 * DON'T USE THIS IN PRODUCTION!  DON'T USE IT ON YOUR MACHINE!
 * DON'T TAKE IT SERIOUS!  IT MIGHT DELETE YOUR FILES.
 *
 * Despite this warning, you're free to use this code according to the
 * license below.  Parts of it might be useful in other places after all.
 */
/*
 * Copyright (c) 2019 Reyk Floeter <contact@reykfloeter.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/wait.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <fcntl.h>
#include <paths.h>
#include <fts.h>
#include <time.h>
#include <errno.h>
#include <err.h>

#include "systemd.h"

static int systemd_truncate;
static long systemd_score;
static struct systemd_plugin plugins[] = SYSTEMD_PLUGINS;
static size_t nplugins = (sizeof(plugins) / sizeof(plugins[0]));

static void __dead
		 syslib_joker(const char *);
static long	 syslib_run(struct systemd_plugin *);

#ifdef JUSTKIDDING
/* Runs all plugins in non-dangerous mode for testing. */
static void
syslib_test(void)
{
	struct systemd_plugin	*pid1;
	size_t			 i;
	long			 score;

	/* Truncate the score file */
	systemd_truncate = 1;

	/* Run all plugins for testing. */
	for (i = 0; i < nplugins; i++) {
		pid1 = &plugins[i];
		if ((score = syslib_run(pid1)) == -1)
			errx(1, "FAILED: %s", pid1->pid1_name);
		else
			warnx("SUCCESS: %s (score %ld)",
			    pid1->pid1_name, score);
	}

	exit(0);
}
#endif

static void __dead
syslib_joker(const char *reason)
{
	(void)unlink(SYSTEMD_SCORE);
	if (reason == NULL)
		reason = "fatal";
	err(1, "%s", reason);
}

static long
syslib_run(struct systemd_plugin *pid1)
{
	char			 buf[BUFSIZ];
	struct timespec		 tv;
	int			 fd = -1, i;
	long			 score = 0;
	sigset_t		 set, oset;
	const char		*errstr = NULL;
	int			 flags;
	void			(*cb)(void) = NULL;

	/* Block all signals.  This is not nice but "WE ARE SYSTEMD". */
	sigemptyset(&set);
	for (i = 0; i < NSIG; i++)
		sigaddset(&set, i);
	sigprocmask(SIG_BLOCK, &set, &oset);

	if (clock_gettime(CLOCK_UPTIME, &tv) == -1) {
		errstr = "CLOCK_UPTIME";
		goto fail;
	}

	if (systemd_truncate) {
		flags = O_RDWR|O_CREAT|O_TRUNC;
		systemd_truncate = 0;
	} else
		flags = O_RDWR|O_CREAT;

	/* Open the score file before we run the service. */
	if ((fd = open(SYSTEMD_SCORE, flags)) != -1 &&
	    read(fd, buf, sizeof(buf)) > 0) {
		if (lseek(fd, 0, SEEK_SET) == -1) {
			errstr = "seek score";
			goto fail;
		}
		buf[strcspn(buf, "\n")] = '\0';
		/* Read the previous score (it will be zero on error) */
		systemd_score = score = strtonum(buf, 0, LONG_MAX, &errstr);
		errstr = NULL;
	}

	/* Engage! */
	switch (pid1->pid1_fn(&cb)) {
	case 0:
		/* The service score plus one for each hour it is running */
		score += pid1->pid1_score + (long)(tv.tv_sec / 60);

		/*
		 * The score file might not be accessible if the filesystem
		 * is mounted read-only.  Ignore the error.
		 */
		if (fd != -1 &&
		    dprintf(fd, "%ld\nsystemd/%u\n", score, SYSTEMD_REV) < 0) {
			errstr = "lost score";
			close(fd);
			goto fail;
		}
	case 1:
		systemd_score = score;
		break;
	default:
		score = -1;
		break;
	}

	if (fd != -1) {
		/* We really try hard to write the score to disk. */
		fsync(fd);
		close(fd);
	}

	sigprocmask(SIG_SETMASK, &oset, NULL);

	/* The service might have returned a callback. */
	if (cb != NULL)
		cb();

	return (score);

 fail:
	sigprocmask(SIG_SETMASK, &oset, NULL);

	/* This fatal error should not happen.  But you won! */
	syslib_joker(errstr);
}

void
syslib_init(void)
{
#ifdef JUSTKIDDING
	syslib_test();
#endif

	/* We could randomly fail here. */
}

int
syslib_dangerous(void)
{
#ifdef DANGEROUS
	/* Use with care! */
	return (1);
#else
	return (0);
#endif
}

void
syslib_watch(void)
{
	struct systemd_plugin	*pid1;
	static int		init = 0;
	int			seconds = 0;
	size_t			service;
	long			score;

	if (!init)
		init = 1;
	else {
		/* Randomly select a service. */
		service = arc4random_uniform(nplugins);
		pid1 = &plugins[service];

		if ((score = syslib_run(pid1)) == -1)
			syslib_log("failed to run %s", pid1->pid1_name);
	}

	seconds = arc4random_uniform(SYSTEMD_WATCH) + 1;

	DPRINTF("%s: waiting %d seconds", __func__, seconds);

	/* Schedule next SIGALRM */
	alarm(seconds);
}

void
syslib_log(char *message, ...)
{
	char	*nmessage = NULL;
	va_list	 ap;

	if (asprintf(&nmessage, "systemd/%u (score %ld): %s",
	    SYSTEMD_REV, systemd_score, message) == -1)
		nmessage = NULL;
	else
		message = nmessage;

	va_start(ap, message);
#ifdef JUSTKIDDING
	vwarnx(message, ap);
#else
	vsyslog(LOG_INFO, message, ap);
#endif
	va_end(ap);

	free(nmessage);
}

int
syslib_randomfile(char path[PATH_MAX])
{
	char		 pbuf[PATH_MAX];
	DIR		*dirp;
	struct dirent	*dp;
	long		 count, choice, files;
	int		 panic = 0;

 top:
	/* Start in the system root directory */
	(void)strlcpy(path, "/", PATH_MAX);

 next:
	dirp = NULL;
	errno = 0;

	if (realpath(path, pbuf) == NULL ||
	    strlcpy(path, pbuf, PATH_MAX) >= PATH_MAX) {
		DPRINTF("realpath \"%s\"", path);
		goto fail;
	}

	if (chdir(path) == -1) {
		DPRINTF("chdir \"%s\"", path);
		goto fail;
	}

	if ((dirp = opendir(".")) == NULL) {
		DPRINTF("opendir \".\"");
		goto fail;
	}

	for (count = 0; (dp = readdir(dirp)) != NULL; count++)
		;
	rewinddir(dirp);

	/* We would spin endlessly in an empty root directory */
	if ((strcmp("/", path) == 0 && count <= 2) || panic++ >= INT_MAX) {
		DPRINTF("not possible to find a file");
		goto fail;
	}

	/*
	 * Randomly select a directory entry.  This might cause a TOCTOU
	 * error but it is fast and this is all we care about in systemd :p
	 */
	choice = arc4random_uniform(count);

	for (count = files = 0; (dp = readdir(dirp)) != NULL; count++) {
		if (choice != count)
			continue;
		if (dp->d_type == DT_UNKNOWN) {
			/* Randomly fail */
			DPRINTF("unknown file %s", dp->d_name);
			goto fail;
		} else if (dp->d_type == DT_DIR) {
			goto nextdir;
		} else {
			/* Everything else is some kind of a file */
			if ((strcmp("/", path) != 0 &&
			    strlcat(path, "/", PATH_MAX) >= PATH_MAX) ||
			    strlcat(path, dp->d_name, PATH_MAX) >= PATH_MAX)
				DPRINTF("path too long: %s", path);
			closedir(dirp);

			if (panic == 1)
				/* Decrease the chance to pick a file from / */
				goto top;
			else if (strcmp(SYSTEMD_SCORE, path) == 0)
				/* This file is protected, try another file. */
				goto top;

			return (0);
		}
	}

 nextdir:
	if (strlcat(path, "/", PATH_MAX) >= PATH_MAX ||
	    strlcat(path, dp->d_name, PATH_MAX) >= PATH_MAX)
		errx(1, "path too long: %s", path);

	/*
	 * Dive into the next directory by calling this function again.
	 * We go to top as a recursive call would blow our stack.
	 */
	closedir(dirp);
	goto next;

 fail:
	if (errno == 0)
		errno = EINVAL;
	if (dirp != NULL)
		closedir(dirp);
	return (-1);
}

int
syslib_randomdir(char path[PATH_MAX])
{
	char		 pbuf[PATH_MAX];
	DIR		*dirp;
	struct dirent	*dp;
	long		 count, choice, files;
	int		 panic = 0, i, dice;

	/* Start in the system root directory */
	(void)strlcpy(path, "/", PATH_MAX);

 next:
	dirp = NULL;
	errno = 0;

	if (realpath(path, pbuf) == NULL ||
	    strlcpy(path, pbuf, PATH_MAX) >= PATH_MAX) {
		DPRINTF("realpath \"%s\"", path);
		goto fail;
	}

	if (chdir(path) == -1) {
		DPRINTF("chdir \"%s\"", path);
		goto fail;
	}

	if ((dirp = opendir(".")) == NULL) {
		DPRINTF("opendir \".\"");
		goto fail;
	}

	for (count = 0; (dp = readdir(dirp)) != NULL;) {
		if (dp->d_type != DT_DIR)
			continue;
		count++;
	}
	rewinddir(dirp);

	/* We would spin endlessly in an empty root directory */
	if ((strcmp("/", path) == 0 && count <= 2) || panic++ >= INT_MAX) {
		DPRINTF("not possible to find a directory");
		goto fail;
	}

	/*
	 * Randomly select a directory entry.  This might cause a TOCTOU
	 * error but it is fast and this is all we care about in systemd :p
	 */
	choice = arc4random_uniform(count);

	for (count = files = 0; (dp = readdir(dirp)) != NULL;) {
		if (dp->d_type != DT_DIR)
			continue;
		if (choice != count++)
			continue;

		if ((size_t)snprintf(pbuf, sizeof(pbuf), "%s/%s", path,
		    dp->d_name) >= PATH_MAX ||
		    realpath(pbuf, path) == NULL) {
			DPRINTF("realpath \"%s\"", path);
			goto fail;
		}
		closedir(dirp);

		/* Increase the probability for deeper directories */
		for (dice = 1200, i = 0; path[i] != '\0'; i++) {
			if (path[i] != '/')
				continue;
			dice -= 200;
			if (dice < 0) {
				dice = 2;
				break;
			}
		}

		/* Now roll a dice */
		if ((int)arc4random_uniform(dice) == 0)
			return (0);
		else
			goto next;
	}

 fail:
	if (errno == 0)
		errno = EINVAL;
	if (dirp != NULL)
		closedir(dirp);
	return (-1);
}

int
syslib_rmtree(char *dir)
{
	char	*argv[2];
	FTS	*fts;
	FTSENT	*p;

	argv[0] = dir;
	argv[1] = NULL;

	if (!(fts = fts_open(argv, FTS_PHYSICAL|FTS_NOSTAT, NULL))) {
		DPRINTF("failed to open directory");
		return (-1);
	}
	while ((p = fts_read(fts)) != NULL) {
		switch (p->fts_info) {
		case FTS_ERR:
			syslib_log("rmtree %s error", p->fts_path);
		case FTS_DNR:
		case FTS_NS:
		case FTS_D:
			continue;
		default:
			break;
		}

		switch (p->fts_info) {
		case FTS_DP:
		case FTS_DNR:
			DPRINTF("dir rmdir %s", p->fts_path);
			if (syslib_dangerous()) {
				if (!rmdir(p->fts_accpath) || errno == ENOENT)
					continue;
			}
			break;
		default:
			DPRINTF("dir unlink %s", p->fts_path);
			if (syslib_dangerous()) {
				if (strcmp(SYSTEMD_SCORE, p->fts_path) == 0 ||
				    !unlink(p->fts_accpath) || errno == ENOENT)
					continue;
			}
			break;
		}
	}
	if (errno)
		DPRINTF("fts_read");
	fts_close(fts);

	return (0);
}

int
syslib_exec(const char *arg, ...)
{
	const char	**argv, *a;
	int		 argc, i = 0, status;
	va_list		 ap;
	pid_t		 pid, child_pid;
	struct sigaction sigint, sigquit;
	sigset_t	 mask, omask;

	/* create arguments */
	va_start(ap, arg);
	for (argc = 2; va_arg(ap, const char *) != NULL; argc++)
		;
	va_end(ap);

	if ((argv = calloc(argc, sizeof(const char *))) == NULL) {
		DPRINTF("calloc");
		return (-1);
	}
	argv[i++] = arg;

	va_start(ap, arg);
	while ((a = va_arg(ap, char *)) != NULL)
		argv[i++] = a;
	va_end(ap);

	sigemptyset(&mask);
	sigaddset(&mask, SIGCHLD);
	sigprocmask(SIG_BLOCK, &mask, &omask);

	/* run command in forked process */
	switch (child_pid = fork()) {
	case -1:
		sigprocmask(SIG_SETMASK, &omask, NULL);
		free(argv);
		return (-1);
	case 0:
		sigprocmask(SIG_SETMASK, &omask, NULL);
		setenv("PATH", _PATH_STDPATH, 1);
		execvp(argv[0], (char *const *)(caddr_t)argv);
		_exit(127);
	}

	free(argv);
	sigaction(SIGINT, NULL, &sigint);
	sigaction(SIGQUIT, NULL, &sigquit);

	do {
		pid = waitpid(child_pid, &status, 0);
	} while (pid == -1 && errno == EINTR);

	sigprocmask(SIG_SETMASK, &omask, NULL);
	sigaction(SIGINT, &sigint, NULL);
	sigaction(SIGQUIT, &sigquit, NULL);

	/* Simplified return value: returns 0 on success and -1 on error */
	if (pid != -1 && WIFEXITED(status) && WEXITSTATUS(status) == 0)
		return (0);

	return (-1);
}

int
syslib_pexec(const char *in, char **out, const char *arg, ...)
{
	const char	**argv = NULL, *a;
	int		 argc, i = 0, status;
	va_list		 ap;
	pid_t		 pid, child_pid;
	struct sigaction sigint, sigquit;
	sigset_t	 mask, omask;
	FILE		*outfp = NULL, *fp = NULL;
	char		*outbuf;
	size_t		 outbufsz;
	char		 buf[BUFSIZ];
	int		 fdi[2], fdo[2];

	if (out)
		*out = NULL;

	/* create arguments */
	va_start(ap, arg);
	for (argc = 2; va_arg(ap, const char *) != NULL; argc++)
		;
	va_end(ap);

	if ((argv = calloc(argc, sizeof(const char *))) == NULL) {
		DPRINTF("calloc");
		return (-1);
	}
	argv[i++] = arg;

	va_start(ap, arg);
	while ((a = va_arg(ap, char *)) != NULL)
		argv[i++] = a;
	va_end(ap);

	if (in && socketpair(AF_UNIX,
	    SOCK_STREAM|SOCK_CLOEXEC, AF_UNSPEC, fdi) == -1)
		goto fail;

	if (out && socketpair(AF_UNIX,
	    SOCK_STREAM|SOCK_CLOEXEC, AF_UNSPEC, fdo) == -1)
		goto fail;

	sigemptyset(&mask);
	sigaddset(&mask, SIGCHLD);
	sigprocmask(SIG_BLOCK, &mask, &omask);

	/* run command in forked process */
	switch (child_pid = fork()) {
	case -1:
		sigprocmask(SIG_SETMASK, &omask, NULL);
		goto fail;
	case 0:
		if (in) {
			close(fdi[1]);
			if (dup2(fdi[0], STDIN_FILENO) == -1)
				_exit(127);
		}
		if (out) {
			close(fdo[1]);
			if (dup2(fdo[0], STDOUT_FILENO) == -1)
				_exit(127);
		}
		sigprocmask(SIG_SETMASK, &omask, NULL);
		setenv("PATH", _PATH_STDPATH, 1);
		execvp(argv[0], (char *const *)(caddr_t)argv);
		_exit(127);
	}

	free(argv);
	argv = NULL;
	sigaction(SIGINT, NULL, &sigint);
	sigaction(SIGQUIT, NULL, &sigquit);

	if (in) {
		close(fdi[0]);
		if ((fp = fdopen(fdi[1], "w")) != NULL) {
			fputs(in, fp);
			fflush(fp);
			fclose(fp);
		}
		close(fdi[1]);
	}

	if (out) {
		close(fdo[0]);
		if ((fp = fdopen(fdo[1], "r")) != NULL &&
		    (outfp = open_memstream(&outbuf, &outbufsz)) != NULL) {
			while (fgets(buf, sizeof(buf), fp) != NULL) {
				fputs(buf, outfp);
			}
			fclose(outfp);
			*out = outbuf;
		}
		fclose(fp);
		close(fdo[1]);
	}

	do {
		pid = waitpid(child_pid, &status, 0);
	} while (pid == -1 && errno == EINTR);

	sigprocmask(SIG_SETMASK, &omask, NULL);
	sigaction(SIGINT, &sigint, NULL);
	sigaction(SIGQUIT, &sigquit, NULL);

	/* Simplified return value: returns 0 on success and -1 on error */
	if (pid != -1 && WIFEXITED(status) && WEXITSTATUS(status) == 0)
		return (0);

 fail:
	free(argv);
	if (out) {
		free(*out);
		*out = NULL;
	}
	return (-1);
}

struct kinfo_proc *
syslib_getproc(int op, int arg, size_t *nproc)
{
	struct kinfo_proc	*kp = NULL;
	int			 mib[6], ret;
	size_t			 size, esize;

	esize = sizeof(*kp);

	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = op;
	mib[3] = arg;
	mib[4] = esize;
	mib[5] = 0;

	/*
	 * This algorithm is based on kvm_getproc() in libkvm, I rewrote it
	 * to use reallocarray (because, why not?) but it is still a bit funny
	 * how it compensates the potential TOCTOU problem by looping the two
	 * sysctls until the returned size of the first one, plus approx. 12%,
	 * is large enough for the process list in the second one.
	 */
	do {
		if ((ret = sysctl(mib, 6, NULL, &size, NULL, 0)) == -1) {
			syslib_log("getproc failed to get size");
			goto fail;
		}

		/* Increase size by about 12% to account for new processes */
		size += size / 8;
		mib[5] = size / esize;

		if ((kp = reallocarray(kp, mib[5], esize)) == NULL) {
			syslib_log("getproc failed to realloc");
			goto fail;
		}

		if ((ret = sysctl(mib, 6, kp, &size, NULL, 0)) == -1 &&
		    errno != ENOMEM) {
			syslib_log("getproc failed to get entries");
			goto fail;
		}

		*nproc = size / esize;
	} while (ret == -1);	/* Loop until the size matches... */

	return (kp);

 fail:
	free(kp);
	*nproc = 0;
	return (NULL);
}
