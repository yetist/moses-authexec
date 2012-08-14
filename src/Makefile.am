INCLUDES = \
	$(DBUS_CFLAGS)				\
	$(DBUS_GLIB_CFLAGS)			\
	-I/usr/include/polkit-1		\
	-DDBUS_COMPILATION

LIBS = \
	$(DBUS_LIBS)				\
	$(DBUS_GLIB_LIBS)			\
	-ldbus-glib-1				\
	-lpolkit-gobject-1

# execscripts-service
bin_PROGRAMS = execscripts

execscripts_SOURCES = execscripts-service.c

BUILT_SOURCES = execscripts-service-glue.h

execscripts-service-glue.h: execscripts-service.xml
	$(LIBTOOL) --mode=execute dbus-binding-tool --prefix=test_obj --mode=glib-server --output=execscripts-service-glue.h $(srcdir)/execscripts-service.xml

CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = execscripts-service.xml 

