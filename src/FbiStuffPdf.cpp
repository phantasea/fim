/* $Id: FbiStuffPdf.cpp 271 2009-12-13 00:03:48Z dezperado $ */
/*
 FbiStuffPdf.cpp : fim functions for decoding PDF files

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

#ifdef HAVE_LIBPOPPLER

/*
 * Note : at the time of writing this, the poppler API is unstable,
 * and subject to change.
 * So when changing these headers here, take care of changing them
 * in the configure script, too.
 */
#include <poppler/poppler-config.h>
#include <poppler/PDFDoc.h>
#include <poppler/OutputDev.h>
#include <poppler/SplashOutputDev.h>
#include <poppler/splash/SplashBitmap.h>
#include <poppler/splash/SplashTypes.h>
#include <poppler/Page.h>
#include <poppler/GlobalParams.h>	/* globalParams lives here */


/*								*/

namespace fim
{

/* ---------------------------------------------------------------------- */
/* load                                                                   */

struct pdf_state_t {
	SplashBitmap*	    bmp ;
	PDFDoc *            pd ;
	SplashOutputDev *   od ;
	int row_stride;    /* physical row width in output buffer */
	unsigned char * first_row_dst;
};


/* ---------------------------------------------------------------------- */

static SplashColor splashColRed;
static SplashColor splashColGreen;
static SplashColor splashColBlue;
static SplashColor splashColWhite;
static SplashColor splashColBlack;
//static SplashColorMode gSplashColorMode = splashModeBGR8;
static SplashColorMode gSplashColorMode = splashModeRGB8;
#define SPLASH_COL_RED_PTR (SplashColorPtr)&(splashColRed[0])
#define SPLASH_COL_GREEN_PTR (SplashColorPtr)&(splashColGreen[0])
#define SPLASH_COL_BLUE_PTR (SplashColorPtr)&(splashColBlue[0])
#define SPLASH_COL_WHITE_PTR (SplashColorPtr)&(splashColWhite[0])
#define SPLASH_COL_BLACK_PTR (SplashColorPtr)&(splashColBlack[0])

static SplashColorPtr  gBgColor = SPLASH_COL_WHITE_PTR;

static void splashColorSet(SplashColorPtr col, Guchar red, Guchar green, Guchar blue, Guchar alpha)
{
    switch (gSplashColorMode)
    {
        case splashModeBGR8:
            col[0] = blue;
            col[1] = green;
            col[2] = red;
            break;
        case splashModeRGB8:
            col[0] = red;
            col[1] = green;
            col[2] = blue;
            break;
        default:
            assert(0);
            break;
    }
}

void SplashColorsInit(void)
{
    splashColorSet(SPLASH_COL_RED_PTR, 0xff, 0, 0, 0);
    splashColorSet(SPLASH_COL_GREEN_PTR, 0, 0xff, 0, 0);
    splashColorSet(SPLASH_COL_BLUE_PTR, 0, 0, 0xff, 0);
    splashColorSet(SPLASH_COL_BLACK_PTR, 0, 0, 0, 0);
    splashColorSet(SPLASH_COL_WHITE_PTR, 0xff, 0xff, 0xff, 0);
}

static void*
pdf_init(FILE *fp, char *filename, unsigned int page,
	  struct ida_image_info *i, int thumbnail)
{
	char _[1];
	_[0]='\0';
	struct pdf_state_t * ds=NULL;
	int rotation=0,pageNo=page+1;
	double zoomReal=250.0*2;
	double hDPI;
	double vDPI;
	GBool  useMediaBox ;
	GBool  crop        ;
	GBool  doLinks     ;
	if(filename==FIM_STDIN_IMAGE_NAME){std::cerr<<"sorry, stdin multipage file reading is not supported\n";return NULL;}	/* a drivers's problem */ 

	if(fp) fclose(fp);

	ds = (struct pdf_state_t*)fim_calloc(sizeof(struct pdf_state_t),1);

	if(!ds)
		return NULL;

    	ds->first_row_dst = NULL;
	ds->bmp = NULL;
	ds->pd = NULL;
	ds->od = NULL;

	SplashColorsInit();

	// WARNING : a global variable from libpoppler! damn!!
	globalParams = new GlobalParams();
	if (!globalParams)
		goto err;

	globalParams->setErrQuiet(gFalse);
	globalParams->setBaseDir(_);


	ds->pd = new PDFDoc(new GooString(filename), NULL, NULL, (void*)NULL);
	if (!ds->pd)
        	goto err;

	if (!ds->pd->isOk())
		goto err;

	if (!ds->od)
	{
        	GBool bitmapTopDown = gTrue;
        	ds->od = new SplashOutputDev(gSplashColorMode, /*4*/3, gFalse, gBgColor, bitmapTopDown,gFalse/*antialias*/);
	        if (ds->od)
			ds->od->startDoc(ds->pd->getXRef());
    	}
        if (!ds->od)
		goto err;

	i->dpi    = 72; /* FIXME */
	hDPI = (double)i->dpi* zoomReal * 0.01;
	vDPI = (double)i->dpi* zoomReal * 0.01;

	useMediaBox = gFalse;
	crop        = gTrue;
	doLinks     = gTrue;

	i->npages = ds->pd->getNumPages();
	if(page>=i->npages || page<0)goto err;
	
	ds->pd->displayPage(ds->od, pageNo, hDPI, vDPI, rotation, useMediaBox, crop, doLinks, NULL, NULL);

	if(!ds->pd) goto err;

	ds->bmp = ds->od->takeBitmap();
	if(!ds->bmp) goto err;

	i->width  = ds->bmp->getWidth();
	i->height = ds->bmp->getHeight();

	return ds;
err:

	if(ds->pd)		delete ds->pd ;
	if(ds->od)	delete ds->od ;
	if (globalParams)	delete globalParams;
	globalParams = NULL;
	if(ds)fim_free(ds);
	return NULL;
}

static void
pdf_read(unsigned char *dst, unsigned int line, void *data)
{
    	struct pdf_state_t *ds = (struct pdf_state_t*)data;
	if(!ds)return;

    	if(ds->first_row_dst == NULL)
    		ds->first_row_dst = dst;
	else return;

	memcpy(dst,ds->bmp->getDataPtr(),ds->bmp->getHeight()*ds->bmp->getWidth()*3);
}

static void
pdf_done(void *data)
{
    	struct pdf_state_t *ds = (struct pdf_state_t*)data;
	if(!ds) return;

	if(ds->pd)		delete ds->pd ;
	if(ds->od)	delete ds->od ;
	if (globalParams)	delete globalParams;
	globalParams = NULL;

	fim_free(ds);
}

/*
0000000: 2550 4446 2d31 2e34 0a25 d0d4 c5d8 0a35  %PDF-1.4.%.....5
*/
static struct ida_loader pdf_loader = {
    /*magic:*/ "%PDF-",// FI/*XME :*/ are sure this is enough ?
    /*moff:*/  0,
    /*mlen:*/  5,
    /*name:*/  "libpoppler",
    /*init:*/  pdf_init,
    /*read:*/  pdf_read,
    /*done:*/  pdf_done,
};

static void __init init_rd(void)
{
    load_register(&pdf_loader);
}

}
#endif // ifdef HAVE_LIBPOPPLER
