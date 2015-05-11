#include <dbus/dbus-glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>
#include <errno.h> 
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

#include <polkit/polkit.h>
#include <syslog.h>

#define ACTION_ID "org.isoft.ExecScripts.ExecScripts"

typedef struct
{
	GMainLoop *loop;
	gboolean auth_ok;
}AuthReturn;

	static void
check_authorization_cb (PolkitAuthority *authority,
		GAsyncResult    *res,
		AuthReturn *ar)
{
	GError *error;
	PolkitAuthorizationResult *result;

	ar->auth_ok = FALSE;
	error = NULL;
	result = polkit_authority_check_authorization_finish (authority, res, &error);
	if (error != NULL)
	{
		g_print ("Error checking authorization: %s\n", error->message);
		g_error_free (error);
	}
	else
	{
		const gchar *result_str;
		if (polkit_authorization_result_get_is_authorized (result))
		{
			result_str = "authorized";
			ar->auth_ok = TRUE;
		}
		else if (polkit_authorization_result_get_is_challenge (result))
		{
			result_str = "challenge";
		}
		else
		{
			result_str = "not authorized";
		}

		g_print ("Authorization result: %s\n", result_str);
	}
	g_main_loop_quit(ar->loop);
}


/*
 * 检查认证
 */

	gboolean
check_auth(const gchar *sender,const gchar *action_id)
{
	syslog(LOG_USER,"===========sender %s action_id %s============\n", sender, action_id);
	PolkitSubject *subject;
	PolkitAuthority *authority = NULL;
	AuthReturn ar;

	GCancellable *cancellable = NULL;
	if(cancellable == NULL)
		cancellable = g_cancellable_new  ();

	ar.loop = g_main_loop_new(NULL,FALSE);
	if(authority == NULL)
		authority = polkit_authority_get ();
	subject = polkit_system_bus_name_new(sender);
	polkit_authority_check_authorization (authority,
			subject,
			action_id,
			NULL, /* PolkitDetails */
			POLKIT_CHECK_AUTHORIZATION_FLAGS_ALLOW_USER_INTERACTION,
			cancellable,
			(GAsyncReadyCallback) check_authorization_cb,
			&ar);

	g_main_loop_run(ar.loop);
	g_object_unref (authority);
	g_object_unref (subject);
	g_object_unref(cancellable);
	return ar.auth_ok;
}

static void lose (const char *fmt, ...) G_GNUC_NORETURN G_GNUC_PRINTF (1, 2);
static void lose_gerror (const char *prefix, GError *error) G_GNUC_NORETURN;

	static void
lose (const char *str, ...)
{
	va_list args;

	va_start (args, str);

	vfprintf (stderr, str, args);
	fputc ('\n', stderr);

	va_end (args);

	exit (1);
}

	static void
lose_gerror (const char *prefix, GError *error) 
{
	lose ("%s: %s", prefix, error->message);
}

typedef struct TestObj TestObj;
typedef struct TestObjClass TestObjClass;

GType test_obj_get_type (void);

struct TestObj
{
	GObject parent;
};

struct TestObjClass
{
	GObjectClass parent;
};

#define TEST_TYPE_OBJECT              (test_obj_get_type ())
#define TEST_OBJECT(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), TEST_TYPE_OBJECT, TestObj))
#define TEST_OBJECT_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), TEST_TYPE_OBJECT, TestObjClass))
#define TEST_IS_OBJECT(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), TEST_TYPE_OBJECT))
#define TEST_IS_OBJECT_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), TEST_TYPE_OBJECT))
#define TEST_OBJECT_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), TEST_TYPE_OBJECT, TestObjClass))

G_DEFINE_TYPE(TestObj, test_obj, G_TYPE_OBJECT)

gboolean test_obj_exec_scripts (TestObj *obj, char *username, char *path, int *ret, GError **error);

#include "execscripts-service-glue.h"

static void test_obj_init (TestObj *obj)
{
}

static void test_obj_class_init (TestObjClass *klass)
{
}

gboolean test_obj_exec_scripts (TestObj *obj, char *username, char *path, int *ret, GError **error)
{
	pid_t pid;
	int status;
	struct passwd *user;

	if ((user = getpwnam(username)) == NULL) {
		g_set_error(error, g_quark_from_string("ExecScripts"), 3, "%s", "No such user.");
		*ret=4;
		return FALSE;
	}

	pid=fork();
	if (pid < 0)
		printf("Error  fork()!");
	else if (pid == 0)
	{
		syslog(LOG_USER,"Check auth successfully\n");
		user = getpwnam(username);
		setreuid(user->pw_uid, 0);
		setregid(user->pw_gid, 0);
		sleep(1);
		execl(path, path, NULL);
	}
	else
	{
		pid = wait(&status);
		if ( WIFEXITED(status) )   /* 如果WIFEXITED返回非零值 */
		{
			printf("The child process %d exit normally.\n", pid);
			printf("the return code is %d.\n", WEXITSTATUS(status));
		}
		else /* 如果WIFEXITED返回零 */
		{
			printf("The child process %d exit abnormally.\n", pid);
		}
		*ret = WEXITSTATUS(status);
	}
	return TRUE;
}

int main (int argc, char **argv)
{
	DBusGConnection *bus;
	DBusGProxy *bus_proxy;
	GError *error = NULL;
	TestObj *obj;
	GMainLoop *mainloop;
	guint request_name_result;

	g_type_init ();

	{
		GLogLevelFlags fatal_mask;

		fatal_mask = g_log_set_always_fatal (G_LOG_FATAL_MASK);
		fatal_mask |= G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL;
		g_log_set_always_fatal (fatal_mask);
	}

	dbus_g_object_type_install_info (TEST_TYPE_OBJECT, &dbus_glib_test_obj_object_info);

	mainloop = g_main_loop_new (NULL, FALSE);

	bus = dbus_g_bus_get (DBUS_BUS_SYSTEM, &error);
	if (!bus)
		lose_gerror ("Couldn't connect to system bus", error);

	bus_proxy = dbus_g_proxy_new_for_name (bus, "org.freedesktop.DBus",
			"/org/freedesktop/DBus",
			"org.freedesktop.DBus");

	if (!dbus_g_proxy_call (bus_proxy, "RequestName", &error,
				G_TYPE_STRING, "org.isoft.ExecScripts",
				G_TYPE_UINT, 0,
				G_TYPE_INVALID,
				G_TYPE_UINT, &request_name_result,
				G_TYPE_INVALID))
		lose_gerror ("Failed to acquire org.isoft.ExecScripts", error);

	obj = g_object_new (TEST_TYPE_OBJECT, NULL);

	dbus_g_connection_register_g_object (bus, "/org/isoft/ExecScripts", G_OBJECT (obj));

	printf ("service running\n");

	g_main_loop_run (mainloop);

	exit (0);
}
