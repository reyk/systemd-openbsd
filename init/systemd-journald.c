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
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <err.h>

#ifdef BINARYLOGS
#include <netinet/in.h>
#include <resolv.h>
#include <string.h>
#endif

#include "systemd.h"

extern long systemd_score;

void
systemd_journal(char *message, ...)
{
	char	*nmessage = NULL, *json = NULL;
	size_t	 i;
	va_list	 ap;
#ifdef BINARYLOGS
	char	*b64 = NULL;
	size_t	 b64len;
#endif

	va_start(ap, message);
	if (vasprintf(&nmessage, message, ap) == -1) {
		/* Print the message without JSON as vintage syslog. */
		vsyslog(LOG_INFO, message, ap);
		nmessage = NULL;
	}
	va_end(ap);

	if (nmessage == NULL)
		return;

	/*
	 * Plain syslog is not modern and most people don't know how to use
	 * tools like tail, grep, awk, or sed to watch the logs manually or
	 * how to parse them in their frontend applications.  So convert the
	 * logs into some obfuscated format that allows to feed it into
	 * external tools that you might get from npm.
	 */
	for (i = 0; nmessage[i] != '\0'; i++)
		if (nmessage[i] == '"')
			nmessage[i] = '\'';
	if (asprintf(&json, "{"
	    "\"name\":\"systemd\","
	    "\"version\":%u,"
	    "\"score\":%ld,"
	    "\"useless-json\":true,"
	    "\"message\":\"%s\"}",
	    SYSTEMD_REV, systemd_score, nmessage) == -1)
		message = nmessage;
	else
		message = json;

#ifdef BINARYLOGS
	/*
	 * XXX To help the sysadmin, convert the JSON to some other binary
	 * XXX format first, like protobuf or ASN.1, before we encode it
	 * XXX to Base64.
	 */
	b64len = strlen(message) * 2;
	if ((b64 = calloc(1, b64len + 1)) != NULL &&
	    b64_ntop(message, strlen(message), b64, b64len) != -1)
		message = b64;
#endif

#ifdef JUSTKIDDING
	warnx("%s", message);
#else
	syslog(LOG_INFO, "%s", message);
#endif

	free(nmessage);
	free(json);
#ifdef BINARYLOGS
	free(b64);
#endif
}
