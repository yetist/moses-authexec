
dbusdir = ${DBUS_SYS_DIR}
dist_dbus_DATA =                    \
									org.isoft.ExecScripts.conf         \
									$(NULL)
dbuspolicydir = ${DBUS_POLICY_DIR}
dist_dbuspolicy_DATA =                    \
									org.isoft.ExecScripts.policy         \
									$(NULL)



servicemaindir       = ${DBUS_SERVICES_DIR}
servicemain_in_files = org.isoft.ExecScripts.service.in
servicemain_DATA     = $(servicemain_in_files:.service.in=.service)
$(servicemain_DATA): $(servicemain_in_files) Makefile
	    @sed -e "s|\@BINDIR\@|$(BINDIR)|" -e "s|\@PACKAGEKIT_USER\@|$(PACKAGEKIT_USER)|" $< > $@

EXTRA_DIST =	$(servicemain_in_files) 
