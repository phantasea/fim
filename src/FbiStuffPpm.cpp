/* $LastChangedDate: 2014-11-17 19:22:17 +0100 (Mon, 17 Nov 2014) $ */
/*
 FbiStuffPpm.cpp : fbi functions for PPM files, modified for fim

 (c) 2008-2014 Michele Martone
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



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

//#include "loader.h"
#include "fim.h"
#include "FbiStuffLoader.h"
#ifdef USE_X11
# include "viewer.h"
#endif /* USE_X11 */

/* ---------------------------------------------------------------------- */
/* load                                                                   */

namespace fim
{

struct ppm_state {
    FILE          *infile;
    int           width,height;
    fim_byte_t *row;
};

static void*
pnm_init(FILE *fp, const fim_char_t *filename, unsigned int page,
	 struct ida_image_info *i, int thumbnail)
{
    struct ppm_state *h;
    fim_char_t line[FIM_FBI_PPM_LINEBUFSIZE],*fr;

    h = (struct ppm_state*) fim_calloc(1,sizeof(*h));
    if(!h)return NULL;

    h->infile = fp;
    fr=fgets(line,sizeof(line),fp); /* Px */
    if(!fr)goto oops;
    fr=fgets(line,sizeof(line),fp); /* width height */
    if(!fr)goto oops;
    while ('#' == line[0])
    {
	fr=fgets(line,sizeof(line),fp); /* skip comments */
        if(!fr)goto oops;
    }
    sscanf(line,"%d %d",&h->width,&h->height);
    fr=fgets(line,sizeof(line),fp); /* ??? */
    if(!fr)goto oops;
    if (0 == h->width || 0 == h->height)
	goto oops;
    i->width  = h->width;
    i->height = h->height;
    i->npages = 1;
    h->row = (fim_byte_t*)fim_malloc(h->width*3);
    if(!h->row)goto oops;

    return h;

 oops:
    fim_fclose(fp);
    if(h->row)fim_free(h->row);
    if(h)fim_free(h);
    return NULL;
}

static void
ppm_read(fim_byte_t *dst, unsigned int line, void *data)
{
    struct ppm_state *h = (struct ppm_state *) data;
    int fr;
    fr=fim_fread(dst,h->width,3,h->infile);
    if(fr){/* FIXME : there should be error handling */}
}

static void
pgm_read(fim_byte_t *dst, unsigned int line, void *data)
{
    struct ppm_state *h = (struct ppm_state *) data;
    fim_byte_t *src;
    int x,fr;

    fr=fim_fread(h->row,h->width,1,h->infile);
    if(!fr){/* FIXME : there should be error handling */ return ; }
    src = h->row;
    for (x = 0; x < h->width; x++) {
	dst[0] = src[0];
	dst[1] = src[0];
	dst[2] = src[0];
	dst += 3;
	src += 1;
    }
}

static void
pnm_done(void *data)
{
    struct ppm_state *h = (struct ppm_state *) data;

    fim_fclose(h->infile);
    fim_free(h->row);
    fim_free(h);
}

struct ida_loader ppm_loader = {
    /*magic:*/ "P6",
    /*moff:*/  0,
    /*mlen:*/  2,
    /*name:*/  "ppm",
    /*init:*/  pnm_init,
    /*read:*/  ppm_read,
    /*done:*/  pnm_done,
};

struct ida_loader pgm_loader = {
    /*magic:*/ "P5",
    /*moff:*/  0,
    /*mlen:*/  2,
    /*name:*/  "pgm",
    /*init:*/  pnm_init,
    /*read:*/  pgm_read,
    /*done:*/  pnm_done,
};

static void __init init_rd(void)
{
    fim_load_register(&ppm_loader);
    fim_load_register(&pgm_loader);
}

#ifdef USE_X11
/* ---------------------------------------------------------------------- */
/* save                                                                   */

static int
ppm_write(FILE *fp, struct ida_image *img)
{
    fprintf(fp,"P6\n"
	    "# written by ida " VERSION "\n"
	    "# http://bytesex.org/ida/\n"
	    "%d %d\n255\n",
            img->i.width,img->i.height);
    fwrite(img->data, img->i.height, 3*img->i.width, fp);
    return 0;
}

static struct ida_writer ppm_writer = {
    /*label:*/  "PPM",
    /*ext:*/    { "ppm", NULL},
    /*write:*/  ppm_write,
};

static void __init init_wr(void)
{
    fim_write_register(&ppm_writer);
}
#endif /* USE_X11 */

}

