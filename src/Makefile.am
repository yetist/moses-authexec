NULL =
bin_PROGRAMS = moses-authexec moses-authexecd

common_CFLAGS = 	\
	-Wall		\
	-DAUTH_EXEC_CONF_DIR=\""$(sysconfdir)/authexec"\"

moses_authexec_CFLAGS = -Wall $(GIO_CFLAGS)
moses_authexec_LDADD = $(GIO_LIBS) 
moses_authexec_SOURCES = $(authexec_built_sources) authexec.c
moses_authexecd_CFLAGS =	\
	$(common_CFLAGS)	\
	$(GIO_CFLAGS)

moses_authexecd_LDADD =	\
	$(GIO_LIBS) 

moses_authexecd_SOURCES =	\
	$(authexec_built_sources)	\
	daemon.c	\
	$(NULL)

authexec_built_sources =	\
	moses-authexec-generated.h	\
	moses-authexec-generated.c	\
	$(NULL)

$(authexec_built_sources) : Makefile.am $(srcdir)/org.moses.authexec.xml
	$(AM_V_GEN) gdbus-codegen \
	 	--interface-prefix org.moses. \
		--c-namespace Moses \
		--generate-c-code moses-authexec-generated \
		$(srcdir)/org.moses.authexec.xml

EXTRA_DIST = 			\
	org.moses.authexec.xml	\
	$(NULL)

-include $(top_srcdir)/git.mk
