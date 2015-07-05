/* $Id: FramebufferDevice.cpp 269 2009-12-08 23:45:10Z dezperado $ */
/*
 FramebufferDevice.cpp : Linux Framebuffer functions from fbi, adapted for fim

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

#include "FbiStuffFbtools.h"
#include "FramebufferDevice.h"

#ifdef FIM_WITH_NO_FRAMEBUFFER
static void foo(){} /* let's make our compiler happy */
#else

#include <sys/user.h>	//  for PAGE_MASK (sometimes it is needed to include it here explicitly)
#if HAVE_LINUX_KD_H 
#include <linux/kd.h>	// KDGETMODE, KDSETMODE, KD_GRAPHICS, ...
#endif
#if HAVE_LINUX_VT_H 
#include <linux/vt.h>	// VT_GETSTATE, .. 
#endif
#include <sys/user.h>	// PAGE_MASK, ... 
#include <sys/mman.h>	// PROT_READ, PROT_WRITE, MAP_SHARED
#include <signal.h>
#include <sys/ioctl.h>

//#include <errno.h>
//#include <sys/ioctl.h>
//#include <sys/mman.h>
//#include <sys/wait.h>
//#include <sys/stat.h>

/*#include <asm/page.h>*/ /* seems like this gives problems */
#if 0
#include <signal.h>	  /* added by dez. missing when compiling with -ansi */
#include <asm/signal.h>	  /* added by dez. missing when compiling with -ansi */
#endif


namespace fim
{

#define	FIM_DEBUGGING_FOR_ARM_WITH_VITALY 0
/*
   this code will be enabled by default if we can make sure it
   won't break often with kernel updates */
#if	FIM_DEBUGGING_FOR_ARM_WITH_VITALY

static void print_vinfo(struct fb_var_screeninfo *vinfo)
{
	FIM_FPRINTF(stderr,  "Printing vinfo:\n");
	FIM_FPRINTF(stderr,  "\txres: %d\n", vinfo->xres);
	FIM_FPRINTF(stderr,  "\tyres: %d\n", vinfo->yres);
	FIM_FPRINTF(stderr,  "\txres_virtual: %d\n", vinfo->xres_virtual);
	FIM_FPRINTF(stderr,  "\tyres_virtual: %d\n", vinfo->yres_virtual);
	FIM_FPRINTF(stderr,  "\txoffset: %d\n", vinfo->xoffset);
	FIM_FPRINTF(stderr,  "\tyoffset: %d\n", vinfo->yoffset);
	FIM_FPRINTF(stderr,  "\tbits_per_pixel: %d\n", vinfo->bits_per_pixel);
	FIM_FPRINTF(stderr,  "\tgrayscale: %d\n", vinfo->grayscale);
	FIM_FPRINTF(stderr,  "\tnonstd: %d\n", vinfo->nonstd);
	FIM_FPRINTF(stderr,  "\tactivate: %d\n", vinfo->activate);
	FIM_FPRINTF(stderr,  "\theight: %d\n", vinfo->height);
	FIM_FPRINTF(stderr,  "\twidth: %d\n", vinfo->width);
	FIM_FPRINTF(stderr,  "\taccel_flags: %d\n", vinfo->accel_flags);
	FIM_FPRINTF(stderr,  "\tpixclock: %d\n", vinfo->pixclock);
	FIM_FPRINTF(stderr,  "\tleft_margin: %d\n", vinfo->left_margin);
	FIM_FPRINTF(stderr,  "\tright_margin: %d\n", vinfo->right_margin);
	FIM_FPRINTF(stderr,  "\tupper_margin: %d\n", vinfo->upper_margin);
	FIM_FPRINTF(stderr,  "\tlower_margin: %d\n", vinfo->lower_margin);
	FIM_FPRINTF(stderr,  "\thsync_len: %d\n", vinfo->hsync_len);
	FIM_FPRINTF(stderr,  "\tvsync_len: %d\n", vinfo->vsync_len);
	FIM_FPRINTF(stderr,  "\tsync: %d\n", vinfo->sync);
	FIM_FPRINTF(stderr,  "\tvmode: %d\n", vinfo->vmode);
	FIM_FPRINTF(stderr,  "\tred: %d/%d\n", vinfo->red.length, vinfo->red.offset);
	FIM_FPRINTF(stderr,  "\tgreen: %d/%d\n", vinfo->green.length, vinfo->green.offset);
	FIM_FPRINTF(stderr,  "\tblue: %d/%d\n", vinfo->blue.length, vinfo->blue.offset);
	FIM_FPRINTF(stderr,  "\talpha: %d/%d\n", vinfo->transp.length, vinfo->transp.offset);
}

static void print_finfo(struct fb_fix_screeninfo *finfo)
{
	FIM_FPRINTF(stderr,  "Printing finfo:\n");
	FIM_FPRINTF(stderr,  "\tsmem_start = %p\n", (char *)finfo->smem_start);
	FIM_FPRINTF(stderr,  "\tsmem_len = %d\n", finfo->smem_len);
	FIM_FPRINTF(stderr,  "\ttype = %d\n", finfo->type);
	FIM_FPRINTF(stderr,  "\ttype_aux = %d\n", finfo->type_aux);
	FIM_FPRINTF(stderr,  "\tvisual = %d\n", finfo->visual);
	FIM_FPRINTF(stderr,  "\txpanstep = %d\n", finfo->xpanstep);
	FIM_FPRINTF(stderr,  "\typanstep = %d\n", finfo->ypanstep);
	FIM_FPRINTF(stderr,  "\tywrapstep = %d\n", finfo->ywrapstep);
	FIM_FPRINTF(stderr,  "\tline_length = %d\n", finfo->line_length);
	FIM_FPRINTF(stderr,  "\tmmio_start = %p\n", (char *)finfo->mmio_start);
	FIM_FPRINTF(stderr,  "\tmmio_len = %d\n", finfo->mmio_len);
	FIM_FPRINTF(stderr,  "\taccel = %d\n", finfo->accel);
}
#endif

#define DITHER_LEVEL 8

typedef unsigned long vector[DITHER_LEVEL];
typedef vector  matrix[DITHER_LEVEL];



//#if DITHER_LEVEL == 8 ||  DITHER_LEVEL == 4
//static int matrix   DM ;
//#endif
#if DITHER_LEVEL == 8
#define DITHER_MASK 7
static matrix   DM =
{
    {0, 32, 8, 40, 2, 34, 10, 42},
    {48, 16, 56, 24, 50, 18, 58, 26},
    {12, 44, 4, 36, 14, 46, 6, 38},
    {60, 28, 52, 20, 62, 30, 54, 22},
    {3, 35, 11, 43, 1, 33, 9, 41},
    {51, 19, 59, 27, 49, 17, 57, 25},
    {15, 47, 7, 39, 13, 45, 5, 37},
    {63, 31, 55, 23, 61, 29, 53, 21}
};

#endif

#if DITHER_LEVEL == 4
#define DITHER_MASK 3
static matrix   DM =
{
    {0, 8, 2, 10},
    {12, 4, 14, 6},
    {3, 11, 1, 9},
    {15, 7, 13, 5}
};

#endif




static FramebufferDevice *ffdp;

static void _fb_switch_signal(int signal)
{
	// WARNING : THIS IS A DIRTY HACK 
	// necessary to enable framebuffer console switching
	ffdp->fb_switch_signal(signal);
}

int FramebufferDevice::fs_puts(struct fs_font *f, unsigned int x, unsigned int y, const unsigned char *str)
{
    unsigned char *pos,*start;
    int i,c,j,w;

    pos  = fb_mem+fb_mem_offset;
    pos += fb_fix.line_length * y;
    for (i = 0; str[i] != '\0'; i++) {
	c = str[i];
	if (NULL == f->eindex[c])
	    continue;
	/* clear with bg color */
	start = pos + x*fs_bpp + f->fontHeader.max_bounds.descent * fb_fix.line_length;
	w = (f->eindex[c]->width+1)*fs_bpp;
#ifdef FIM_IS_SLOWER_THAN_FBI
	for (j = 0; j < f->height; j++) {
/////	    memset_combine(start,0x20,w);
	    memset(start,0,w);
	    start += fb_fix.line_length;
	}
#else
	//sometimes we can gather multiple calls..
	if(fb_fix.line_length==(unsigned int)w)
	{
		//contiguous case
		memset(start,0,w*f->height);
	    	start += fb_fix.line_length*f->height;
	}
	else
	for (j = 0; j < f->height; j++) {
	    memset(start,0,w);
	    start += fb_fix.line_length;
	}
#endif
	/* draw char */
	start = pos + x*fs_bpp + fb_fix.line_length * (f->height-f->eindex[c]->ascent);
	fs_render_fb(start,fb_fix.line_length,f->eindex[c],f->gindex[c]);
	x += f->eindex[c]->width;
	if (x > fb_var.xres - f->width)
	    return -1;
    }
    return x;
}

void FramebufferDevice::fs_render_fb(unsigned char *ptr, int pitch, FSXCharInfo *charInfo, unsigned char *data)
{

/* 
 * These preprocessor macros should serve *only* for font handling purposes.
 * */
#define BIT_ORDER       BitmapFormatBitOrderMSB
#ifdef BYTE_ORDER
#undef BYTE_ORDER
#endif
#define BYTE_ORDER      BitmapFormatByteOrderMSB
#define SCANLINE_UNIT   BitmapFormatScanlineUnit8
#define SCANLINE_PAD    BitmapFormatScanlinePad8
#define EXTENTS         BitmapFormatImageRectMin

#define SCANLINE_PAD_BYTES 1
#define GLWIDTHBYTESPADDED(bits, nBytes)                                    \
        ((nBytes) == 1 ? (((bits)  +  7) >> 3)          /* pad to 1 byte  */\
        :(nBytes) == 2 ? ((((bits) + 15) >> 3) & ~1)    /* pad to 2 bytes */\
        :(nBytes) == 4 ? ((((bits) + 31) >> 3) & ~3)    /* pad to 4 bytes */\
        :(nBytes) == 8 ? ((((bits) + 63) >> 3) & ~7)    /* pad to 8 bytes */\
        : 0)

    int row,bit,bpr,x;

    bpr = GLWIDTHBYTESPADDED((charInfo->right - charInfo->left),
			     SCANLINE_PAD_BYTES);
    for (row = 0; row < (charInfo->ascent + charInfo->descent); row++) {
	for (x = 0, bit = 0; bit < (charInfo->right - charInfo->left); bit++) {
	    if (data[bit>>3] & fs_masktab[bit&7])
		// WARNING !
		fs_setpixel(ptr+x,fs_white);
	    x += fs_bpp;
	}
	data += bpr;
	ptr += pitch;
    }

#undef BIT_ORDER
#undef BYTE_ORDER
#undef SCANLINE_UNIT
#undef SCANLINE_PAD
#undef EXTENTS
#undef SCANLINE_PAD_BYTES
#undef GLWIDTHBYTESPADDED
}


	int FramebufferDevice::framebuffer_init()
	{
		int rc=0;
		//initialization of the framebuffer text
		FontServer::fb_text_init1(fontname,&f);	// FIXME : move this outta here
		/*
		 * will initialized with the user set (or default ones)
		 *  - framebuffer device
		 *  - framebuffer mode
		 *  - virtual terminal
		 * */
		fd = fb_init(fbdev, fbmode, vt);
		if(fd==-1)
			fd = fb_init(fbdev, fbmode, vt,0xbabebabe==0xbabebabe);//maybe we are under screen..
		if(fd==-1)
			if(fd==-1)exit(1);
			//return -1;//this is a TEMPORARY and DEAF,DUMB, AND BLIND bug noted by iam
		//setting signals to handle in the right ways signals
		fb_catch_exit_signals();
		fb_switch_init();
		/*
		 * C-z is inhibited now (for framebuffer's screen safety!)
		 */
		signal(SIGTSTP,SIG_IGN);
		//signal(SIGSEGV,cleanup();
		//set text color to white ?
		
		//initialization of the framebuffer device handlers
		if((rc=fb_text_init2()))return rc;
	
			switch (fb_var.bits_per_pixel) {
		case 8:
			svga_dither_palette(8, 8, 4);
			dither = TRUE;
			init_dither(8, 8, 4, 2);
			break;
		case 15:
	    	case 16:
	        	if (fb_fix.visual == FB_VISUAL_DIRECTCOLOR)
	        	    linear_palette(5);
			if (fb_var.green.length == 5) {
			    lut_init(15);
			} else {
			    lut_init(16);
			}
			break;
		case 24:
	        	if (fb_fix.visual == FB_VISUAL_DIRECTCOLOR)
	      	      linear_palette(8);
			break;
		case 32:
	        	if (fb_fix.visual == FB_VISUAL_DIRECTCOLOR)
	          	  linear_palette(8);
			lut_init(24);
			break;
		default:
			FIM_FPRINTF(stderr,  "Oops: %i bit/pixel ???\n",
				fb_var.bits_per_pixel);
			std::exit(1);
	    	}
	    	if (fb_fix.visual == FB_VISUAL_DIRECTCOLOR ||
			fb_var.bits_per_pixel == 8)
		{
			if (-1 == ioctl(fd,FBIOPUTCMAP,&cmap)) {
		    		perror("ioctl FBIOPUTCMAP");
			    std::exit(1);
			}
		}
		return 0;
	}

void FramebufferDevice::dev_init(void)
{
    struct stat dummy;

    if (NULL != devices)
	return;
    if (0 == stat("/dev/.devfsd",&dummy))
	devices = &devs_devfs;
    else
	devices = &devs_default;

}


void FramebufferDevice::console_switch(int is_busy)
{
	//FIXME
	switch (fb_switch_state) {
	case FB_REL_REQ:
		fb_switch_release();
	case FB_INACTIVE:
		visible = 0;
	break;
	case FB_ACQ_REQ:
		fb_switch_acquire();
	case FB_ACTIVE:
		//when stepping in console..
		visible = 1;
		ioctl(fd,FBIOPAN_DISPLAY,&fb_var);
		redraw = 1;
	/*
	 * thanks to the next line, the image is redrawn each time 
	 * the console is switched! 
	 */
		mc.cc.redisplay();
	//if (is_busy) status("busy, please wait ...", NULL);		
	break;
	default:
	break;
    	}
	switch_last = fb_switch_state;
	return;
}

//void FramebufferDevice::svga_display_image_new(struct ida_image *img, int xoff, int yoff,unsigned int bx,unsigned int bw,unsigned int by,unsigned int bh,int mirror,int flip)
void FramebufferDevice::svga_display_image_new(
	struct ida_image *img,
	int yoff,
	int xoff,
		int irows,int icols,// rows and columns in the input image
		int icskip,	// input columns to skip for each line
	unsigned int by,
	unsigned int bx,
	unsigned int bh,
	unsigned int bw,
		int ocskip,// output columns to skip for each line
	int flags)
{
/*	bx is the box's x origin
 *	by is the box's y origin
 *	bw is the box's width
 *	bh is the box's heigth
 * */

	/*
	 * WARNING : SHOULD ASSeRT BX+BW < FB_VAR.XReS ..!!
	 * */
    unsigned int     dwidth  = MIN(img->i.width,  bw);
    unsigned int     dheight = MIN(img->i.height, bh);
    unsigned int     data, video, bank, offset, bytes, y;
    int yo=(bh-dheight)/2;
    int xo=(bw-dwidth )/2;
    int cxo=bw-dwidth-xo;
    //int cyo=bh-yo;
    int mirror=flags&FIM_FLAG_MIRROR, flip=flags&FIM_FLAG_FLIP;
    if (!visible)/*COMMENT THIS IF svga_display_image IS NOT IN A CYCLE*/
	return;
    /*fb_clear_screen();//EXPERIMENTAL
    if(xoff&&yoff)clear_rect(0,xoff,0,yoff);*/

    bytes = FB_BPP;

    /* offset for image data (image > screen, select visible area) */
    offset = (yoff * img->i.width + xoff) * 3;
    
    /* offset for video memory (image < screen, center image) */
    video = 0, bank = 0;
    video += FB_BPP * (bx);
    if (img->i.width < bw)
    {	    
	    video += FB_BPP * (xo);
    }
    
    video += fb_fix.line_length * (by);
    if (img->i.height < bh )
    {	   
	    video += fb_fix.line_length * (yo);
    }

    if (dheight < bh ) 
    {	    
    	/* clear by lines */
#ifdef FIM_FASTER_CLEARLINES
	if(bw==fb_var.xres && bx==0)
	{
		/*
		 * +------------------------------+
		 * | whole screen line clear join |
		 * +------------------------------+
		 */
		// wide screen clear
		{ clear_line(FB_BPP, by, bw*(bh), FB_MEM(bx,by)); }
		
		//top and bottom lines clear : maybe better
		//{ clear_line(FB_BPP, by, bw*(yo), FB_MEM(bx,by)); }
		//{ clear_line(FB_BPP, by+yo, bw*(dheight), FB_MEM(bx,by+yo)); }
		//{ clear_line(FB_BPP, by+dheight+yo, bw*(bh-yo-dheight), FB_MEM(bx,by+yo+dheight)); }
	}
	else
#endif
	{
	    	for ( y = by; y < by+yo;++y) { clear_line(FB_BPP, y, bw, FB_MEM(bx,y)); }
		for ( y = by+dheight+yo; y < by+bh;++y) { clear_line(FB_BPP, y, bw, FB_MEM(bx,y)); }
	}
    }

    if (dwidth < bw )
    {	    
#ifdef FIM_FASTER_CLEARLINES
    	    if(bw==fb_var.xres && bx==0)
	    {
	    	if (dheight >= bh ) 
			clear_line(FB_BPP, by, bw*(bh), FB_MEM(bx,by));
	    }
	    else
#endif
    	    for ( y = by; y < by+bh;++y)
	    {
		    clear_line(FB_BPP, y, xo, FB_MEM(bx,y));
		    clear_line(FB_BPP, y, cxo,FB_MEM(bx+xo+dwidth,y));
	    }
    }
    /*for ( y = 0; y < fb_var.yres;y+=100)fb_line(0, fb_var.xres, y, y);*/

    /* go ! */
    /*flip patch*/
#ifndef min
#define min(x,y) ((x)<(y)?(x):(y))
#endif
    int fb_fix_line_length=FB_MEM_LINE_OFFSET;
    if(flip) {	fb_fix_line_length*=-1; video += (min(img->i.height,dheight)-1)*(fb_fix.line_length);}
    /*flip patch*/
    /* FIXME : COMPLETE ME ... */

#ifndef FIM_IS_SLOWER_THAN_FBI
    unsigned char *(FramebufferDevice::*convert_line_f)(int , int , int , char unsigned *, char unsigned *, int );
    if(fb_var.bits_per_pixel==8)
 	   convert_line_f=&fim::FramebufferDevice::convert_line_8;
    else
 	   convert_line_f=&fim::FramebufferDevice::convert_line;
#endif

    for (data = 0, y = by;
	 data < img->i.width * img->i.height * 3
	     && data / img->i.width / 3 < dheight;
	 data += img->i.width * 3, video += fb_fix_line_length)
    {
#ifndef FIM_IS_SLOWER_THAN_FBI
	(this->*convert_line_f)
#else
	convert_line
#endif
	(fb_var.bits_per_pixel, y++, dwidth,
		     fb_mem+video, img->data + data + offset,mirror);/*<- mirror patch*/
    }
}

int FramebufferDevice::fb_init(const char *device, char *mode, int vt, int try_boz_patch)
{
    /*
     * This method will probe for a valid framebuffer device.
     *
     * The try_boz_patch will make fim go straight ahead ignoring lots of errors.
     * Like the ones when running fim under screen.
     * Like the ones when running fim under X. :)
     * */
    char   fbdev[16];
    struct vt_stat vts;

    dev_init();
    tty = 0;
    if (vt != 0)
	fb_setvt(vt);

#ifdef FIM_BOZ_PATCH
    if(!try_boz_patch)
#endif
    if (-1 == ioctl(tty,VT_GETSTATE, &vts)) {
	FIM_FPRINTF(stderr, "ioctl VT_GETSTATE: %s (not a linux console?)\n",
		strerror(errno));
	return -1;
//	exit(1);
    }
    
    /* no device supplied ? we will probe for one */
    if (NULL == device) {
	device = fim_getenv("FRAMEBUFFER");
	/* no environment - supplied device ? */
	if (NULL == device) {
	    struct fb_con2fbmap c2m;
	    if (-1 == (fb = open(devices->fb0,O_RDWR /* O_WRONLY */,0))) {
		FIM_FPRINTF(stderr, "open %s: %s\n",devices->fb0,strerror(errno));
		exit(1);
	    }
	    c2m.console = vts.v_active;
#ifdef FIM_BOZ_PATCH
    if(!try_boz_patch){
#endif
	    if (-1 == ioctl(fb, FBIOGET_CON2FBMAP, &c2m)) {
		perror("ioctl FBIOGET_CON2FBMAP");
		exit(1);
	    }
	    close(fb);
/*	    FIM_FPRINTF(stderr, "map: vt%02d => fb%d\n",
		    c2m.console,c2m.framebuffer);*/
	    sprintf(fbdev,devices->fbnr,c2m.framebuffer);
	    device = fbdev;
#ifdef FIM_BOZ_PATCH
    	    }
    else
	    device = "/dev/fb0";
#endif
	}
    }

    /* get current settings (which we have to restore) */
    if (-1 == (fb = open(device,O_RDWR /* O_WRONLY */))) {
	FIM_FPRINTF(stderr, "open %s: %s\n",device,strerror(errno));
	exit(1);
    }
    if (-1 == ioctl(fb,FBIOGET_VSCREENINFO,&fb_ovar)) {
	perror("ioctl FBIOGET_VSCREENINFO");
	exit(1);
    }
#if	FIM_DEBUGGING_FOR_ARM_WITH_VITALY
	print_vinfo(&fb_ovar);
#endif
    if (-1 == ioctl(fb,FBIOGET_FSCREENINFO,&fb_fix)) {
	perror("ioctl FBIOGET_FSCREENINFO");
	exit(1);
    }
#if	FIM_DEBUGGING_FOR_ARM_WITH_VITALY
	print_finfo(&fb_fix);
#endif
    if (fb_ovar.bits_per_pixel == 8 ||
	fb_fix.visual == FB_VISUAL_DIRECTCOLOR) {
	if (-1 == ioctl(fb,FBIOGETCMAP,&ocmap)) {
	    perror("ioctl FBIOGETCMAP");
	    exit(1);
	}
    }
#ifdef FIM_BOZ_PATCH
    if(!try_boz_patch)
#endif
    if (-1 == ioctl(tty,KDGETMODE, &kd_mode)) {
	perror("ioctl KDGETMODE");
	exit(1);
    }
#ifdef FIM_BOZ_PATCH
    if(!try_boz_patch)
#endif
    if (-1 == ioctl(tty,VT_GETMODE, &vt_omode)) {
	perror("ioctl VT_GETMODE");
	exit(1);
    }
    tcgetattr(tty, &term);
    
    /* switch mode */
    if(-1 == fb_setmode(mode)){
#if 0
	/* 
	 * FIXME:
	 * mm's strict mode ckecking (right now, this function triggers an exit() but things should change) */
#ifdef FIM_BOZ_PATCH
    	if(!try_boz_patch)
#endif
	{
		perror("failed setting mode");
		exit(1);
	}
#endif
    }

    
    /* checks & initialisation */
    if (-1 == ioctl(fb,FBIOGET_FSCREENINFO,&fb_fix)) {
	perror("ioctl FBIOGET_FSCREENINFO");
	exit(1);
    }
    if (fb_fix.type != FB_TYPE_PACKED_PIXELS) {
	FIM_FPRINTF(stderr, "can handle only packed pixel frame buffers\n");
	goto err;
    }
#if 0
    switch (fb_var.bits_per_pixel) {
    case 8:
	white = 255; black = 0; bpp = 1;
	break;
    case 15:
    case 16:
	if (fb_var.green.length == 6)
	    white = 0xffff;
	else
	    white = 0x7fff;
	black = 0; bpp = 2;
	break;
    case 24:
	white = 0xffffff; black = 0; bpp = fb_var.bits_per_pixel/8;
	break;
    case 32:
	white = 0xffffff; black = 0; bpp = fb_var.bits_per_pixel/8;
	fb_setpixels = fb_setpixels4;
	break;
    default:
	FIM_FPRINTF(stderr,  "Oops: %i bit/pixel ???\n",
		fb_var.bits_per_pixel);
	goto err;
    }
#endif
#ifdef PAGE_MASK
    fb_mem_offset = (unsigned long)(fb_fix.smem_start) & (~PAGE_MASK);
#else
    /* some systems don't have this symbol outside their kernel headers - will do any harm ? */
    /* FIXME : what are the wider implications of this ? */
    fb_mem_offset = 0;
#endif
    fb_mem = (unsigned char*) mmap(NULL,fb_fix.smem_len+fb_mem_offset,
		  PROT_READ|PROT_WRITE,MAP_SHARED,fb,0);
    /*
     * FIXME : this is not 64 bits safe
     * */
    if (-1L == (long)fb_mem) {
	perror("mmap failed");
	goto err;
    }
    /* move viewport to upper left corner */
    if (fb_var.xoffset != 0 || fb_var.yoffset != 0) {
	fb_var.xoffset = 0;
	fb_var.yoffset = 0;
	if (-1 == ioctl(fb,FBIOPAN_DISPLAY,&fb_var)) {
	    perror("ioctl FBIOPAN_DISPLAY");
	    goto err;
	}
    }
#ifdef FIM_BOZ_PATCH
    if(!try_boz_patch)
#endif
    if (-1 == ioctl(tty,KDSETMODE, KD_GRAPHICS)) {
	perror("ioctl KDSETMODE");
	goto err;
    }
    fb_activate_current(tty);

    /* cls */
    fb_memset(fb_mem+fb_mem_offset,0,fb_fix.smem_len);

#ifdef FIM_BOZ_PATCH
    with_boz_patch=try_boz_patch;
#endif
    return fb;

 err:
    cleanup();
    exit(1);
}

void FramebufferDevice::fb_memset (void *addr, int c, size_t len)
{
#if 1 /* defined(__powerpc__) */
    unsigned int i;
    
    i = (c & 0xff) << 8;
    i |= i << 16;
    len >>= 2;	/* FIXME : WHY ? */
#ifdef FIM_IS_SLOWER_THAN_FBI
    unsigned int *p;
    for (p = (unsigned int*) addr; len--; p++)
	*p = i;
#else
    memset(addr, i, len );
#endif
#else
    memset(addr, c, len);
#endif
}

void FramebufferDevice::fb_setvt(int vtno)
{
    struct vt_stat vts;
    char vtname[12];
    
    if (vtno < 0) {
	if (-1 == ioctl(tty,VT_OPENQRY, &vtno) || vtno == -1) {
	    perror("ioctl VT_OPENQRY");
	    exit(1);
	}
    }

    vtno &= 0xff;
    sprintf(vtname, devices->ttynr, vtno);
    if ( chown(vtname, getuid(), getgid())){
	FIM_FPRINTF(stderr, "chown %s: %s\n",vtname,strerror(errno));
	exit(1);
    }
    if (-1 == access(vtname, R_OK | W_OK)) {
	FIM_FPRINTF(stderr, "access %s: %s\n",vtname,strerror(errno));
	exit(1);
    }
    switch (fork()) {
    case 0:
	break;
    case -1:
	perror("fork");
	exit(1);
    default:
	exit(0);
    }
    close(tty);
    close(0);
    close(1);
    close(2);
    setsid();
    open(vtname,O_RDWR);
    int ndd;/* FIXME : on some systems, we get 'int dup(int)', declared with attribute warn_unused_result */
    ndd=dup(0);
    ndd=dup(0);

#ifdef FIM_BOZ_PATCH
    if(!with_boz_patch)
#endif
    if (-1 == ioctl(tty,VT_GETSTATE, &vts)) {
	perror("ioctl VT_GETSTATE");
	exit(1);
    }
    orig_vt_no = vts.v_active;
    if (-1 == ioctl(tty,VT_ACTIVATE, vtno)) {
	perror("ioctl VT_ACTIVATE");
	exit(1);
    }
    if (-1 == ioctl(tty,VT_WAITACTIVE, vtno)) {
	perror("ioctl VT_WAITACTIVE");
	exit(1);
    }
}

int FramebufferDevice::fb_setmode(char *name)
{
    FILE *fp;
    char line[80],label[32],value[16];
    int  geometry=0, timings=0;
 
    /* load current values */
    if (-1 == ioctl(fb,FBIOGET_VSCREENINFO,&fb_var)) {
	perror("ioctl FBIOGET_VSCREENINFO");
	exit(1);
    }
    

#if 0
#ifdef FIM_WANTS_DOUBLE_BUFFERING
            /* FIXME : the page flipping mechanisms missing (unfinished)
             * please note that in addition to this code, we should also
             * specify timing parameters.
             * The following is experimental code only.
             * Note that on the developement machine neither fbset does work!
	     */
            fb_var.xres_virtual = fb_var.xres ;
            fb_var.yres_virtual = fb_var.yres ;
            fb_var.yres_virtual = fb_var.yres * 2;
	    //printf("%d %d %d %d\n", fb_var.xres_virtual,fb_var.xres, fb_var.yres_virtual,fb_var.yres);
	    fb_var.activate = FB_ACTIVATE_NOW;
            fb_var.accel_flags = 0;
            /* ... */
	    if (-1 == ioctl(fb,FBIOPUT_VSCREENINFO,&fb_var))
		perror("ioctl FBIOPUT_VSCREENINFO"),printf("!failed\n");
	    printf("%d %d %d %d\n", fb_var.xres_virtual,fb_var.xres, fb_var.yres_virtual,fb_var.yres);
#endif
#endif
    /* name="640x480-72"; */

    if (NULL == name)
	return -1;
    if (NULL == (fp = fopen("/etc/fb.modes","r")))
	return -1;
    while (NULL != fgets(line,79,fp)) {
	if (1 == sscanf(line, "mode \"%31[^\"]\"",label) &&
	    0 == strcmp(label,name)) {
	    /* fill in new values */
	    fb_var.sync  = 0;
	    fb_var.vmode = 0;
	    while (NULL != fgets(line,79,fp) &&
		   NULL == strstr(line,"endmode")) {
//		if (5 == sscanf(line," geometry %d %d %d %d %d",
		if (5 == sscanf(line," geometry %u %u %u %u %u",
				&fb_var.xres,&fb_var.yres,
				&fb_var.xres_virtual,&fb_var.yres_virtual,
				&fb_var.bits_per_pixel))
		    geometry = 1;
//		if (7 == sscanf(line," timings %d %d %d %d %d %d %d",
		if (7 == sscanf(line," timings %u %u %u %u %u %u %u",
				&fb_var.pixclock,
				&fb_var.left_margin,  &fb_var.right_margin,
				&fb_var.upper_margin, &fb_var.lower_margin,
				&fb_var.hsync_len,    &fb_var.vsync_len))
		    timings = 1;
		if (1 == sscanf(line, " hsync %15s",value) &&
		    0 == strcasecmp(value,"high"))
		    fb_var.sync |= FB_SYNC_HOR_HIGH_ACT;
		if (1 == sscanf(line, " vsync %15s",value) &&
		    0 == strcasecmp(value,"high"))
		    fb_var.sync |= FB_SYNC_VERT_HIGH_ACT;
		if (1 == sscanf(line, " csync %15s",value) &&
		    0 == strcasecmp(value,"high"))
		    fb_var.sync |= FB_SYNC_COMP_HIGH_ACT;
		if (1 == sscanf(line, " extsync %15s",value) &&
		    0 == strcasecmp(value,"true"))
		    fb_var.sync |= FB_SYNC_EXT;
		if (1 == sscanf(line, " laced %15s",value) &&
		    0 == strcasecmp(value,"true"))
		    fb_var.vmode |= FB_VMODE_INTERLACED;
		if (1 == sscanf(line, " double %15s",value) &&
		    0 == strcasecmp(value,"true"))
		    fb_var.vmode |= FB_VMODE_DOUBLE;
	    }
	    /* ok ? */
	    if (!geometry || !timings)
		return -1;
	    /* set */
	    fb_var.xoffset = 0;
	    fb_var.yoffset = 0;

#if 0
#ifdef FIM_WANTS_DOUBLE_BUFFERING
            // FIXME : the page flipping mechanisms missing (unfinished)
            fb_var.xres_virtual = fb_var.xres;
            fb_var.yres_virtual = fb_var.yres * 2;
#endif
#endif

	    if (-1 == ioctl(fb,FBIOPUT_VSCREENINFO,&fb_var))
		perror("ioctl FBIOPUT_VSCREENINFO");

	    /*
	     * FIXME
	     * mm : this should be placed here and uncommented : */
	    /*
	    if (-1 == ioctl(fb,FBIOGET_FSCREENINFO,&fb_fix)) {
		perror("ioctl FBIOGET_VSCREENINFO");
		exit(1);
	    }*/
	    /* look what we have now ... */
	    if (-1 == ioctl(fb,FBIOGET_VSCREENINFO,&fb_var)) {
		perror("ioctl FBIOGET_VSCREENINFO");
		exit(1);
	    }
	    return 0;
	}
    }
    return -1;
}

int FramebufferDevice::fb_activate_current(int tty)
{
/* Hmm. radeonfb needs this. matroxfb doesn't. (<- fbi comment) */
    struct vt_stat vts;
    
#ifdef FIM_BOZ_PATCH
    if(!with_boz_patch)
#endif
    if (-1 == ioctl(tty,VT_GETSTATE, &vts)) {
	perror("ioctl VT_GETSTATE");
	return -1;
    }
#ifdef FIM_BOZ_PATCH
    if(!with_boz_patch)
#endif
    if (-1 == ioctl(tty,VT_ACTIVATE, vts.v_active)) {
	perror("ioctl VT_ACTIVATE");
	return -1;
    }
#ifdef FIM_BOZ_PATCH
    if(!with_boz_patch)
#endif
    if (-1 == ioctl(tty,VT_WAITACTIVE, vts.v_active)) {
	perror("ioctl VT_WAITACTIVE");
	return -1;
    }
    return 0;
}

int FramebufferDevice::status_line(const unsigned char *msg)
{
    int y;
    
    if (!visible)
	goto ret;

    if(fb_var.yres< 1 + f->height + ys)
	/* we need enough pixels, and have no assumptions on weird visualization devices */
	goto rerr;

    y = fb_var.yres -1 - f->height - ys;
//    fb_memset(fb_mem + fb_fix.line_length * y, 0, fb_fix.line_length * (f->height+ys));
    clear_rect(0, fb_var.xres-1, y+1,y+f->height+ys);

    fb_line(0, fb_var.xres, y, y);
    fs_puts(f, 0, y+ys, msg);
ret:
    return 0;
rerr:
    return -1;
}

void FramebufferDevice::fb_edit_line(unsigned char *str, int pos)
{
    int x,y;
    
    if (!visible)
	return;

    y = fb_var.yres - f->height - ys;
    x = pos * f->width;
    fb_memset(fb_mem + fb_fix.line_length * y, 0,
	      fb_fix.line_length * (f->height+ys));
    fb_line(0, fb_var.xres, y, y);
    fs_puts(f, 0, y+ys, str);
    fb_line(x, x + f->width, fb_var.yres-1, fb_var.yres-1);
    fb_line(x, x + f->width, fb_var.yres-2, fb_var.yres-2);
}

void FramebufferDevice::fb_text_box(int x, int y, char *lines[], unsigned int count)
{
    unsigned int i,len,max, x1, x2, y1, y2;

    if (!visible)
	return;

    max = 0;
    for (i = 0; i < count; i++) {
	len = strlen(lines[i]);
	if (max < len)
	    max = len;
    }
    x1 = x;
    x2 = x + max * f->width;
    y1 = y;
    y2 = y + count * f->height;

    x += xs; x2 += 2*xs;
    y += ys; y2 += 2*ys;
    
    clear_rect(x1, x2, y1, y2);
    fb_rect(x1, x2, y1, y2);
    for (i = 0; i < count; i++) {
	fs_puts(f,x,y,(unsigned char*)lines[i]);
	y += f->height;
    }
}

void FramebufferDevice::fb_line(int x1, int x2, int y1,int y2)
{
/*static void fb_line(int x1, int x2, int y1,int y2)*/
    int x,y,h;
    float inc;

    if (x2 < x1)
	h = x2, x2 = x1, x1 = h;
    if (y2 < y1)
	h = y2, y2 = y1, y1 = h;
    if (x2 - x1 < y2 - y1) {
	inc = (float)(x2-x1)/(float)(y2-y1);
	for (y = y1; y <= y2; y++) {
	    x = x1 + (int)(inc * (float)(y - y1));
	    fb_setpixel(x,y,fs_white);
	}
    } else {
	inc = (float)(y2-y1)/(float)(x2-x1);
	for (x = x1; x <= x2; x++) {
	    y = y1 + (int)(inc * (float)(x - x1));
	    fb_setpixel(x,y,fs_white);
	}
    }
}


void FramebufferDevice::fb_rect(int x1, int x2, int y1,int y2)
{
    fb_line(x1, x2, y1, y1);
    fb_line(x1, x2, y2, y2);
    fb_line(x1, x1, y1, y2);
    fb_line(x2, x2, y1, y2);
}

void FramebufferDevice::fb_setpixel(int x, int y, unsigned int color)
{
    unsigned char *ptr;

    ptr  = fb_mem;
    ptr += y * fb_fix.line_length;
    ptr += x * fs_bpp;
    fs_setpixel(ptr, color);
}

void FramebufferDevice::fb_clear_rect(int x1, int x2, int y1,int y2)
{
    unsigned char *ptr;
    int y,h;

    if (!visible)
	return;

    if (x2 < x1)
	h = x2, x2 = x1, x1 = h;
    if (y2 < y1)
	h = y2, y2 = y1, y1 = h;
    ptr  = fb_mem;
    ptr += y1 * fb_fix.line_length;
    ptr += x1 * fs_bpp;

    for (y = y1; y <= y2; y++) {
	fb_memset(ptr, 0, (x2 - x1 + 1) * fs_bpp * 4 /* FIXME : 4 */);
	ptr += fb_fix.line_length;
    }
}

void FramebufferDevice::clear_screen(void)
{
    if (visible)
	fb_memset(fb_mem,0,fb_fix.smem_len);
}

void FramebufferDevice::cleanup(void)
{
    /* restore console */
    if (-1 == ioctl(fb,FBIOPUT_VSCREENINFO,&fb_ovar))
	perror("ioctl FBIOPUT_VSCREENINFO");
    if (-1 == ioctl(fb,FBIOGET_FSCREENINFO,&fb_fix))
	perror("ioctl FBIOGET_FSCREENINFO");
#if 0
    printf("id:%s\t%ld\t%ld\t%ld\t\n",fb_fix.id,fb_fix.accel,fb_fix.xpanstep,fb_fix.xpanstep);
#endif
    if (fb_ovar.bits_per_pixel == 8 ||
	fb_fix.visual == FB_VISUAL_DIRECTCOLOR) {
	if (-1 == ioctl(fb,FBIOPUTCMAP,&ocmap))
	    perror("ioctl FBIOPUTCMAP");
    }
    close(fb);

#ifdef FIM_BOZ_PATCH
    if(!with_boz_patch)
#endif
    if (-1 == ioctl(tty,KDSETMODE, kd_mode))
	perror("ioctl KDSETMODE");
#ifdef FIM_BOZ_PATCH
    if(!with_boz_patch)
#endif
    if (-1 == ioctl(tty,VT_SETMODE, &vt_omode))
	perror("ioctl VT_SETMODE");
    if (orig_vt_no && -1 == ioctl(tty, VT_ACTIVATE, orig_vt_no))
	perror("ioctl VT_ACTIVATE");
    if (orig_vt_no && -1 == ioctl(tty, VT_WAITACTIVE, orig_vt_no))
	perror("ioctl VT_WAITACTIVE");

    // there's no need to restore the tty : this is performed outside ( 20081221 )
    //tcsetattr(tty, TCSANOW, &term);
    //close(tty);
}

unsigned char * FramebufferDevice::convert_line_8(int bpp, int line, int owidth, char unsigned *dest, char unsigned *buffer, int mirror)/*dez's mirror patch*/
{
    unsigned char  *ptr  = (unsigned char *)dest;
	dither_line(buffer, ptr, line, owidth, mirror);
	ptr += owidth;
	return ptr;
}

unsigned char * FramebufferDevice::convert_line(int bpp, int line, int owidth, char unsigned *dest, char unsigned *buffer, int mirror)/*dez's mirror patch*/
{
    unsigned char  *ptr  = (unsigned char *)dest;
    unsigned short *ptr2 = (unsigned short*)dest;
    unsigned long  *ptr4 = (unsigned long *)dest;
    int x;
    int xm;/*mirror patch*/

    switch (fb_var.bits_per_pixel) {
    case 8:
	dither_line(buffer, ptr, line, owidth, mirror);
	ptr += owidth;
	return ptr;
    case 15:
    case 16:
#ifdef FIM_IS_SLOWER_THAN_FBI
	/*swapped RGB patch*/
	for (x = 0; x < owidth; x++) {
            xm=mirror?owidth-1-x:x;
	    ptr2[xm] = lut_red[buffer[x*3]] |
		lut_green[buffer[x*3+1]] |
		lut_blue[buffer[x*3+2]];
	}
#else
	if(FIM_LIKELY(!mirror))
	for (x = 0; x < owidth; x++) {
	    ptr2[x] = lut_red[buffer[x*3+2]] |
		lut_green[buffer[x*3+1]] |
		lut_blue[buffer[x*3]];
	}
	else
	for (x = 0,xm=owidth; x < owidth; x++) {
            xm--;
	    ptr2[xm] = lut_red[buffer[x*3+2]] |
		lut_green[buffer[x*3+1]] |
		lut_blue[buffer[x*3]];
	}
#endif
	ptr2 += owidth;
	return (unsigned char*)ptr2;
    case 24:
#ifdef FIM_IS_SLOWER_THAN_FBI
	for (x = 0; x < owidth; x++) {
            xm=mirror?owidth-1-x:x;
	    ptr[3*xm+2] = buffer[3*x+0];
	    ptr[3*xm+1] = buffer[3*x+1];
	    ptr[3*xm+0] = buffer[3*x+2];
	}
#else
	/*swapped RGB patch*/
	if(FIM_LIKELY(!mirror))
	{
		/*
		 * this code could be faster if using processor specific routines..
		 * ... or maybe even not ?
		 */
		//owidth*=3;
#if 0
		for (x = 0; x < owidth; x+=3)
		{
	            ptr[x+2] = buffer[x+0];
		    ptr[x+1] = buffer[x+1];
		    ptr[x+0] = buffer[x+2];
		}
#else
		/*
		 * this is far worse than the preceding !
		 */
		memcpy(ptr,buffer,owidth*3);
		//register char t;
		//register i=x;
		/*since RGB and GBR swap already done, this is not necessary*/
		/*for (i = 0; i < owidth; i+=3)
		{
	            t=ptr[i];
	            ptr[i]=ptr[i+2];
	            ptr[i+2]=t;
		}*/
#endif
		//owidth/=3;
	}else
/*this is still slow ... FIXME*/
#if 0
	for (x = 0; x < owidth; x++) {
	    x*=3;
            xm=3*owidth-x-3;
	    ptr[xm+2] = buffer[x+0];
	    ptr[xm+1] = buffer[x+1];
	    ptr[xm+0] = buffer[x+2];
	    x/=3;
	}
#else
	for (x = 0; x < owidth; x++) {
	    x*=3;
            xm=3*owidth-x-3;
	    ptr[xm+2] = buffer[x+2];
	    ptr[xm+1] = buffer[x+1];
	    ptr[xm+0] = buffer[x+0];
	    x/=3;
	}
#endif
#endif
	ptr += owidth * 3;
	return ptr;
    case 32:
#ifndef FIM_IS_SLOWER_THAN_FBI
	/*swapped RGB patch*/
	for (x = 0; x < owidth; x++) {
            xm=mirror?owidth-1-x:x;
	    ptr4[xm] = lut_red[buffer[x*3+2]] |
		lut_green[buffer[x*3+1]] |
		lut_blue[buffer[x*3]];
	}
#else
	if(FIM_LIKELY(!mirror))
	for (x = 0; x < owidth; x++) {
	    ptr4[x] = lut_red[buffer[x*3]] |
		lut_green[buffer[x*3+1]] |
		lut_blue[buffer[x*3+2]];
	}
	else
	for (x = 0; x < owidth; x++) {
	    ptr4[owidth-1-x] = lut_red[buffer[x*3]] |
		lut_green[buffer[x*3+1]] |
		lut_blue[buffer[x*3+2]];
	}
#endif
	ptr4 += owidth;
	return (unsigned char*)ptr4;
    default:
	/* keep compiler happy */
	return NULL;
    }
}

/*dez's*/
/*unsigned char * FramebufferDevice::clear_lines(int bpp, int lines, int owidth, char unsigned *dest)
{

}*/

unsigned char * FramebufferDevice::clear_line(int bpp, int line, int owidth, char unsigned *dest)
{
    unsigned char  *ptr  = (unsigned char*)dest;
    unsigned short *ptr2 = (unsigned short*)dest;
    unsigned long  *ptr4 = (unsigned long*)dest;
    unsigned ZERO_BYTE=0x00;
#ifdef FIM_IS_SLOWER_THAN_FBI
    int x;
#endif

    switch (fb_var.bits_per_pixel) {
    case 8:
	bzero(ptr, owidth);
	ptr += owidth;
	return ptr;
    case 15:
    case 16:
#ifdef FIM_IS_SLOWER_THAN_FBI
	for (x = 0; x < owidth; x++) {
	    ptr2[x] = 0x0;
	}
#else
	memset(ptr,ZERO_BYTE,2*owidth);
#endif
	ptr2 += owidth;
	return (unsigned char*)ptr2;
    case 24:
#ifdef FIM_IS_SLOWER_THAN_FBI
	for (x = 0; x < owidth; x++) {
	    ptr[3*x+2] = 0x0;
	    ptr[3*x+1] = 0x0;
	    ptr[3*x+0] = 0x0;
	}
#else
	memset(ptr,ZERO_BYTE,3*owidth);
#endif
	ptr += owidth * 3;
	return ptr;
    case 32:
#ifdef FIM_IS_SLOWER_THAN_FBI
	for (x = 0; x < owidth; x++) {
	    ptr4[x] = 0x0;
	}
#else
	memset(ptr,ZERO_BYTE,4*owidth);
#endif
	ptr4 += owidth;
	return (unsigned char*)ptr4;
    default:
	/* keep compiler happy */
	return NULL;
    }
}

void FramebufferDevice::init_dither(int shades_r, int shades_g, int shades_b, int shades_gray)
{
    int             i, j;
    unsigned char   low_shade, high_shade;
    unsigned short  index;
    float           red_colors_per_shade;
    float           green_colors_per_shade;
    float           blue_colors_per_shade;
    float           gray_colors_per_shade;

    red_mult = shades_g * shades_b;
    green_mult = shades_b;

    red_colors_per_shade = 256.0 / (shades_r - 1);
    green_colors_per_shade = 256.0 / (shades_g - 1);
    blue_colors_per_shade = 256.0 / (shades_b - 1);
    gray_colors_per_shade = 256.0 / (shades_gray - 1);

    /* this avoids a shift when checking these values */
    for (i = 0; i < DITHER_LEVEL; i++)
	for (j = 0; j < DITHER_LEVEL; j++)
	    DM[i][j] *= 0x10000;

    /*  setup arrays containing three bytes of information for red, green, & blue  */
    /*  the arrays contain :
     *    1st byte:    low end shade value
     *    2nd byte:    high end shade value
     *    3rd & 4th bytes:    ordered dither matrix index
     */

    for (i = 0; i < 256; i++) {

	/*  setup the red information  */
	{
	    low_shade = (unsigned char) (i / red_colors_per_shade);
	    high_shade = low_shade + 1;

	    index = (unsigned short)
		(((i - low_shade * red_colors_per_shade) / red_colors_per_shade) *
		 (DITHER_LEVEL * DITHER_LEVEL + 1));

	    low_shade *= red_mult;
	    high_shade *= red_mult;

	    red_dither[i] = (index << 16) + (high_shade << 8) + (low_shade);
	}

	/*  setup the green information  */
	{
	    low_shade = (unsigned char) (i / green_colors_per_shade);
	    high_shade = low_shade + 1;

	    index = (unsigned short)
		(((i - low_shade * green_colors_per_shade) / green_colors_per_shade) *
		 (DITHER_LEVEL * DITHER_LEVEL + 1));

	    low_shade *= green_mult;
	    high_shade *= green_mult;

	    green_dither[i] = (index << 16) + (high_shade << 8) + (low_shade);
	}

	/*  setup the blue information  */
	{
	    low_shade = (unsigned char) (i / blue_colors_per_shade);
	    high_shade = low_shade + 1;

	    index = (unsigned short)
		(((i - low_shade * blue_colors_per_shade) / blue_colors_per_shade) *
		 (DITHER_LEVEL * DITHER_LEVEL + 1));

	    blue_dither[i] = (index << 16) + (high_shade << 8) + (low_shade);
	}

	/*  setup the gray information  */
	{
	    low_shade = (unsigned char) (i / gray_colors_per_shade);
	    high_shade = low_shade + 1;

	    index = (unsigned short)
		(((i - low_shade * gray_colors_per_shade) / gray_colors_per_shade) *
		 (DITHER_LEVEL * DITHER_LEVEL + 1));

	    gray_dither[i] = (index << 16) + (high_shade << 8) + (low_shade);
	}
    }
}

void inline FramebufferDevice::dither_line(unsigned char *src, unsigned char *dest, int y, int width,int mirror)
{
    register long   a, b
#ifndef FIM_IS_SLOWER_THAN_FBI
    __attribute((aligned(16)))
#endif
    ;

    long           *ymod, xmod;

    ymod = (long int*) DM[y & DITHER_MASK];
    /*	mirror patch by dez	*/
    register const int inc=mirror?-1:1;
    dest=mirror?dest+width-1:dest;
    /*	mirror patch by dez	*/
    if(FIM_UNLIKELY(width<1))goto nodither; //who knows

#ifndef FIM_IS_SLOWER_THAN_FBI
    switch(width%8){
    	case 0:	goto dither0; break ;
    	case 1:	goto dither1; break ;
    	case 2:	goto dither2; break ;
    	case 3:	goto dither3; break ;
    	case 4:	goto dither4; break ;
    	case 5:	goto dither5; break ;
    	case 6:	goto dither6; break ;
    	case 7:	goto dither7; break ;
    }
#endif

    while (FIM_LIKELY(width)) {

#if 0

 #define DITHER_CORE \
	xmod = --width & DITHER_MASK; \
\
	b = blue_dither[*(src++)];  \
	b >>= (ymod[xmod] < b)?8:0; \
	a = green_dither[*(src++)]; \
	a >>= (ymod[xmod] < a)?8:0; \
	b += a; \
	a = red_dither[*(src++)]; \
	a >>= (ymod[xmod] < a)?8:0; \
	*(dest) = b+a & 0xff; \
    /*	mirror patch by dez	*/ \
	dest+=inc;

#else
 #define DITHER_CORE \
	{ \
	width--; \
	xmod = width & DITHER_MASK; \
 	const long ymod_xmod=ymod[xmod]; \
\
	b = blue_dither[*(src++)]; \
	if (ymod_xmod < b) \
	    b >>= 8; \
\
	a = green_dither[*(src++)]; \
	if (ymod_xmod < a) \
	    a >>= 8; \
	b += a; \
\
	a = red_dither[*(src++)]; \
	if (ymod_xmod < a) \
	    a >>= 8; \
	b += a; \
	*(dest) = b & 0xff; \
    /*	mirror patch by dez	*/ \
	dest+=inc; \
	} \
	/*	*(dest++) = b & 0xff;*/ 
#endif

#ifndef FIM_IS_SLOWER_THAN_FBI
dither0:
	DITHER_CORE
dither7:
	DITHER_CORE
dither6:
	DITHER_CORE
dither5:
	DITHER_CORE
dither4:
	DITHER_CORE
dither3:
	DITHER_CORE
dither2:
	DITHER_CORE
#endif
dither1:
	DITHER_CORE
    }
nodither:
	return;
}
void FramebufferDevice::dither_line_gray(unsigned char *src, unsigned char *dest, int y, int width)
{
    long           *ymod, xmod;
    register long   a;

    ymod = (long int*) DM[y & DITHER_MASK];

    while (width--) {
	xmod = width & DITHER_MASK;

	a = gray_dither[*(src++)];
	if (ymod[xmod] < a)
	    a >>= 8;

	*(dest++) = a & 0xff;
    }
}
void FramebufferDevice::fb_switch_release()
{
    ioctl(tty, VT_RELDISP, 1);
    fb_switch_state = FB_INACTIVE;
    if (debug)
	FIM_FPRINTF(stderr, "vt: release\n");
}
void FramebufferDevice::fb_switch_acquire()
{
    ioctl(tty, VT_RELDISP, VT_ACKACQ);
    fb_switch_state = FB_ACTIVE;
    if (debug)
	FIM_FPRINTF(stderr, "vt: acquire\n");
}
int FramebufferDevice::fb_switch_init()
{
    struct sigaction act,old;

    memset(&act,0,sizeof(act));
    
    ffdp=this;// WARNING : A DIRTY HACK
    act.sa_handler  = _fb_switch_signal;
    sigemptyset(&act.sa_mask);
    sigaction(SIGUSR1,&act,&old);
    sigaction(SIGUSR2,&act,&old);
#ifdef FIM_BOZ_PATCH
    if(!with_boz_patch)
#endif
    if (-1 == ioctl(tty,VT_GETMODE, &vt_mode)) {
	perror("ioctl VT_GETMODE");
	exit(1);
    }
    vt_mode.mode   = VT_PROCESS;
    vt_mode.waitv  = 0;
    vt_mode.relsig = SIGUSR1;
    vt_mode.acqsig = SIGUSR2;
    
#ifdef FIM_BOZ_PATCH
    if(!with_boz_patch)
#endif
    if (-1 == ioctl(tty,VT_SETMODE, &vt_mode)) {
	perror("ioctl VT_SETMODE");
	exit(1);
    }
    return 0;
}

void FramebufferDevice::fb_switch_signal(int signal)
{
    if (signal == SIGUSR1) {
	/* release */
	fb_switch_state = FB_REL_REQ;
	if (debug)
	    FIM_FPRINTF(stderr, "vt: SIGUSR1\n");
    }
    if (signal == SIGUSR2) {
	/* acquisition */
	fb_switch_state = FB_ACQ_REQ;
	if (debug)
	    FIM_FPRINTF(stderr, "vt: SIGUSR2\n");
    }
}


int FramebufferDevice::fb_text_init2(void)
{
    return fs_init_fb(255);
}
	int  FramebufferDevice::fb_font_width(void) { return f->width; }
	int  FramebufferDevice::fb_font_height(void) { return f->height; }

int FramebufferDevice::fs_init_fb(int white8)
{
    switch (fb_var.bits_per_pixel) {
    case 8:
	fs_white = white8; fs_black = 0; fs_bpp = 1;
	fs_setpixel = setpixel1;
	break;
    case 15:
    case 16:
	if (fb_var.green.length == 6)
	    fs_white = 0xffff;
	else
	    fs_white = 0x7fff;
	fs_black = 0; fs_bpp = 2;
	fs_setpixel = setpixel2;
	break;
    case 24:
	fs_white = 0xffffff; fs_black = 0; fs_bpp = fb_var.bits_per_pixel/8;
	fs_setpixel = setpixel3;
	break;
    case 32:
	fs_white = 0xffffff; fs_black = 0; fs_bpp = fb_var.bits_per_pixel/8;
	fs_setpixel = setpixel4;
	break;
    default:
	FIM_FPRINTF(stderr,  "Oops: %i bit/pixel ???\n",
		fb_var.bits_per_pixel);
	return -1;
    }
    return 0;
}

/*
 *	This function treats the framebuffer screen as a text outout terminal.
 *	So it prints all the contents of its buffer on screen..
 *	if noDraw is set, the screen will be not refreshed.
	 *	NULL,NULL is the clearing combination !!

	//FIX ME : move this functionality to some new class and add ways to scroll and manipulate it
	dez's
 */
#if 0
#ifndef FIM_KEEP_BROKEN_CONSOLE
void FramebufferDevice::status_screen(const char *msg, int draw)
{	
	/* current code */
	return fb_status_screen_new(msg, draw,0);
}
#else
void FramebufferDevice::status_screen(const char *msg, int draw)
{
	/* dead code */

	/*	WARNING		*/
	//noDraw=0;
	/*	WARNING		*/
	int y,i,j,l,w;
	// R rows, C columns
	int R=(fb_var.yres/fb_font_height())/2,/* half screen : more seems evil */
	C=(fb_var.xres/fb_font_width());
	static char **columns=NULL;
	static char *columns_data=NULL;
	if(R<1 || C < 1)return;		/* sa finimm'acca', nun ce sta nient'a fa! */
	/* R rows and C columns; the last one for string terminators..
	 */
	if(!columns)columns=(char**)fim_calloc(sizeof(char**)*R,1);
	if(!columns_data)columns_data=(char*)fim_calloc(sizeof(char)*(R*(C+1)),1);
	/* 
	 * seems tricky : we allocate one single buffer and use it as console 
	 * storage and console pointers storage ...
	 *
	 * note that we don't deallocate this area until program termination.
	 * it is because we keep the framebuffer...
	 * */
	if(!columns || !columns_data)
	{
		if(columns)fim_free(columns);
		if(columns_data)fim_free(columns_data);
		return;
	}

	for(i=0;i<R;++i)columns[i]=columns_data+i*(C+1);

	static int cline=0,	//current line		[0..R-1]
		   ccol=0;	//current column	[0..C]
	const char *p=msg,	//p points to the substring not yet printed
	      	    *s=p;	//s advances and updates p

	if(!msg)
	{
		cline=0;
		ccol=0;
		p=NULL;
		/*noDraw=0;*/
	}
	if(msg&&*msg=='\0')return;

	if(p)while(*p)
	{
	    //while there are characters to put on screen, we advance
	    while(*s && *s!='\n')++s;
	    //now s points to an endline or a NUL
	    l=s-p;
	    //l is the number of characters which should go on screen (from *p to s[-1])
	    w=0;
	    while(l>0)	//line processing
	    {
		    //w is the number of writable characters on this line ( w in [0,C-ccol] )
		    w=min(C-ccol,l);
		    //there remains l-=w non '\n' characters yet to process in the first substring
		    l-=w;
		    //we place the characters on the line (not padded,though)
		    strncpy(columns[cline]+ccol,p,w);
		    sanitize_string_from_nongraph(columns[cline]+ccol,w);
		    //the current column index is updated,too
		    ccol+=w;
		    //we blank the rest of the line (SHOULD BE UNNECESSARY)
		    for(i=ccol;i<C;++i)columns[cline][i]=' ';
		    //we terminate the line with a NUL
		    columns[cline][C]='\0';
		    //please note that ccol could still point to the middle of the line
		    //the last writable column index is C
	
#ifdef CERCO_GRANE
		    if(ccol>=C+1){cleanup();return;}	//ehm.. who knows
#else
		    if(ccol>=C+1)return;
#endif
		    if(ccol==C)
		    {
			    //So if we are at the end of the line, we prepare 
			    //for a new line
			    ccol=0;
			    cline=(cline+1)%(R);
			    if(cline==0)
			    for(i=0;i<R;++i)
			    {
				    for(j=0;j<C;++j)columns[i][j]=' ';
				    columns[i][C]='\0';
			    }
			    //we clean the new line (SHOULD BE NECESSARY ONLY WITH THE FIRST LINE!)
		    	    for(i=0;i<C;++i)columns[cline][i]=' ';
		    }
	            //we advance in the string for w chars 
	    	    p+=w;	//a temporary assignment
	    }
	    	/*
		 * after the chars in [p,s-1] are consumed, we can continue
		 */
		    while(*s=='\n')
		    {
			    ++s;
			    ccol=0;
			    cline=(cline+1)%(R);
			    if(cline==0)
			    for(i=0;i<R;++i)
			    {
				    for(j=0;j<C;++j)columns[i][j]=' ';
				    columns[i][C]='\0';
			    }
		    }
	    p=s;
	}

	//if(!mc.cc.drawOutput() || noDraw)return;//CONVENTION!
	if(!draw )return;//CONVENTION!

	    y = 1*fb_font_height();
	    for(i=0  ;i<R ;++i) fs_puts(fb_font_get_current_font(), 0, y*(i), (unsigned char*)columns[i]);

	    /*
	     *WARNING : note that columns and columns_data arrays are not freed and should not, as long as they are static.
	     * */
#endif
}
#endif


	FramebufferDevice::FramebufferDevice(MiniConsole & mc_):	
	DisplayDevice(mc_)
	,vt(0)
	,dither(FALSE)
	,pcd_res(3)
	,steps(50)
	,fbgamma(1.0)
	,visible(1)
	,x11_font("10x20")
	,ys( 3)
	,xs(10)
	,fs_setpixel(NULL)
	,fbdev(NULL)
	,fbmode(NULL)
#ifdef FIM_BOZ_PATCH
	,with_boz_patch(0)
#endif
	,fb_mem_offset(0)
	,fb_switch_state(FB_ACTIVE)
	,orig_vt_no(0)
	,devices(NULL)
#ifndef FIM_KEEP_BROKEN_CONSOLE
	//mc(48,12),
//	int R=(fb_var.yres/fb_font_height())/2,/* half screen : more seems evil */
//	C=(fb_var.xres/fb_font_width());
#endif
	{

		cmap.start  =  0;
		cmap.len    =  256;
		cmap.red  =  red;
		cmap.green  =  green;
		cmap.blue  =  blue;
		//! transp!
		devs_default.fb0=   "/dev/fb0";
		devs_default.fbnr=  "/dev/fb%d";
		devs_default.ttynr= "/dev/tty%d";
		devs_devfs.fb0=   "/dev/fb/0";
		devs_devfs.fbnr=  "/dev/fb/%d";
		devs_devfs.ttynr= "/dev/vc/%d";
		ocmap.start = 0;
		ocmap.len   = 256;
		ocmap.red=ored;
		ocmap.green=ogreen;
		ocmap.blue=oblue;
		/*
		 * fbgamma and fontname are fbi - defined variables.
		 * */
		const char *line;

	    	if (NULL != (line = fim_getenv("FBGAMMA")))
	        	fbgamma = fim_atof(line);
	    	if (NULL != (line = fim_getenv("FBFONT")))
			fontname = line;
	}

}

int FramebufferDevice::display(
	void *ida_image_img,
	int yoff,
	int xoff,
	int irows,int icols,// rows and columns in the input image
	int icskip,	// input columns to skip for each line
	int by,//FIXME : this four arguments should be unsigned !
	int bx,
	int bh,
	int bw,
	int ocskip,// output columns to skip for each line
	int flags)
{
	if(by<0)return -1;
	if(bx<0)return -1;
	if(bw<0)return -1;
	if(bh<0)return -1;

	svga_display_image_new(
	(struct ida_image*)ida_image_img,
	yoff,
	xoff,
	irows,icols,// rows and columns in the input image
	icskip,	// input columns to skip for each line
	by,
	bx,
	bh,
	bw,
	ocskip,// output columns to skip for each line
	flags);
	return 0;
}

void FramebufferDevice::finalize (void)
{
	finalized=true;
	clear_screen();
	cleanup();
}

FramebufferDevice::~FramebufferDevice()
{
	/* added in fim : fbi did not have this */
	if(f)
	{
		if(f->eindex) fim_free(f->eindex);
		if(f->gindex) fim_free(f->gindex);
		if(f->glyphs) fim_free(f->glyphs);
		if(f->extents) fim_free(f->extents);
		fim_free(f);
	}
}

#endif  //ifdef FIM_WITH_NO_FRAMEBUFFER, else


