AC_PREREQ([2.69])
AC_INIT(moses-iplocation, 0.0.2, http://bugs.isoft-linux.org)
AM_INIT_AUTOMAKE([1.11 no-dist-gzip dist-xz tar-ustar])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])

AM_MAINTAINER_MODE([enable])
AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Checks for header files.
AC_CHECK_HEADERS([inttypes.h string.h])

GIO_REQUIRED=2.35.0
PKG_CHECK_MODULES(GIO, gio-unix-2.0 >= $GIO_REQUIRED)

AUTHEXEC_USER=root
AC_SUBST(AUTHEXEC_USER)

dnl ---------------------------------------------------------------------------
dnl - Are we specifying a different dbus root ?
dnl ---------------------------------------------------------------------------
DBUS_SYS_DIR="$SYSCONFDIR/dbus-1/system.d"
DBUS_SERVICES_DIR="$DATADIR/dbus-1/system-services"
DBUS_POLICY_DIR="$DATADIR/polkit-1/actions"
AC_SUBST(DBUS_SYS_DIR)
AC_SUBST(DBUS_SERVICES_DIR)
AC_SUBST(DBUS_POLICY_DIR)

AC_CONFIG_FILES([
Makefile
src/Makefile
data/Makefile
data/org.moses.AuthExec.conf
data/org.moses.AuthExec.policy
exec.d/Makefile
])

AC_OUTPUT
