/* $Id: FbiStuffGif.cpp 271 2009-12-13 00:03:48Z dezperado $ */
/*
 FbiStuffGif.cpp : fbi functions for GIF files, modified for fim

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gif_lib.h>

//#include "loader.h"
#include "FbiStuff.h"
#include "FbiStuffLoader.h"


namespace fim
{


struct gif_state {
    FILE         *infile;
    GifFileType  *gif;
    GifPixelType *row;
    GifPixelType *il;
    int w,h;
};

static GifRecordType
gif_fileread(struct gif_state *h)
{
    GifRecordType RecordType;
    GifByteType *Extension;
    int ExtCode, rc;
    const char *type;

    for (;;) {
	if (GIF_ERROR == DGifGetRecordType(h->gif,&RecordType)) {
	    if (FbiStuff::fim_filereading_debug())
		FIM_FBI_PRINTF("gif: DGifGetRecordType failed\n");
	    //PrintGifError();  //add by sim1: comment this line if compiling error
	    return (GifRecordType)-1;
	}
	switch (RecordType) {
	case IMAGE_DESC_RECORD_TYPE:
	    if (FbiStuff::fim_filereading_debug())
		FIM_FBI_PRINTF("gif: IMAGE_DESC_RECORD_TYPE found\n");
	    return RecordType;
	case EXTENSION_RECORD_TYPE:
	    if (FbiStuff::fim_filereading_debug())
		FIM_FBI_PRINTF("gif: EXTENSION_RECORD_TYPE found\n");
	    for (rc = DGifGetExtension(h->gif,&ExtCode,&Extension);
		 NULL != Extension;
		 rc = DGifGetExtensionNext(h->gif,&Extension)) {
		if (rc == GIF_ERROR) {
		    if (FbiStuff::fim_filereading_debug())
			FIM_FBI_PRINTF("gif: DGifGetExtension failed\n");
		    //PrintGifError();  //add by sim1: comment this line if compiling error
		    return (GifRecordType)-1;
		}
		if (FbiStuff::fim_filereading_debug()) {
		    switch (ExtCode) {
		    case COMMENT_EXT_FUNC_CODE:     type="comment";   break;
		    case GRAPHICS_EXT_FUNC_CODE:    type="graphics";  break;
		    case PLAINTEXT_EXT_FUNC_CODE:   type="plaintext"; break;
		    case APPLICATION_EXT_FUNC_CODE: type="appl";      break;
		    default:                        type="???";       break;
		    }
		    FIM_FBI_PRINTF("gif: extcode=0x%x [%s]\n",ExtCode,type);
		}
	    }
	    break;
	case TERMINATE_RECORD_TYPE:
	    if (FbiStuff::fim_filereading_debug())
		FIM_FBI_PRINTF("gif: TERMINATE_RECORD_TYPE found\n");
	    return RecordType;
	default:
	    if (FbiStuff::fim_filereading_debug())
		FIM_FBI_PRINTF("gif: unknown record type [%d]\n",RecordType);
	    return (GifRecordType)-1;
	}
    }
}

#if 0
static void
gif_skipimage(struct gif_state *h)
{
    unsigned char *line;
    int i;

    if (FbiStuff::fim_filereading_debug())
	FIM_FBI_PRINTF("gif: skipping image record ...\n");
    DGifGetImageDesc(h->gif);
    line = fim_malloc(h->gif->SWidth);
    for (i = 0; i < h->gif->SHeight; i++)
	DGifGetLine(h->gif, line, h->gif->SWidth);
    fim_free(line);
}
#endif

static void*
gif_init(FILE *fp, char *filename, unsigned int page,
	 struct ida_image_info *info, int thumbnail)
{
    struct gif_state *h;
    GifRecordType RecordType;
    int i, image = 0;
    int *perror = NULL;  //add by sim1: add this line if compiling error
    
    h = (gif_state*)fim_calloc(sizeof(*h),1);
    if(!h)goto oops;
    memset(h,0,sizeof(*h));

    h->infile = fp;
    //h->gif = DGifOpenFileHandle(fileno(fp));
    h->gif = DGifOpenFileHandle(fileno(fp), perror);  //add by sim1: use this line if compiling error
    h->row = (GifPixelType*)fim_malloc(h->gif->SWidth * sizeof(GifPixelType));
    if(!h->row)goto oops;

    while (0 == image) {
	RecordType = gif_fileread(h);
	switch (RecordType) {
	case IMAGE_DESC_RECORD_TYPE:
	    if (GIF_ERROR == DGifGetImageDesc(h->gif)) {
		if (FbiStuff::fim_filereading_debug())
		    FIM_FBI_PRINTF("gif: DGifGetImageDesc failed\n");
		//PrintGifError();  //add by sim1: comment this line if compiling error
	    }
	    if (NULL == h->gif->SColorMap &&
		NULL == h->gif->Image.ColorMap) {
		if (FbiStuff::fim_filereading_debug())
		    FIM_FBI_PRINTF("gif: oops: no colormap found\n");
		goto oops;
	    }
#if 0
	    info->width  = h->w = h->gif->SWidth;
	    info->height = h->h = h->gif->SHeight;
#else
	    info->width  = h->w = h->gif->Image.Width;
	    info->height = h->h = h->gif->Image.Height;
#endif
            info->npages = 1;
	    image = 1;
	    if (FbiStuff::fim_filereading_debug())
		FIM_FBI_PRINTF("gif: reading image record ...\n");
	    if (h->gif->Image.Interlace) {
		if (FbiStuff::fim_filereading_debug())
		    FIM_FBI_PRINTF("gif: interlaced\n");
		{
			h->il = (GifPixelType*)fim_malloc(h->w * h->h * sizeof(GifPixelType));
    			if(!h->il)goto oops;
		}
		for (i = 0; i < h->h; i += 8)
		    DGifGetLine(h->gif, h->il + h->w*i,h->w);
		for (i = 4; i < h->gif->SHeight; i += 8)
		    DGifGetLine(h->gif, h->il + h->w*i,h->w);
		for (i = 2; i < h->gif->SHeight; i += 4)
		    DGifGetLine(h->gif, h->il + h->w*i,h->w);
	    }
	    break;
	case TERMINATE_RECORD_TYPE:
	default:
	    goto oops;
	}
    }
    if (0 == info->width || 0 == info->height)
	goto oops;

    if (FbiStuff::fim_filereading_debug())
	FIM_FBI_PRINTF("gif: s=%dx%d i=%dx%d\n",
		h->gif->SWidth,h->gif->SHeight,
		h->gif->Image.Width,h->gif->Image.Height);
    return h;

 oops:
    if (FbiStuff::fim_filereading_debug())
	FIM_FBI_PRINTF("gif: fatal error, aborting\n");
    //DGifCloseFile(h->gif);
    DGifCloseFile(h->gif, perror);  //add by sim1: use this line if compiling error
    fclose(h->infile);
    if(h && h->il )fim_free(h->il );
    if(h && h->row)fim_free(h->row);
    if(h)fim_free(h);
    return NULL;
}

static void
gif_read(unsigned char *dst, unsigned int line, void *data)
{
    struct gif_state *h = (struct gif_state *) data;
    GifColorType *cmap;
    int x;
    
    if (h->gif->Image.Interlace) {
	if (line % 2) {
	    DGifGetLine(h->gif, h->row, h->w);
	} else {
	    memcpy(h->row, h->il + h->w * line, h->w);
	}
    } else {
	DGifGetLine(h->gif, h->row, h->w);
    }
    cmap = h->gif->Image.ColorMap ?
	h->gif->Image.ColorMap->Colors : h->gif->SColorMap->Colors;
    for (x = 0; x < h->w; x++) {
        dst[0] = cmap[h->row[x]].Red;
	dst[1] = cmap[h->row[x]].Green;
	dst[2] = cmap[h->row[x]].Blue;
	dst += 3;
    }
}

static void
gif_done(void *data)
{
    int *perror = NULL;  //add by sim1: add this line if compiling error
    struct gif_state *h = (struct gif_state *) data;

    if (FbiStuff::fim_filereading_debug())
	FIM_FBI_PRINTF("gif: done, cleaning up\n");
    //DGifCloseFile(h->gif);
    DGifCloseFile(h->gif, perror);  //add by sim1: use this line if compiling error
    fclose(h->infile);
    if (h->il)
	fim_free(h->il);
    fim_free(h->row);
    fim_free(h);
}

static struct ida_loader gif_loader = {
    /*magic:*/ "GIF",
    /*moff:*/  0,
    /*mlen:*/  3,
    /*name:*/  "libungif",
    /*init:*/  gif_init,
    /*read:*/  gif_read,
    /*done:*/  gif_done,
};

static void __init init_rd(void)
{
    load_register(&gif_loader);
}

}
