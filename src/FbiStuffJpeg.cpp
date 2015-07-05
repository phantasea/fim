/* $Id: FbiStuffJpeg.cpp 271 2009-12-13 00:03:48Z dezperado $ */
/*
 FbiStuffJpeg.cpp : fbi functions for JPEG files, modified for fim

 (c) 2007-2009 Michele Martone
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


#include <cstdio>
#include <cstdlib>

/*

#ifdef HAVE_STDLIB_H
//#undef HAVE_STDLIB_H
#define HAVE_STDLIB_H_BACKUP HAVE_STDLIB_H
#endif
*/


#include <cstring>
//#include <stddef.h>
//#include <errno.h>

#ifdef HAVE_LIBEXIF
# include <libexif/exif-data.h>
#endif

#include "FbiStuff.h"
#include "FbiStuffLoader.h"
//#include "loader.h"
//
extern "C"
{
// we require C linkage for these symbols
#include <jpeglib.h>
}


/*
	From fbi's misc.h :
*/

#define container_of(ptr, type, member) ({			\
        const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
        (type *)( (char *)__mptr - offsetof(type,member) );})

#define array_size(x) (sizeof(x)/sizeof(x[0]))

/*								*/

#ifdef USE_X11
# include <X11/Xlib.h>
# include <X11/Intrinsic.h>
# include <Xm/Xm.h>
# include <Xm/Text.h>
# include <Xm/SelectioB.h>
# include "RegEdit.h"
# include "ida.h"
# include "viewer.h"
#endif
namespace fim
{


   int fim_jerr=0;

/* ---------------------------------------------------------------------- */
/* load                                                                   */

struct jpeg_state {
    FILE * infile;                /* source file */
    
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPARRAY buffer;            /* Output row buffer */
    int row_stride,linelength;    /* physical row width in output buffer */
    unsigned char *image,*ptr;

    /* thumbnail */
    unsigned char  *thumbnail;
    unsigned int   tpos, tsize;
};

/* ---------------------------------------------------------------------- */
/* data source manager for thumbnail images                               */

/*static void thumbnail_src_init(struct jpeg_decompress_struct *cinfo)
{
    struct jpeg_state *h  = container_of(cinfo, struct jpeg_state, cinfo);
    cinfo->src->next_input_byte = h->thumbnail;
    cinfo->src->bytes_in_buffer = h->tsize;
}*/

static int thumbnail_src_fill(struct jpeg_decompress_struct *cinfo)
{
    FIM_FBI_PRINTF("jpeg: panic: no more thumbnail input data\n");
    exit(1);
}

static void thumbnail_src_skip(struct jpeg_decompress_struct *cinfo,
			       long num_bytes)
{
    cinfo->src->next_input_byte += num_bytes;
}

static void thumbnail_src_term(struct jpeg_decompress_struct *cinfo)
{
    /* nothing */
}

//!!
//see jpeg_init
   static struct jpeg_source_mgr thumbnail_mgr ;
/*
   static struct jpeg_source_mgr thumbnail_mgr = {
    .init_source         = thumbnail_src_init,
    .fill_input_buffer   = thumbnail_src_fill,
    .skip_input_data     = thumbnail_src_skip,
    .resync_to_restart   = jpeg_resync_to_restart,
    .term_source         = thumbnail_src_term,
};
*/
//!!


static void fim_error_exit (j_common_ptr cinfo)
{
	/*
		FIM : NEW, UNFINISHED (if used now, it would break things)
		read jpeglib's docs to finish this :)
	*/
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
//  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
//  longjmp(myerr->setjmp_buffer, 1);
   fim_jerr=1;
}


/* ---------------------------------------------------------------------- */
/* jpeg loader                                                            */

static void*
jpeg_init(FILE *fp, char *filename, unsigned int page,
	  struct ida_image_info *i, int thumbnail)
{
    struct jpeg_state *h;
    jpeg_saved_marker_ptr mark;
    fim_jerr=0;
    
    h = (struct jpeg_state *)fim_calloc(sizeof(*h),1);
    if(!h) goto oops;

    memset(h,0,sizeof(*h));
    h->infile = fp;

    h->jerr.error_exit=NULL; // ?
    h->cinfo.err = jpeg_std_error(&h->jerr);	/* FIXME : should use an error manager of ours (this one exits the program!) */
//    h->jerr.error_exit = fim_error_exit;	/* FIXME : should use an error manager of ours (this one exits the program!) */
    h->jerr.error_exit = NULL ;	/* FIXME : should use an error manager of ours (this one exits the program!) */
//    if(h->jerr.msg_code)goto oops;
    jpeg_create_decompress(&h->cinfo);
//    if(h->jerr.msg_code)goto oops;
    jpeg_save_markers(&h->cinfo, JPEG_COM,    0xffff); /* comment */
//    if(h->jerr.msg_code)goto oops;
    jpeg_save_markers(&h->cinfo, JPEG_APP0+1, 0xffff); /* EXIF */
//    if(h->jerr.msg_code)goto oops;
    jpeg_stdio_src(&h->cinfo, h->infile);
//    if(h->jerr.msg_code)goto oops;
//    if(jpeg_read_header(&h->cinfo, TRUE)==0)	goto oops;
    jpeg_read_header(&h->cinfo, TRUE);
//    if(h->jerr.msg_code)goto oops;	// this triggers with apparently good file

    for (mark = h->cinfo.marker_list; NULL != mark; mark = mark->next) {
	switch (mark->marker) {
	case JPEG_COM:
	    if (FbiStuff::fim_filereading_debug())
		FIM_FBI_PRINTF("jpeg: comment found (COM marker) [%.*s]\n",
			(int)mark->data_length, mark->data);
	    load_add_extra(i,EXTRA_COMMENT,mark->data,mark->data_length);
	    break;
	case JPEG_APP0 +1:
	    if (FbiStuff::fim_filereading_debug())
		FIM_FBI_PRINTF("jpeg: exif data found (APP1 marker)\n");
	    load_add_extra(i,EXTRA_COMMENT,mark->data,mark->data_length);

#ifdef HAVE_LIBEXIF
	    if (thumbnail) {
		ExifData *ed;
		
		ed = exif_data_new_from_data(mark->data,mark->data_length);
		if (ed->data &&
		    ed->data[0] == 0xff &&
		    ed->data[1] == 0xd8) {
		    if (FbiStuff::fim_filereading_debug())
			FIM_FBI_PRINTF("jpeg: exif thumbnail found\n");

		    /* save away thumbnail data */
		    h->thumbnail = fim_malloc(ed->size);
    		    if(!h->thumbnail) goto oops;
		    h->tsize = ed->size;
		    memcpy(h->thumbnail,ed->data,ed->size);
		}
		exif_data_unref(ed);
	    }
#endif
	    break;
	}
    }

    // !! 
    thumbnail_mgr.init_source         = /*thumbnail_src_init*/ NULL /* it is not useful, and breaks c++98 standard C++ compilation */;
    thumbnail_mgr.fill_input_buffer   = thumbnail_src_fill;
    thumbnail_mgr.skip_input_data     = thumbnail_src_skip;
    thumbnail_mgr.resync_to_restart   = jpeg_resync_to_restart;
    thumbnail_mgr.term_source         = thumbnail_src_term;
    // !! 

    if (h->thumbnail) {
	/* save image size */
	i->thumbnail   = 1;
	i->real_width  = h->cinfo.image_width;
	i->real_height = h->cinfo.image_height;

	/* re-setup jpeg */
	jpeg_destroy_decompress(&h->cinfo);
    //    if(h->jerr.msg_code)goto oops; // this triggers with apparently good files 
	fclose(h->infile);
	h->infile = NULL;
	jpeg_create_decompress(&h->cinfo);
  //      if(h->jerr.msg_code)goto oops;
	h->cinfo.src = &thumbnail_mgr;
	jpeg_read_header(&h->cinfo, TRUE);
//        if(h->jerr.msg_code)goto oops;
    }

    h->cinfo.out_color_space = JCS_RGB;
    jpeg_start_decompress(&h->cinfo);
//    if(h->jerr.msg_code)goto oops;
    i->width  = h->cinfo.image_width;
    i->height = h->cinfo.image_height;
    i->npages = 1;
    switch (h->cinfo.density_unit) {
    case 0: /* unknown */
	break;
    case 1: /* dot per inch */
	i->dpi = h->cinfo.X_density;
	break;
    case 2: /* dot per cm */
	i->dpi = res_cm_to_inch(h->cinfo.X_density);
	break;
    }

    return h;
oops:
std::cerr << "OOPS: problems decoding "<< filename <<"...\n";
    if( h && h->thumbnail) fim_free(h->thumbnail);
    if( h ) fim_free(h);
    return NULL;
}

static void
jpeg_read(unsigned char *dst, unsigned int line, void *data)
{
    struct jpeg_state *h = (struct jpeg_state*)data;
    JSAMPROW row = dst;
//    if(h->jerr.msg_code)goto oops;
    jpeg_read_scanlines(&h->cinfo, &row, 1);
    oops:
    return;
}

static void
jpeg_done(void *data)
{
    struct jpeg_state *h = (struct jpeg_state*)data;
    jpeg_destroy_decompress(&h->cinfo);
    if (h->infile)
	fclose(h->infile);
    if (h->thumbnail)
	fim_free(h->thumbnail);
    fim_free(h);
}

static struct ida_loader jpeg_loader = {
    /*magic:*/ "\xff\xd8",
    /*moff:*/  0,
    /*mlen:*/  2,
    /*name:*/  "libjpeg",
    /*init:*/  jpeg_init,
    /*read:*/  jpeg_read,
    /*done:*/  jpeg_done,
};

static void __init init_rd(void)
{
    load_register(&jpeg_loader);
}

#ifdef USE_X11

/* ---------------------------------------------------------------------- */
/* jpeg writer                                                            */

static Widget jpeg_shell;
static Widget jpeg_text;
static int jpeg_quality = 75;

static void
jpeg_button_cb(Widget widget, XtPointer clientdata, XtPointer call_data)
{
    XmSelectionBoxCallbackStruct *cb = call_data;

    if (XmCR_OK == cb->reason) {
	jpeg_quality = atoi(XmTextGetString(jpeg_text));
	do_save_print();
    }
    XtUnmanageChild(jpeg_shell);
}

static int
jpeg_conf(Widget parent, struct ida_image *img)
{
    char tmp[32];
    
    if (!jpeg_shell) {
	/* build dialog */
	jpeg_shell = XmCreatePromptDialog(parent,"jpeg",NULL,0);
	XmdRegisterEditres(XtParent(jpeg_shell));
	XtUnmanageChild(XmSelectionBoxGetChild(jpeg_shell,XmDIALOG_HELP_BUTTON));
        jpeg_text = XmSelectionBoxGetChild(jpeg_shell,XmDIALOG_TEXT);
	XtAddCallback(jpeg_shell,XmNokCallback,jpeg_button_cb,NULL);
	XtAddCallback(jpeg_shell,XmNcancelCallback,jpeg_button_cb,NULL);
    }
    sprintf(tmp,"%d",jpeg_quality);
    XmTextSetString(jpeg_text,tmp);
    XtManageChild(jpeg_shell);
    return 0;
}

static int
jpeg_write(FILE *fp, struct ida_image *img)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    unsigned char *line;
    unsigned int i;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, fp);
    cinfo.image_width  = img->i.width;
    cinfo.image_height = img->i.height;
    if (img->i.dpi) {
	cinfo.density_unit = 1;
	cinfo.X_density = img->i.dpi;
	cinfo.Y_density = img->i.dpi;
    }
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, jpeg_quality, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    for (i = 0, line = img->data; i < img->i.height; i++, line += img->i.width*3)
        jpeg_write_scanlines(&cinfo, &line, 1);
    
    jpeg_finish_compress(&(cinfo));
    jpeg_destroy_compress(&(cinfo));
    return 0;
}

struct ida_writer jpeg_writer = {
    /*l/*abel:*/*/  "JPEG",
   /*/* ext:*/*/    { "jpg", "jpeg", NULL},
    /*w/*rite:*/*/  jpeg_write,
    /*/*conf:*/*/   jpeg_conf,
};

static void __init init_wr(void)
{
    write_register(&jpeg_writer);
}


#endif
/*
#ifdef HAVE_STDLIB_H_BACKUP 
#define HAVE_STDLIB_H HAVE_STDLIB_H_BACKUP 
#undef HAVE_STDLIB_H_BACKUP 
#endif
*/
}
