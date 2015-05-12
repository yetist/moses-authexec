data_dbusdir = $(sysconfdir)/dbus-1/system.d
data_dbus_DATA = org.moses.AuthExec.conf

data_policydir = $(datadir)/polkit-1/actions
data_policy_DATA =  org.moses.AuthExec.policy 

data_servicedir = $(datadir)/dbus-1/system-services
data_service_in_files = org.moses.AuthExec.service.in
data_service_DATA     = $(data_service_in_files:.service.in=.service)
$(data_service_DATA): $(data_service_in_files) Makefile
	    @sed -e "s|\@bindir\@|$(bindir)|" -e "s|\@AUTHEXEC_USER\@|$(AUTHEXEC_USER)|" $< > $@

EXTRA_DIST = $(data_service_in_files) 

CLEANFILES =	 			\
	$(data_dbus_DATA)		\
	$(data_policy_DATA)		\
	$(data_service_DATA)

DISTCLEANFILES = 			\
	$(data_dbus_DATA)		\
	$(data_policy_DATA)		\
	$(data_service_DATA)

-include $(top_srcdir)/git.mk
