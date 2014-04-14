/*
Copyright (c) 2013-2014 René Ladan. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
*/

#include "config.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *key[] = {
	"pin", "activehigh", "freq",
	"summermonth", "wintermonth", "leapsecmonths", "outlogfile"
};

#define NUM_KEYS (sizeof(key) / sizeof(key[0]))

#define MAX_KEYLEN 20
#define MAX_VALLEN 255
#define MAX_LEN (MAX_KEYLEN + 3 + MAX_VALLEN + 2) /* "k = v\n\0" */

char *value[NUM_KEYS];

int
getpos(char *kw)
{
	int i;

	for (i = 0; i < NUM_KEYS; i++)
		if (strcmp(key[i], kw) == 0)
			return i;
	return -1;
}

char *
strip(char *s)
{
	int i;
	char *t;

	for (t = s; t[0] == ' ' || t[0] == '\n' || t[0] == '\r' ||
	    t[0] == '\t'; t++)
		;
	for (i = strlen(t) - 1; t[i] == ' ' || t[i] == '\n' || t[i] == '\r' ||
	    t[i] == '\t'; i--)
		t[i] = '\0';
	return t;
}

#define END_CONFIG(ret) \
do { \
	fclose(configfile); \
	free(freeptr); \
	return (ret); \
} while (0)

int
read_config_file(char *filename)
{
	int i;
	FILE *configfile;
	char *k, *v;
	char *line, *freeptr;

	k = v = NULL;
	line = malloc(MAX_LEN);
	if (line == NULL) {
		perror("malloc(configfile)");
		return errno;
	}
	freeptr = line;
	for (i = 0; i < NUM_KEYS; i++)
		value[i] = NULL;

	configfile = fopen(filename, "r");
	if (configfile == NULL) {
		perror("fopen (configfile)");
		free(freeptr);
		return errno;
	}

	while (feof(configfile) == 0) {
		if (fgets(line, MAX_LEN, configfile) == NULL) {
			if (feof(configfile))
				break;
			printf("read_config_file: error reading file\n");
			END_CONFIG(-1);
		}
		if ((k = strsep(&line, "=")) != NULL)
			v = line;
		else {
			printf("read_config_file: no key/value pair found\n");
			END_CONFIG(-1);
		}
		i = strlen(k);
		k = strip(k);
		v = strip(v);
		if (i > MAX_KEYLEN + 1 || strlen(k) == 0 ||
		    strlen(k) > MAX_KEYLEN) {
			printf("read_config_file: item with bad key length\n");
			END_CONFIG(-1);
		}
		i = getpos(k);
		if (i == -1) {
			printf("read_config_file: skipping invalid key '%s'\n",
			    k);
			continue;
		}
		if (strlen(v) > MAX_VALLEN) {
			printf("read_config_file: item with too long value\n");
			END_CONFIG(-1);
		}
		if (value[i] != NULL)
			printf("read_config_file: overwriting value for key"
			    " '%s'\n", k);
		value[i] = strdup(v);
	}
	for (i = 0; i < NUM_KEYS; i++)
		if (value[i] == NULL) {
			printf("read_config_file: missing value for key '%s'\n",
			    key[i]);
			END_CONFIG(-1);
		}
	END_CONFIG(0);
}

char *
get_config_value(char *keyword)
{
	int i;

	i = getpos(keyword);
	return i == -1 ? NULL : value[i];
}
