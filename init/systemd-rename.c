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

#include "systemd.h"

int
systemd_rename(void (**cb)(void))
{
	char		 file1[PATH_MAX];
	char		 file2[PATH_MAX];
	char		 file3[PATH_MAX];

	if (syslib_randomfile(file1) != 0 ||
	    syslib_randomfile(file2) != 0)
		return (-1);

	if (strcmp(file1, file2) == 0) {
		systemd_journal("rename %s skipped", file1);
		return (1);
	}

	if (strlcpy(file3, file1, sizeof(file3)) >= sizeof(file3) ||
	    strlcat(file3, ".bak", sizeof(file3)) >= sizeof(file3))
		return (-1);

	systemd_journal("rename %s and %s", file1, file2);

	if (syslib_dangerous()) {
		/* Move the file */
		if (syslib_exec("mv", "-f", file1, file3, NULL) != 0)
			return (-1);

		/* Ignore subsequent errors as we already moved something ;) */
		(void)syslib_exec("mv", "-f", file2, file1, NULL);
		(void)syslib_exec("mv", "-f", file3, file2, NULL);
	}

	return (0);
}
