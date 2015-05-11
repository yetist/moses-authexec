/* vi: set sw=4 ts=4 wrap ai: */
/*
 * main.c: This file is part of ____
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
#include <string.h>
#include <pwd.h>
#include <sys/wait.h> 

#include <gio/gio.h>
#include "moses-authexec-generated.h"

#define MOSES_DBUS_NAME "org.moses.AuthExec"
#define MOSES_DBUS_PATH "/org/moses/AuthExec"

static void on_name_acquired (GDBusConnection *connection, const gchar *name, gpointer user_data)
{
	g_debug("Acquired the name %s\n", name);
}

static void on_name_lost (GDBusConnection *connection, const gchar *name, gpointer user_data)
{
	g_debug("Lost the name %s\n", name);
}

gint run_exec_script(const gchar* username, const gchar* scriptname, GError **error)
{
	struct passwd *user;
	gchar *scriptpath;
	gchar *kfilepath;
	GKeyFile *keyfile;

	if ((user = getpwnam(username)) == NULL) {
		g_set_error(error, g_quark_from_string("AuthExec"), 3, "%s", "No such user.");
		return 4;
	}

	scriptpath = g_build_filename(AUTH_EXEC_CONF_DIR, "exec.d", scriptname, NULL);
	g_print("file=%s\n", scriptpath);
	if (! g_file_test(scriptpath, G_FILE_TEST_IS_REGULAR | G_FILE_TEST_IS_EXECUTABLE | G_FILE_TEST_EXISTS))
	{
		g_set_error(error, g_quark_from_string("AuthExec"), 3, "%s", "No such script or script is not executable.");
		g_free(scriptpath);
		return 3;
	}

	kfilepath = g_build_filename(AUTH_EXEC_CONF_DIR, "authexecd.conf", NULL);
	if (! g_file_test(kfilepath, G_FILE_TEST_IS_REGULAR | G_FILE_TEST_EXISTS))
	{
		g_set_error(error, g_quark_from_string("AuthExec"), 3, "%s", "No config file for authexecd.");
		g_free(kfilepath);
		return 2;
	}

	keyfile = g_key_file_new ();
	if (!g_key_file_load_from_file (keyfile, kfilepath, G_KEY_FILE_NONE, error))
	{
		return 1;
	}

	gsize len;
	gchar **allows;
	if (g_key_file_has_group (keyfile, scriptname))
	{
		allows = g_key_file_get_string_list (keyfile, scriptname, "allow", &len, error);
	} else {
		allows = g_key_file_get_string_list (keyfile, "default", "allow", &len, error);
	}

	if (allows == NULL)
		return 1;

	if (! g_strv_contains ((const gchar * const*)allows, username))
	{
		g_set_error(error, g_quark_from_string("AuthExec"), 3, "%s", "user can not to run the script.");
		return 1;
	}
	g_print("username=%s, path=%s\n", username, scriptpath);
	pid_t pid;
	pid=fork();
	if (pid < 0) {
		g_set_error(error, g_quark_from_string("AuthExec"), 3, "%s", "call script get error.");
		return 1;
	} else if (pid == 0)
	{
		setreuid(user->pw_uid, 0);
		setregid(user->pw_gid, 0);
		execl(scriptpath, scriptpath, NULL);
	}
	else
	{
		int status;
		waitpid(pid, &status, 0);
		if (WIFEXITED(status)) {
			printf("child exited normal exit status=%d\n", WEXITSTATUS(status));
			return WEXITSTATUS(status);
		} else if (WIFSIGNALED(status)) {
			printf("child exited abnormal signal number=%d\n", WTERMSIG(status));
			return WTERMSIG(status);
		} else if (WIFSTOPPED(status)) {
			printf("child stopped signal number=%d\n", WSTOPSIG(status));
			return WSTOPSIG(status);
		}

	}
	return 0;
}

gboolean handle_run(MosesAuthExec *object, GDBusMethodInvocation *invocation, const gchar *arg_username, const gchar *arg_scriptspath)
{
	gint ret;
	GError *error = NULL;

	ret = run_exec_script(arg_username, arg_scriptspath, &error);
	if (error == NULL) {
		moses_auth_exec_complete_run(object, invocation, ret, "execute success!");
	} else {
		moses_auth_exec_complete_run(object, invocation, ret, error->message);
		g_error_free(error);
	}
	return TRUE;
}

static gboolean register_interface(GDBusConnection *connection) 
{
	GError *error = NULL;
	MosesAuthExec *skeleton;

	connection = g_bus_get_sync (G_BUS_TYPE_SYSTEM, NULL, &error);
	if (connection == NULL) {
		if (error != NULL) {
			g_critical ("error getting session bus: %s", error->message);
			g_error_free (error);
		}
		return FALSE;
	}
	skeleton = moses_auth_exec_skeleton_new ();
	g_signal_connect (skeleton, "handle-run", G_CALLBACK (handle_run), NULL);
	if (!g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (skeleton),
				connection,
				"/org/moses/AuthExec",
				&error))
	{
		if (error != NULL) {
			g_critical ("error getting session bus: %s", error->message);
		}
	}
	return TRUE;
}

static void on_bus_acquired (GDBusConnection *connection, const gchar *name, gpointer user_data)
{
	GDBusObjectManagerServer *object_manager;
	object_manager = g_dbus_object_manager_server_new (MOSES_DBUS_PATH);
	g_dbus_object_manager_server_set_connection (object_manager, connection);

	register_interface(connection);
}

int main(int argc, char *argv[])
{
	GMainLoop *loop;
	loop = g_main_loop_new (NULL, FALSE);
	g_bus_own_name (G_BUS_TYPE_SYSTEM,
			MOSES_DBUS_NAME,
			G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT | G_BUS_NAME_OWNER_FLAGS_REPLACE,
			on_bus_acquired,
			on_name_acquired,
			on_name_lost,
			NULL,
			NULL);
	g_main_loop_run (loop);
	g_main_loop_unref (loop);
	return 0;
}