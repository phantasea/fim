/* $Id: FontServer.h 206 2009-02-13 00:52:29Z dezperado $ */
/*
 FontServer.h : Font Server code from fbi, adapted for fim.

 (c) 2008-2009 Michele Martone
 (c) 1998-2006 Gerd Knorr <kraxel@bytesex.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/
/*
 * This file comes from fbi, and will undergo severe reorganization.
 * */


#ifndef FIM_FONT_SERVER_H
#define FIM_FONT_SERVER_H

#include "fim.h"

# include <errno.h>
# include <unistd.h> //for NULL
# include <stdio.h> //for stderr..
# include <stdlib.h> //for exit..
# include <string.h> //for strlen..
namespace fim
{


#ifndef X_DISPLAY_MISSING
# include <FSlib.h>

struct fs_font {
    Font               font;
    FSXFontInfoHeader  fontHeader;
    FSPropInfo         propInfo;
    FSPropOffset       *propOffsets;
    unsigned char      *propData;

    FSXCharInfo        *extents;
    FSOffset           *offsets;
    unsigned char      *glyphs;

    int                maxenc,width,height;
    FSXCharInfo        **eindex;
    unsigned char      **gindex;
};

#else

typedef struct _FSXCharInfo {
    short       left;
    short       right;
    short       width;
    short       ascent;
    short       descent;
    /*unsigned short      attributes;*/
} FSXCharInfo;

typedef struct _FSXFontInfoHeader {
    /*int         flags;
    //FSRange     char_range;
    //unsigned    draw_direction;
    //FSChar2b    default_char;
    */
    FSXCharInfo min_bounds;
    FSXCharInfo max_bounds;
    short       font_ascent;
    short       font_descent;
} FSXFontInfoHeader;

struct fs_font {
    FSXFontInfoHeader  fontHeader;
    /*unsigned char      *propData;*/
    FSXCharInfo        *extents;
    unsigned char      *glyphs;
    int                maxenc,width,height;
    FSXCharInfo        **eindex;
    unsigned char      **gindex;
};

#endif





static const unsigned fs_masktab[] = {
    (1 << 7), (1 << 6), (1 << 5), (1 << 4),
    (1 << 3), (1 << 2), (1 << 1), (1 << 0),
};




class FontServer
{


public:
	FontServer( );

#if 1
/* 20080507 unused, as default_font ? */
static void fb_text_init1(const char *font, struct fs_font **_f);

static struct fs_font* fs_consolefont(const char **filename);
#endif



};

}

#endif

