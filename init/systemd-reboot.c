/*
 * This file is part of the satirical systemd-init for OpenBSD.
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

#include <sys/reboot.h>

#include <stdlib.h>
#include <unistd.h>

#include "systemd.h"

static void
systemd_doreboot(void)
{
	int	 sync;

	sync = arc4random_uniform(2) ? RB_NOSYNC : 0;
	syslib_log("reboot %s", sync ? "sync" : "nosync");

	if (syslib_dangerous()) {
		/* For extra reliability, don't sync the disk. */
		(void)reboot(sync);
	}
}

int
systemd_reboot(void (**cb)(void))
{
	/* Decrease the chance that we're actually rebooting. */
	if (arc4random_uniform(3) == 0)
		*cb = systemd_doreboot;
	else {
		syslib_log("reboot skipped");
		*cb = NULL;
	}

	/* "1" means success but not actually executed. */
	return (*cb == NULL ? 1 : 0);
}
