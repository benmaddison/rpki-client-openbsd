/*	$OpenBSD: output-json.c,v 1.8 2020/04/28 15:04:05 deraadt Exp $ */
/*
 * Copyright (c) 2019 Claudio Jeker <claudio@openbsd.org>
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

#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <openssl/ssl.h>

#include "extern.h"

static int
outputheader_json(FILE *out, struct stats *st)
{
	char		hn[NI_MAXHOST], tbuf[26];
	time_t		t;

	time(&t);
	setenv("TZ", "UTC", 1);
	ctime_r(&t, tbuf);
	*strrchr(tbuf, '\n') = '\0';

	gethostname(hn, sizeof hn);

	if (fprintf(out,
	    "{\n\t\"metadata\": {\n"
	    "\t\t\"buildmachine\": \"%s\",\n"
	    "\t\t\"buildtime\": \"%s\",\n"
	    "\t\t\"roas\": %zu,\n"
	    "\t\t\"failedroas\": %zu,\n"
	    "\t\t\"invalidroas\": %zu,\n"
	    "\t\t\"tals\": %zu,\n"
	    "\t\t\"talfiles\": \"%s\",\n"
	    "\t\t\"certificates\": %zu,\n"
	    "\t\t\"failcertificates\": %zu,\n"
	    "\t\t\"invalidcertificates\": %zu,\n"
	    "\t\t\"manifests\": %zu,\n"
	    "\t\t\"failedmanifests\": %zu,\n"
	    "\t\t\"stalemanifests\": %zu,\n"
	    "\t\t\"crls\": %zu,\n"
	    "\t\t\"repositories\": %zu,\n"
	    "\t\t\"vrps\": %zu,\n"
	    "\t\t\"uniquevrps\": %zu\n"
	    "\t},\n\n",
	    hn, tbuf,
	    st->roas, st->roas_fail, st->roas_invalid,
	    st->tals, st->talnames,
	    st->certs, st->certs_fail, st->certs_invalid,
	    st->mfts, st->mfts_fail, st->mfts_stale,
	    st->crls,
	    st->repos,
	    st->vrps, st->uniqs) < 0)
		return -1;
	return 0;
}

int
output_json(FILE *out, struct vrp_tree *vrps, struct stats *st)
{
	char		 buf[64];
	struct vrp	*v;
	int		 first = 1;

	if (outputheader_json(out, st) < 0)
		return -1;

	if (fprintf(out, "\t\"roas\": [\n") < 0)
		return -1;

	RB_FOREACH(v, vrp_tree, vrps) {
		if (first)
			first = 0;
		else {
			if (fprintf(out, ",\n") < 0)
				return -1;
		}

		ip_addr_print(&v->addr, v->afi, buf, sizeof(buf));

		if (fprintf(out, "\t\t{ \"asn\": \"AS%u\", \"prefix\": \"%s\", "
		    "\"maxLength\": %u, \"ta\": \"%s\" }",
		    v->asid, buf, v->maxlength, v->tal) < 0)
			return -1;
	}

	if (fprintf(out, "\n\t]\n}\n") < 0)
		return -1;
	return 0;
}
