dnl Process this file with autoconf to produce a configure script.
dnl $Id: configure.ac 265 2009-10-14 21:05:40Z dezperado $

AC_PREREQ([2.54])

# we'll move forward, one day or another
#AC_PREREQ([2.61])

# for future use, too
#AC_PROG_CC(cc gcc)
#AC_PROG_CXX(CC g++ gcc c++ cxx)

define([svnversion],esyscmd([sh -c "svnversion | tr -d '\n'"]))dnl
# WARNING : here 0.0 denotes the package version, which will be corrected some lines after this.
AC_INIT([fim],[0.3],[dezperado_FOobAr_autistici_Baz_org( by replacing _FOobAr_ with a '@' and _Baz_ with a '.')])

AC_DEFINE(SVN_REVISION,"svnversion",[SVN REVISION])
AC_SUBST(SVN_REVISION,"svnversion")

AC_MSG_NOTICE([
        Welcome to the Fim (revision $SVN_REVISION) configure script!
	This script will create the right Makefile for building Fim on your system.
	But first, it will check if your system has the right configuratin for Fim to build.

	There are options to this script, please run :
	  ./configure --help=short
	for a complete list of these options.

	Please read BUGS and README and report any bug found to the Fim maintainer!
])


#AM_CONFIG_HEADER([config.h])
#AC_CONFIG_SRCDIR(pinentry/pinentry.h)

#
#AM_INIT_AUTOMAKE($PACKAGE_NAME, $PACKAGE_VERSION)
AC_GNU_SOURCE
#AC_MANTAINER_MODE
#AC_CANONICAL_HOST

#missing_dir=`cd $ac_aux_dir && pwd`
#AM_MISSING_PROG(ACLOCAL, aclocal, $missing_dir)
#AM_MISSING_PROG(AUTOCONF, autoconf, $missing_dir)
#AM_MISSING_PROG(AUTOMAKE, automake, $missing_dir)
#AM_MISSING_PROG(AUTOHEADER, autoheader, $missing_dir)
#AM_MISSING_PROG(MAKEINFO, makeinfo, $missing_dir)



AC_PROG_CC
AC_PROG_CPP
AC_PROG_YACC
AC_PROG_LEX

AC_PROG_INSTALL
AC_PROG_RANLIB

AC_REVISION([$Revision$])

AC_PROG_CXX
#AC_PROG_CXXCPP
AC_CHECK_TOOL(AR, ar, ar)
fim_cv_version=`cat VERSION`
AC_LANG([C++])
#AC_LANG_PUSH([C++])
AM_INIT_AUTOMAKE(fim, $fim_cv_version)
#AM_INIT_AUTOMAKE(fim, 1.0)
AC_SUBST([CONFIG_STATUS_DEPENDENCIES], ['$(top_srcdir)/VERSION'])
CPPFLAGS=''


AC_SUBST(FIM_LIBS)
AC_SUBST(FIM_LIB_OBJECTS)
AC_SUBST(VERSION)
AC_SUBST(HTMLDOCS)
#AC_LANG_POP([C++])

fim="fim"
AC_MSG_CHECKING([for prefix])
if test x$prefix = xNONE; then
  fim_cv_prefix=$ac_default_prefix
else
  fim_cv_prefix=$prefix
fi
AC_MSG_RESULT($fim_cv_prefix)

AC_ISC_POSIX
AM_C_PROTOTYPES
if test "x$U" != "x"; then
  AC_MSG_ERROR(Compiler not ANSI compliant)
fi
#AC_PROG_CPP
AC_PROG_MAKE_SET

AC_C_INLINE
AC_C_CONST

AC_SYS_LARGEFILE
AC_FUNC_FSEEKO
AC_CHECK_SIZEOF(off_t)
AM_MAINTAINER_MODE

AH_TEMPLATE([HAVE_GETENV])
AC_CHECK_FUNC([getenv], AC_DEFINE([HAVE_GETENV], [1], [If present, the getenv function allows fim to read environment variables.]))
AC_CHECK_FUNC([getline], AC_DEFINE([HAVE_GETLINE], [1], [If present, will be used for stdin reading. (on GNU)]))
AC_CHECK_FUNC([fgetln], AC_DEFINE([HAVE_FGETLN], [1], [If present, fgetln() will be used as a replacement for getline(). (on BSD)]))
AC_CHECK_FUNC([get_current_dir_name], AC_DEFINE([HAVE_GET_CURRENT_DIR_NAME], [1], [Should live when _GNU_SOURCE, but it could be not the case.]))

AC_CHECK_LIB([m],[pow])
AC_CHECK_LIB([fl],[yylex])

have_default_bindings=yes
have_default_mapping=yes
have_default_config=yes

AC_ARG_ENABLE(default_bindings, AC_HELP_STRING([--disable-default-bindings], [Builtin default minimal key binding]),
[       if test x$enableval = xyes; then
		have_default_bindings=yes
	else
                have_default_bindings=no
        fi
])

AC_ARG_ENABLE(default_mapping, AC_HELP_STRING([--disable-default-mapping], [Builtin default minimal key mapping]),
[       if test x$enableval = xyes; then
                have_default_mapping=yes
	else
		have_default_mapping=no
        fi
])

AC_ARG_ENABLE(default_config, AC_HELP_STRING([--disable-default-config], [Builtin default minimal configuration]),
[       if test x$enableval = xyes; then
                have_default_config=yes
	else
		have_default_config=no
        fi
])

	AH_TEMPLATE([FIM_DEFAULT_CONFIG])
	if test x$have_default_bindings != xno; then
		FIM_DEFS="$FIM_DEFS -D FIM_DEFAULT_CONFIG"
		AC_DEFINE([FIM_DEFAULT_CONFIG])
	fi
	AH_TEMPLATE([FIM_DEFAULT_KEY_CONFIG])
	if test x$have_default_mapping != xno; then
		FIM_DEFS="$FIM_DEFS -D FIM_DEFAULT_KEY_CONFIG"
		AC_DEFINE([FIM_DEFAULT_KEY_CONFIG])
	fi
	AH_TEMPLATE([FIM_DEFAULT_CONFIGURATION])
	if test x$have_default_config != xno; then
		FIM_DEFS="$FIM_DEFS -D FIM_DEFAULT_CONFIGURATION"
		AC_DEFINE([FIM_DEFAULT_CONFIGURATION])
	fi

#CPPFLAGS=$DEPS
#AC_SUBST(LIBSDL_CXXFLAGS)
#AC_SUBST(LIBPNG_CXXFLAGS)
#AC_SUBST(LIBSDL_CXXFLAGS)
#AC_SUBST(LIBPNG_CXXFLAGS)
#AC_SUBST(LIBPNG_LIBS)

AC_ARG_ENABLE(system, AC_HELP_STRING([--disable-system], [Disable system() and pipe() calls.]),
[       if test x$enableval = xno ; then
                fim_system_calls=no
        fi
])

dnl new: 
AC_ARG_ENABLE(framebuffer, AC_HELP_STRING([--disable-framebuffer], [Disable the framebuffer device driver.]),
[       if test x$enableval = xno ; then
                fim_handle_framebuffer=no
        fi
])

AC_MSG_NOTICE([Checking for headers required by the Linux framebuffer device.])
AC_CHECK_HEADERS([linux/fb.h], [break], AC_MSG_WARN([[Cannot find linux/fb.h.  Fim Linux framebuffer driver requires it!.]]))
AC_CHECK_HEADERS([linux/vt.h], [break], AC_MSG_WARN([[Cannot find linux/vt.h.  Fim Linux framebuffer driver requires it!.]]))
AC_CHECK_HEADERS([linux/kd.h], [break], AC_MSG_WARN([[Cannot find linux/kd.h.  Fim Linux framebuffer driver requires it!.]]))

if test x$ac_cv_header_linux_fb_h != xyes -o  \
	 x$ac_cv_header_linux_vt_h != xyes -o \
	 x$ac_cv_header_linux_kd_h != xyes ; then
	AC_MSG_WARN([Turning off Linux framebuffer support due to missing headers])
	fim_handle_framebuffer=no 
fi

if test x$fim_handle_framebuffer = xno ; then
	AC_DEFINE(FIM_WITH_NO_FRAMEBUFFER, 1, [Defined when framebuffer device support is turned off.])
fi

AC_HEADER_STDC
AC_FUNC_MMAP

AC_CHECK_FUNCS([bzero memset memcmp strncmp strcpy])
AC_CHECK_FUNCS([bcopy bcmp])
AC_CHECK_FUNCS([drand48 random rand])
dnl AC_FUNC_REALLOC()	dnl triggers error in automake 1.10.1

#AC_CHECK_HEADERS([stddef.h])
#AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_HEADERS([string.h strings.h])

AC_HEADER_DIRENT()
AC_CHECK_HEADERS([stdarg.h sys/ioctl.h ioctl.h sysexits.h])
AC_CHECK_HEADERS([sys/time.h sys/resource.h])
AC_CHECK_HEADERS([unix.h])
AC_CHECK_HEADERS([cstdio stdio.h], [break],AC_MSG_ERROR([[Cannot find cstdio or stdio.h.  Fim requires at least one or the other.]]))

AC_CHECK_HEADERS([climits limits.h], [break], AC_MSG_ERROR([[Cannot find limits.h nor climits.  Fim requires it!.]]))

AC_CHECK_SIZEOF([int])
AC_CHECK_SIZEOF([int *])
AC_C_BIGENDIAN([fim_big_endian=yes],[fim_big_endian=no],[fim_big_endian=hmmm])

if test "x$fim_big_endian" = "xyes" ; then
	AH_TEMPLATE([FIM_BIG_ENDIAN])
	AC_DEFINE([FIM_BIG_ENDIAN])
fi


dnl ============= begin misc external programs detection
dnl ============= end misc external programs detection

dnl if test "X$ac_cv_header_cstdio" != "Xyes" && test "X$ac_cv_header_stdio_h" != "Xyes"; then
dnl   AC_MSG_ERROR([[Cannot find cstdio or stdio.h.  Fim requires at least one or the other.]])
dnl fi

dnl if test "X$ac_cv_header_linux_fb_h" != "Xyes" ; then
dnl   AC_MSG_ERROR([[Cannot find linux/fb.h.  Fim requires it!.]])
dnl fi



dnl AC_CHECK_FUNCS(setrlimit getsid)

AC_TYPE_SIGNAL


AC_ARG_WITH(regex, AC_HELP_STRING([--with-regex], [Use the GNU regex library insteat of the POSIX one]),
        [fim_cv_regex=yes],
        [AC_CHECK_FUNCS(regcomp, fim_cv_regex=no, fim_cv_regex=yes)])

if test X$fim_cv_regex = Xno ; then
AC_CACHE_CHECK([whether your system's regexp library is completely broken],
        [fim_cv_regex_broken],
        AC_TRY_RUN([
#include <unistd.h>
#include <regex.h>
main() { regex_t blah ; regmatch_t p; p.rm_eo = p.rm_eo; return regcomp(&blah, "foo.*bar", REG_NOSUB) || regexec (&blah, "foobar", 0, NULL, 0); }],
        fim_cv_regex_broken=no, fim_cv_regex_broken=yes, fim_cv_regex_broken=yes))
        if test X$fim_cv_regex_broken = Xyes ; then
                echo "Using the included GNU regex instead." >&AC_FD_MSG
                fim_cv_regex=yes
        fi
fi

if test X$fim_cv_regex = Xyes; then
        AC_DEFINE(USE_GNU_REGEX,1,[ Define if you want to use the included (GNU) regex.c. ])
        AC_LIBOBJ(regex)
fi



AC_MSG_CHECKING(where to put the documentation...)
dnl AC_ARG_WITH(docdir, AC_HELP_STRING([--with-docdir=PATH], [Specify where to put the documentation]),
dnl         [fim_cv_docdir=$withval],
dnl        [fim_cv_docdir='${prefix}/doc/fim-${fim_cv_version}'])
# FIXME : the above line seems troublesome, BUT IT IS NOT (see the muttrc for this trick).

AC_MSG_RESULT($fim_cv_docdir)

dnl docdir=$fim_cv_docdir
dnl AC_SUBST(docdir)
AC_SUBST(fim_cv_version)

dnl ============= gif file handling

AC_ARG_ENABLE(gif, AC_HELP_STRING([--disable-gif], [Disable gif support.]),
[       if test x$enableval = xno ; then
		fim_handle_gif=no
        fi
])
if test x$fim_handle_gif != xno ; then
  fim_handle_gif=yes
  have_gif=yes
  if test x$with_libgif != xno && test -z "$LIBGIF"; then
    AC_CHECK_LIB(gif,DGifOpenFileHandle ,
      have_gif=yes,
      have_gif=no
      AC_MSG_WARN([*** GIF loader will not be built (GIF library not found) ***]))
    if test x"$have_gif" = xyes; then
      AC_MSG_CHECKING([for gif.h])
      AC_TRY_CPP(
[#include <stdio.h>
#undef PACKAGE
#undef VERSION
#undef HAVE_STDLIB_H
#include <gif_lib.h>],
        have_gif=yes,
        have_gif=no)
      AC_MSG_RESULT($have_gif)
      if test x"$have_gif" = xyes; then
        LIBGIF='-lgif'
      else
          AC_MSG_WARN([*** GIF loader will not be built (GIF header file not found) ***])
      fi
    fi
  fi

#  if test x$with_libgif != xno ; then
# the following needs thorough testing
  if test x"$have_gif" == xyes ; then
  FIM_LIB_OBJECTS="$FIM_LIB_OBJECTS FbiStuffGif.o"
  LIBS="$LIBS $LIBGIF"
  AC_DEFINE([FIM_HANDLE_GIF])
  else
    fim_handle_gif=no
  fi

#  if test x$with_libgif != xno && test -z "$LIBGIF"; then
  #
#  fi
else
  fim_handle_gif=no
fi
AC_SUBST(LIBGIF)
AM_CONDITIONAL(ENABLE_GIF, test x$have_gif = xyes)


dnl ============= tiff file handling

AC_ARG_ENABLE(tiff, AC_HELP_STRING([--disable-tiff], [Disable tiff support.]),
[       if test x$enableval = xno ; then
		fim_handle_tiff=no
        fi
])
if test x$fim_handle_tiff != xno ; then
  fim_handle_tiff=yes
  have_tiff=yes
  if test x$with_libtiff != xno && test -z "$LIBTIFF"; then
    AC_CHECK_LIB(tiff,TIFFOpen ,
      have_tiff=yes,
      have_tiff=no
      AC_MSG_WARN([*** TIFF loader will not be built (TIFF library not found) ***]))
    if test x"$have_tiff" = xyes; then
      AC_MSG_CHECKING([for tiff.h])
      AC_TRY_CPP(
[#include <stdio.h>
#undef PACKAGE
#undef VERSION
#undef HAVE_STDLIB_H
#include <tiff.h>],
        have_tiff=yes,
        have_tiff=no)
      AC_MSG_RESULT($have_tiff)
      if test x"$have_tiff" = xyes; then
        LIBTIFF='-ltiff'
      else
          AC_MSG_WARN([*** TIFF loader will not be built (TIFF header file not found) ***])
      fi
    fi
  fi

#  if test x$with_libtiff != xno ; then
# the following needs thorough testing
  if test x"$have_tiff" == xyes ; then
  FIM_LIB_OBJECTS="$FIM_LIB_OBJECTS FbiStuffTiff.o"
  LIBS="$LIBS $LIBTIFF"
  AC_DEFINE([FIM_HANDLE_TIFF])
  else
    fim_handle_tiff=no
  fi

#  if test x$with_libtiff != xno && test -z "$LIBTIFF"; then
  #
#  fi
else
  fim_handle_tiff=no
fi
AC_SUBST(LIBTIFF)
AM_CONDITIONAL(ENABLE_TIFF, test x$have_tiff = xyes)


dnl ============= cacalib : optional
AC_CHECK_LIB(caca, caca_init,have_cacalib=yes,have_cacalib=no)

dnl ============= GNU gpm, unofficial yet

#    AC_CHECK_LIB(gpm,Gpm_Open,
#      ,true,
#      AC_MSG_ERROR(*** gpm lib not found ... no problem ... ***))
#LIBS="$LIBS -lgpm"

dnl ============= no readline case handling

fim_want_readline=yes
AC_ARG_ENABLE(readline, AC_HELP_STRING([--disable-readline], [Disable readline support (EXPERIMENTAL).]),
[       if test x$enableval = xno ; then
		fim_want_readline=no
	else
		fim_want_readline=yes
        fi
])

dnl ============= GNU readline : essential

dnl Please note that when the readline library is compiled with the curses library, this check will fail, because
dnl we cannot know in advance if it is so.
AH_TEMPLATE([FIM_USE_READLINE])
AH_TEMPLATE([FIM_HANDLE_GIF])
AH_TEMPLATE([FIM_HANDLE_TIFF])

have_readline=no

# Sometimes the readline library is linked to the curses library, and we here try to detect this case..
if test x$have_readline = xno; then
  if test x$fim_want_readline = xyes ; then
    AC_CHECK_LIB(readline, rl_replace_line,
      have_readline=yes
      LIBS="$LIBS -lreadline"
      AC_DEFINE([FIM_USE_READLINE])
      ,have_readline=no
      AC_MSG_ERROR(*** GNU readline library headers not found !  please install them or run ./configure --disable-readline ***)
      ,[ -lcurses ]
      )
  else
      have_readline=no
      AC_MSG_NOTICE(*** Fim will be built withoud GNU readline library headers ! Are you sure ?  ***)
  fi
fi

dnl FIXME : we try to detecht curses, although we don't know if the preceding did use it
if test x$have_readline = xyes; then
  if test x$fim_want_readline = xyes ; then
    AC_CHECK_LIB(curses, tputs,
      have_curses=yes
      LIBS="$LIBS -lcurses"
      ,have_curses=no
      )
  fi
fi

dnl ============= jpeg file handling

#AC_ARG_WITH(jpeg, [  --disable-jpeg ], [ disable jpeg file support])
AC_ARG_ENABLE(jpeg, AC_HELP_STRING([--disable-jpeg], [Disable jpeg support.]),
[       if test x$enableval = xno ; then
		fim_handle_jpeg=no
        fi
])
if test x$fim_handle_jpeg != xno ; then
  fim_handle_jpeg=yes
  have_jpeg=yes
  if test x$with_libjpeg != xno && test -z "$LIBJPEG"; then
    AC_CHECK_LIB(jpeg, jpeg_destroy_decompress,
      have_jpeg=yes,
      have_jpeg=no
      AC_MSG_WARN([*** JPEG loader will not be built (JPEG library not found) ***]))
    if test x"$have_jpeg" = xyes; then
      AC_MSG_CHECKING([for jpeglib.h])
      AC_TRY_CPP(
[#include <stdio.h>
#undef PACKAGE
#undef VERSION
#undef HAVE_STDLIB_H
#include <jpeglib.h>],
        have_jpeg=yes,
        have_jpeg=no)
      AC_MSG_RESULT($have_jpeg)
      if test x"$have_jpeg" = xyes; then
        LIBJPEG='-ljpeg'
      else
          AC_MSG_WARN([*** JPEG loader will not be built (JPEG header file not found) ***])
      fi
    fi
  fi

#  if test x$with_libjpeg != xno ; then
# the following needs thorough testing
  if test x"$have_jpeg" == xyes ; then
  FIM_LIB_OBJECTS="$FIM_LIB_OBJECTS FbiStuffJpeg.o"
  LIBS="$LIBS $LIBJPEG"
  AC_DEFINE([HAVE_LIBJPEG],[1],[We have libjpeg])
  else
    fim_handle_jpeg=no
  fi

#  if test x$with_libjpeg != xno && test -z "$LIBJPEG"; then
#     AC_MSG_ERROR([
#     Checks for JPEG loader failed. You can build without it by passing
#    --disable-jpeg to ./configure. ])
#  fi
else
  fim_handle_jpeg=no
fi
AC_SUBST(LIBJPEG)
AM_CONDITIONAL(ENABLE_JPEG, test x$have_jpeg = xyes)

dnl ============= dvi file handling (via TeX-Guy)
dnl 
dnl AC_ARG_ENABLE(dvi, AC_HELP_STRING([--disable-dvi], [Disable dvi support.]),
dnl [       if test x$enableval = xno ; then
dnl 		fim_handle_dvi=no
dnl         fi
dnl ])
dnl if test x$fim_handle_dvi != xno ; then
dnl   fim_handle_dvi=yes
dnl   have_dvi=yes
dnl   if test x$with_libdvi != xno && test -z "$LIBDVI"; then
dnl     AC_CHECK_LIB(dvilib2, DVI_init,
dnl       have_dvi=yes,
dnl       have_dvi=no
dnl       AC_MSG_WARN([*** DVI loader will not be built (libdvi29 library not found) ***]))
dnl     if test x"$have_dvi" = xyes; then
dnl       AC_MSG_CHECKING([for libdvi29.h])
dnl       AC_TRY_CPP(
dnl [#include <stdio.h>
dnl #undef PACKAGE
dnl #undef VERSION
dnl #undef HAVE_STDLIB_H
dnl #include <libdvi29.h>],
dnl         have_dvi=yes,
dnl         have_dvi=no)
dnl       AC_MSG_RESULT($have_dvi)
dnl       if test x"$have_dvi" = xyes; then
dnl         LIBDVI='-ldvilib2'
dnl       else
dnl           AC_MSG_WARN([*** DVI loader will not be built (libdvi29.h header file not found) ***])
dnl       fi
dnl     fi
dnl   fi
dnl 
dnl # the following needs thorough testing
dnl   if test x"$have_dvi" == xyes ; then
dnl   FIM_LIB_OBJECTS="$FIM_LIB_OBJECTS FbiStuffDvi.o"
dnl   LIBS="$LIBS $LIBDVI"
dnl   AC_DEFINE([HAVE_LIBSPECTRE],[1],[We have libdvilib2])
dnl   else
dnl     fim_handle_dvi=no
dnl   fi
dnl 
dnl #  if test x$with_libdvi != xno && test -z "$LIBDVI"; then
dnl #     AC_MSG_ERROR([
dnl #     Checks for DVI loader failed. You can build without it by passing
dnl #    --disable-dvi to ./configure. ])
dnl #  fi
dnl else
dnl   fim_handle_dvi=no
dnl fi
dnl AC_SUBST(LIBDVI)
dnl AM_CONDITIONAL(ENABLE_DVI, test x$have_dvi = xyes)

dnl ============= ps file handling

AC_ARG_ENABLE(ps, AC_HELP_STRING([--disable-ps], [Disable ps support.]),
[       if test x$enableval = xno ; then
		fim_handle_ps=no
        fi
])
if test x$fim_handle_ps != xno ; then
  fim_handle_ps=yes
  have_ps=yes
  if test x$with_libps != xno && test -z "$LIBPS"; then
    AC_CHECK_LIB(spectre, spectre_page_render,
      have_ps=yes,
      have_ps=no
      AC_MSG_WARN([*** PS loader will not be built (spectre library not found) ***]))
    if test x"$have_ps" = xyes; then
      AC_MSG_CHECKING([for libspectre/spectre.h])
      AC_TRY_CPP(
[#include <stdio.h>
#undef PACKAGE
#undef VERSION
#undef HAVE_STDLIB_H
#include <libspectre/spectre.h>],
        have_ps=yes,
        have_ps=no)
      AC_MSG_RESULT($have_ps)
      if test x"$have_ps" = xyes; then
        LIBPS='-lspectre'
      else
          AC_MSG_WARN([*** PS loader will not be built (PS header file not found) ***])
      fi
    fi
  fi

# the following needs thorough testing
  if test x"$have_ps" == xyes ; then
  FIM_LIB_OBJECTS="$FIM_LIB_OBJECTS FbiStuffPs.o"
  LIBS="$LIBS $LIBPS"
  AC_DEFINE([HAVE_LIBSPECTRE],[1],[We have libspectre])
  else
    fim_handle_ps=no
  fi

#  if test x$with_libps != xno && test -z "$LIBPS"; then
#     AC_MSG_ERROR([
#     Checks for PS loader failed. You can build without it by passing
#    --disable-ps to ./configure. ])
#  fi
else
  fim_handle_ps=no
fi
AC_SUBST(LIBPS)
AM_CONDITIONAL(ENABLE_PS, test x$have_ps = xyes)



dnl ============= djvu file handling

AC_ARG_ENABLE(djvu, AC_HELP_STRING([--disable-djvu], [Disable djvu support.]),
[       if test x$enableval = xno ; then
		fim_handle_djvu=no
        fi
])
if test x$fim_handle_djvu != xno ; then
  fim_handle_djvu=yes
  have_djvu=yes
  if test x$with_libdjvu != xno && test -z "$LIBDJVU"; then
    AC_CHECK_LIB(djvulibre, ddjvu_page_create_by_pageno,
      have_djvu=yes,
      have_djvu=no
      AC_MSG_WARN([*** DJVU loader will not be built (DJVU library not found) ***]))
    if test x"$have_djvu" = xyes; then
      AC_MSG_CHECKING([for libdjvu/ddjvuapi.h])
      AC_TRY_CPP(
[#include <stdio.h>
#undef PACKAGE
#undef VERSION
#undef HAVE_STDLIB_H
#include <libdjvu/ddjvuapi.h>],
        have_djvu=yes,
        have_djvu=no)
      AC_MSG_RESULT($have_djvu)
      if test x"$have_djvu" = xyes; then
        LIBDJVU='-ldjvulibre'
      else
          AC_MSG_WARN([*** DJVU loader will not be built (DJVU header file not found) ***])
      fi
    fi
  fi

# the following needs thorough testing
  if test x"$have_djvu" == xyes ; then
  FIM_LIB_OBJECTS="$FIM_LIB_OBJECTS FbiStuffDjvu.o"
  LIBS="$LIBS $LIBDJVU"
  AC_DEFINE([HAVE_LIBDJVU],[1],[We have libdjvulibre])
  else
    fim_handle_djvu=no
  fi

#  if test x$with_libdjvu != xno && test -z "$LIBDJVU"; then
#     AC_MSG_ERROR([
#     Checks for DJVU loader failed. You can build without it by passing
#    --disable-djvu to ./configure. ])
#  fi
else
  fim_handle_djvu=no
fi
AC_SUBST(LIBDJVU)
AM_CONDITIONAL(ENABLE_DJVU, test x$have_djvu = xyes)


dnl ============= pdf file handling with libpoppler 
dnl
dnl By default, we disable poppler : its API is still immature.
dnl
fim_handle_pdf=no
AC_ARG_ENABLE(poppler, AC_HELP_STRING([--enable-poppler], [Enable poppler (PDF rendering) support.]),
[       if test x$enableval = xyes ; then
		fim_handle_pdf=yes
        fi
])
dnl FIXME : checking only for libpoppler-glib, should also check for libpoppler !
if test x$fim_handle_pdf != xno ; then
  fim_handle_pdf=yes
  have_poppler=yes
        if test x$with_libpoppler != xno && test -z "$LIBPOPPLER"; then
dnl FIXME : the following is a dirty hack
          AC_CHECK_LIB(poppler, gfree,
            have_poppler=yes,
            have_poppler=no
      AC_MSG_WARN([*** POPPLER loader will not be built (POPPLER library not found) ***]) )
    if test x"$have_poppler" = xyes; then
      AC_MSG_CHECKING([for poppler/poppler-config.h])
       AC_TRY_CPP(
       [#include <stdio.h>
       #undef PACKAGE
       #undef VERSION
       #undef HAVE_STDLIB_H
       /* fim uses these headers. let's hope poppler will stabilize soon!*/
       #include <poppler/poppler-config.h>
       #include <poppler/PDFDoc.h>
       #include <poppler/OutputDev.h>
       #include <poppler/SplashOutputDev.h>
       #include <poppler/splash/SplashBitmap.h>
       #include <poppler/splash/SplashTypes.h>
       #include <poppler/Page.h>
       #include <poppler/GlobalParams.h>	/* globalParams lives here */
       ],
               have_poppler=yes,
               have_poppler=no)
      have_poppler=yes
      AC_MSG_RESULT($have_poppler)
      if test x"$have_poppler" = xyes; then
        LIBPOPPLER='-lpoppler'
	# FIXME: the following is a dirty hack!
        LIBPOPPLER_CXXFLAGS="-I/usr/include/poppler/"
        CXXFLAGS="$CXXFLAGS $LIBPOPPLER_CXXFLAGS"
dnl	are we sure poppler doesn't have something like the following ?
dnl        LIBPOPPLER_LIBS=`$LIBPOPPLER_CONFIG --libs`
      else
          AC_MSG_WARN([*** POPPLER/PDF loader will not be built (POPPLER header file not found) ***])
      fi
    fi
  fi

# the following needs thorough testing
  if test x"$have_poppler" == xyes ; then
  FIM_LIB_OBJECTS="$FIM_LIB_OBJECTS FbiStuffPdf.o"
  LIBS="$LIBS $LIBPOPPLER"
  AC_DEFINE([HAVE_LIBPOPPLER],[1],[We have libpoppler (Warning: the API could still break! (tested with v.0.8.7))])
  else
    fim_handle_pdf=no
  fi

#  if test x$with_libpoppler != xno && test -z "$LIBPOPPLER"; then
#     AC_MSG_ERROR([
#     Checks for POPPLER loader failed. You can build without it by passing
#    --disable-poppler to ./configure. ])
#  fi
else
  fim_handle_pdf=no
fi
AC_SUBST(LIBPOPPLER)
AM_CONDITIONAL(ENABLE_POPPLER, test x$have_poppler = xyes)

dnl FIXME : we have two libpoppler checks ... :
dnl ================== pdf checks ===================================================
AC_ARG_ENABLE(pdf,
              [AC_HELP_STRING([--enable-pdf], [Compile with pdf support])],enable_pdf="$enableval",enable_pdf=yes)

if test "x$enable_pdf" = "xyes"; then
    POPPLER_REQUIRED=0.8.7
    # dear fim hackers, you should install pkg-config to get this macro expanded
    PKG_CHECK_MODULES(POPPLER, poppler >= $POPPLER_REQUIRED,enable_pdf=yes,enable_pdf=no)

    if test "x$enable_pdf" = "xyes"; then
            AC_DEFINE([ENABLE_PDF], [1], [Enable pdf support.])
            SHELL_LIBS="$SHELL_LIBS $POPPLER_LIBS"
            SHELL_CXXFLAGS="$SHELL_CXXFLAGS $POPPLER_CXXFLAGS"

            fim_save_LIBS=$LIBS
            LIBS="$LIBS $POPPLER_LIBS"
            LIBS=$fim_save_LIBS

    else
            AC_MSG_WARN(["PDF support is disabled since poppler library version $POPPLER_REQUIRED or newer not found"])
    fi
fi

AM_CONDITIONAL(ENABLE_PDF, test x$enable_pdf = xyes)
dnl ================== end of pdf checks ============================================

dnl ================================================================================

## png file handling

AC_ARG_WITH(png-prefix, AC_HELP_STRING([--with-png-prefix=PFX], [prefix where libpng is installed (optional)]),
libpng_config_prefix="$withval", libpng_config_prefix="")

fim_handle_png=yes

AC_ARG_ENABLE(png, AC_HELP_STRING([--disable-png], [Disable png support.]),
[       if test x$enableval = xno ; then
		fim_handle_png=no
        fi
])

if test x$fim_handle_png = xyes; then
	# support for custom library configurator path
        if test x$libpng_config_prefix != x; then
                LIBPNG_CONFIG="$libpng_config_prefix/bin/libpng-config"
        else
		# default path configurator
                AC_PATH_PROG(LIBPNG_CONFIG, libpng-config, no)
        fi
        if test "x$LIBPNG_CONFIG" = "xno"; then
      		AC_MSG_WARN([*** PNG loader will not be built (PNG library not found) ***])
		fim_handle_png=no
        else
                CXXFLAGS="$CXXFLAGS `$LIBPNG_CONFIG --cflags`"
                LIBS="$LIBS `$LIBPNG_CONFIG --libs`"
		# finish this !
                FIM_LIB_OBJECTS="$FIM_LIB_OBJECTS FbiStuffPng.o"
                AC_DEFINE(FIM_WITH_LIBPNG, 1, [Defined, if libpng support is enabled])
        fi
fi

#AM_CONDITIONAL(BUILD_PNG, test x$fim_handle_png = xyes)

dnl ================================================================================

AC_ARG_ENABLE(aa, AC_HELP_STRING([--enable-aa], [Turns on aalib support (EXPERIMENTAL)]),
	[if test x$enableval = xyes; then
		fim_cv_aalib=yes
	        if test x$libaa_config_prefix != x; then
			LIBAA_CONFIG="$libaa_config_prefix/bin/aalib-config"
		else
			# default path configurator
			AC_PATH_PROG(LIBAA_CONFIG, aalib-config, no)
		fi
		if test "x$LIBAA_CONFIG" = "xno"; then
			# uhm
	      		AC_MSG_WARN([***  no aalib-config ? ***])
			fim_handle_png=no
			fim_cv_aalib=no
		else
			CXXFLAGS="$CXXFLAGS `$LIBAA_CONFIG --cflags`"
			LIBS="$LIBS `$LIBAA_CONFIG --libs`"
			# finish this !
		fi
		AC_CHECK_LIB(aa, aa_getfirst,have_aalib=yes,have_aalib=no)
		if test x$have_aalib = xyes; then
      			AC_MSG_NOTICE([Enabling aalib driver.])
		else
			fim_cv_aalib=no
		        AC_MSG_ERROR([Sorry, it doesn't seem like aalib is installed or detectable.
			Please run --disable-aa or install aalib in a system-wide directory instead.
			])
		fi
	fi],[fim_cv_aalib=no])

dnl ================================================================================

## sdl driver

AC_ARG_WITH(sdl-prefix, AC_HELP_STRING([--with-sdl-prefix=PFX], [prefix where libsdl is installed (optional)]),
libsdl_config_prefix="$withval", libsdl_config_prefix="")

fim_handle_sdl=no

dnl AC_ARG_ENABLE(sdl, AC_HELP_STRING([--disable-sdl], [Disable libsdl support.]),
dnl [       if test x$enableval = xno ; then
dnl 		fim_handle_sdl=no
dnl         fi
dnl ])

AC_ARG_ENABLE(sdl, AC_HELP_STRING([--enable-sdl], [Enable libsdl support.]),
[       if test x$enableval = xyes ; then
		fim_handle_sdl=yes
        fi
])

# tested with sdl 1.2
if test x$fim_handle_sdl = xyes; then
	# support for custom library configurator path
        if test x$libsdl_config_prefix != x; then
                LIBSDL_CONFIG="$libsdl_config_prefix/bin/sdl-config"
        else
		# default path configurator
                AC_PATH_PROG(LIBSDL_CONFIG, sdl-config, no)
        fi
        if test "x$LIBSDL_CONFIG" = "xno"; then
                AC_MSG_ERROR([LIBSDL not found])
        else
                LIBSDL_CXXFLAGS=`$LIBSDL_CONFIG --cflags`
                CXXFLAGS="$CXXFLAGS $LIBSDL_CXXFLAGS"
                LIBS="$LIBS `$LIBSDL_CONFIG --libs`"
		# finish this !
                #FIM_LIB_OBJECTS="$FIM_LIB_OBJECTS SDLDevice.o"
                AC_DEFINE(FIM_WITH_LIBSDL, 1, [Defined, if libsdl support is enabled])
        fi
fi



#AC_DEFINE([FIM_DEFAULT_KEY_CONFIG]   ,1, [Defined, if GPGME support is enabled])
#AC_DEFINE([FIM_DEFAULT_CONFIG], 1, [Defined, if GPGME support is enabled])

#AC_DEFINE([FIM_DEFAULT_CONFIGURATION], 1, [Defined, if GPGME support is enabled])

#AC_MSG_CHECKING(whether struct dirent defines d_ino)

dnl ==================

fim_cv_autocommands=yes

AC_ARG_ENABLE(autocommands, AC_HELP_STRING([--disable-autocommands], [Fim will by default use autocommands.]),
[if test x$enableval = xno; then
        fim_cv_autocommands=no
fi])

AH_TEMPLATE([FIM_AUTOCMDS])
if test x$fim_cv_autocommands = xyes; then
	have_autocommands=yes
	AC_DEFINE([FIM_AUTOCMDS])
else
	have_autocommands=no
fi


dnl ==================

fim_cv_windows=yes

AC_ARG_ENABLE(windows, AC_HELP_STRING([--disable-windows], [Disable Fim's own windowing system. (deprecated)]),
[if test x$enableval = xno; then
        fim_cv_windows=no
fi])

AH_TEMPLATE([FIM_WINDOWS])
if test x$fim_cv_windows = xyes; then
	have_windows=yes
	AC_DEFINE([FIM_WINDOWS],1,[Fim windowing support])
else
	have_windows=no
fi


dnl ==================

fim_cv_fimrc=yes

AC_ARG_ENABLE(fimrc, AC_HELP_STRING([--disable-fimrc], [Fim will by default load the ~.fimrc file.]),
[if test x$enableval = xno; then
        fim_cv_fimrc=no
fi])

AH_TEMPLATE([FIM_NOFIMRC])
if test x$fim_cv_fimrc = xyes; then
	have_fimrc=yes
else
	AC_DEFINE([FIM_NOFIMRC])
fi

dnl ==================

fim_cv_warnings=no
AC_ARG_ENABLE(screen, AC_HELP_STRING([--disable-screen], [Fim will try to work under screen by default (please read BUGS)]),
[       if test x$enableval = xno; then
		fim_cv_screen=no
	else
                fim_cv_screen=yes
        fi
],[ fim_cv_screen=yes ]
)

dnl ==================

fim_cv_resize_optimizations=yes
AC_ARG_ENABLE(resize-optimizations, AC_HELP_STRING([--disable-resize-optimizations], [Some old (fbi) code is optimized for speedup in fim.]),
[       if test x$enableval = xno; then
		fim_cv_resize_optimizations=no
	else
                fim_cv_resize_optimizations=yes
        fi
],[ fim_cv_resize_optimizations=yes ]
)

dnl ==================

fim_cv_read_stdin_image=yes
AC_ARG_ENABLE(stdin-image-reading, AC_HELP_STRING([--disable-stdin-image-reading], [Fim will read an image from stdin with the -i switch. (experimental)]),
[       if test x$enableval = xno; then
		fim_cv_read_stdin_image=no
	else
                fim_cv_read_stdin_image=yes
        fi
],[ fim_cv_read_stdin_image=yes ]
)

dnl ==================
fim_cv_scan_consolefonts=yes
AC_ARG_ENABLE(scan-consolefonts, AC_HELP_STRING([--enable-scan-consolefonts], [Fim will scan some predefined directory in look for console fonts as a fallback. (experimental)]),
[       if test x$enableval = xyes; then
		fim_cv_scan_consolefonts=yes
	else
		fim_cv_scan_consolefonts=no
        fi
],[ fim_cv_scan_consolefonts=yes ]
)
dnl ==================
fim_cv_seek_magic=yes
AC_ARG_ENABLE(seek-magic, AC_HELP_STRING([--enable-seek-magic], [Fim will be capable to seek on a specified magic string. (experimental)]),
[       if test x$enableval = xyes; then
		fim_cv_seek_magic=yes
	else
                fim_cv_seek_magic=no
        fi
],[ fim_cv_seek_magic=no ]
)
dnl ==================
AC_ARG_ENABLE(read-dirs, AC_HELP_STRING([--enable-read-dirs], [Fim will try scan directories for files.]),
[       if test x$enableval = xno; then
		fim_cv_read_dirs=no
	else
                fim_cv_read_dirs=yes
        fi
],[ fim_cv_read_dirs=no ]
)
dnl ==================
AC_ARG_ENABLE(recursive-dirs, AC_HELP_STRING([--enable-recursive-dirs], [Fim will try to recurse directories (implies --enable-read-dirs )]),
[       if test x$enableval = xno; then
		fim_cv_recursive_dirs=no
	else
                fim_cv_recursive_dirs=yes
                fim_cv_read_dirs=yes
        fi
],[ fim_cv_recursive_dirs=no ]
)
dnl ==================

AH_TEMPLATE([FIM_RECURSIVE_DIRS])
AH_TEMPLATE([FIM_WANTS_SLOW_RESIZE])
AH_TEMPLATE([FIM_READ_DIRS])
AH_TEMPLATE([FIM_BOZ_PATCH])
AH_TEMPLATE([FIM_READ_STDIN])
AH_TEMPLATE([FIM_READ_STDIN_IMAGE])
AH_TEMPLATE([FIM_SMART_COMPLETION])
AH_TEMPLATE([FIM_LINUX_CONSOLEFONTS_DIR_SCAN])
dnl ==================

if test x$fim_cv_resize_optimizations != xyes; then
  AC_DEFINE([FIM_WANTS_SLOW_RESIZE])
fi

dnl ==================

if test x$fim_cv_read_dirs == xyes; then
  AC_DEFINE([FIM_READ_DIRS])
  if test x$fim_cv_recursive_dirs == xyes; then
	  AC_DEFINE([FIM_RECURSIVE_DIRS])
  fi
fi

dnl ==================

if test x$fim_cv_screen != xno; then
# boz patch is on
	AC_DEFINE([FIM_BOZ_PATCH])
fi

dnl ==================

fim_cv_read_stdin=yes

if test x$fim_cv_read_stdin != xno; then
	AC_DEFINE([FIM_READ_STDIN])
fi

dnl ==================

dnl NOTE THAT IN THE CODE FIM_READ_STDIN_IMAGE depends on FIM_READ_STDIN, too
if test x$fim_cv_read_stdin_image = xyes -a x$fim_cv_read_stdin = xyes; then
	AC_DEFINE([FIM_READ_STDIN_IMAGE])
else
	# we revert to no just for the printout
	fim_cv_read_stdin_image=no
fi

dnl ==================

fim_cv_warnings=no
AC_ARG_ENABLE(debug, AC_HELP_STRING([--enable-debug], [Turns on debug options (recommended for debugging)]),
[if test x$enableval = xyes; then
        fim_cv_debug=yes
fi])

dnl ==================

fim_cv_warnings=no
AC_ARG_ENABLE(warnings, AC_HELP_STRING([--enable-warnings], [Turns on compiler warnings (recommended, for debugging)]),
[if test x$enableval = xyes; then
        fim_cv_warnings=yes
fi])

if test x$GCC = xyes ; then
	if test x$fim_cv_debug = xyes; then
        # CXXFLAGS are resetted
        CXXFLAGS="-g"
	fi
fi

if test x$GCC = xyes ; then
	if test x$fim_cv_warnings = xyes ; then
        CXXFLAGS="-Wall -pedantic $CXXFLAGS"
	fi
fi

dnl ==================

dnl ==================

fim_cv_cacalib=no

dnl AC_ARG_ENABLE(cacalib, AC_HELP_STRING([--enable-cacalib], [Turns on cacalib support (STILL INCOMPLETE)]),
dnl 	[if test x$enableval = xyes; then
dnl 	if test x$have_cacalib = xyes; then
dnl 	fim_cv_cacalib=yes
dnl 	else
dnl     AC_MSG_ERROR([Sorry, it doesn't seem like cacalib is installed or detectable.
dnl 	Please run --disable-cacalib or install cacalib in a system-wide directory instead.
dnl 	])
dnl 	fi
dnl 	fi])

dnl ==================

# This code still awaits for integration ...
AC_ARG_WITH(cflags, AC_HELP_STRING([--with-cflags=CXXFLAGS], 
	[Specify a custom CXXFLAGS append value.  e.g.: -pg
	 You can't call this argument multiple times.
	 In case you want completely reset CXXFLAGS use :
	 CXXFLAGS='<all that you want>' ./configure ..
	 Note that Fim is a C++ program, so we set CXXFLAGS=CXXFLAGS.
	]),
        [if test "x$withval" != x; then
                AC_DEFINE_UNQUOTED(CXXFLAGS, "$withval",
                 [custom CXXFLAGS])
		CXXFLAGS="$withval"
		CXXFLAGS="$CXXFLAGS"
		CXXFLAGS="$CXXFLAGS"
                AC_DEFINE_UNQUOTED(CXXFLAGS, "$withval",
                 [custom CXXFLAGS])
		dnl ...else CXXFLAGS="$CXXFLAGS"
        fi],
        [AC_DEFINE_UNQUOTED(EXECSHELL, "/bin/sh")])


dnl ==================

# This code still awaits for integration ...
AC_ARG_WITH(exec-shell, AC_HELP_STRING([--with-exec-shell=SHELL], [Specify alternate shell (ONLY if /bin/sh is broken)]),
        [if test x$withval != xyes; then
                AC_DEFINE_UNQUOTED(EXECSHELL, "$withval",
                 [program to use for shell commands])
         else
                AC_DEFINE_UNQUOTED(EXECSHELL, "/bin/sh")
        fi],
        [AC_DEFINE_UNQUOTED(EXECSHELL, "/bin/sh")])

dnl ==================
AC_ARG_ENABLE(convert, AC_HELP_STRING([--disable-smartautocompletion], [Fim will treat as a proper, unescaped string, nonquoted and nonevaluated strings which happen to match an existing filename.]),
[       if test x$enableval = xno; then
		fim_cv_smartautcompletion=no
	else
                fim_cv_smartautcompletion=yes
        fi
],[ fim_cv_smartautcompletion=yes ]
)
dnl ==================
if test x$fim_cv_smartautcompletion = xyes; then
	AC_DEFINE([FIM_SMART_COMPLETION])
fi
dnl ==================
if test x$fim_cv_scan_consolefonts = xyes; then
	AC_DEFINE([FIM_LINUX_CONSOLEFONTS_DIR_SCAN])
fi
dnl ==================

AC_ARG_ENABLE(convert, AC_HELP_STRING([--disable-convert], [Fim will try to use ImageMagick's convert as a last resort for opening unknown format files.]),
[       if test x$enableval = xno; then
		fim_cv_convert=no
	else
                fim_cv_convert=yes
        fi
],[ fim_cv_convert=yes ]
)



dnl ==================

AC_ARG_ENABLE(xcftopnm, AC_HELP_STRING([--disable-xcftopnm], [Fim will try to use Gimp's xcftopnm for opening XCF format files.]),
[       if test x$enableval = xno; then
		fim_cv_xcftopnm=no
	else
                fim_cv_xcftopnm=yes
        fi
],[ fim_cv_xcftopnm=yes ]
)

dnl ==================

fim_cv_inkscape=yes
AC_ARG_ENABLE(inkscape, AC_HELP_STRING([--disable-inkscape], [Fim will try to use Inkscape for opening SVG format files.]),
[       if test x$enableval = xno; then
		fim_cv_inkscape=no
	else
                fim_cv_inkscape=yes
        fi
],[ fim_cv_inkscape=yes ]
)

dnl ==================

AC_ARG_ENABLE(xfig, AC_HELP_STRING([--disable-xfig], [Fim will try to use xfig (fig2dev) for opening FIG format files.]),
[       if test x$enableval = xno; then
		fim_cv_xfig=no
	else
                fim_cv_xfig=yes
        fi
],[ fim_cv_xfig=yes ]
)

dnl ==================

AC_ARG_ENABLE(dia, AC_HELP_STRING([--disable-dia], [Fim will try to use dia for opening DIA format files.]),
[       if test x$enableval = xno; then
		fim_cv_dia=no
	else
                fim_cv_dia=yes
        fi
],[ fim_cv_dia=yes ]
)

dnl ==================
AC_ARG_WITH(tmpfile, AC_HELP_STRING([--with-tmpfile=FILE], [Specify where to put temporary files created when interacting with external conversion programs (default is "/tmp/__FIM_TEMPORARY_FILE").]),
        [fim_cv_tmpfile=$withval],
        [fim_cv_tmpfile="/tmp/__FIM_TEMPORARY_FILE"])
dnl ==================

AC_ARG_ENABLE(optimizations, AC_HELP_STRING([--enable-optimizations], [Will try to enhance compilation flags (EXPERIMENTAL)]),
[       if test x$enableval = xno; then
		fim_cv_optimizations=no
	else
                fim_cv_optimizations=yes
        fi
],[ fim_cv_optimizations=no ]
)

dnl ==================

AH_TEMPLATE([FIM_TRY_XCFTOPNM])
AH_TEMPLATE([FIM_TRY_CONVERT])
AH_TEMPLATE([FIM_TRY_INKSCAPE])
AH_TEMPLATE([FIM_TRY_XFIG])
AH_TEMPLATE([FIM_TRY_DIA])
AH_TEMPLATE([FIM_USE_ZCAT])

dnl ==================

AC_CHECK_PROG(have_zcat,zcat,yes,no)
        if test x$have_zcat = xyes ; then
		# legacy fbi feature : should not rely on this. FIXME
		AC_MSG_NOTICE([Fim will use 'zcat' to uncompress "gzipped" font files.])
		AC_DEFINE([FIM_USE_ZCAT])
	fi

AC_CHECK_PROG(have_man2html,man2html,yes,no)
        if test x$have_man2html = xyes ; then
      		AC_MSG_NOTICE([The installer will try to invoke 'man2html' to build html versions of the man pages.])
		HTMLDOCS="FIM.html fim.man.html fimgs.man.html"
	fi

AC_CHECK_PROG(have_convert,convert,yes,no)
        if test x$have_convert = xyes -a x$fim_cv_convert = xyes ; then
		true
dnl      		AC_MSG_WARN([Fim will try to invoke 'convert' to handle unknown format files, even if no 'convert' is currently in the program path: please install the Imagemagick package to obviate the problem or re-configure with --disable-convert.])
	else
		fim_cv_convert=no
	fi

AC_CHECK_PROG(have_xcftopnm,xcftopnm,yes,no)
        if test x$have_xcftopnm = xyes -a x$fim_cv_xcftopnm = xyes ; then
		true
dnl      		AC_MSG_WARN([Fim will try to invoke 'xcftopnm' to handle XCF format files, even if no 'xcftopnm' is currently in the program path: please install the Gimp package to obviate the problem or re-configure with --disable-xcftopnm.])
	else
		fim_cv_xcftopnm=no
	fi

AC_CHECK_PROG(have_inkscape,inkscape,yes,no)
        if test x$have_inkscape = xyes -a x$fim_cv_inkscape = xyes ; then
		true
dnl      		AC_MSG_WARN([Fim will try to invoke 'inkscape' to handle SVG format files, even if no 'inkscape' is currently in the program path: please install the Inkscape package to obviate the problem or re-configure with --disable-inkscape.])
	else
		fim_cv_inkscape=no
	fi

AC_CHECK_PROG(have_xfig,fig2dev,yes,no)
        if test x$have_xfig = xyes -a x$fim_cv_xfig = xyes ; then
		true
dnl      		AC_MSG_WARN([Fim will try to invoke 'xfig' to handle FIG format files, even if no 'fig2dev' is currently in the program path: please install the Xfig package to obviate the problem or re-configure with --disable-xfig.])
	else
		fim_cv_xfig=no
	fi

AC_CHECK_PROG(have_dia,dia,yes,no)
        if test x$have_dia = xyes -a x$fim_cv_dia = xyes ; then
		true
dnl      		AC_MSG_WARN([Fim will try to invoke 'dia' to handle DIA format files, even if no 'dia' is currently in the program path: please install the Dia package to obviate the problem or re-configure with --disable-dia.])
	else
		fim_cv_dia=no
	fi

dnl ==================

        if test x$fim_cv_optimizations = xyes ; then
		uname=`uname`
        	if test x$uname = xLinux ; then
dnl			AC_MSG_NOTICE([we run on linux! Good.])
	     		if grep "^model name.*Pentium(R) 4" /proc/cpuinfo > /dev/null; then
				CXXFLAGS="$CXXFLAGS -march=native"
		     	fi

		else
			AC_MSG_NOTICE([uhm. try to specify your own optimizations..])

		fi
	fi

dnl ==================

        if test x$fim_cv_convert = xyes ; then
		AC_DEFINE([FIM_TRY_CONVERT])
	fi

        if test x$fim_cv_xcftopnm = xyes ; then
		AC_DEFINE([FIM_TRY_XCFTOPNM])
	fi

        if test x$fim_cv_inkscape = xyes ; then
		AC_DEFINE([FIM_TRY_INKSCAPE])
	fi

        if test x$have_xfig = xyes ; then
		AC_DEFINE([FIM_TRY_XFIG])
	fi

        if test x$have_dia = xyes ; then
		AC_DEFINE([FIM_TRY_DIA])
	fi

dnl ==================

    AH_TEMPLATE([FIM_WITH_AALIB])
            if test x$fim_cv_aalib = xyes ; then
    		AC_DEFINE([FIM_WITH_AALIB])
    	fi

dnl ==================

    AH_TEMPLATE([FIM_WITH_CACALIB])
            if test x$fim_cv_cacalib = xyes ; then
    		AC_DEFINE([FIM_WITH_CACALIB])
    		LIBS="$LIBS -lcaca"
    	fi

dnl ==================
    AH_TEMPLATE([FIM_WANT_SEEK_MAGIC])
            if test x$fim_cv_seek_magic = xyes ; then
    		AC_DEFINE([FIM_WANT_SEEK_MAGIC])
    	fi

dnl ==================

dnl uhmmmmmm : doesn't seem to work
dnl AC_ARG_ENABLE(verbose_make,[Enables verbose macros passing to the compiler (for a funnier compilation process.)], 
dnl         if test x$enableval = xyes; then
dnl		AC_SUBST(DEFS)
dnl	else
dnl		AC_CONFIG_HEADERS([config.h])
dnl        fi,[])
AC_CONFIG_HEADERS([config.h])


AH_TEMPLATE([FIM_RANDOM])
AC_DEFINE([FIM_RANDOM])
AH_TEMPLATE([FIM_DEFAULT_CONFIG])
AC_DEFINE([FIM_DEFAULT_CONFIG])
AH_TEMPLATE([FIM_NOFB])
AC_DEFINE([FIM_NOFB])
AH_TEMPLATE([FIM_RECORDING])
AC_DEFINE([FIM_RECORDING])
AH_TEMPLATE([FIM_SWITCH_FIXUP])
AC_DEFINE([FIM_SWITCH_FIXUP])
AH_TEMPLATE([FIM_CHECK_DUPLICATES])
AC_DEFINE([FIM_CHECK_DUPLICATES])
AH_TEMPLATE([FIM_CHECK_FILE_EXISTENCE])
AC_DEFINE([FIM_CHECK_FILE_EXISTENCE])
AH_TEMPLATE([FIM_REMOVE_FAILED])
AC_DEFINE([FIM_REMOVE_FAILED])
AH_TEMPLATE([FIM_AUTOSKIP_FAILED])
AC_DEFINE([FIM_AUTOSKIP_FAILED])
AH_TEMPLATE([FIM_COMMAND_AUTOCOMPLETION])
AC_DEFINE([FIM_COMMAND_AUTOCOMPLETION])
AH_TEMPLATE([FIM_NAMESPACES])
AC_DEFINE([FIM_NAMESPACES])
AH_TEMPLATE([FIM_CACHE_DEBUG])
dnl AC_DEFINE([FIM_CACHE_DEBUG])
AH_TEMPLATE([FIM_BUGGED_CACHE])
dnl AC_DEFINE([FIM_BUGGED_CACHE])
AH_TEMPLATE([FIM_VERSION])
AC_DEFINE_UNQUOTED([FIM_VERSION],"${VERSION}")
AH_TEMPLATE([X_DISPLAY_MISSING])
AC_DEFINE([X_DISPLAY_MISSING])
AH_TEMPLATE([FIM_TMP_FILENAME])
AC_DEFINE_UNQUOTED([FIM_TMP_FILENAME],"$fim_cv_tmpfile")
AH_TEMPLATE([FIM_ITERATED_COMMANDS])
AC_DEFINE([FIM_ITERATED_COMMANDS])
AH_TEMPLATE([FIM_SKIP_KNOWN_FILETYPES])
AC_DEFINE([FIM_SKIP_KNOWN_FILETYPES])
AH_TEMPLATE([FIM_EXPERIMENTAL_ROTATION])
AC_DEFINE([FIM_EXPERIMENTAL_ROTATION])
AH_TEMPLATE([FIM_KEEP_BROKEN_CONSOLE])
dnl AC_DEFINE([FIM_KEEP_BROKEN_CONSOLE])
AH_TEMPLATE([FIM_PIPE_IMAGE_READ])
dnl AC_DEFINE([FIM_PIPE_IMAGE_READ])
AH_TEMPLATE([FIM_USE_DESIGNATED_INITIALIZERS])
dnl AC_DEFINE([FIM_USE_DESIGNATED_INITIALIZERS])
dnl AH_TEMPLATE([FIM_IS_SLOWER_THAN_FBI])
dnl AC_DEFINE([FIM_IS_SLOWER_THAN_FBI])
dnl AH_TEMPLATE([FIM_WITH_LIBSDL])
dnl AC_DEFINE([FIM_WITH_LIBSDL])
AH_TEMPLATE([FIM_AUTHOR])
AC_DEFINE([FIM_AUTHOR],["Michele Martone <dezperado _CUT_ autistici _CUT_ org>"])
dnl ###############################################################################
dnl this is experimental stuff
dnl ###############################################################################
dnl AH_TEMPLATE([HAVE_MATRIX_MARKET_DECODER])
dnl AC_DEFINE([HAVE_MATRIX_MARKET_DECODER])
dnl CXXFLAGS="$CXXFLAGS -I${HOME}/src/libmmvbr/"
dnl LIBS="$LIBS -L${HOME}/src/libmmvbr/ -lmmvbr"
dnl ###############################################################################


dnl ==================

AC_SUBST(LIBIMAP)
AC_SUBST(LIBIMAPDEPS)

AC_SUBST(LIBS)
AC_SUBST(FIM_DEFS)

AC_SUBST(FIM_LIB_OBJECTS)
AC_SUBST(FIM_SVN_REPOSITORY,[http://code.autistici.org/svn/fim/])

dnl we want these symbols defined in our code
AC_SUBST(CXXFLAGS)

#AC_CONFIG_FILES([Makefile],[src/Makefile])

AC_OUTPUT(Makefile src/Makefile doc/Makefile distros/Makefile scripts/Makefile
	 scripts/maintenance/cron-build.sh
	 scripts/maintenance/remote-build.sh
	 scripts/maintenance/cron-svndump.pl
	 scripts/maintenance/cron-rsync.sh
	)
#AC_OUTPUT

AC_MSG_NOTICE([
	Fim v${VERSION} has been configured as follows:

	default minimal key mapping         : ${have_default_mapping:-no}
	readline library support            : ${fim_want_readline}
	default minimal key bindings        : ${have_default_bindings:-no}
	default configuration file builtin  : ${have_default_config:-no}
        compile time warnings               : ${fim_cv_warnings:-no} (additional CXXFLAGS: ${CXXFLAGS} )
        resize optimizations                : ${fim_cv_resize_optimizations:-no}
        system,popen calls wrappers enabled : ${fim_system_calls:-yes}
	will read directories               : ${fim_cv_read_dirs}
	will recurse directories            : ${fim_cv_recursive_dirs}
	png  file support                   : ${fim_handle_png:-no}
	jpeg file support                   : ${fim_handle_jpeg:-no}
	tiff file support                   : ${fim_handle_tiff:-no}
	gif  file support                   : ${fim_handle_gif:-no}
	djvu file support                   : ${fim_handle_djvu:-no}
	pdf  file support (libpoppler)      : ${fim_handle_pdf:-no}
	ps   file support (libspectre)      : ${fim_handle_ps:-no}
dnl	dvi  file support (dvilib29)        : ${fim_handle_dvi:-no}
	patch for running under screen      : ${fim_cv_screen:-no}
	~/.fimrc file loading on startup    : ${have_fimrc:-no}
	autocommands enabled                : ${have_autocommands:-no}
	internal windowing system           : ${have_windows:-no}
	will read image files from stdin(-i): ${fim_cv_read_stdin_image:-no}
	GNU regexp's (default is Posix)     : ${fim_cv_regex:-no}
	use 'xcftopnm' for XCF files        : ${fim_cv_xcftopnm}
	use 'convert' for unknown files     : ${fim_cv_convert}
	use 'inkscape' for SVG files        : ${fim_cv_inkscape}
	use 'fig2dev' for FIG files         : ${fim_cv_xfig}
	ascii art (aalib) support           : ${fim_cv_aalib}
	X (sdllib) experimental support     : ${fim_handle_sdl}
        framebuffer support                 : ${fim_handle_framebuffer:-yes}
        seek magic signatures               : ${fim_cv_seek_magic:-no}
dnl	coloured ascii art (cacalib) support: ${fim_cv_cacalib}


	If you are not satisfied with this configuration, please run :

	  ./configure --help=short

	for a complete list of configure options specific to Fim.

	If you are satisfied, run 'make' to build Fim and its documentation.
	Otherwise, 'make fim' to build only Fim.

	To install the program and its documentation, run 'make install' if you are root,
	or run 'su -c '"make install"' if you are not root.

	p.s.: documentation is found in the doc/ directory !
	Please see doc/FIM.TXT, doc/FIM.html, doc/fim.man.

	Enjoy! Please see the BUGS file for bug reports, and the documentation anyway!
])


