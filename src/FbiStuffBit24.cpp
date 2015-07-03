/* $LastChangedDate: 2014-11-17 19:22:17 +0100 (Mon, 17 Nov 2014) $ */
/*
 FbiStuffBit24.cpp : fbi functions for reading ELF files as they were raw 24 bit per pixel pixelmaps

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

struct bit24_state {
    FILE *fp;
    uint32 w;
    uint32 h;
    uint32 flen;
};

static void*
bit24_init(FILE *fp, const fim_char_t *filename, unsigned int page,
	 struct ida_image_info *i, int thumbnail)
{
    struct bit24_state *h=NULL;
    long ftellr;
    fim_int prw=cc.getIntVariable(FIM_VID_PREFERRED_RENDERING_WIDTH);
    prw=prw<1?FIM_BITRENDERING_DEF_WIDTH:prw;
    
    h = (struct bit24_state *)fim_calloc(1,sizeof(*h));
    if(!h)
	    goto oops;
    h->fp = fp;
    if(fseek(fp,0,SEEK_END)!=0)
	    goto oops;
    ftellr=ftell(fp);
    if((ftellr)==-1)
	    goto oops;
    i->width  = h->w = prw;
    i->height = h->h = (h->flen+(h->w*3-1)) / ( h->w*3 ); // should pad
    return h;
 oops:
    if(h)fim_free(h);
    return NULL;
}

static void
bit24_read(fim_byte_t *dst, unsigned int line, void *data)
{
    struct bit24_state *h = (struct bit24_state *) data;
    unsigned int ll,y,x = 0;
    
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
//	if(y==h->h-1) fim_bzero(dst,h->w*3-3*x);
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
    /*name:*/  "Bit24",
    /*init:*/  bit24_init,
    /*read:*/  bit24_read,
    /*done:*/  bit24_done,
};

static void __init init_rd(void)
{
    fim_load_register(&bit24_loader);
}


}
#endif /* FIM_WANT_RAW_BITS_RENDERING */
