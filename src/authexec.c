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

#include <glib.h>
#include <gio/gio.h>
#include "moses-authexec-generated.h"

#define MOSES_DBUS_NAME "org.moses.AuthExec"
#define MOSES_DBUS_PATH "/org/moses/AuthExec"

static void usage (void)
{
    g_printerr ("Run 'authexec --help' to see a full list of available command line options.");
    g_printerr ("\n");
}

int main(int argc, char** argv)
{
	MosesAuthExec *authexec;
	GError *error = NULL;

    gint arg_index;
	gint ret;
	gchar *info;
	gchar *username;
	gchar *scriptname;

    for (arg_index = 1; arg_index < argc; arg_index++)
    {
        gchar *arg = argv[arg_index];

        if (!g_str_has_prefix (arg, "-"))
            break;
      
        if (strcmp (arg, "-h") == 0 || strcmp (arg, "--help") == 0)
        {
            g_printerr ("Usage:\n"
                        "  authexec <USER> <SCRIPTNAME> - Run script as other user.\n"
                        "\n"
                        "Options:\n"
                        "  -h, --help        Show help options\n"
                        "  -v, --version     Show release version\n"
                        "\n");
            return EXIT_SUCCESS;
        }
        else if (strcmp (arg, "-v") == 0 || strcmp (arg, "--version") == 0)
        {
            g_printerr ("authexec %s\n", VERSION);
            return EXIT_SUCCESS;
        }
        else
        {
            g_printerr ("Unknown option %s\n", arg);
            usage ();
            return EXIT_FAILURE;
        }
    }

	if (argc - arg_index != 2) {
		usage();
		return EXIT_FAILURE;
	}

	username = argv[arg_index++];
	scriptname = argv[arg_index];
	
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
	moses_auth_exec_call_run_sync (authexec, username, scriptname, &ret, &info, NULL, &error);
	if (error != NULL) {
		g_printerr ("Unable to contact server: %s\n", error->message);
		g_error_free(error);
		return EXIT_FAILURE;
	}
	g_printerr ("%s\n", info);
	return ret;
}
