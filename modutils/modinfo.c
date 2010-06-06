/* vi: set sw=4 ts=4: */
/*
 * modinfo - retrieve module info
 * Copyright (c) 2008 Pascal Bellard
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

//applet:IF_MODINFO(APPLET(modinfo, _BB_DIR_SBIN, _BB_SUID_DROP))

//kbuild:lib-$(CONFIG_MODINFO) += modinfo.o

//config:config MODINFO
//config:	bool "modinfo"
//config:	default y
//config:	help
//config:	  Show information about a Linux Kernel module

#include <fnmatch.h>
#include <sys/utsname.h> /* uname() */
#include "libbb.h"
#include "modutils.h"


enum {
	OPT_TAGS = (1 << 6) - 1,
	OPT_F = (1 << 6), /* field name */
	OPT_0 = (1 << 7),  /* \0 as separator */
};

struct modinfo_env {
	char *field;
	int tags;
};

static int display(const char *data, const char *pattern, int flag)
{
	if (flag) {
		int n = printf("%s:", pattern);
		while (n++ < 16)
			bb_putchar(' ');
	}
	return printf("%s%c", data, (option_mask32 & OPT_0) ? '\0' : '\n');
}

static void modinfo(const char *path, struct modinfo_env *env)
{
	static const char *const shortcuts[] = {
		"filename",
		"description",
		"author",
		"license",
		"vermagic",
		"parm",
	};
	size_t len;
	int j, length;
	char *ptr, *the_module;
	const char *field = env->field;
	int tags = env->tags;

	if (tags & 1) { /* filename */
		display(path, shortcuts[0], 1 != tags);
	}
	len = MAXINT(ssize_t);
	the_module = xmalloc_open_zipped_read_close(path, &len);
	if (!the_module)
		return;
	if (field)
		tags |= OPT_F;
	for (j = 1; (1<<j) & (OPT_TAGS + OPT_F); j++) {
		const char *pattern = field;
		if ((1<<j) & OPT_TAGS)
			pattern = shortcuts[j];
		if (!((1<<j) & tags))
			continue;
		length = strlen(pattern);
		ptr = the_module;
		while (1) {
			ptr = memchr(ptr, *pattern, len - (ptr - (char*)the_module));
			if (ptr == NULL) /* no occurance left, done */
				break;
			if (strncmp(ptr, pattern, length) == 0 && ptr[length] == '=') {
				ptr += length + 1;
				ptr += display(ptr, pattern, (1<<j) != tags);
			}
			++ptr;
		}
	}
	free(the_module);
}

//usage:#define modinfo_trivial_usage
//usage:       "[-adlp0] [-F keyword] MODULE"
//usage:#define modinfo_full_usage "\n\n"
//usage:       "Options:"
//usage:     "\n	-a		Shortcut for '-F author'"
//usage:     "\n	-d		Shortcut for '-F description'"
//usage:     "\n	-l		Shortcut for '-F license'"
//usage:     "\n	-p		Shortcut for '-F parm'"
//usage:     "\n	-F keyword	Keyword to look for"
//usage:     "\n	-0		Separate output with NULs"
//usage:#define modinfo_example_usage
//usage:       "$ modinfo -F vermagic loop\n"

int modinfo_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int modinfo_main(int argc UNUSED_PARAM, char **argv)
{
	struct modinfo_env env;
	char name[MODULE_NAME_LEN];
	struct utsname uts;
	parser_t *p;
	char *colon, *tokens[2];
	unsigned opts;
	unsigned i;

	env.field = NULL;
	opt_complementary = "-1"; /* minimum one param */
	opts = getopt32(argv, "fdalvpF:0", &env.field);
	env.tags = opts & OPT_TAGS ? opts & OPT_TAGS : OPT_TAGS;
	argv += optind;

	uname(&uts);
	p = config_open2(
		concat_path_file(
			concat_path_file(CONFIG_DEFAULT_MODULES_DIR, uts.release),
			CONFIG_DEFAULT_DEPMOD_FILE),
		xfopen_for_read
	);

	while (config_read(p, tokens, 2, 1, "# \t", PARSE_NORMAL)) {
		colon = last_char_is(tokens[0], ':');
		if (colon == NULL)
			continue;
		*colon = '\0';
		filename2modname(tokens[0], name);
		for (i = 0; argv[i]; i++) {
			if (fnmatch(argv[i], name, 0) == 0) {
				modinfo(tokens[0], &env);
				argv[i] = (char *) "";
			}
		}
	}
	for (i = 0; argv[i]; i++) {
		if (argv[i][0]) {
			modinfo(argv[i], &env);
		}
	}
	return 0;
}