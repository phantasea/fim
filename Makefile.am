# This file should be processed with automake..
#
# $Id: Makefile.am 265 2009-10-14 21:05:40Z dezperado $
#
# inspired by mutt's  Makefile.am 
#
# yet not optimal

# no-dependencies was due to problems with conditional sources
# nostdinc disables the standard -I. include which breaks a correct string.h include
AUTOMAKE_OPTIONS = nostdinc no-dependencies # dejagnu
SUBDIRS = src doc distros scripts

dist_doc_DATA=README README.FIRST TODO VERSION BUGS  FAQ.TXT AUTHORS ChangeLog NEWS
# no INSTALL INSTALL.TXT is necessary in rules
EXTRA_DIST=  COPYING  \
	Makefile.am configure.ac configure \
	 aclocal.m4 config.h.in \
	autogen.sh \
	media/image.jpg \
	media/image.png \
	media/fim.png \
	depcomp missing install-sh

# mhmhmhm 
# CLEANFILES = autom4te.cache

srcdir = src

# still unfinished
signed-tgzdist: dist
	gpg -sbv -u EF1258B8 $(distdir).tar.gz
	ls -l $(distdir).tar.gz  $(distdir).tar.gz.sig

# still unfinished
signed-dist: dist
	make dist-bzip2 && gpg -sbv -u EF1258B8 $(distdir).tar.bz2
	ls -l $(distdir).tar.bz2 $(distdir).tar.bz2.sig
	gpg -sbv -u EF1258B8 $(distdir).tar.gz
	ls -l $(distdir).tar.gz  $(distdir).tar.gz.sig

#%.sig: %
#	        gpg -sbv -u EF1258B8 $@ $<
#	        gpg -sbav -u EF1258B8 $@ $<


# this rule is deprecated, too: testing should be performed with 'make tests' (FIXME)
sanity-check:	all
	@./src/fim -E scripts/tests/sanity.fim -t

# this is the official way of testing fim
tests:	all
	@./scripts/tests/version.sh
	@cd ./src/testdir && make

FIMSCRIPTSDIR=`pwd`/scripts/maintenance/

# new target, still not working
tests-all: tests $(FIMSCRIPTSDIR)/configure-brute-check.sh
	cd src && which cppcheck && cppcheck *cpp *.h || true
	$(FIMSCRIPTSDIR)/configure-brute-check.sh
	
#news-dump:
#	awk  -- 'v=0;($$1 == "Version" && $$2=="'`cat VERSION`'"){while((input=getline) && $$1!="Version"){print}};'  NEWS > f

# this rule is deprecated, too
test:	all
	@./src/fim -N media/* #~/M*s/*g

# this rule is deprecated, too
aatest:	all
	@./src/fim -N media/* -o aa #~/M*s/*g

# this rule is old and deprecated
#tgz:	clean
#	tar -czf ../fim.`date +%Y%m%d%H%M`.tgz ../fim/*
#	ls -l ../fim*.tgz  -v

# this rule is encouraged
report:
	@cat VERSION
	@echo '-'
	@$(LEX) -V   2>&1
	@echo '-'
	@$(YACC) -V  2>&1
	@echo '-'
	@$(CC) -v    2>&1
	@echo '-'
	@echo 'now please report the bug with this information to the author via email' ;

# this rule is redundant
exec:   test

# this rule is smart, isn't it ? I love it !
edit:
	$(EDITOR) $(srcdir)/fim.cpp +':split $(srcdir)/fim.h' # Vim ! :)

.PHONY:
	@true

# this rule is informative and for acting like a peacock 
wc:
	wc $(srcdir)/*.cpp $(srcdir)/*.h $(srcdir)/yacc.ypp $(srcdir)/lex.lex
	@#wc $(FIM) # missing headers..

help:
	@echo -----------------------------------------------
	@echo '			short Makefile help'
	@echo -----------------------------------------------
	@echo
	@echo 'Please read the doc/FIM.TXT file before complaining!'
	@echo
	@echo '..or README, or INSTALL, or some other file around here. There is plenty!'
	@echo '...'
	@echo
	@echo 'If you are experiencing problems, please write at dezperado, the author of fim.'
	@echo
	@echo 'His mail box resides as dezperado , on the server autistici dot org.'
	@echo
	@echo 'You are encouraged to read the documentation before, and emailing the output of `make report`!'
	@echo 'Thanks for your collaboration'
	@echo
	@echo

up: upload news

news: freshmeat mail-announce

# fixme : missing integration with ChangeLog file
mail-announce:
	@ figlet fim | mutt -s "fim-`cat VERSION` release" "fbi-improved-devel@nongnu.org"  -i -

log: changelog

# from trunk, we should call svn2cl .. to get the full fim changelog history 
changelog:
	svn2cl ..

release: upload freshmeat

upload: savannah 

savannah: signed-dist
	echo -en "cd /releases/fbi-improved\nput $(distdir).tar.gz.sig\n put fim-${fim_cv_version}.tar.gz\n put fim-${fim_cv_version}.tar.bz2\n put fim-${fim_cv_version}.tar.bz2.sig"  | sftp "dezperado@dl.sv.gnu.org" -b 
	lynx -dump dl.sv.gnu.org/releases/fbi-improved

savannah-lookup:
	lynx -dump dl.sv.gnu.org/releases/fbi-improved

# fixme : missing integration with ChangeLog file and such information
freshmeat_old: 
	@ awk  -- 'v=0;($$1 == "Version" && $$2=="'`cat VERSION`'"){while((input=getline) && $$1!="Version"){print}};'  NEWS 
	@echo freshmeat-submit -v "fbi-`cat VERSION`" \
	--project fbi-improved \
	--license GPL \
	--mailing-list-url      $$(cat README | grep "^Mailing List :"|sed s/^.*\ :.//g) \
	--home-page-url         $$(cat README | grep "^Web Page  "|sed s/^.*\ :.//g) \
	--cvs-url               $$(cat README | grep "^Repository"|sed s/^.*\ :.//g) \
	--gzipped-tar-url       $$(cat README | grep "^Releases  "|sed s/^.*\ :.//g)/"fim-`cat VERSION`".tar.gz \
	--mirror-site-url       $$(cat README | grep "^Off.*rror : "|sed s/^.*\ :.//g) \
	--changelog-url         $$(cat README | grep "^ChangeLog  "|sed s/^.*\ :.//g)
	@ echo "are you sure ? (any key to abort, yes to continue)"  && read yes && [[ "$${yes}" == "yes" ]] && \
	awk  -- 'v=0;($$1 == "Version" && $$2=="'`cat VERSION`'"){while((input=getline) && $$1!="Version"){print}};'  NEWS | \
	 freshmeat-submit -v "fbi-`cat VERSION`" \
	--project fbi-improved \
	--license GPL \
	--mailing-list-url      $$(cat README | grep "^Mailing List :"|sed s/^.*\ :.//g) \
	--home-page-url         $$(cat README | grep "^Web Page  "|sed s/^.*\ :.//g) \
	--cvs-url               $$(cat README | grep "^Repository"|sed s/^.*\ :.//g) \
	--gzipped-tar-url       $$(cat README | grep "^Releases  "|sed s/^.*\ :.//g)/"fim-`cat VERSION`".tar.gz \
	--mirror-site-url       $$(cat README | grep "^Off.*rror : "|sed s/^.*\ :.//g) \
	--changelog-url         $$(cat README | grep "^ChangeLog  "|sed s/^.*\ :.//g)
#	--url-demo         $$(cat README | grep "^Web Page  "|sed s/^.*:.//g)
#	--bzipped-tar-url  $$(cat README | grep "^Web Page  "|sed s/^.*:.//g)

freshmeat-submit:
	@echo Project: fim
	@echo Version: `cat VERSION`
	@echo Release-Focus: Major feature enhancements
	@echo Hide: N
	@echo Home-Page-URL: $$(cat README | grep "^Web Page  "|sed s/^.*\ :.//g)
	@echo Mailing-List-URL: $$(cat README | grep "^Mailing List :"|sed s/^.*\ :.//g)
	@echo Gzipped-Tar-URL: $$(cat README | grep "^Releases  "|sed s/^.*\ :.//g)/"fim-`cat VERSION`".tar.gz
	@echo Bzipped-Tar-URL: $$(cat README | grep "^Releases  "|sed s/^.*\ :.//g)/"fim-`cat VERSION`".tar.bz2
	@echo License: GPL
	@echo Mirror-Site-URL: $$(cat README | grep "^Off.*rror : "|sed s/^.*\ :.//g)
	@echo ChangeLog-URL: $$(cat README | grep "^ChangeLog  "|sed s/^.*\ :.//g)
	@echo CVS-URL: $$(cat README | grep "^Repository"|sed s/^.*\ :.//g)
	@echo 
	@ awk  -- 'v=0;($$1 == "Version" && $$2=="'`cat VERSION`'"){while((input=getline) && $$1!="Version"){print}};'  NEWS 

freshmeat:
	@ echo "are you sure you want to freshmeat-submit ? (any key to abort, yes to continue)"  && read yes && [[ "$${yes}" == "yes" ]] && \
	make freshmeat-submit | grep -v '^make\[' | freshmeat-submit

commit: log
	svn commit

# This awaits completion.
#rpm: dist
#        rpmbuild -ta @PACKAGE@-@VERSION@.tar.gz

