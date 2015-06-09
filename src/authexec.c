/* vi: set sw=4 ts=4 wrap ai: */
/*
 * authexec.c: This file is part of ____
 *
 * Copyright (C) 2015 yetist <xiaotian.wu@i-soft.com.cn>
 *
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 * */
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <argp.h>

#include <glib.h>
#include <gio/gio.h>
#include "moses-authexec-generated.h"

#define MOSES_DBUS_NAME "org.moses.AuthExec"
#define MOSES_DBUS_PATH "/org/moses/AuthExec"

const char *argp_program_version = PACKAGE_STRING;
const char *argp_program_bug_address = PACKAGE_BUGREPORT;

static char doc[] =
"\nRun script as other user.\n\n"
"COMMAND:\n"
"  run <SCRIPT>    Run script as other user.\n"
"  list            List all scripts and allowed users.\n"
"\nOPTIONS:";

static char args_doc[] = "<COMMAND> [SCRIPT...]";

static struct argp_option options[] = {
	{"user",     'u', "root",  0, "Run script as user.(Default:root)"},
	{ 0 }
};

struct arguments
{
	char *command;
	char *user;
	char **scripts;
};

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	struct arguments *arguments = state->input;

	switch (key)
	{
		case 'u':
			arguments->user = arg;
			break;
		case ARGP_KEY_NO_ARGS:
			argp_usage (state);

		case ARGP_KEY_ARG:
			if (!(strcmp(arg, "run") == 0 || strcmp(arg, "list") == 0)) {
				argp_usage (state);
			}
			arguments->command = arg;
			arguments->scripts = &state->argv[state->next];
			state->next = state->argc;
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

void parse_var(GVariant *dictionary)
{
	GVariantIter item;
	GVariant *value;
	gchar *key;

	g_variant_iter_init (&item, dictionary);
	while (g_variant_iter_loop (&item, "{sv}", &key, &value))
	{
		gchar *str;
		GVariantIter *iter;
		g_print("[%s]\nallow = ", key);
		g_variant_get (value, "as", &iter);
		while (g_variant_iter_loop (iter, "s", &str))
			g_print ("%s;", str);
		g_print("\n");
		g_variant_iter_free (iter);
		g_variant_unref(value);
	}
	g_free(key);
}

int main (int argc, char **argv)
{

	int i;
	gint ret;
	gchar *info;
	GVariant *result;
	GError *error = NULL;
	MosesAuthExec *authexec;
	struct arguments arguments;

	struct argp argp = { options, parse_opt, args_doc, doc };

	arguments.command = "list";
	arguments.user = "root";

	authexec = moses_auth_exec_proxy_new_for_bus_sync (
			G_BUS_TYPE_SYSTEM,
			G_DBUS_PROXY_FLAGS_NONE,
			MOSES_DBUS_NAME,
			MOSES_DBUS_PATH,
			NULL,
			&error);
	if (error != NULL) {
		g_printerr ("Unable to contact dbus server: %s\n", error->message);
		g_error_free(error);
		return EXIT_FAILURE;
	}

	argp_parse (&argp, argc, argv, 0, 0, &arguments);

	if (strcmp(arguments.command, "list") == 0) {
		moses_auth_exec_call_list_sync (authexec, &result, NULL, &error);
		if (error != NULL) {
			g_printerr ("Unable to contact server: %s\n", error->message);
			g_error_free(error);
			return EXIT_FAILURE;
		}
		parse_var(result);
		return EXIT_SUCCESS;
	} else {
		for (i = 0; arguments.scripts[i]; i++) {
			moses_auth_exec_call_run_sync (authexec, arguments.user, arguments.scripts[i], &ret, &info, NULL, &error);
			if (error != NULL) {
				g_printerr ("Unable to contact server: %s\n", error->message);
				g_error_free(error);
				return EXIT_FAILURE;
			}
			g_printerr ("%s\n", info);
			g_free(info);
		}
	}
	return ret;
}
