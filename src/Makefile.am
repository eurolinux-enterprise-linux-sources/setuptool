SUBDIRS = po setuptool.d
if GCC
AM_CFLAGS = -Wall -Wextra
endif
sbin_PROGRAMS = setup
setup_SOURCES = setup.c
setup_LDADD = -lnewt
pamdir=$(sysconfdir)/pam.d
setuptoolddir=$(sysconfdir)/setuptool.d
othersetuptoolddir=$(pkgdatadir)/setuptool.d
consoledir=$(sysconfdir)/security/console.apps
EXTRA_DIST = setuptool.spec intltool-extract.in intltool-merge.in intltool-update.in setup.pamd setup.console setuptool.d.h mkinstalldirs
@INTLTOOL_DESKTOP_RULE@
DEFS += -DGETTEXT_PACKAGE=\"@GETTEXT_PACKAGE@\" -DSETUPTOOLDDIR=\"$(setuptoolddir)\" -DOTHERSETUPTOOLDDIR=\"$(othersetuptoolddir)\"
DISTCLEANFILES = intltool-extract intltool-merge intltool-update po/stamp-it
noinst_HEADERS = setuptool.d.h
dist_man_MANS = setup.1
install-data-local:
	mkdir -p $(DESTDIR)/$(othersetuptoolddir)
	mkdir -p $(DESTDIR)/$(pamdir)
	install -m644 $(srcdir)/setup.pamd    $(DESTDIR)/$(pamdir)/setup
	mkdir -p $(DESTDIR)/$(consoledir)
	install -m644 $(srcdir)/setup.console $(DESTDIR)/$(consoledir)/setup
uninstall-local:
	rm -f $(DESTDIR)/$(bindir)/setup
	rm -f $(DESTDIR)/$(pamdir)/setup
	rm -f $(DESTDIR)/$(consoledir)/setup
install-exec-local:
	mkdir -p $(DESTDIR)/$(bindir)
	ln -sf consolehelper $(DESTDIR)/$(bindir)/setup

CVSTAG=$(shell rpm -q --specfile setuptool.spec | head -1 | sed -e 's,\.,_,g' -e 's,\-,_,g')

setuptool.d.h: $(srcdir)/setuptool.d/[0-9]*
	cat $(srcdir)/setuptool.d/[0-9]* | cut -f2 -d \| | awk '{print "const char *unnamed = N_(\"" $$0 "\");" }' | sort -u > $@

tag:
	cvs tag -cR $(CVSTAG)

forcetag:
	cvs tag -cFR $(CVSTAG)

archive:
	rm -fr /tmp/$(PACKAGE) $(PACKAGE)-$(VERSION).tar.gz
	dir=`pwd` ; \
	root=`cat CVS/Root` ; \
	cd /tmp ; \
	cvs -d "$$root" export -r $(CVSTAG) -d $(PACKAGE) $(PACKAGE) ; \
	cd $(PACKAGE) ; \
	./autogen.sh ; \
	make distcheck ; \
	mv $(PACKAGE)-$(VERSION).tar.gz $$dir
