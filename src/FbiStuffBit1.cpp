/* $LastChangedDate: 2014-11-17 19:22:17 +0100 (Mon, 17 Nov 2014) $ */
/*
 FbiStuffBit1.cpp : fbi functions for reading ELF files as they were raw 1 bit per pixel pixelmaps

 (c) 2007-2014 Michele Martone
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

#if FIM_WANT_RAW_BITS_RENDERING

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef HAVE_ENDIAN_H
# include <endian.h>
#endif /* HAVE_ENDIAN_H */

namespace fim
{

extern CommandConsole cc;

/* ---------------------------------------------------------------------- */

typedef unsigned int   uint32;
typedef unsigned short uint16;

/* ---------------------------------------------------------------------- */
/* load                                                                   */

struct bit1_state {
    FILE *fp;
    uint32 w;
    uint32 h;
    uint32 flen;
};

static void*
bit1_init(FILE *fp, const fim_char_t *filename, unsigned int page,
	 struct ida_image_info *i, int thumbnail)
{
    struct bit1_state *h=NULL;
    fim_int prw=cc.getIntVariable(FIM_VID_PREFERRED_RENDERING_WIDTH);
    prw=prw<1?FIM_BITRENDERING_DEF_WIDTH:prw;

    h = (struct bit1_state *)fim_calloc(1,sizeof(*h));
    if(!h)goto oops;
    h->fp = fp;
    if(fseek(fp,0,SEEK_END)!=0) goto oops;
    if((h->flen=ftell(fp))==-1)goto oops;
    i->width  = h->w = prw;	// must be congruent to 8
    i->height = h->h = (8*h->flen + h->w-1) / ( i->width ); // should pad
    return h;
 oops:
    if(h)fim_free(h);
    return NULL;
}

static void
bit1_read(fim_byte_t *dst, unsigned int line, void *data)
{
    struct bit1_state *h = (struct bit1_state *) data;
    unsigned int ll,y,x = 0;
    
	y  = line ;
	if(y==h->h-1)
	{
		ll = h->flen - h->w * (h->h-1) / 8;
	}
	else
		ll = h->w / 8;

	fseek(h->fp,0 + y * ll,SEEK_SET);

	for (x = 0; x < h->w; x+=8)
	{
		fim_byte_t c = fgetc(h->fp);
		*(dst++) = (c & 1 << 0)?255:0;
		*(dst++) = (c & 1 << 0)?255:0;
		*(dst++) = (c & 1 << 0)?255:0;
		*(dst++) = (c & 1 << 1)?255:0;
		*(dst++) = (c & 1 << 1)?255:0;
		*(dst++) = (c & 1 << 1)?255:0;
		*(dst++) = (c & 1 << 2)?255:0;
		*(dst++) = (c & 1 << 2)?255:0;
		*(dst++) = (c & 1 << 2)?255:0;
		*(dst++) = (c & 1 << 3)?255:0;
		*(dst++) = (c & 1 << 3)?255:0;
		*(dst++) = (c & 1 << 3)?255:0;
		*(dst++) = (c & 1 << 4)?255:0;
		*(dst++) = (c & 1 << 4)?255:0;
		*(dst++) = (c & 1 << 4)?255:0;
		*(dst++) = (c & 1 << 5)?255:0;
		*(dst++) = (c & 1 << 5)?255:0;
		*(dst++) = (c & 1 << 5)?255:0;
		*(dst++) = (c & 1 << 6)?255:0;
		*(dst++) = (c & 1 << 6)?255:0;
		*(dst++) = (c & 1 << 6)?255:0;
		*(dst++) = (c & 1 << 7)?255:0;
		*(dst++) = (c & 1 << 7)?255:0;
		*(dst++) = (c & 1 << 7)?255:0;
	}
//	if(y==h->h-1) fim_bzero(dst,h->w*8-8*x);
}

static void
bit1_done(void *data)
{
    struct bit1_state *h = (struct bit1_state *) data;

    fclose(h->fp);
    fim_free(h);
}

struct ida_loader bit1_loader = {
/*
 * 0000000: 7f45 4c46 0101 0100 0000 0000 0000 0000  .ELF............
 */
    /*magic:*/ "ELF",
    /*moff:*/  1,
    /*mlen:*/  3,
    /*name:*/  "Bit1",
    /*init:*/  bit1_init,
    /*read:*/  bit1_read,
    /*done:*/  bit1_done,
};

static void __init init_rd(void)
{
    fim_load_register(&bit1_loader);
}


}
#endif /* FIM_WANT_RAW_BITS_RENDERING */
