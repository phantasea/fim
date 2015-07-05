/* $Id: FramebufferDevice.h 256 2009-08-30 10:15:16Z dezperado $ */
/*
 FramebufferDevice.h : Linux Framebuffer functions from fbi, adapted for fim

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

#ifndef FIM_FRAMEBUFFER_DEVICE_H
#define FIM_FRAMEBUFFER_DEVICE_H

/*
 * These are fbi's internals adapted to C++ and adapting to a loose OOP.
 * */



#include "fim.h"
#include "FontServer.h"
#include "DisplayDevice.h"

#ifndef FIM_WITH_NO_FRAMEBUFFER

#include <stdio.h>
#include <errno.h>
#include <math.h>	//pow

#if HAVE_LINUX_VT_H
#include <linux/vt.h>
#endif
#if HAVE_LINUX_FB_H
#include <linux/fb.h>	// fb_fix_screeninfo
#endif


/* from fbtools.h */
#define FB_ACTIVE    0
#define FB_REL_REQ   1
#define FB_INACTIVE  2
#define FB_ACQ_REQ   3


#define TRUE            1
#define FALSE           0

namespace fim
{
struct DEVS {
    const char *fb0;
    const char *fbnr;
    const char *ttynr;
};



//void _fb_switch_signal(int signal);



 




class FramebufferDevice:public DisplayDevice 
{


	long     red_mult, green_mult;
	long     red_dither[256]  FIM_ALIGNED;
	long     green_dither[256]FIM_ALIGNED;
	long     blue_dither[256] FIM_ALIGNED;
	long     gray_dither[256] FIM_ALIGNED;

	/*
	 * A class providing access to a single framebuffer device.
	 *
	 * Let's say in future we want to be able to manage multiple framebuffer devices.
	 * Then framebuffer variables should be incapsulated well in separate objects.
	 * We are heading forward on this road, slowly.
	 * */
#if 0
	void fb_text_init1(char *font)
	{
	    char   *fonts[2] = { font, NULL };
	
	    if (NULL == f)
		f = fs_consolefont(font ? fonts : NULL);
	#ifndef X_DISPLAY_MISSING
	    if (NULL == f && 0 == fs_connect(NULL))
		f = fs_open(font ? font : x11_font);
	#endif
	    if (NULL == f) {
		fprintf(stderr,"font \"%s\" is not available\n",font);
		exit(1);
	    }
	}
#endif
	private:

	int             vt ;
	public:
	int32_t         lut_red[256], lut_green[256], lut_blue[256];
	int             dither , pcd_res , steps ;
	private:
	float fbgamma ;

	/*static float fbgamma = 1;*/
	public:


	// FS.C
	unsigned int       fs_bpp, fs_black, fs_white;//STILL UNINITIALIZED
	int fs_init_fb(int white8);
	private:
	/* public */
	int visible ;

	/* private */
	char *x11_font ;

	int ys ;
	int xs ;

	public:
	void (*fs_setpixel)(void *ptr, unsigned int color);
	private:

	static void setpixel1(void *ptr, unsigned int color)
	{
	    unsigned char *p = (unsigned char *) ptr;
	    *p = color;
	}

	static void setpixel2(void *ptr, unsigned int color)
	{
	    unsigned char *p = (unsigned char *) ptr;
	    *p = color;
	}

	static void setpixel3(void *ptr, unsigned int color)
	{
	    unsigned char *p = (unsigned char *) ptr;
	    *(p++) = (color >> 16) & 0xff;
	    *(p++) = (color >>  8) & 0xff;
	    *(p++) =  color        & 0xff;
	}

	static void setpixel4(void *ptr, unsigned int color)
	{
	    unsigned long *p = (unsigned long*) ptr;
	    *p = color;
	}

	/* framebuffer */
	char                       *fbdev;
	char                       *fbmode;

	public:
	/*
	 * FIXME : should be a static string, or troubles will come!
	 * */
	int set_fbdev(char *fbdev)
	{
		/* only possible before init() */
		if(fb_mem)
			return -1;
		if(fbdev)
			this->fbdev=fbdev;
		return 0;
	}

	/*
	 * FIXME : should be a static string, or troubles will come!
	 * */
	int set_fbmode(char *fbmode)
	{
		/* only possible before init() */
		if(fb_mem)
			return -1;
		if(fbmode)
			this->fbmode=fbmode;
		return 0;
	}

	int set_default_vt(int default_vt)
	{
		/* only possible before init() */
		if(fb_mem)
			return -1;
		if(default_vt)
			this->vt=default_vt;
		return 0;
	}

	int set_default_fbgamma(float fbgamma)
	{
		/* only possible before init() */
		if(fb_mem)
			return -1;
		if(fbgamma)
			this->fbgamma=fbgamma;
		return 0;
	}

	//private:
	int                        fd, switch_last;

	unsigned short red[256],  green[256],  blue[256];
	struct fb_cmap cmap;


	//were static ..
	struct fb_cmap            ocmap;
	unsigned short            ored[256], ogreen[256], oblue[256];


	struct DEVS devs_default;
	struct DEVS devs_devfs;

#ifdef FIM_BOZ_PATCH
	int with_boz_patch;
#endif

	FramebufferDevice(MiniConsole & mc_);


/* -------------------------------------------------------------------- */
	/* exported stuff                                                       */
	public:
	struct fb_fix_screeninfo   fb_fix;
	struct fb_var_screeninfo   fb_var;
	//private:
	unsigned char             *fb_mem;
	int			   fb_mem_offset;
	int                        fb_switch_state;

/* -------------------------------------------------------------------- */
	/* internal variables                                                   */

	int                       fb,tty;
	#if 0
	static int                       bpp,black,white;
	#endif

	int                       orig_vt_no;
	struct vt_mode            vt_mode;
	int                       kd_mode;
	struct vt_mode            vt_omode;
	struct termios            term;
	struct fb_var_screeninfo  fb_ovar;


	public:
	int framebuffer_init();

	struct DEVS *devices;



	void dev_init(void);
	private:
	int fb_init(const char *device, char *mode, int vt
			, int try_boz_patch=0
			);
	public:

	void fb_memset (void *addr, int c, size_t len);
	void fb_setcolor(int c) { fb_memset(fb_mem+fb_mem_offset,c,fb_fix.smem_len); }



	void fb_setvt(int vtno);
	int fb_setmode(char *name);
	int fb_activate_current(int tty);

	void console_switch(int is_busy);

	int  fb_font_width(void);
	int  fb_font_height(void);

	int status_line(const unsigned char *msg);

	void fb_edit_line(unsigned char *str, int pos);

	void fb_text_box(int x, int y, char *lines[], unsigned int count);

	void fb_line(int x1, int x2, int y1,int y2);


	void fb_rect(int x1, int x2, int y1,int y2);

	void fb_setpixel(int x, int y, unsigned int color);
	int fs_puts(struct fs_font *f, unsigned int x, unsigned int y, const unsigned char *str);

	void fb_clear_rect(int x1, int x2, int y1,int y2);
	int clear_rect(int x1, int x2, int y1,int y2)
	{
		fb_clear_rect(x1, x2, y1,y2);
		return 0;
	}

	void clear_screen(void);
	void cleanup(void);

	int initialize (key_bindings_t &key_bindings){/*still unused : FIXME */ ;return 0;}
	void finalize (void);
	struct fs_font * fb_font_get_current_font(void)
	{
	    return f;
	}

	void switch_if_needed()
	{
		//fim's
		if (switch_last != fb_switch_state)
		    console_switch(1);
	}

#define TRUE            1
#define FALSE           0
#define MAX(x,y)        ((x)>(y)?(x):(y))
#define MIN(x,y)        ((x)<(y)?(x):(y))
#define ARRAY_SIZE(x)   (sizeof(x)/sizeof(x[0]))

/*
 * framebuffer memory offset for x pixels left and y right from the screen
 * (by dez)
 */
#define FB_BPP  (((fb_var.bits_per_pixel+7)/8))
#define FB_MEM_LINE_OFFSET  ((FB_BPP*fb_var.xres))
#define FB_MEM_OFFSET(x,y)  (( FB_BPP*(x) + FB_MEM_LINE_OFFSET * (y) ))
#define FB_MEM(x,y) ((fb_mem+FB_MEM_OFFSET((x),(y))))



//void svga_display_image_new(struct ida_image *img, int xoff, int yoff,unsigned int bx,unsigned int bw,unsigned int by,unsigned int bh,int mirror,int flip);
//void svga_display_image_new(struct ida_image *img, int xoff, int yoff,unsigned int bx,unsigned int bw,unsigned int by,unsigned int bh,int mirror,int flip);

int display(
	//struct ida_image *img,
	void *ida_image_img, // source image structure
	int yoff,
	int xoff,
	int irows,int icols,// rows and columns in the input image
	int icskip,	// input columns to skip for each line
	int by,
	int bx,
	int bh,
	int bw,
	int ocskip,// output columns to skip for each line
	int flags);


void svga_display_image_new(
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
	int flags);

/* ---------------------------------------------------------------------- */
/* by dez
 */
inline unsigned char * clear_line(int bpp, int line, int owidth, char unsigned *dest);
unsigned char * convert_line(int bpp, int line, int owidth, char unsigned *dest, char unsigned *buffer, int mirror);/*dez's mirror patch*/
unsigned char * convert_line_8(int bpp, int line, int owidth, char unsigned *dest, char unsigned *buffer, int mirror);/*dez's mirror patch*/







void init_dither(int shades_r, int shades_g, int shades_b, int shades_gray);
inline void dither_line(unsigned char *src, unsigned char *dest, int y, int width,int mirror);

void dither_line_gray(unsigned char *src, unsigned char *dest, int y, int width);


void fb_switch_release();

void fb_switch_acquire();

int fb_switch_init();

void fb_switch_signal(int signal);

int fb_text_init2(void);


/*static void*/
void svga_dither_palette(int r, int g, int b)
{
    int             rs, gs, bs, i;

    rs = 256 / (r - 1);
    gs = 256 / (g - 1);
    bs = 256 / (b - 1);
    for (i = 0; i < 256; i++) {
	red[i]   = calc_gamma(rs * ((i / (g * b)) % r), 255);
	green[i] = calc_gamma(gs * ((i / b) % g),       255);
	blue[i]  = calc_gamma(bs * ((i) % b),           255);
    }
}


unsigned short calc_gamma(int n, int max)
{
    int ret =(int)(65535.0 * pow((float)n/(max), 1 / fbgamma)); 
    if (ret > 65535) ret = 65535;
    if (ret <     0) ret =     0;
    return ret;
}

void linear_palette(int bit)
{
    int i, size = 256 >> (8 - bit);
    
    for (i = 0; i < size; i++)
        red[i] = green[i] = blue[i] = calc_gamma(i,size);
}

void lut_init(int depth)
{
    if (fb_var.red.length   &&
	fb_var.green.length &&
	fb_var.blue.length) {
	/* fb_var.{red|green|blue} looks sane, use it */
	init_one(lut_red,   fb_var.red.length,   fb_var.red.offset);
	init_one(lut_green, fb_var.green.length, fb_var.green.offset);
	init_one(lut_blue,  fb_var.blue.length,  fb_var.blue.offset);
    } else {
	/* fallback */
	int i;
	switch (depth) {
	case 15:
	    for (i = 0; i < 256; i++) {
		lut_red[i]   = (i & 0xf8) << 7;	/* bits -rrrrr-- -------- */
		lut_green[i] = (i & 0xf8) << 2;	/* bits ------gg ggg----- */
		lut_blue[i]  = (i & 0xf8) >> 3;	/* bits -------- ---bbbbb */
	    }
	    break;
	case 16:
	    for (i = 0; i < 256; i++) {
		lut_red[i]   = (i & 0xf8) << 8;	/* bits rrrrr--- -------- */
		lut_green[i] = (i & 0xfc) << 3;	/* bits -----ggg ggg----- */
		lut_blue[i]  = (i & 0xf8) >> 3;	/* bits -------- ---bbbbb */
	    }
	    break;
	case 24:
	    for (i = 0; i < 256; i++) {
		lut_red[i]   = i << 16;	/* byte -r-- */
		lut_green[i] = i << 8;	/* byte --g- */
		lut_blue[i]  = i;		/* byte ---b */
	    }
	    break;
	}
    }
}

void init_one(int32_t *lut, int bits, int shift)
{
    int i;
    
    if (bits > 8)
	for (i = 0; i < 256; i++)
	    lut[i] = (i << (bits + shift - 8));
    else
	for (i = 0; i < 256; i++)
	    lut[i] = (i >> (8 - bits)) << shift;
}

	int width()
	{
		return fb_var.xres;
	}

	int height()
	{
		return fb_var.yres;
	}

	int get_chars_per_column()
	{
		return fb_var.yres / fb_font_height();
	}

	int get_chars_per_line()
	{
		return fb_var.xres / fb_font_width();
	}

	int handle_console_switch()
	{
		if (switch_last == fb_switch_state)return false;

		console_switch(1);
		return 1;
        }

	//void status_screen(const char *msg, int draw);
	void fs_render_fb(unsigned char *ptr, int pitch, FSXCharInfo *charInfo, unsigned char *data);
	int get_bpp(){return fb_var.bits_per_pixel; };
	virtual ~FramebufferDevice();
};

}



#endif
#endif  //ifndef FIM_WITH_NO_FRAMEBUFFER

