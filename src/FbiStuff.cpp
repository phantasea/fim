/* $Id: FbiStuff.cpp 271 2009-12-13 00:03:48Z dezperado $ */
/*
 FbiStuff.cpp : Misc fbi functions, modified for fim

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


#include "fim.h"
#include "common.h"

#include <stdio.h>	/* fdopen, tmpfile */
#include <unistd.h>	/* execlp (popen is dangerous) */
#include <stdlib.h>	/* mkstemp */
#include <math.h>
#include <string.h>
#include <stdarg.h>	/* va_start, va_end, ... */

namespace fim
{

extern CommandConsole cc;


// filter.c

/* ----------------------------------------------------------------------- */

static void
op_grayscale(struct ida_image *src, struct ida_rect *rect,
	     unsigned char *dst, int line, void *data)
{
    unsigned char *scanline;
    int i,g;

    scanline = src->data + line * src->i.width * 3;
    memcpy(dst,scanline,src->i.width * 3);
    if (line < rect->y1 || line >= rect->y2)
	return;
    dst      += 3*rect->x1;
    scanline += 3*rect->x1;
    for (i = rect->x1; i < rect->x2; i++) {
	g = (scanline[0]*30 + scanline[1]*59+scanline[2]*11)/100;
	dst[0] = g;
	dst[1] = g;
	dst[2] = g;
	scanline += 3;
	dst += 3;
    }
}

/* ----------------------------------------------------------------------- */

struct op_3x3_handle {
    struct op_3x3_parm filter;
    int *linebuf;
};

static void*
op_3x3_init(struct ida_image *src, struct ida_rect *rect,
	    struct ida_image_info *i, void *parm)
{
    struct op_3x3_parm *args = (struct op_3x3_parm*)parm;
    struct op_3x3_handle *h;

    h = (struct op_3x3_handle*)fim_malloc(sizeof(*h));
    if(!h)goto oops;
    memcpy(&h->filter,args,sizeof(*args));
    h->linebuf = (int*)fim_malloc(sizeof(int)*3*(src->i.width));
    if(!h->linebuf)goto oops;

    *i = src->i;
    return h;
oops:
    if(h && h->linebuf)fim_free(h->linebuf);
    if(h)fim_free(h);
    return NULL;
}

static int inline
op_3x3_calc_pixel(struct op_3x3_parm *p, unsigned char *s1,
		  unsigned char *s2, unsigned char *s3)
{
    int val = 0;

    val += p->f1[0] * s1[0];
    val += p->f1[1] * s1[3];
    val += p->f1[2] * s1[6];
    val += p->f2[0] * s2[0];
    val += p->f2[1] * s2[3];
    val += p->f2[2] * s2[6];
    val += p->f3[0] * s3[0];
    val += p->f3[1] * s3[3];
    val += p->f3[2] * s3[6];
    if (p->mul && p->div)
	val = val * p->mul / p->div;
    val += p->add;
    return val;
}

static void
op_3x3_calc_line(struct ida_image *src, struct ida_rect *rect,
		 int *dst, unsigned int line, struct op_3x3_parm *p)
{
    unsigned char b1[9],b2[9],b3[9];
    unsigned char *s1,*s2,*s3;
    unsigned int i,left,right;

    s1 = src->data + (line-1) * src->i.width * 3;
    s2 = src->data +  line    * src->i.width * 3;
    s3 = src->data + (line+1) * src->i.width * 3;
    if (0 == line)
	s1 = src->data + line * src->i.width * 3;
    if (src->i.height-1 == line)
	s3 = src->data + line * src->i.width * 3;

    left  = rect->x1;
    right = rect->x2;
    if (0 == left) {
	/* left border special case: dup first col */
	memcpy(b1,s1,3);
	memcpy(b2,s2,3);
	memcpy(b3,s3,3);
	memcpy(b1+3,s1,6);
	memcpy(b2+3,s2,6);
	memcpy(b3+3,s3,6);
	dst[0] = op_3x3_calc_pixel(p,b1,b2,b3);
	dst[1] = op_3x3_calc_pixel(p,b1+1,b2+1,b3+1);
	dst[2] = op_3x3_calc_pixel(p,b1+2,b2+2,b3+2);
	left++;
    }
    if (src->i.width == right) {
	/* right border */
	memcpy(b1,s1+src->i.width*3-6,6);
	memcpy(b2,s2+src->i.width*3-6,6);
	memcpy(b3,s3+src->i.width*3-6,6);
	memcpy(b1+3,s1+src->i.width*3-3,3);
	memcpy(b2+3,s2+src->i.width*3-3,3);
	memcpy(b3+3,s3+src->i.width*3-3,3);
	dst[src->i.width*3-3] = op_3x3_calc_pixel(p,b1,b2,b3);
	dst[src->i.width*3-2] = op_3x3_calc_pixel(p,b1+1,b2+1,b3+1);
	dst[src->i.width*3-1] = op_3x3_calc_pixel(p,b1+2,b2+2,b3+2);
	right--;
    }
    
    dst += 3*left;
    s1  += 3*(left-1);
    s2  += 3*(left-1);
    s3  += 3*(left-1);
    for (i = left; i < right; i++) {
	dst[0] = op_3x3_calc_pixel(p,s1++,s2++,s3++);
	dst[1] = op_3x3_calc_pixel(p,s1++,s2++,s3++);
	dst[2] = op_3x3_calc_pixel(p,s1++,s2++,s3++);
	dst += 3;
    }
}

static void
op_3x3_clip_line(unsigned char *dst, int *src, int left, int right)
{
    int i,val;

    src += left*3;
    dst += left*3;
    for (i = left*3; i < right*3; i++) {
	val = *(src++);
	if (val < 0)
	    val = 0;
	if (val > 255)
	    val = 255;
	*(dst++) = val;
    }
}

static void
op_3x3_work(struct ida_image *src, struct ida_rect *rect,
	    unsigned char *dst, int line, void *data)
{
    struct op_3x3_handle *h = (struct op_3x3_handle *)data;
    unsigned char *scanline;

    scanline = (unsigned char*) src->data + line * src->i.width * 3;
    memcpy(dst,scanline,src->i.width * 3);
    if (line < rect->y1 || line >= rect->y2)
	return;

    op_3x3_calc_line(src,rect,h->linebuf,line,&h->filter);
    op_3x3_clip_line(dst,h->linebuf,rect->x1,rect->x2);
}

static void
op_3x3_free(void *data)
{
    struct op_3x3_handle *h = (struct op_3x3_handle *)data;

    fim_free(h->linebuf);
    fim_free(h);
}
	    
/* ----------------------------------------------------------------------- */

struct op_sharpe_handle {
    int  factor;
    int  *linebuf;
};

static void*
op_sharpe_init(struct ida_image *src, struct ida_rect *rect,
	       struct ida_image_info *i, void *parm)
{
    struct op_sharpe_parm *args = (struct op_sharpe_parm *)parm;
    struct op_sharpe_handle *h;

    h = (struct op_sharpe_handle *)fim_calloc(sizeof(*h),1);
    if(!h)goto oops;
    h->factor  = args->factor;
    h->linebuf = (int *)fim_malloc(sizeof(int)*3*(src->i.width));
    if(!h->linebuf)goto oops;

    *i = src->i;
    return h;
oops:
    if(h && h->linebuf)fim_free(h->linebuf);
    if(h)fim_free(h);
    return NULL;
}

static void
op_sharpe_work(struct ida_image *src, struct ida_rect *rect,
	       unsigned char *dst, int line, void *data)
{
    static struct op_3x3_parm laplace = {
/*  	f1: {  1,  1,  1 },
	f2: {  1, -8,  1 },
	f3: {  1,  1,  1 },*/
  	 {  1,  1,  1 },
	 {  1, -8,  1 },
	 {  1,  1,  1 },
	 0,0,0
    };
    struct op_sharpe_handle *h = (struct op_sharpe_handle *)data;
    unsigned char *scanline;
    int i;

    scanline = src->data + line * src->i.width * 3;
    memcpy(dst,scanline,src->i.width * 3);
    if (line < rect->y1 || line >= rect->y2)
	return;

    op_3x3_calc_line(src,rect,h->linebuf,line,&laplace);
    for (i = rect->x1*3; i < rect->x2*3; i++)
	h->linebuf[i] = scanline[i] - h->linebuf[i] * h->factor / 256;
    op_3x3_clip_line(dst,h->linebuf,rect->x1,rect->x2);
}

static void
op_sharpe_free(void *data)
{
    struct op_sharpe_handle *h = (struct op_sharpe_handle *)data;

    fim_free(h->linebuf);
    fim_free(h);
}

/* ----------------------------------------------------------------------- */

struct op_resize_state {
    float xscale,yscale,inleft;
    float *rowbuf;
    unsigned int width,height,srcrow;
};

static void*
op_resize_init(struct ida_image *src, struct ida_rect *rect,
	       struct ida_image_info *i, void *parm)
{
    struct op_resize_parm *args = (struct op_resize_parm *)parm;
    struct op_resize_state *h;

    h = (struct op_resize_state *)fim_calloc(sizeof(*h),1);
    if(!h)goto oops;
    h->width  = args->width;
    h->height = args->height;
    h->xscale = (float)args->width/src->i.width;
    h->yscale = (float)args->height/src->i.height;
    h->rowbuf = (float*)fim_malloc(src->i.width * 3 * sizeof(float));
    if(!h->rowbuf)goto oops;
    h->srcrow = 0;
    h->inleft = 1;

    *i = src->i;
    i->width  = args->width;
    i->height = args->height;
    i->dpi    = args->dpi;
    return h;
    oops:
    if(h)fim_free(h);
    return NULL;
}

#if 1
void op_resize_work_row_expand(struct ida_image *src, struct ida_rect *rect, unsigned char *dst, int line, void *data)
{
	struct op_resize_state *h = (struct op_resize_state *)data;
//#ifndef FIM_WANTS_SLOW_RESIZE	/*uncommenting this triggers failure */
       int sr=h->srcrow;if(sr<0)sr=-sr;//who knows
//#endif
	unsigned char* srcline=src->data+src->i.width*3*(sr);
	const int Mdx=h->width;
	register int sx=0,dx;

	/*
	 * this gives a ~ 50% gain
	 * */
		float d0f=0.0,d1f=0.0;
		int   d0i=0,d1i=0;

		dx=0;
		if(src->i.width) for (sx=0;sx<(int)src->i.width-1;++sx )
		{
			d1f+=h->xscale;
			d1i=(unsigned int)d1f;

			sx*=3;
			for (dx=d0i;dx<(int)d1i;++dx )//d1i < Mdx
			{
				dst[3*dx+0] = srcline[  sx  ];
				dst[3*dx+1] = srcline[  sx+1];
				dst[3*dx+2] = srcline[  sx+2] ;
			}
			sx/=3;
	
			d0f=d1f;
			d0i=(unsigned int)d0f;
		}
		{
		// contour fix
			sx*=3;
			for (dx=d0i;dx<Mdx;++dx )//d1i < Mdx
			{
				dst[3*dx+0] = srcline[  sx  ];
				dst[3*dx+1] = srcline[  sx+1];
				dst[3*dx+2] = srcline[  sx+2] ;
			}
			sx/=3;
		}
		//for (dx=0;dx<Mdx;++dx ) { dst[3*dx+0]=0x00; dst[3*dx+1]=0x00; dst[3*dx+2]=0x00; }dx=0;
		if(line==(int)h->height-1)for (dx=0;dx<Mdx;++dx ) { dst[3*dx+0]=0x00; dst[3*dx+1]=0x00; dst[3*dx+2]=0x00; }dx=0;
}


inline void op_resize_work_row_expand_i_unrolled(struct ida_image *src, struct ida_rect *rect, unsigned char *dst, int line, void *data, int sr)
{
	struct op_resize_state *h = (struct op_resize_state *)data;
	unsigned char* srcline=src->data+src->i.width*3*(sr);
	const int Mdx=h->width;
	register int sx,dx;
	/*
	 * interleaved loop unrolling ..
	 * this gives a ~ 50% gain
	 * */
		float d0f=0.0,d1f=0.0;
		int   d0i=0,d1i=0;
		dx=0;
		sx=0;
		if(src->i.width)
		for (   ;sx<(int)src->i.width-1-4;sx+=4)
		{
			d1f+=h->xscale;
			d1i=(unsigned int)d1f;

			sx*=3;
			for (dx=d0i;dx<d1i;++dx )//d1i < Mdx
			{
				dst[3*dx+0] = srcline[  sx  ];
				dst[3*dx+1] = srcline[  sx+1];
				dst[3*dx+2] = srcline[  sx+2] ;
			}
	
			d0f=d1f;
			d0i=(unsigned int)d0f;

			d1f+=h->xscale;
			d1i=(unsigned int)d1f;

			sx+=3;
			for (dx=d0i;dx<d1i;++dx )//d1i < Mdx
			{
				dst[3*dx+0] = srcline[  sx  ];
				dst[3*dx+1] = srcline[  sx+1];
				dst[3*dx+2] = srcline[  sx+2] ;
			}
	
			d0f=d1f;
			d0i=(unsigned int)d0f;

			d1f+=h->xscale;
			d1i=(unsigned int)d1f;

			sx+=3;
			for (dx=d0i;dx<d1i;++dx )//d1i < Mdx
			{
				dst[3*dx+0] = srcline[  sx  ];
				dst[3*dx+1] = srcline[  sx+1];
				dst[3*dx+2] = srcline[  sx+2] ;
			}
	
			d0f=d1f;
			d0i=(unsigned int)d0f;

			d1f+=h->xscale;
			d1i=(unsigned int)d1f;

			sx+=3;
			for (dx=d0i;dx<d1i;++dx )//d1i < Mdx
			{
				dst[3*dx+0] = srcline[  sx  ];
				dst[3*dx+1] = srcline[  sx+1];
				dst[3*dx+2] = srcline[  sx+2] ;
			}
			sx-=9;
			sx/=3;

			d0f=d1f;
			d0i=(unsigned int)d0f;
		}

		for (  ;sx<(int)src->i.width ;++sx)
		{
			d1f+=h->xscale;
			d1i=(unsigned int)d1f;
			// contour fix
			for (dx=d0i;dx<Mdx;++dx )//d1i < Mdx
			{
				dst[3*dx+0] = srcline[  3*sx  ];
				dst[3*dx+1] = srcline[  3*sx+1];
				dst[3*dx+2] = srcline[  3*sx+2] ;
			}
			d0f=d1f;
			d0i=(unsigned int)d0f;
		}
		//for (dx=0;dx<Mdx;++dx ) { dst[3*dx+0]=0x00; dst[3*dx+1]=0x00; dst[3*dx+2]=0x00; }dx=0;
		if(line==(int)h->height-1)for (dx=0;dx<Mdx;++dx ) { dst[3*dx+0]=0x00; dst[3*dx+1]=0x00; dst[3*dx+2]=0x00; }dx=0;
}

inline void op_resize_work_unrolled4_row_expand(struct ida_image *src, struct ida_rect *rect, unsigned char *dst, int line, void *data, int sr)
{
	struct op_resize_state *h = (struct op_resize_state *)data;
	unsigned char* srcline=src->data+src->i.width*3*(sr);
	const int Mdx=h->width;
	register int sx,dx;

	/*
	 * this gives a ~ 70% gain
	 * */
		float d0f=0.0,d1f=0.0;
		int   d0i=0,d1i=0;

		sx=0;
		dx=0;
		if(src->i.width) for (   ;sx<(int)src->i.width;++sx )
		{
			d1f+=h->xscale;
			d1i=(unsigned int)d1f;

			register unsigned char r,g,b;
			r=srcline[  3*sx+ 0];
			g=srcline[  3*sx+ 1];
			b=srcline[  3*sx+ 2];

			for (dx=d0i;dx<d1i-4;dx+=4 )//d1i < Mdx
			{
				dst[3*dx+ 0] = r;
				dst[3*dx+ 1] = g;
				dst[3*dx+ 2] = b;

				dst[3*dx+ 3] = r;
				dst[3*dx+ 4] = g;
				dst[3*dx+ 5] = b;

				dst[3*dx+ 6] = r;
				dst[3*dx+ 7] = g;
				dst[3*dx+ 8] = b;

				dst[3*dx+ 9] = r;
				dst[3*dx+10] = g;
				dst[3*dx+11] = b;
			}
			for (   ;dx<d1i;++dx )//d1i < Mdx
			{
				dst[3*dx+0] = r;
				dst[3*dx+1] = g;
				dst[3*dx+2] = b;
			}
	
			d0f=d1f;
			d0i=(unsigned int)d0f;
		}
		// contour fix
		sx=src->i.width-1;
		for (dx=d0i;dx<Mdx;++dx )//d1i < Mdx
		{
			dst[3*dx+0] = srcline[  3*sx  ];
			dst[3*dx+1] = srcline[  3*sx+1];
			dst[3*dx+2] = srcline[  3*sx+2] ;
		}
		//for (dx=0;dx<Mdx;++dx ) { dst[3*dx+0]=0x00; dst[3*dx+1]=0x00; dst[3*dx+2]=0x00; }dx=0;
		if(line==(int)h->height-1)for (dx=0;dx<Mdx;++dx ) { dst[3*dx+0]=0x00; dst[3*dx+1]=0x00; dst[3*dx+2]=0x00; }dx=0;
}

inline void op_resize_work_unrolled2_row_expand(struct ida_image *src, struct ida_rect *rect, unsigned char *dst, int line, void *data, int sr)
{
	struct op_resize_state *h = (struct op_resize_state *)data;
	unsigned char* srcline=src->data+src->i.width*3*(sr);
	const int Mdx=h->width;
	register int sx,dx;

	/*
	 * this gives a ~ 60% gain
	 * */
		float d0f=0.0,d1f=0.0;
		int   d0i=0,d1i=0;

		sx=0;
		dx=0;
		if(src->i.width) for (   ;sx<(int)src->i.width-1;++sx )
		{
			d1f+=h->xscale;
			d1i=(unsigned int)d1f;

			register unsigned char r,g,b;
			r=srcline[  3*sx+ 0];
			g=srcline[  3*sx+ 1];
			b=srcline[  3*sx+ 2];

			for (dx=d0i;FIM_LIKELY(dx<d1i-2);dx+=2 )//d1i < Mdx
			{
				dst[3*dx+ 0] = r;
				dst[3*dx+ 1] = g;
				dst[3*dx+ 2] = b;

				dst[3*dx+ 3] = r;
				dst[3*dx+ 4] = g;
				dst[3*dx+ 5] = b;
			}
			for (   ;dx<d1i;++dx )//d1i < Mdx
			{
				dst[3*dx+0] = r;
				dst[3*dx+1] = g;
				dst[3*dx+2] = b;
			}
	
			d0f=d1f;
			d0i=(unsigned int)d0f;
		}
		{
		// contour fix
			sx*=3;
			for (dx=d0i;dx<Mdx;++dx )//d1i < Mdx
			{
				dst[3*dx+0] = srcline[  sx  ];
				dst[3*dx+1] = srcline[  sx+1];
				dst[3*dx+2] = srcline[  sx+2] ;
			}
			sx/=3;
		}
		//for (dx=0;dx<Mdx;++dx ) { dst[3*dx+0]=0x00; dst[3*dx+1]=0x00; dst[3*dx+2]=0x00; }dx=0;
		if(line==(int)h->height-1)for (dx=0;dx<Mdx;++dx ) { dst[3*dx+0]=0x00; dst[3*dx+1]=0x00; dst[3*dx+2]=0x00; }dx=0;
}

#endif

static void
op_resize_work(struct ida_image *src, struct ida_rect *rect,
	       unsigned char *dst, int line, void *data)
{
    struct op_resize_state *h = (struct op_resize_state *)data;
    float outleft,left,weight,d0,d1,d2;
    unsigned char *csrcline;
    float *fsrcline;

    register unsigned int i,sx,dx;

#ifndef FIM_WANTS_SLOW_RESIZE
    int sr=h->srcrow;if(sr<0)sr=-sr;//who knows
#endif

    /* scale y */
    memset(h->rowbuf, 0, src->i.width * 3 * sizeof(float));
    outleft = 1/h->yscale;
    while (outleft > 0  &&  h->srcrow < src->i.height) {
	if (outleft < h->inleft) {
	    weight     = outleft * h->yscale;
	    h->inleft -= outleft;
	    outleft    = 0;
	} else {
	    weight     = h->inleft * h->yscale;
	    outleft   -= h->inleft;
	    h->inleft  = 0;
	}
#if 0
	if (cc.displaydevice->debug)
	    FIM_FBI_PRINTF("y:  %6.2f%%: %d/%d => %d/%d\n",
		    weight*100,h->srcrow,src->height,line,h->height);
#endif
	csrcline = src->data + h->srcrow * src->i.width * 3;
	for (i = 0; i < src->i.width * 3; i++)
	    h->rowbuf[i] += (float)csrcline[i] * weight;
	if (0 == h->inleft) {
	    h->inleft = 1;
	    h->srcrow++;
	}
    }

#ifndef FIM_WANTS_SLOW_RESIZE
	/*
	 * a little tweaked resize : the following loop takes the most of cpu resources in a typical fim run!
	 */
	/* scale x */
	left = 1.0f;
	fsrcline = h->rowbuf;
    	const float c_outleft = 1.0f/h->xscale;
	//	cout << "c_outleft : "  << c_outleft << "\n";
	//	cout << "h->width : "  << (int)h->width << "\n";
	const unsigned int Mdx=h->width,msx=src->i.width;
	if(h->xscale>1.0)//here we handle the case of magnification
	{
#ifdef FIM_WANTS_SLOW_RESIZE
		unsigned char* srcline=src->data+src->i.width*3*(sr);
#endif

#ifndef FIM_WANTS_SLOW_RESIZE
		if(h->xscale>2.0)
		{
			if(h->xscale>4.0)
				op_resize_work_unrolled4_row_expand( src, rect, dst, line, data, sr);
			else
				op_resize_work_unrolled2_row_expand( src, rect, dst, line, data, sr);
		}
		else
			op_resize_work_row_expand_i_unrolled( src, rect, dst, line, data, sr);
//			op_resize_work_row_expand( src, rect, dst, line, data);

#else
		float fsx=0.0;
		for (sx=0,dx=0; dx<Mdx; ++dx)
		{
	#if 1
			fsx+=c_outleft;		// += is usually much lighter than a single *
			sx=((unsigned int)fsx)%src->i.width;// % is essential
			dst[0] = srcline[3*sx];
			dst[1] = srcline[3*sx+1]; 
			dst[2] = srcline[3*sx+2] ;
			dst += 3;
	#else
			fsx+=c_outleft;
			sx=((unsigned int)fsx)%src->i.width;// % is (maybe) essential
			dst[0] = (unsigned char) fsrcline[3*sx];
			dst[1] = (unsigned char) fsrcline[3*sx+1]; 
			dst[2] = (unsigned char) fsrcline[3*sx+2] ;
			dst += 3;
	#endif
		}
#endif
	}
#define ZEROF 0.0f
	else    // image minification
	for (sx = 0, dx = 0; dx < Mdx; dx++) {
	d0 = d1 = d2 = ZEROF;
	outleft = c_outleft;
	while (outleft > ZEROF &&  sx < msx) {
	    if (outleft < left) {
		weight   = outleft * h->xscale;
		left    -= outleft;
		outleft  = ZEROF;
	    } else {
		weight   = left * h->xscale;
		outleft -= left;
		left     = ZEROF;
	    }
	    d0 += fsrcline[3*sx] * weight;
	    d1 += fsrcline[3*sx+1] * weight;
	    d2 += fsrcline[3*sx+2] * weight;
	
	    if (ZEROF == left) {
		left = 1.0f;
		sx++;
	    }
	}
	dst[0] = (unsigned char)d0;
	dst[1] = (unsigned char)d1;
	dst[2] = (unsigned char)d2;
	dst += 3;
    }
	return ;
#else
    /* the original, slow cycle */
    /* scale x */
    left = 1;
    fsrcline = h->rowbuf;
    for (sx = 0, dx = 0; dx < h->width; dx++) {
	d0 = d1 = d2 = 0;
	outleft = 1/h->xscale;
	while (outleft > 0  &&  dx < h->width  &&  sx < src->i.width) {
	    if (outleft < left) {
		weight   = outleft * h->xscale;
		left    -= outleft;
		outleft  = 0;
	    } else {
		weight   = left * h->xscale;
		outleft -= left;
		left     = 0;
	    }
#if 0
	    if (cc.displaydevice->debug)
		FIM_FBI_PRINTF(" x: %6.2f%%: %d/%d => %d/%d\n",
			weight*100,sx,src->width,dx,h->width);
#endif
	    d0 += fsrcline[3*sx+0] * weight;
	    d1 += fsrcline[3*sx+1] * weight;
	    d2 += fsrcline[3*sx+2] * weight;
	    if (0 == left) {
		left = 1;
		sx++;
	    }
	}
	dst[0] = (unsigned char)d0;
	dst[1] = (unsigned char)d1;
	dst[2] = (unsigned char)d2;
	dst += 3;
    }
#endif
}

static void
op_resize_done(void *data)
{
    struct op_resize_state *h = (struct op_resize_state *)data;

    fim_free(h->rowbuf);
    fim_free(h);
}
    
/* ----------------------------------------------------------------------- */

struct op_rotate_state {
    float angle,sina,cosa;
    struct ida_rect calc;
    int cx,cy;
};

static void*
op_rotate_init(struct ida_image *src, struct ida_rect *rect,
	       struct ida_image_info *i, void *parm)
{
    struct op_rotate_parm *args = (struct op_rotate_parm *)parm;
    struct op_rotate_state *h;
    float  diag;

    h = (struct op_rotate_state *)fim_malloc(sizeof(*h));
    if(!h)return NULL;
    /* dez's : FIXME : NULL check missing */
    h->angle = args->angle * 2 * M_PI / 360;
    h->sina  = sin(h->angle);
    h->cosa  = cos(h->angle);
    /* dez's : cX means source center's X */
    h->cx    = (rect->x2 - rect->x1) / 2 + rect->x1;
    h->cy    = (rect->y2 - rect->y1) / 2 + rect->y1;

    /* the area we have to process (worst case: 45°) */
    diag     = sqrt((rect->x2 - rect->x1)*(rect->x2 - rect->x1) +
		    (rect->y2 - rect->y1)*(rect->y2 - rect->y1))/2;
    /* dez's : diag is a half diagonal */
    /* dez's : calc is the source input rectangle bounded
     * by source image valid coordinates ... */
    h->calc.x1 = (int)(h->cx - diag);
    h->calc.x2 = (int)(h->cx + diag);
    h->calc.y1 = (int)(h->cy - diag);
    h->calc.y2 = (int)(h->cy + diag);
    if (h->calc.x1 < 0)
	h->calc.x1 = 0;
    if (h->calc.x2 > (int)src->i.width)
	h->calc.x2 = (int)src->i.width;
    if (h->calc.y1 < 0)
	h->calc.y1 = 0;
    if (h->calc.y2 > (int)src->i.height)
	h->calc.y2 = (int)src->i.height;

     *i = src->i;
     /* TODO : it would be nice to expand and contract
      * the whole canvas just to fit the rotated image in !
      * */
    return h;
}

static inline
unsigned char* op_rotate_getpixel(struct ida_image *src, struct ida_rect *rect,
				  int sx, int sy, int dx, int dy)
{
    static unsigned char black[] = { 0, 0, 0};
#if 0
    int xdiff  =   rect->x2 - rect->x1;
    int ydiff  =   rect->y2 - rect->y1;
#endif
    if (sx < rect->x1 || sx >= rect->x2 ||
	sy < rect->y1 || sy >= rect->y2) {
#if 0
	/* experimental : textured rotation (i.e.: with wrapping) */
	while(sx <  rect->x1)sx+=xdiff;
	while(sx >= rect->x2)sx-=xdiff;
	while(sy <  rect->y1)sy+=ydiff;
	while(sy >= rect->y2)sy-=ydiff;
	return src->data + sy * src->i.width * 3 + sx * 3;
#else
	/* original */
	if (dx < rect->x1 || dx >= rect->x2 ||
	    dy < rect->y1 || dy >= rect->y2)
	    /* dez's : FIXME : i can't understand what this code stands for ! */
	    return src->data + dy * src->i.width * 3 + dx * 3;
	return black;
#endif
    }
    return src->data + sy * src->i.width * 3 + sx * 3;
}

static void
op_rotate_work(struct ida_image *src, struct ida_rect *rect,
	       unsigned char *dst, int y, void *data)
{
    struct op_rotate_state *h = (struct op_rotate_state *) data;
    unsigned char *pix;
    float fx,fy,w;
    int x,sx,sy;

    pix = src->data + y * src->i.width * 3;
    /*
     * useless (dez)
     * memcpy(dst,pix,src->i.width * 3);
     */
    if (y < h->calc.y1 || y >= h->calc.y2)
	return;
/*
    fx = h->cosa * (0 - h->cx) - h->sina * (y - h->cy) + h->cx;
    sx = (int)fx;
    sx *= 0;
    dst += 3*(h->calc.x1+sx);*/
    memset(dst, 0, (h->calc.x2-h->calc.x1) * 3);
   for (x = h->calc.x1; x < h->calc.x2; x++, dst+=3) {
	fx = h->cosa * (x - h->cx) - h->sina * (y - h->cy) + h->cx;
	fy = h->sina * (x - h->cx) + h->cosa * (y - h->cy) + h->cy;
	sx = (int)fx;
	sy = (int)fy;
	if (fx < 0)
	    sx--;
	if (fy < 0)
	    sy--;
	fx -= sx;
	fy -= sy;

	pix = op_rotate_getpixel(src,rect,sx,sy,x,y);
	w = (1-fx) * (1-fy);
	dst[0] += (unsigned char)(pix[0] * w);
	dst[1] += (unsigned char)(pix[1] * w);
	dst[2] += (unsigned char)(pix[2] * w);
	pix = op_rotate_getpixel(src,rect,sx+1,sy,x,y);
	w = fx * (1-fy);
	dst[0] += (unsigned char)(pix[0] * w);
	dst[1] += (unsigned char)(pix[1] * w);
	dst[2] += (unsigned char)(pix[2] * w);
	pix = op_rotate_getpixel(src,rect,sx,sy+1,x,y);
	w = (1-fx) * fy;
	dst[0] += (unsigned char)(pix[0] * w);
	dst[1] += (unsigned char)(pix[1] * w);
	dst[2] += (unsigned char)(pix[2] * w);
	pix = op_rotate_getpixel(src,rect,sx+1,sy+1,x,y);
	w = fx * fy;
	dst[0] += (unsigned char)(pix[0] * w);
	dst[1] += (unsigned char)(pix[1] * w);
	dst[2] += (unsigned char)(pix[2] * w);
    }
}

static void
op_rotate_done(void *data)
{
    struct op_rotate_state *h = (struct op_rotate_state *)data;

    fim_free(h);
}

/* ----------------------------------------------------------------------- */

struct ida_op desc_grayscale = {
    /*name:*/  "grayscale",
    /*init:*/  op_none_init,
    /*work:*/  op_grayscale,
    /*done:*/  op_none_done,
};
struct ida_op desc_3x3 = {
    /*name:*/  "3x3",
    /*init:*/  op_3x3_init,
    /*work:*/  op_3x3_work,
    /*done:*/  op_3x3_free,
};
struct ida_op desc_sharpe = {
    /*name:*/  "sharpe",
    /*init:*/  op_sharpe_init,
    /*work:*/  op_sharpe_work,
    /*done:*/  op_sharpe_free,
};
struct ida_op desc_resize = {
    /*name:*/  "resize",
    /*init:*/  op_resize_init,
    /*work:*/  op_resize_work,
    /*done:*/  op_resize_done,
};
struct ida_op desc_rotate = {
    /*name:*/  "rotate",
    /*init:*/  op_rotate_init,
    /*work:*/  op_rotate_work,
    /*done:*/  op_rotate_done,
};

// end filter.c
//



// op.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ----------------------------------------------------------------------- */
/* functions                                                               */

static char op_none_data_;

static void
op_flip_vert_(struct ida_image *src, struct ida_rect *rect,
	     unsigned char *dst, int line, void *data)
{
    char *scanline;

    scanline = (char*)src->data + (src->i.height - line - 1) * src->i.width * 3;
    memcpy(dst,scanline,src->i.width*3);
}

static void
op_flip_horz_(struct ida_image *src, struct ida_rect *rect,
	     unsigned char *dst, int line, void *data)
{
    char *scanline;
    unsigned int i;

    scanline = (char*)src->data + (line+1) * src->i.width * 3;
    for (i = 0; i < src->i.width; i++) {
	scanline -= 3;
	dst[0] = scanline[0];
	dst[1] = scanline[1];
	dst[2] = scanline[2];
	dst += 3;
    }
}

static void*
op_rotate_init_(struct ida_image *src, struct ida_rect *rect,
	       struct ida_image_info *i, void *parm)
{
    *i = src->i;
    i->height = src->i.width;
    i->width  = src->i.height;
    i->dpi    = src->i.dpi;
    return &op_none_data_;
}

static void
op_rotate_cw_(struct ida_image *src, struct ida_rect *rect,
	     unsigned char *dst, int line, void *data)
{
    char *pix;
    unsigned int i;

    pix = (char*) src->data + src->i.width * src->i.height * 3 + line * 3;
    for (i = 0; i < src->i.height; i++) {
	pix -= src->i.width * 3;
	dst[0] = pix[0];
	dst[1] = pix[1];
	dst[2] = pix[2];
	dst += 3;
    }
}

static void
op_rotate_ccw_(struct ida_image *src, struct ida_rect *rect,
	      unsigned char *dst, int line, void *data)
{
    char *pix;
    unsigned int i;

    pix = (char*) src->data + (src->i.width-line-1) * 3;
    for (i = 0; i < src->i.height; i++) {
	dst[0] = pix[0];
	dst[1] = pix[1];
	dst[2] = pix[2];
	pix += src->i.width * 3;
	dst += 3;
    }
}

static void
op_invert_(struct ida_image *src, struct ida_rect *rect,
	  unsigned char *dst, int line, void *data)
{
    unsigned char *scanline;
    int i;

    scanline = src->data + line * src->i.width * 3;
    memcpy(dst,scanline,src->i.width * 3);
    if (line < rect->y1 || line >= rect->y2)
	return;
    dst      += 3*rect->x1;
    scanline += 3*rect->x1;
    for (i = rect->x1; i < rect->x2; i++) {
	dst[0] = 255-scanline[0];
	dst[1] = 255-scanline[1];
	dst[2] = 255-scanline[2];
	scanline += 3;
	dst += 3;
    }
}

static void*
op_crop_init_(struct ida_image *src, struct ida_rect *rect,
	     struct ida_image_info *i, void *parm)
{
    if (rect->x2 - rect->x1 == (int)src->i.width &&
	rect->y2 - rect->y1 == (int)src->i.height)
	return NULL;
    *i = src->i;
    i->width  = rect->x2 - rect->x1;
    i->height = rect->y2 - rect->y1;
    return &op_none_data_;
}

static void
op_crop_work_(struct ida_image *src, struct ida_rect *rect,
	     unsigned char *dst, int line, void *data)
{
    unsigned char *scanline;
    int i;

    scanline = src->data + (line+rect->y1) * src->i.width * 3 + rect->x1 * 3;
    for (i = rect->x1; i < rect->x2; i++) {
	dst[0] = scanline[0];
	dst[1] = scanline[1];
	dst[2] = scanline[2];
	scanline += 3;
	dst += 3;
    }
}

static void*
op_autocrop_init_(struct ida_image *src, struct ida_rect *unused,
		 struct ida_image_info *i, void *parm)
{
#ifdef FIM_USE_DESIGNATED_INITIALIZERS
    static struct op_3x3_parm filter = {
	/*f1:*/ { -1, -1, -1 },
	/*f2:*/ { -1,  8, -1 },
	/*f3:*/ { -1, -1, -1 },
    };
#else
    /* I have no quick fix for this ! (m.m.) 
     * However, designated initializers are a a C99 construct
     * and are usually tolerated by g++.
     * */
    static struct op_3x3_parm filter = {
	/*f1:*/ { -1, -1, -1 },
	/*f2:*/ { -1,  8, -1 },
	/*f3:*/ { -1, -1, -1 },
    };
#endif
    struct ida_rect rect;
    struct ida_image img;
    int x,y,limit;
    unsigned char *line;
    void *data;
    
    /* detect edges */
    rect.x1 = 0;
    rect.x2 = src->i.width;
    rect.y1 = 0;
    rect.y2 = src->i.height;
    data = desc_3x3.init(src, &rect, &img.i, &filter);

    img.data   = (unsigned char*)fim_malloc(img.i.width * img.i.height * 3);
    if(!img.data)return NULL;

    for (y = 0; y < (int)img.i.height; y++)
	desc_3x3.work(src, &rect, img.data+3*img.i.width*y, y, data);
    desc_3x3.done(data);
    limit = 64;

    /* y border */
    for (y = 0; y < (int)img.i.height; y++) {
	line = img.data + img.i.width*y*3;
	for (x = 0; x < (int)img.i.width; x++)
	    if (line[3*x+0] > limit ||
		line[3*x+1] > limit ||
		line[3*x+2] > limit)
		break;
	if (x != (int)img.i.width)
	    break;
    }
    rect.y1 = y;
    for (y = (int)img.i.height-1; y > rect.y1; y--) {
	line = img.data + img.i.width*y*3;
	for (x = 0; x < (int)img.i.width; x++)
	    if (line[3*x+0] > limit ||
		line[3*x+1] > limit ||
		line[3*x+2] > limit)
		break;
	if (x != (int)img.i.width)
	    break;
    }
    rect.y2 = y+1;

    /* x border */
    for (x = 0; x < (int)img.i.width; x++) {
	for (y = 0; y < (int)img.i.height; y++) {
	    line = img.data + (img.i.width*y+x) * 3;
	    if (line[0] > limit ||
		line[1] > limit ||
		line[2] > limit)
		break;
	}
	if (y != (int)img.i.height)
	    break;
    }
    rect.x1 = x;
    for (x = (int)img.i.width-1; x > rect.x1; x--) {
	for (y = 0; y < (int)img.i.height; y++) {
	    line = img.data + (img.i.width*y+x) * 3;
	    if (line[0] > limit ||
		line[1] > limit ||
		line[2] > limit)
		break;
	}
	if (y != (int)img.i.height)
	    break;
    }
    rect.x2 = x+1;

    fim_free(img.data);
    if (cc.displaydevice->debug)
	FIM_FBI_PRINTF("y: %d-%d/%d  --  x: %d-%d/%d\n",
		rect.y1, rect.y2, img.i.height,
		rect.x1, rect.x2, img.i.width);

    if (0 == rect.x2 - rect.x1  ||  0 == rect.y2 - rect.y1)
	return NULL;
    
    *unused = rect;
    *i = src->i;
    i->width  = rect.x2 - rect.x1;
    i->height = rect.y2 - rect.y1;
    return &op_none_data_;
}

/* ----------------------------------------------------------------------- */

static char op_none_data;

void* op_none_init(struct ida_image *src,  struct ida_rect *sel,
		   struct ida_image_info *i, void *parm)
{
    *i = src->i;
    return &op_none_data;
}

void  op_none_done(void *data) {}
void  op_free_done(void *data) { fim_free(data); }

/* ----------------------------------------------------------------------- */

struct ida_op desc_flip_vert = {
    /*name:*/  "flip-vert",
    /*init:*/  op_none_init,
    /*work:*/  op_flip_vert_,
    /*done:*/  op_none_done,
};
struct ida_op desc_flip_horz = {
    /*name:*/  "flip-horz",
    /*init:*/  op_none_init,
    /*work:*/  op_flip_horz_,
    /*done:*/  op_none_done,
};
struct ida_op desc_rotate_cw = {
    /*name:*/  "rotate-cw",
    /*init:*/  op_rotate_init_,
    /*work:*/  op_rotate_cw_,
    /*done:*/  op_none_done,
};
struct ida_op desc_rotate_ccw = {
    /*name:*/  "rotate-ccw",
    /*init:*/  op_rotate_init_,
    /*work:*/  op_rotate_ccw_,
    /*done:*/  op_none_done,
};
struct ida_op desc_invert = {
    /*name:*/  "invert",
    /*init:*/  op_none_init,
    /*work:*/  op_invert_,
    /*done:*/  op_none_done,
};
struct ida_op desc_crop = {
    /*name:*/  "crop",
    /*init:*/  op_crop_init_,
    /*work:*/  op_crop_work_,
    /*done:*/  op_none_done,
};
struct ida_op desc_autocrop = {
    /*name:*/  "autocrop",
    /*init:*/  op_autocrop_init_,
    /*work:*/  op_crop_work_,
    /*done:*/  op_none_done,
};

// end op.c







#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef USE_X11
# include "viewer.h"
#endif

/* ---------------------------------------------------------------------- */
/* load                                                                   */


#ifdef FIM_WITH_LIBPNG 
	extern struct ida_loader png_loader ;
#endif

extern struct ida_loader ppm_loader ;
extern struct ida_loader pgm_loader ;
extern struct ida_loader bit24_loader ;
extern struct ida_loader bit1_loader ;

// 20080108 WARNING
// 20080801 removed the loader functions from this file, as init_rd was not __init : did I break something ?
//static void __init init_rd(void)
/*static void init_rd(void)
{
    load_register(&ppm_loader);
    load_register(&pgm_loader);
}*/

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
    /*  label:*/  "PPM",
    /*  ext:*/    { "ppm", NULL},
    /*  write:*/  ppm_write,
    /* FIXME : still missing some struct members */
};

// 20080108 WARNING
//static void __init init_wr(void)
static void init_wr(void)
{
    write_register(&ppm_writer);
}
#endif




/*static void free_image(struct ida_image *img)*/
void FbiStuff::free_image(struct ida_image *img)
{
    if (img) {
	if (img->data)
	    fim_free(img->data);
	fim_free(img);
    }
}

FILE* FbiStuff::fim_execlp(const char *cmd, ...)
{
	/* new */
	va_list ap;
        int rc;
	FILE *fp=NULL;
	int p[2];
	#define FIM_SUBPROCESS_MAXARGV 128
	char * argv[FIM_SUBPROCESS_MAXARGV],*s;	/* FIXME */
	int argc=0;
	if(0!=pipe(p))
		return NULL;

	switch(fork())
	{
		case -1:
		perror("fork");
		close(p[0]);
		close(p[1]);
		return NULL;
		case 0:/* child */
		dup2(p[1],1/*stdout*/);
		close(p[0]);
		close(p[1]);
	        va_start(ap,cmd);
		while(NULL!=(s=va_arg(ap,char*)) && argc<FIM_SUBPROCESS_MAXARGV-1)
		{
			argv[argc]=s;
			argc++;
		}
		argv[argc]=NULL;

	        va_end(ap);
	        rc=execvp(cmd,argv);
		exit(rc);
		default:/* parent */
		close(p[1]);
		fp = fdopen(p[0],"r");
		if(NULL==fp)
			return NULL;
		return fp;
	}
	return NULL;
}

static long find_regexp_offset(FILE *fp, const char *byte_stream, size_t base_offset)
{
	/*
		FIXME : EXPERIMENTAL, UNFINISHED
		should merge this code with read_fd_chunk_something..
		note  : won't find zeros..
	*/
	/*size_t*/
	/*l ong*/
	int  read_offset=0,rb,sl,off,goff=0;
	char buf[FIM_FILE_BUF_SIZE];
	int rs;/* FIXME : could overflow */

	if(!byte_stream)
		return 0;/* bad argument */

	sl=strlen(byte_stream);

	if(sl>FIM_FILE_BUF_SIZE-1)
		return 0;/* FIXME : a limitation */

	if(base_offset)
	{
    		if(fseek(fp,base_offset,SEEK_SET)!=0);// NEW
		{
    			// fseek(fp,0,SEEK_SET);
			// should handle in some better way..
			return 0;
		}
		goff=base_offset;
	}
	else
		goff+=ftell(fp);

	/* we read in a good chunk of the file */
	while((rb=fread(buf,1,FIM_FILE_BUF_SIZE,fp))>0)
	{
		bzero(buf,(FIM_FILE_BUF_SIZE-rb));/* sanitization */
		off=rb-sl;
		while( off > 0 )
		{

			if( 0==bcmp( buf+off, byte_stream, sl ) )
			{
				/* hit */
				std::cout << "hit!" << off << "\n";
				return goff+off;
//				return ftell(fp);
			}
			--off;
		}
		goff+=off;
		goff+=rb;
	}
	return 0;
}

/*static struct ida_image**/
struct ida_image* FbiStuff::read_image(char *filename, FILE* fd, int page)
{
    /*
     * This function is complicated and should be reworked, in some way.
     * FIXME : many memory allocations are not checked for failure: DANGER
     * */
    char command[1024];
    struct ida_loader *loader = NULL;
    struct ida_image *img=NULL;
    struct list_head *item=NULL;
    char blk[512];
    FILE *fp;
    unsigned int y;
    void *data;
    int fr=0;
    
    //WARNING
    //new_image = 1;

    // Warning: this fd passing 
    // is a trick for reading stdin...
    // ... and it is simpler that rewriting loader stuff.
    // but much dirtier :/
    if(fd==NULL) {
    /* open file */
    if (NULL == (fp = fopen(filename, "r"))) {
	//comment by dez, temporary
	if(cc.displaydevice->debug)
		FIM_FBI_PRINTF("open %s: %s\n",filename,strerror(errno));
	return NULL;
    }
    } else fp=fd;

    //size_t read_offset=cc.getIntVariable("g:"FIM_VID_OPEN_OFFSET);
    size_t read_offset=0;
    read_offset=cc.getIntVariable(FIM_VID_OPEN_OFFSET);/* warning : user could supply negative values */

    if(read_offset>0)fseek(fp,read_offset,SEEK_SET);// NEW

#ifdef FIM_WANT_SEEK_MAGIC
	/* FIXME : EXPERIMENTAL */
	string sm;
   	sm=cc.getStringVariable(FIM_VID_SEEK_MAGIC);
	/*
		the user should specify a magix string like:
		sm="\xFF\xD8\xFF\xE0";
	*/
   	if(sm!="")
	{
		read_offset=find_regexp_offset(fp, sm.c_str() , read_offset);
		if(read_offset>0)fseek(fp,read_offset,SEEK_SET);// NEW
		cc.setVariable(FIM_VID_OPEN_OFFSET ,(int)read_offset);
	}
#endif

    memset(blk,0,sizeof(blk));
    if((fr=fread(blk,1,sizeof(blk),fp))<0)
    {
      /* should we care about the error code ? */
      return NULL;	/* new */
    }
    rewind(fp);
    if(read_offset>0)fseek(fp,read_offset,SEEK_SET);// NEW

    if(cc.getIntVariable(FIM_VID_BINARY_DISPLAY)!=0)
    {
        /* a funny feature */
    	if(cc.getIntVariable(FIM_VID_BINARY_DISPLAY)==1)
		loader = &bit1_loader;
	else
		loader = &bit24_loader;
    }
    /* pick loader */
#ifdef FIM_SKIP_KNOWN_FILETYPES
    if (NULL == loader && (*blk==0x42) && (*(unsigned char*)(blk+1)==0x5a))
    {
	cc.set_status_bar("skipping 'bz2'...", "*");
	return NULL;
    }
/* gz is another ! */
/*    if (NULL == loader && (*blk==0x30) && (*(unsigned char*)(blk+1)==0x30))
    {
	cc.set_status_bar("skipping 'gz'...", "*");
	return NULL;
    }*/
#ifndef HAVE_LIBPOPPLER
    if (NULL == loader && (*blk==0x25) && (*(unsigned char*)(blk+1)==0x50 )
     && NULL == loader && (*(unsigned char*)(blk+2)==0x44) && (*(unsigned char*)(blk+3)==0x46))
    {
	cc.set_status_bar("skipping 'pdf' (use fimgs for this)...", "*");
	return NULL;
    }
#endif
#ifndef HAVE_LIBSPECTRE
    if (NULL == loader && (*blk==0x25) && (*(unsigned char*)(blk+1)==0x21 )
     && NULL == loader && (*(unsigned char*)(blk+2)==0x50) && (*(unsigned char*)(blk+3)==0x53))
    {
	cc.set_status_bar("skipping 'ps' (use fimgs for this)...", "*");
	return NULL;
    }
#endif
#endif
    if(NULL==loader)/* we could have forced one */
    list_for_each(item,&loaders) {
        loader = list_entry(item, struct ida_loader, list);
	if (NULL == loader->magic)
	    break;
	if (0 == memcmp(blk+loader->moff,loader->magic,loader->mlen))
	    break;
	loader = NULL;
    }
     
#ifdef FIM_WITH_LIBPNG 
#ifdef FIM_TRY_DIA
    if (NULL == loader && (*blk==0x1f) && (*(unsigned char*)(blk+1)==0x8b))// i am not sure if this is the FULL signature!
    {
	cc.set_status_bar("please wait while piping through 'dia'...", "*");
    	/*
	 * dez's
	 * */
	/* a gimp xcf file was found, and we try to use xcftopnm */
	cc.set_status_bar("please wait while piping through 'dia'...", "*");
	if(NULL!=(fp=fim_execlp("dia","dia",filename,"-e",FIM_TMP_FILENAME".png",NULL))&& 0==fclose (fp))
	{
		if (NULL == (fp = fopen(FIM_TMP_FILENAME".png","r")))
		/* this could happen in case dia was removed from the system */
			return NULL;
		else
		{
			unlink(FIM_TMP_FILENAME".png");
			loader = &png_loader;
		}
   	}
   }
#endif
#endif
#ifdef FIM_TRY_XFIG
    if (NULL == loader && (0 == memcmp(blk,"#FIG",4)))
    {
	cc.set_status_bar("please wait while piping through 'fig2dev'...", "*");
    	/*
	 * dez's
	 * */
	/* a xfig file was found, and we try to use fig2dev */
	if(NULL==(fp=fim_execlp("fig2dev","fig2dev","-L","ppm",filename,NULL)))
	    return NULL;
	loader = &ppm_loader;
    }
#endif
#ifdef FIM_TRY_XCFTOPNM
    if (NULL == loader && (0 == memcmp(blk,"gimp xcf file",13)))
    {
	cc.set_status_bar("please wait while piping through 'xcftopnm'...", "*");
    	/*
	 * dez's
	 * */
	/* a gimp xcf file was found, and we try to use xcftopnm */
	if(NULL==(fp=fim_execlp("xcftopnm","xcftopnm",filename,NULL)))
	    return NULL;
	loader = &ppm_loader;
    }
#endif
//#if 0
#ifdef FIM_TRY_INKSCAPE
#ifdef FIM_WITH_LIBPNG 
    if (NULL == loader && (0 == memcmp(blk,"<?xml version=\"1.0\" encoding=\"UTF-8\"",36)))
    //if(regexp_match(filename,".*svg$"))
    {
    	/*
	 * FIXME : use tmpfile() here. DANGER!
	 * */
	/* an svg file was found, and we try to use inkscape with it
	 * note that braindamaged inkscape doesn't export to stdout ...
	 * */
	cc.set_status_bar("please wait while piping through 'inkscape'...", "*");
	sprintf(command,"inkscape \"%s\" --export-png \"%s\"",
		filename,FIM_TMP_FILENAME );
#if 0
	/* FIXME : the following code should work, but it doesn't */
	if(NULL!=(fp=fim_execlp("inkscape","inkscape",filename,"--export-png","/dev/stdout",NULL)))
	{
		fp=fim_fread_tmpfile(fp);
		if(fp==NULL) return NULL;
		else
		{
			loader = &png_loader;
		}
	}
#else
	if(NULL!=(fp=fim_execlp("inkscape","inkscape",filename,"--export-png",FIM_TMP_FILENAME,NULL))&&0==fclose(fp))
	{
		if (NULL == (fp = fopen(FIM_TMP_FILENAME,"r")))
			    return NULL;
		else
		{
			unlink(FIM_TMP_FILENAME);
			loader = &png_loader;
		}
	}
    }
#endif
#endif
#if 0
/*
 * Warning : this is potentially dangerous and so we wait a little before working on this.
 * */
    if((NULL == loader && (0 == memcmp(blk,"#!/usr/bin/fim",14))) ||
       (NULL == loader && (0 == memcmp(blk,"#!/usr/sbin/fim",15))) ||
       (NULL == loader && (0 == memcmp(blk,"#!/usr/local/bin/fim",20))) ||
       (NULL == loader && (0 == memcmp(blk,"#!/usr/local/sbin/fim",21)))
       )
    {
	cc.set_status_bar("loading Fim script file ...", "*");
	cc.executeFile(filename);
	return NULL;
    }
#endif
#endif
//#endif
#ifdef FIM_TRY_CONVERT
    if (NULL == loader) {
	cc.set_status_bar(string("please wait while piping ")+string(filename)+string(" through 'convert'..."), "*");
	/* no loader found, try to use ImageMagick's convert */
	if(NULL==(fp=fim_execlp("convert","convert",filename,"ppm:-",NULL)))
		return NULL;
	loader = &ppm_loader;
    }
#endif
    /*
     * no appropriate loader found for this image
     * */
    if (NULL == loader) return NULL;

    /* load image */
    img = (struct ida_image*)fim_calloc(sizeof(*img),1);/* calloc, not malloc: we want zeros */
    if(!img)goto errl;
    memset(img,0,sizeof(*img));
#ifdef FIM_EXPERIMENTAL_ROTATION
    /* 
     * warning : there is a new field in ida_image_info (fim_extra_flags) 
     * which gets cleared to 0 (default) in this way.
     * */
#endif
    data = loader->init(fp,filename,page,&img->i,0);
#ifdef FIM_READ_STDIN_IMAGE
    if(strcmp(filename,FIM_STDIN_IMAGE_NAME)==0) { close(0); if(dup(2)){/* FIXME : should we report this ?*/}/* if the image is loaded from stdin, we close its stream */}
#endif
    if (NULL == data) {
	if(cc.displaydevice->debug)
		FIM_FBI_PRINTF("loading %s [%s] FAILED\n",filename,loader->name);
	free_image(img);
	return NULL;
    }
    img->data = (unsigned char*)fim_malloc(img->i.width * img->i.height * 3);
    if(!img->data)goto errl;
#ifndef FIM_IS_SLOWER_THAN_FBI
    for (y = 0; y < img->i.height; y++) {
	loader->read(img->data + img->i.width * 3 * y, y, data);
    }
#else
    for (y = 0; y < img->i.height; y++) {
	cc.displaydevice->switch_if_needed();
	loader->read(img->data + img->i.width * 3 * y, y, data);
    }
#endif

#ifndef FIM_IS_SLOWER_THAN_FBI
    /*
     * this patch aligns the pixel bytes in the order they should
     * be dumped to the video memory, resulting in much faster image
     * drawing in fim than in fbi !
     * */
    {
	register char t;
	register char	*p=(char*) img->data,
		 	*pm=(char*)p+img->i.width*3*y;
	while(p<pm)
	{
            t=*p;
            *p=p[2];
            p[2]=t;
	    p+=3;
	}
    }
#endif
    loader->done(data);
    return img;
errl:
    if(img && img->data)fim_free(img->data);
    if(img )fim_free(img);
    return NULL;
}

/*all dez's
 *
 * rotate the image 90 degrees (M_PI/2) at a time
 * */
struct ida_image*
FbiStuff::rotate_image90(struct ida_image *src, unsigned int rotation)
{
    struct op_resize_parm p;
    struct ida_rect  rect;
    struct ida_image *dest;
    void *data;
    unsigned int y;
    struct ida_op *desc_p;

    dest =(ida_image*) fim_malloc(sizeof(*dest));
    /* dez: */ if(!dest)return NULL;
    memset(dest,0,sizeof(*dest));
    memset(&rect,0,sizeof(rect));
    memset(&p,0,sizeof(p));
    
    p.width  = src->i.width;
    p.height = src->i.height;
    p.dpi    = src->i.dpi;
    if (0 == p.width)
	p.width = 1;
    if (0 == p.height)
	p.height = 1;
    
    rotation%=2;
    if(rotation==0){desc_p=&desc_rotate_ccw;}
    else	   {desc_p=&desc_rotate_cw ;}

    data = desc_p->init(src,&rect,&dest->i,&p);
    dest->data = (unsigned char*)fim_malloc(dest->i.width * dest->i.height * 3);
    /* dez: */ if(!(dest->data)){fim_free(dest);return NULL;}
    for (y = 0; y < dest->i.height; y++) {
	cc.displaydevice->switch_if_needed();
	desc_p->work(src,&rect,
			 dest->data + 3 * dest->i.width * y,
			 y, data);
    }
    desc_p->done(data);
    return dest;
}

struct ida_image*	
FbiStuff::rotate_image(struct ida_image *src, float angle)
{
    /*
     * dez's:
     * this whole code was written for a fixed canvas rotation,
     * not allowing any canvas adaptation at all ... urgh
     * */
    struct op_rotate_parm p;
    /* dez's 20080831 */
    struct ida_rect  rect;
    struct ida_image *dest;
    void *data;
    unsigned int y;

    dest = (ida_image*)fim_malloc(sizeof(*dest));
    /* dez: */ if(!dest)return NULL;
    memset(dest,0,sizeof(*dest));
    memset(&rect,0,sizeof(rect));
    memset(&p,0,sizeof(p));

    /* source rectangle */
    rect.x1=0;
    rect.x2=src->i.width;
    rect.y1=0;
    rect.y2=src->i.height;
    
#ifdef FIM_EXPERIMENTAL_ROTATION
    if(! src->i.fim_extra_flags)
    {
    /*
     * this is code for a preliminary 'canvas' enlargement prior to image rotation.
     * experimental code.
     *
     * WARNING : this code seems buggy! (it is a horrible hack)
     * */   
    int diagonal = (int) ceilf( sqrtf( (float)( src->i.width * src->i.width  +  src->i.height * src->i.height) ) + 1.0f );
    int n_extra  = (diagonal - src->i.height  )/2;
    int s_extra  = (diagonal - src->i.height - n_extra     );
    int w_extra  = (diagonal - src->i.width      )/2;
    int e_extra  = (diagonal - src->i.width - w_extra  );
    /* we allocate a new, larger canvas */
    unsigned char * larger_data = (unsigned char*)fim_calloc(diagonal * diagonal * 3,1);
    if(larger_data)
    {
	    for(y = n_extra; y < (unsigned int) diagonal - s_extra; ++y )
	    	memcpy(larger_data + (y * diagonal + w_extra )*3 , src->data + (y-n_extra) * src->i.width * 3 , src->i.width*3);
	    src->i.width = diagonal;
	    src->i.height = diagonal;
	    /* source rectangle fix */
	    rect.x1+=w_extra;
	    rect.x2+=e_extra;
	    rect.y1+=n_extra;
	    rect.y2+=s_extra;
	    fim_free(src->data);
	    src->data=larger_data;
    	    src->i.fim_extra_flags=1;	/* to avoid this operation to repeat on square images or already rotated images */
    }
    /* on allocation failure (e.g.: a very long and thin image) we cannot do more.
     * uh, maybe we could tell the user about the allocation failure..*/
    else
    	cc.set_status_bar( "rescaling failed (insufficient memory?!)", "*");
    }
#endif

    p.angle    = (int) angle;
    data = desc_rotate.init(src,&rect,&dest->i,&p);
    dest->data = (unsigned char*)fim_calloc(dest->i.width * dest->i.height * 3,1);
    /* dez: */ if(!(dest->data)){fim_free(dest);return NULL;}
    for (y = 0; y < dest->i.height; y++) {
	cc.displaydevice->switch_if_needed();
	desc_rotate.work(src,&rect,
			 dest->data + 3 * dest->i.width * y,
			 y, data);
    }


    desc_rotate.done(data);


    //std::cout << "diagonal     : " << diagonal << "\n";
   // std::cout << "src->i.width : " << src->i.width << "\n";

    return dest;
}


struct ida_image*	
FbiStuff::scale_image(struct ida_image *src, float scale, float ascale)
{
    struct op_resize_parm p;
    struct ida_rect  rect;
    struct ida_image *dest;
    void *data;
    unsigned int y;
    /* dez: */ if(ascale<=0.0||ascale>=100.0)ascale=1.0;

    dest = (ida_image*)fim_malloc(sizeof(*dest));
    /* dez: */ if(!dest)return NULL;
    memset(dest,0,sizeof(*dest));
    memset(&rect,0,sizeof(rect));
    memset(&p,0,sizeof(p));
    
//    p.width  = (int)(src->i.width  * scale * ascale);
//    p.height = (int)(src->i.height * scale);
    // ceil() : new
    p.width  = (int)ceilf((float)src->i.width  * scale * ascale);
    p.height = (int)ceilf((float)src->i.height * scale);
    p.dpi    = (int)(src->i.dpi);
    if (0 == p.width)
	p.width = 1;
    if (0 == p.height)
	p.height = 1;
    data = desc_resize.init(src,&rect,&dest->i,&p);
    dest->data = (unsigned char*)fim_malloc(dest->i.width * dest->i.height * 3);
    /* dez: */ if(!(dest->data)){fim_free(dest);return NULL;}
    for (y = 0; y < dest->i.height; y++) {
	cc.displaydevice->switch_if_needed();
	desc_resize.work(src,&rect,
			 dest->data + 3 * dest->i.width * y,
			 y, data);
    }
    desc_resize.done(data);

    return dest;
}

struct ida_image * fbi_image_clone(struct ida_image *img)
{
	/* note that to fulfill free_image(), the descriptor and data couldn't be allocated together
	 * */
	if(!img || !img->data)return NULL;
	struct ida_image *nimg=NULL;
	int n;
	if(!(nimg=(ida_image*)fim_calloc(1,sizeof(struct ida_image))))return NULL;
	memcpy(nimg,img,sizeof(struct ida_image));
	/*note .. no checks .. :P */
	n = img->i.width * img->i.height * 3;
	
	nimg->data = (unsigned char*)fim_malloc( n );
	if(!(nimg->data))
	{
		fim_free(nimg);
		return NULL;
	}
	memcpy(nimg->data, img->data,n);
	return nimg;
}


	int FbiStuff::fim_filereading_debug()
	{
		return cc.displaydevice ?
			cc.displaydevice->debug:
			0;
	}

}
