/* $Id: FbiStuffPs.cpp 271 2009-12-13 00:03:48Z dezperado $ */
/*
 FbiStuffPs.cpp : fim functions for decoding PS files

 (c) 2008-2009 Michele Martone
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
 * this code should be fairly correct, although unfinished
 * */

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "FbiStuff.h"
#include "FbiStuffLoader.h"

#ifdef HAVE_LIBSPECTRE

extern "C"
{
// we require C linkage for these symbols
#include <libspectre/spectre.h>
}

/*								*/

namespace fim
{

/* ---------------------------------------------------------------------- */
/* load                                                                   */

struct ps_state_t {
	int row_stride;    /* physical row width in output buffer */
	unsigned char * first_row_dst;
	int w,h;
	SpectreDocument * sd;
	SpectrePage * sp;
	SpectreRenderContext * src;
	SpectreStatus ss;
};


/* ---------------------------------------------------------------------- */

static void*
ps_init(FILE *fp, char *filename, unsigned int page,
	  struct ida_image_info *i, int thumbnail)
{
	double scale = 1.5;
	double rcscale = scale;

	struct ps_state_t * ds=NULL;

	if(filename==FIM_STDIN_IMAGE_NAME){std::cerr<<"sorry, stdin multipage file reading is not supported\n";return NULL;}	/* a drivers's problem */ 
	if(fp) fclose(fp);

	ds = (struct ps_state_t*)fim_calloc(sizeof(struct ps_state_t),1);

	if(!ds)
		return NULL;

    	ds->first_row_dst = NULL;
	ds->sd = NULL;
	ds->sp = NULL;
	ds->src = NULL;
	ds->ss = SPECTRE_STATUS_SUCCESS;

	ds->sd = spectre_document_new();
	if(!ds->sd)
		goto err;

	spectre_document_load(ds->sd,filename);

	ds->ss = spectre_document_status(ds->sd);
	if(ds->ss != SPECTRE_STATUS_SUCCESS)
		goto err;

	ds->src = spectre_render_context_new();
	if(!ds->src)
		goto err;

	i->dpi    = 1.0*72; /* FIXME */

	spectre_render_context_set_scale(ds->src,scale,scale);
	spectre_render_context_set_rotation(ds->src,0);
	spectre_render_context_set_resolution(ds->src,i->dpi,i->dpi);

	i->npages = spectre_document_get_n_pages(ds->sd);
	if(page>=i->npages || page<0)goto err;

	ds->sp = spectre_document_get_page(ds->sd,page);/* pages, from 0 */
	if(!ds->sp)
		goto err;
	ds->ss = spectre_page_status(ds->sp);
	if(ds->ss != SPECTRE_STATUS_SUCCESS)
		goto err;

	spectre_page_get_size(ds->sp, (int*)(&i->width), (int*)(&i->height));
//	spectre_render_context_get_page_size(ds->src, (int*)(&i->width), (int*)(&i->height));
//	spectre_document_get_page_size(ds->sd, (int*)(&i->width), (int*)(&i->height));

	i->width  *= scale;
	i->height *= scale;


	spectre_render_context_set_page_size(ds->src, (int)(i->width), (int)(i->height));
	spectre_render_context_set_scale(ds->src,rcscale,rcscale);

	if(i->width<1 || i->height<1)
		goto err;

	ds->w=i->width;
	ds->h=i->height;

	return ds;

err:

	if(ds->sd )spectre_document_free(ds->sd);
	if(ds->sp )spectre_page_free(ds->sp);
	if(ds->src)spectre_render_context_free(ds->src);
	if(ds)fim_free(ds);
	return NULL;
}

static void
ps_read(unsigned char *dst, unsigned int line, void *data)
{
    	struct ps_state_t *ds = (struct ps_state_t*)data;
	if(!ds)return;

    	if(ds->first_row_dst == NULL)
    		ds->first_row_dst = dst;
	else return;

	unsigned char       *page_data=NULL;

	//render in RGB32 format
	//spectre_page_render(ds->sp,ds->src,&page_data,&ds->row_stride);
	spectre_page_render_slice(ds->sp,ds->src,0,0,ds->w,ds->h,&page_data,&ds->row_stride);

	ds->ss = spectre_page_status(ds->sp);
	if(ds->ss != SPECTRE_STATUS_SUCCESS)
		return;

	int i,j;
	for(i=0;i<ds->h;++i)
		for(j=0;j<ds->w;++j)
		{
			dst[ds->w*i*3+3*j+0]=page_data[ds->row_stride*i+4*j+0];
			dst[ds->w*i*3+3*j+1]=page_data[ds->row_stride*i+4*j+1];
			dst[ds->w*i*3+3*j+2]=page_data[ds->row_stride*i+4*j+2];
		}
	fim_free(page_data);
}

static void
ps_done(void *data)
{
    	struct ps_state_t *ds = (struct ps_state_t*)data;
	if(!ds) return;

	if(ds->sd )spectre_document_free(ds->sd);
	if(ds->sp )spectre_page_free(ds->sp);
	if(ds->src)spectre_render_context_free(ds->src);

	fim_free(ds);
}

/*
0000000: 2521 5053 2d41 646f 6265 2d33 2e30 0a25  %!PS-Adobe-3.0.%
*/
static struct ida_loader ps_loader = {
    /*magic:*/ "%!PS-",// FI/*XME :*/ are sure this is enough ?
    /*moff:*/  0,
    /*mlen:*/  5,
    /*name:*/  "libspectre",
    /*init:*/  ps_init,
    /*read:*/  ps_read,
    /*done:*/  ps_done,
};

static void __init init_rd(void)
{
    load_register(&ps_loader);
}

}
#endif // ifdef HAVE_LIBSPECTRE
