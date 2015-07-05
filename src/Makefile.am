# This file should be processed with automake to generate Makefile.in
#
# $Id: Makefile.am 263 2009-10-10 11:57:05Z dezperado $
#
# originally inspired by mutt's Makefile.am 

subdir = src
AUTOMAKE_OPTIONS = nostdinc no-dependencies dejagnu
SUBDIRS = 
check_PROGRAMS = fim
bin_PROGRAMS = fim
bin_SCRIPTS = fimgs
doc_DATA = fimrc

LFLAGS = -+
YFLAGS = -v -d

#lex.yy.cc : lex.lex
lex.yy.cc: lex.lex
	$(LEX) $(LFLAGS) lex.lex

yacc.tab.cpp: yacc.ypp lex.lex
	$(YACC) $(YFLAGS) yacc.ypp -o yacc.tab.cpp

FIM_LIB_OBJECTS = yacc.tab.o lex.yy.o

conf.h:       fimrc
	echo 'const char * FIM_DEFAULT_CONFIG_FILE_CONTENTS =' > conf.h
	sed  's/"/\\\"/g;s/^/"/g;s/$$/\\n"/g;' fimrc >> conf.h
	echo '"";' >> conf.h

# in the following : for some freebsd oddity, i was forced to replace ..0-9_]\+ with ..0-9_]*
version.h:	../config.h
	grep define ../config.h | grep FIM | sort | sed 's/^#//g;s/define //g;s/^\([A-Za-z0-9_]*\).*$$/+\1  /g;s/^/\"/g;s/$$/\"/g' > version.h
	echo "\"\n\"" >> version.h
	grep define ../config.h | grep FIM.*'"' | sort | sed 's/^#//g;s/define //g;s/^\([A-Za-z0-9_]*\) "\(.*\)"$$/"\1 = \\"\2\\"\\n"/g'   >> version.h
	grep undef ../config.h | grep FIM | sort | sed 's/^#//g;s/.*undef //g;s/^\([A-Za-z0-9_]*\).\+$$/-\1  /g;s/^/\"/g;s/$$/\"/g' >> version.h
	echo  '"\n'CXXFLAGS=$(CXXFLAGS)'\n"' >> version.h

help.cpp: fim.h
	echo -e "#ifdef FIM_WANT_INLINE_HELP\n" > help.cpp
	grep   '^#define FIM_VID' fim.h | sed 's/^#define //g;s/\(^[A-Z0-9_]\+\)\s\+\("[^ ]*"\)\s*\/\*\(.*\)\*\/$$/fim_var_help_db[\1]=\3;/g' >> help.cpp
	echo '#endif' >> help.cpp
	
# the following sources are built at make time
# old bison versions could have problems with this (e.g.: generating yacc.tab.cpp.h instead of yacc.tab.hpp)
BUILT_SOURCES = lex.yy.cc yacc.tab.cpp conf.h yacc.tab.hpp 

# this hook triggers on 'make dist'
dist-hook:
	-for file in $(BUILT_SOURCES) ; do rm -f $(distdir)/$$file ; done

EXTRA_fim_SOURCES = 
# The following files are marked as source files by configure.ac
EXTRA_fim_SOURCES += FbiStuffGif.cpp
EXTRA_fim_SOURCES += FbiStuffJpeg.cpp
EXTRA_fim_SOURCES += FbiStuffTiff.cpp
EXTRA_fim_SOURCES += FbiStuffPng.cpp
EXTRA_fim_SOURCES += FbiStuffDjvu.cpp
EXTRA_fim_SOURCES += FbiStuffPdf.cpp
EXTRA_fim_SOURCES += FbiStuffPs.cpp

CLEANFILES=$(BUILT_SOURCES) yacc.output 

#CPP_FLAGS="$CPP_FLAGS -D X_DISPLAY_MISSING"
AM_CPPFLAGS=
fim_DEPENDENCIES = @FIM_LIB_OBJECTS@ @LIBOBJS@ $(INTLDEPS)
fim_LDADD = @FIM_LIB_OBJECTS@ @LIBOBJS@ $(FIMLIBS) $(LEXLIB)
fim_SOURCES  = $(BUILT_SOURCES) \
	fim.cpp \
	fim.h \
	AADevice.h \
	CACADevice.h \
	SDLDevice.h \
	Arg.h \
	Browser.h \
	Cache.h \
	Command.h \
	CommandConsole.h \
	DebugConsole.h \
	DisplayDevice.h \
	DummyDisplayDevice.h \
	Image.h \
	FbiStuff.h \
	FbiStuffList.h \
	FbiStuffFbtools.h \
	FbiStuffLoader.h \
	FontServer.h \
	FramebufferDevice.h \
	Namespace.h \
	Var.h \
	Viewport.h \
	Window.h \
	common.h \
	conf.h \
	help.cpp \
	version.h \
	fim.h \
	fim_stream.h \
	fim_stream.cpp \
	fimgs \
	lex.h \
	readline.h \
	readline.cpp \
	string.h \
	AADevice.cpp \
	CACADevice.cpp \
	SDLDevice.cpp \
	Arg.cpp \
	Browser.cpp \
	Cache.cpp \
	Command.cpp \
	CommandConsole.cpp \
	CommandConsole-cmd.cpp \
	DebugConsole.cpp \
	DisplayDevice.cpp \
	FbiStuff.cpp \
	FbiStuffFbtools.cpp \
	FbiStuffLoader.cpp \
	FbiStuffPpm.cpp \
	FbiStuffBmp.cpp \
	FbiStuffBit1.cpp \
	FbiStuffBit24.cpp \
	FbiStuffMatrixMarket.cpp \
	FontServer.cpp \
	FramebufferDevice.cpp \
	Image.cpp \
	Namespace.cpp \
	Var.cpp \
	Viewport.cpp \
	Window.cpp \
	common.cpp \
	interpreter.cpp \
	string.cpp

Command.o: Command.h
CommandConsole.o: conf.h defaultConfiguration.o
fim.o: version.h help.cpp
Var.o: help.cpp
fim.cpp: version.h help.cpp
lex.yy.o: lex.yy.cc

# regex.c is work in progress and awaits for integration
EXTRA_DIST = lex.lex yacc.ypp fimrc defaultConfiguration.cpp fimgs \
	testdir/*ok testdir/*in testdir/Makefile \
	testsuite/*/* \
	regex.c

RUNTESTDEFAULTFLAGS = --tool ./fim FIM=$$srcdir/./fim  --srcdir $$srcdir/testsuite
#RUNTESTDEFAULTFLAGS = --all --tool ./fim FIM=$$srcdir/./fim --tool_opts=$$srcdir/../media/fim.png --srcdir $$srcdir/testsuite


