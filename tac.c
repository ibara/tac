/*
 * Copyright (c) 2018 Brian Callahan <bcallah@openbsd.org>
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

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef LIBBSD
#include <bsd/stdlib.h>
#include <bsd/unistd.h>
#endif

static char	delim = '\n';
static int	bflag;

/*
 * tac -- concatenate and print files in reverse.
 */

static void
tac(const char *input, int argn, size_t inputlen)
{
	const char     *s;
	char	      **args = NULL, *argt;
	int 		i = 0;
	size_t 		len;

	if ((args = reallocarray(args, argn, sizeof(char *))) == NULL)
		errx(1, "memory exhausted");

	while (i < argn) {
		for (s = input; *s != delim; s++) {
			if (inputlen-- == 0)
				break;
		}
		len = s - input;

		if ((args[i] = malloc(len + 1)) == NULL)
			err(1, "malloc");
		argt = args[i++];

		while (len-- > 0)
			*argt++ = *input++;
		*argt = '\0';

		input++;
		if (inputlen > 0) {
			if (--inputlen == 0)
				break;
		}
	}

	for (i = argn - 1; i >= 0; i--) {
		if (bflag)
			fwrite(&delim, 1, 1, stdout);
		fwrite(args[i], strlen(args[i]), 1, stdout);
		if (!bflag)
			fwrite(&delim, 1, 1, stdout);
	}

	for (i = 0; i < argn; i++) {
		free(args[i]);
		args[i] = NULL;
	}

	free(args);
	args = NULL;
}

static void
usage(void)
{

	fprintf(stderr, "usage: %s [-bhv] [file ...]\n", getprogname());

	exit(1);
}

static void
version(void)
{

	fputs("tac 0.1\n"
	      "Copyright (c) 2018 Brian Callahan <bcallah@openbsd.org>\n"
	      "\nPermission to use, copy, modify, and distribute this software"
	      " for any\npurpose with or without fee is hereby granted, "
	      "provided that the above\ncopyright notice and this permission "
	      "notice appear in all copies.\n\nTHE SOFTWARE IS PROVIDED \"AS "
	      "IS\" AND THE AUTHOR DISCLAIMS ALL WARRANTIES\nWITH REGARD TO "
	      "THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF\n", stderr);
	fputs("MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE "
	      "LIABLE FOR\nANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL "
	      "DAMAGES OR ANY DAMAGES\nWHATSOEVER RESULTING FROM LOSS OF USE, "
	      "DATA OR PROFITS, WHETHER IN AN\nACTION OF CONTRACT, NEGLIGENCE "
	      "OR OTHER TORTIOUS ACTION, ARISING OUT OF\nOR IN CONNECTION "
	      "WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.\n", stderr);

	exit(1);
}

int
main(int argc, char *argv[])
{
	FILE	       *ifile;
	char	       *buf, *nbuf = NULL;
	int 		argn = 1, ch, prev = -1;
	size_t 		buflen = 0, bufsize = 1024, nbufsize;

#ifdef HAVE_PLEDGE
	if (pledge("stdio rpath", NULL) == -1)
		errx(1, "pledge");
#endif

	while ((ch = getopt(argc, argv, "bhv")) != -1) {
		switch (ch) {
		case 'b':
			bflag = 1;
			break;
		case 'h':
			usage();
		case 'v':
			version();
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;

again:
	if ((buf = malloc(bufsize)) == NULL)
		err(1, "malloc failed");

	if (argc == 0 || !strcmp(*argv, "-")) {
		ifile = stdin;
	} else {
		if ((ifile = fopen(*argv, "r")) == NULL)
			err(1, "could not open %s", *argv);
	}

	while ((ch = fgetc(ifile)) != EOF) {
		buf[buflen++] = ch;

		if (buflen == bufsize) {
			nbufsize = bufsize << 1;
			if ((nbuf = realloc(buf, nbufsize)) == NULL) {
				free(buf);
				buf = NULL;
				err(1, "realloc failed");
			}
			buf = nbuf;
			bufsize = nbufsize;
		}

		if (prev == delim)
			++argn;
		prev = ch;
	}
	buf[buflen] = '\0';

	if (ifile != stdin)
		fclose(ifile);

	tac(buf, argn, buflen);

	free(buf);
	buf = NULL;

	buflen = 0;
	argn = 1;
	prev = -1;

	if (argc != 0 && *++argv != NULL)
		goto again;

	return 0;
}
