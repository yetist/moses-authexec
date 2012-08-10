
servicemaindir       = /usr/share/dbus-1/services/
servicemain_in_files = org.isoft.ExecScripts.service.in
servicemain_DATA     = $(servicemain_in_files:.service.in=.service)
$(servicemain_DATA): $(servicemain_in_files) Makefile
	    @sed -e "s|\@BINDIR\@|$(BINDIR)|" -e "s|\@PACKAGEKIT_USER\@|$(PACKAGEKIT_USER)|" $< > $@

EXTRA_DIST =	$(servicemain_in_files) 
