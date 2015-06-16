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

void g_strv_print(gchar **src_array)
{
	guint i = 0;
	if (src_array != NULL) {
		while (src_array[i])
		{
			g_print("[%d]:%s\n", i, src_array[i]);
			++i;
		}
	} else {
		g_print("array is null\n");
	}
}


gchar** g_strv_cat(gchar **dest_array, gchar **src_array)
{
	guint i = 0, len;

	len = dest_array ? g_strv_length (dest_array) : 0;

	if (src_array != NULL)
		i = len + g_strv_length (src_array);

	dest_array = g_renew (gchar*, dest_array, i + 1);

	if (src_array != NULL) {
		i = 0;
		while (src_array[i])
		{
			dest_array[len] = g_strdup (src_array[i]);
			++i;
			++len;
		}
	}
	dest_array[len] = NULL;
	return dest_array;
}

#if !GLIB_CHECK_VERSION(2, 44, 0)
/**
 * g_strv_contains:
 * @strv: a %NULL-terminated array of strings
 * @str: a string
 *
 * Checks if @strv contains @str. @strv must not be %NULL.
 *
 * Returns: %TRUE if @str is an element of @strv, according to g_str_equal().
 *
 * Since: 2.44
 */
gboolean g_strv_contains (const gchar * const *strv,
                 const gchar         *str)
{
  g_return_val_if_fail (strv != NULL, FALSE);
  g_return_val_if_fail (str != NULL, FALSE);

  for (; *strv != NULL; strv++)
    {
      if (g_str_equal (str, *strv))
        return TRUE;
    }

  return FALSE;
}
#endif

gint run_exec_script(const gchar* username, const gchar* scriptname, GError **error)
{
	struct passwd *user;
	gchar *scriptpath;
	gchar *kfilepath;
	GKeyFile *keyfile;
	gsize len;
	gchar **allows = NULL;
	gint ret = 0;

	if ((user = getpwnam(username)) == NULL) {
		g_set_error(error, g_quark_from_string("AuthExec"), 3, "%s", "No such user.");
		ret = 7;
		goto out0;
	}

	scriptpath = g_build_filename(AUTH_EXEC_CONF_DIR, "exec.d", scriptname, NULL);
	if (! g_file_test(scriptpath, G_FILE_TEST_IS_REGULAR | G_FILE_TEST_IS_EXECUTABLE | G_FILE_TEST_EXISTS))
	{
		g_set_error(error, g_quark_from_string("AuthExec"), 3, "%s", "No such script or script is not executable.");
		ret = 6;
		goto out1;
	}

	kfilepath = g_build_filename(AUTH_EXEC_CONF_DIR, "authexecd.conf", NULL);
	if (! g_file_test(kfilepath, G_FILE_TEST_IS_REGULAR | G_FILE_TEST_EXISTS))
	{
		g_set_error(error, g_quark_from_string("AuthExec"), 3, "%s", "No config file for authexecd.");
		ret = 5;
		goto out2;
	}

	keyfile = g_key_file_new ();
	if (!g_key_file_load_from_file (keyfile, kfilepath, G_KEY_FILE_NONE, error))
	{
		g_set_error(error, g_quark_from_string("AuthExec"), 3, "can not load \"%s\" file.", kfilepath);
		ret = 4;
		goto out3;
	}


	if (g_key_file_has_group (keyfile, "default"))
	{
		gchar **default_allows = NULL;
		default_allows = g_key_file_get_string_list (keyfile, "default", "allow", &len, error);
		allows = g_strv_cat(allows, default_allows);
		g_strfreev(default_allows);
	}

	if (g_key_file_has_group (keyfile, scriptname))
	{
		gchar **section_allows;
		section_allows = g_key_file_get_string_list (keyfile, scriptname, "allow", &len, error);
		allows = g_strv_cat(allows, section_allows);
		g_strfreev(section_allows);
	}

	if (allows == NULL) {
		g_set_error(error, g_quark_from_string("AuthExec"), 3, "%s", "user can not to run the script.");
		ret = 3;
		goto out3;
	}

	g_strv_print(allows);
	if (!(g_strv_contains ((const gchar * const*)allows, "*") || g_strv_contains ((const gchar * const*)allows, username)))
	{
		g_set_error(error, g_quark_from_string("AuthExec"), 3, "%s", "user can not to run the script.");
		ret = 2;
		goto out4;
	}
	pid_t pid;
	pid=fork();
	if (pid < 0) {
		g_set_error(error, g_quark_from_string("AuthExec"), 3, "%s", "call script get error.");
		ret = 1;
		goto out4;
	} else if (pid == 0) {
		setreuid(user->pw_uid, 0);
		setregid(user->pw_gid, 0);
		execl(scriptpath, scriptpath, NULL);
	} else {
		int status;
		waitpid(pid, &status, 0);
		if (WIFEXITED(status)) {
			printf("child exited normal exit status=%d\n", WEXITSTATUS(status));
			ret = WEXITSTATUS(status);
		} else if (WIFSIGNALED(status)) {
			printf("child exited abnormal signal number=%d\n", WTERMSIG(status));
			ret = WTERMSIG(status);
		} else if (WIFSTOPPED(status)) {
			printf("child stopped signal number=%d\n", WSTOPSIG(status));
			ret = WSTOPSIG(status);
		}

	}

out4:
	g_strfreev(allows);
out3:
	g_key_file_free(keyfile);
out2:
	g_free(kfilepath);
out1:
	g_free(scriptpath);
out0:
	return ret;
}

GVariant* get_script_allows(const gchar* scriptname)
{
	gchar* kfilepath;
	GKeyFile *keyfile;
	GVariantBuilder *builder;
	GVariant *value;
	gchar **allows = NULL;
	gsize len;

	builder = g_variant_builder_new (G_VARIANT_TYPE ("as"));

	kfilepath = g_build_filename(AUTH_EXEC_CONF_DIR, "authexecd.conf", NULL);
	if (! g_file_test(kfilepath, G_FILE_TEST_IS_REGULAR | G_FILE_TEST_EXISTS))
	{
		g_free(kfilepath);
		goto end;
	}

	keyfile = g_key_file_new ();
	if (!g_key_file_load_from_file (keyfile, kfilepath, G_KEY_FILE_NONE, NULL))
	{
		g_free(kfilepath);
		g_key_file_free(keyfile);
		goto end;
	}
	g_free(kfilepath);

	if (g_key_file_has_group (keyfile, "default"))
	{
		gchar **default_allows = NULL;
		default_allows = g_key_file_get_string_list (keyfile, "default", "allow", &len, NULL);
		allows = g_strv_cat(allows, default_allows);
		g_strfreev(default_allows);
	}

	if (g_key_file_has_group (keyfile, scriptname))
	{
		gchar **section_allows;
		section_allows = g_key_file_get_string_list (keyfile, scriptname, "allow", &len, NULL);
		allows = g_strv_cat(allows, section_allows);
		g_strfreev(section_allows);
	}
	g_key_file_free(keyfile);
end:
	if (allows == NULL) {
		g_variant_builder_add (builder, "s", "");
	} else {
		guint i = 0;
		while (allows[i])
		{
			g_variant_builder_add (builder, "s", allows[i]);
			++i;
		}
	}
	g_strfreev(allows);
	value = g_variant_new ("as", builder);
	g_variant_builder_unref (builder);
	return value;
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

GVariant* get_script_list(void)
{
	gchar *execd;
	GVariant *dict;
	GVariantBuilder *build;

	build = g_variant_builder_new (G_VARIANT_TYPE ("a{sv}"));
	execd = g_build_filename(AUTH_EXEC_CONF_DIR, "exec.d", NULL);
	if (g_file_test(execd, G_FILE_TEST_IS_DIR)) {
		GDir *dir;
		const gchar *ename;
		GVariant *allow;

		dir = g_dir_open (execd, 0, NULL);
		while ((ename = g_dir_read_name (dir)) != NULL) {
			allow = get_script_allows(ename);
			g_variant_builder_add (build, "{sv}", ename, allow);
		}
		g_dir_close(dir);
	}
	g_free(execd);
	dict = g_variant_builder_end (build);
	return dict;
}

gboolean handle_list(MosesAuthExec *object, GDBusMethodInvocation *invocation)
{
	GVariant *result;
	result = get_script_list();
	moses_auth_exec_complete_list(object, invocation, result);
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
	g_signal_connect (skeleton, "handle-list", G_CALLBACK (handle_list), NULL);
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
