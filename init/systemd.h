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

#include <stdarg.h>
#include <limits.h>
#include <err.h>

#ifndef SYSTEMD_LIB
#define SYSTEMD_LIB

/*
 * A note about the preprocessor definitions:
 * -DDANGEROUS: Compile with this flag to make system_dangerous() succeed.
 * -DDEBUG: Enable extra verbose debug printing.
 * -DJUSTKIDDING: Compile with this flag to build the tests instead of init.
 */
#if defined(DANGEROUS) && defined(JUSTKIDDING)
#error "DANGEROUS and JUSTKIDDING are mutually exclusive"
#endif

#ifndef DEBUG
#define DPRINTF(x...)	do {} while (0)
#else
#define DPRINTF		syslib_log
#endif

/* The revision (bumped for every new service or score alg change). */
#define SYSTEMD_REV	6

/* The score file. */
#define SYSTEMD_SCORE	"/systemd-score.txt"

/* The maximum number of seconds systemd waits until the next action. */
#define SYSTEMD_WATCH	30

/* Init systemd service.  To be called by the executable first. */
void	 syslib_init(void);

/* Returns a true value if dangerous mode is enabled.  Use with care. */
int	 syslib_dangerous(void);

/* Runs the next random things. */
void	 syslib_watch(void);

/* For noisy logging. */
void	 syslib_log(char *, ...);

/* Select a random file.  Pass a PATH_MAX buffer. */
int	 syslib_randomfile(char [PATH_MAX])
	 __attribute__((__bounded__(__minbytes__,1,PATH_MAX)));

/* Select a random directory.  Pass a PATH_MAX buffer. */
int	 syslib_randomdir(char [PATH_MAX])
	 __attribute__((__bounded__(__minbytes__,1,PATH_MAX)));

/* Recursively delete a directory. */
int	 syslib_rmtree(char *);

/* Execute a program. */
int	 syslib_exec(const char *, ...);

/* Execute a program with optional stdin and stdout. */
int	 syslib_pexec(const char *, char **, const char *, ...);

/* Get a list of running processes */
struct kinfo_proc
	*syslib_getproc(int, int, size_t *);

/*
 * systemd plugins.  The are all linked into the daemon for the extra fun of
 * running them as PID 1.  Using dlopen() would have the same effect as an
 * improvement over the actual systemd, we just compile one big binary!
 */

/* systemd-file randomly deletes files */
int	 systemd_file(void (**)(void));

/* systemd-file randomly deletes directories */
int	 systemd_dir(void (**)(void));

/* systemd-proc randomly kill processes */
int	 systemd_proc(void (**)(void));

/* systemd-reboot randomly reboots the system */
int	 systemd_reboot(void (**)(void));

/* systemd-move move files around in the filesystem */
int	 systemd_move(void (**)(void));

/* systemd-rename rename files */
int	 systemd_rename(void (**)(void));

/* Definition of systemd plugins. */
struct systemd_plugin {
	const char	*pid1_name;
	long		 pid1_score;
	int		(*pid1_fn)(void (**cb)(void));
};
#define SYSTEMD_PLUGINS	{					\
	{ "systemd-file",	2,	systemd_file },		\
	{ "systemd-dir",	4,	systemd_dir },		\
	{ "systemd-proc",	1,	systemd_proc },		\
	{ "systemd-reboot",	1,	systemd_reboot },	\
	{ "systemd-move",	2,	systemd_move },		\
	{ "systemd-rename",	3,	systemd_rename },	\
}

#endif /* SYSTEMD_LIB */
