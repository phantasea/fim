/* $Id: FbiStuffBit24.cpp 271 2009-12-13 00:03:48Z dezperado $ */
/*
 FbiStuffBit24.cpp : fbi functions for reading ELF files as they were raw 24 bit per pixel pixelmaps

 (c) 2007-2009 Michele Martone
 based on code (c) 1998-2006 Gerd Knorr <kraxel@bytesex.org>

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
 * this is basically toy code, so enjoy!
 * */


#include "fim.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef HAVE_ENDIAN_H
# include <endian.h>
#endif

namespace fim
{


/* ---------------------------------------------------------------------- */

typedef unsigned int   uint32;
typedef unsigned short uint16;

/* ---------------------------------------------------------------------- */
/* load                                                                   */

struct bit24_state {
    FILE *fp;
    uint32 w;
    uint32 h;
    uint32 flen;
};

static void*
bit24_init(FILE *fp, char *filename, unsigned int page,
	 struct ida_image_info *i, int thumbnail)
{
    struct bit24_state *h;
    
    h = (struct bit24_state *)fim_calloc(sizeof(*h),1);
    if(!h)goto oops;
    memset(h,0,sizeof(*h));
    h->fp = fp;
    if(fseek(fp,0,SEEK_END)!=0) goto oops;
    if((h->flen=ftell(fp))==-1)goto oops;
    i->width  = h->w = 1024;
    i->height = h->h = (h->flen+(h->w*3-1)) / ( h->w*3 ); // should pad
    return h;
 oops:
    if(h)fim_free(h);
    return NULL;
}

static void
bit24_read(unsigned char *dst, unsigned int line, void *data)
{
    struct bit24_state *h = (struct bit24_state *) data;
    unsigned int ll,y,x,pixel,byte = 0;
    
	y  = line ;
	if(y==h->h-1)
		ll = h->flen - h->w*3 * (h->h-1);
	else
	{
		ll = h->w * 3;
	}

	fseek(h->fp,0 + y * ll,SEEK_SET);

	for (x = 0; x < h->w; x++)
	{
		*(dst++) = fgetc(h->fp);
		*(dst++) = fgetc(h->fp);
		*(dst++) = fgetc(h->fp);
	}
//	if(y==h->h-1) bzero(dst,h->w*3-3*x);
}

static void
bit24_done(void *data)
{
    struct bit24_state *h = (struct bit24_state *) data;

    fclose(h->fp);
    fim_free(h);
}

struct ida_loader bit24_loader = {
/*
 * 0000000: 7f45 4c46 0101 0100 0000 0000 0000 0000  .ELF............
 */
    /*magic:*/ "ELF",
    /*moff:*/  1,
    /*mlen:*/  3,
    /*name:*/  "bmp",
    /*init:*/  bit24_init,
    /*read:*/  bit24_read,
    /*done:*/  bit24_done,
};

static void __init init_rd(void)
{
    load_register(&bit24_loader);
}


}

