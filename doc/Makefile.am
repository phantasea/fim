
# $Id: Makefile.am 245 2009-04-28 21:28:38Z dezperado $

CLEANFILES= doctags *.o  tags

HTMLDOCS=FIM.html fim.man.html  fimgs.man.html
EXTRA_DIST = FIM.TXT fim.man doctags.c fim-stylesheet.css ./vim2html.pl fimgs.man

man1_MANS = fim.man fimgs.man

# html stuff is installed in the documentation directories, too
htmldir = $(docdir)
dist_html_DATA = $(HTMLDOCS)
dist_doc_DATA = FIM.TXT

all:	$(EXTRA_DIST)

htmldocs:        $(HTMLDOCS)
#docs:        $(HTMLDOCS)

FIM.html fim-stylesheet.css: doctags FIM.TXT
	./doctags FIM.TXT > tags ; ./vim2html.pl tags FIM.TXT

fim.man.html: fim.man
	man2html fim.man >  fim.man.html || echo "$(ERRMSG) man2html fim.man >  fim.man.html"

ERRMSG="no man2html at build time : please install man2html and reissue :"

fimgs.man.html: fimgs.man
	man2html fimgs.man >  fimgs.man.html || echo "$(ERRMSG) man2html fimgs.man >  fimgs.man.html"

doctags:
	        $(CC) -o doctags doctags.c

#clean:
#	        rm -f  doctags.o doctags FIM.html tags fim-stylesheet.css fim.man.html fimgs.man.html

