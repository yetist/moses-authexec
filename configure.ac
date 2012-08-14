AC_INIT()
AM_INIT_AUTOMAKE(hello-dbus3, 0.1)
AM_CONFIG_HEADER(config.h)

AC_PROG_CC

AC_SUBST(SYSCONFDIR, $sysconfdir)
AC_SUBST(DATADIR, $datadir)
AC_SUBST(BINDIR, $bindir)


# Dbus detection
PKG_CHECK_MODULES(DBUS, dbus-1 >= 1.1, have_dbus=yes, have_dbus=no)

if test x$have_dbus = xno ; then
    AC_MSG_ERROR([DBus development libraries not found])
fi
AM_CONDITIONAL(HAVE_DBUS, test x$have_dbus = xyes)

AC_SUBST(DBUS_CFLAGS)
AC_SUBST(DBUS_LIBS)

PACKAGEKIT_USER=root
AC_SUBST(PACKAGEKIT_USER)

# Glib detection
PKG_CHECK_MODULES(DBUS_GLIB, gobject-2.0 >= 2.6, have_glib=yes, have_glib=no)

if test x$have_glib = xno ; then
    AC_MSG_ERROR([GLib development libraries not found])
fi

AM_CONDITIONAL(HAVE_GLIB, test x$have_glib = xyes)

AC_SUBST(DBUS_GLIB_CFLAGS)
AC_SUBST(DBUS_GLIB_LIBS)


dnl ---------------------------------------------------------------------------
dnl - Are we specifying a different dbus root ?
dnl ---------------------------------------------------------------------------
DBUS_SYS_DIR="$SYSCONFDIR/dbus-1/system.d"
DBUS_SERVICES_DIR="$DATADIR/dbus-1/system-services"
DBUS_POLICY_DIR="$DATADIR/polkit-1/actions"
AC_SUBST(DBUS_SYS_DIR)
AC_SUBST(DBUS_SERVICES_DIR)
AC_SUBST(DBUS_POLICY_DIR)

AC_CONFIG_FILES([data/org.isoft.ExecScripts.conf
				 data/org.isoft.ExecScripts.policy
				 ])
AC_OUTPUT


AC_OUTPUT([Makefile
		   data/Makefile
           src/Makefile])
