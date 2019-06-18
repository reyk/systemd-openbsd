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
#include <sys/sysctl.h>

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "systemd.h"

int
systemd_proc(void (**cb)(void))
{
	size_t			 nproc;
	struct kinfo_proc	*kp, *p;
	pid_t			 pid;
	int			 sig, ret;

	if ((kp = syslib_getproc(KERN_PROC_ALL, 0, &nproc)) == NULL)
		return (-1);

	p = &kp[arc4random_uniform(nproc)];
	pid = p->p_pid;
	free(kp);

	sig = arc4random_uniform(2) ? SIGKILL : SIGTERM;
	ret = 0;

	if (syslib_dangerous()) {
		/* kill the process */
		if (kill(pid, sig) == -1) {
			/* Lucky you! The process is already gone. */
			if (errno == ESRCH)
				ret = 1;
			else {
				syslib_log("failed to %s pid %d",
				    strsignal(sig), pid);
				return (-1);
			}
		}
	}

	syslib_log("proc %s pid %d%s", strsignal(sig), pid,
	    ret == 0 ? "" : " skipped");

	return (ret);
}
