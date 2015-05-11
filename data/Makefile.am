dbusdir = $(sysconfdir)/dbus-1/system.d
dbus_DATA = org.moses.AuthExec.conf

policydir = $(datadir)/polkit-1/actions
policy_DATA =  org.moses.AuthExec.policy 

servicedir = $(datadir)/dbus-1/system-services
service_in_files = org.moses.AuthExec.service.in
service_DATA     = $(service_in_files:.service.in=.service)
$(service_DATA): $(service_in_files) Makefile
	    @sed -e "s|\@bindir\@|$(bindir)|" -e "s|\@AUTHEXEC_USER\@|$(AUTHEXEC_USER)|" $< > $@

EXTRA_DIST = $(service_in_files) 
