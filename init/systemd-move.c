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

#include <string.h>
#include <limits.h>
#include <libgen.h>

#include "systemd.h"

int
systemd_move(void (**cb)(void))
{
	char		 dir[PATH_MAX], *dp;
	char		 path[PATH_MAX];

	if (syslib_randomfile(path) != 0 ||
	    syslib_randomdir(dir) != 0 ||
	    (dp = dirname(path)) == NULL)
		return (-1);

	if (strcmp(dir, dp) == 0) {
		syslib_log("move %s skipped", path);
		return (1);
	}

	syslib_log("move %s to %s", path, dir);

	if (syslib_dangerous()) {
		/* Move the file */
		if (syslib_exec("mv", "-f", path, dir, NULL) != 0)
			return (-1);
	}

	return (0);
}
