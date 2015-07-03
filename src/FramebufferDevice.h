/* $LastChangedDate: 2015-01-23 19:16:51 +0100 (Fri, 23 Jan 2015) $ */
/*
 FramebufferDevice.h : Linux Framebuffer functions from fbi, adapted for fim

 (c) 2008-2013 Michele Martone
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
#endif /* HAVE_LINUX_VT_H */
#if HAVE_LINUX_FB_H
#include <linux/fb.h>	// fb_fix_screeninfo
#endif /* HAVE_LINUX_FB_H */


/* from fbtools.h */
#define FB_ACTIVE    0
#define FB_REL_REQ   1
#define FB_INACTIVE  2
#define FB_ACQ_REQ   3


#define FIM_FBI_TRUE            1
#define FIM_FBI_FALSE           0

namespace fim
{
struct DEVS {
    const fim_char_t *fb0;
    const fim_char_t *fbnr;
    const fim_char_t *ttynr;
};



//void _fb_switch_signal(int signal);



 




class FramebufferDevice:public DisplayDevice 
{


	long     red_mult_, green_mult_;
	long     red_dither_[256]  FIM_ALIGNED;
	long     green_dither_[256]FIM_ALIGNED;
	long     blue_dither_[256] FIM_ALIGNED;
	long     gray_dither_[256] FIM_ALIGNED;

	/*
	 * A class providing access to a single framebuffer device.
	 *
	 * Let's say in future we want to be able to manage multiple framebuffer devices.
	 * Then framebuffer variables should be incapsulated well in separate objects.
	 * We are heading forward on this road, slowly.
	 * */
#if 0
	void fb_text_init1(fim_char_t *font)
	{
	    fim_char_t   *fonts[2] = { font, NULL };
	
	    if (NULL == f_)
		f_ = fs_consolefont(font ? fonts : NULL);
	#ifdef FIM_USE_X11_FONTS
	    if (NULL == f_ && 0 == fs_connect(NULL))
		f_ = fs_open(font ? font : x11_font_);
	#endif /* FIM_USE_X11_FONTS */
	    if (NULL == f_) {
		fprintf(stderr,"font \"%s\" is not available\n",font);
		exit(1);
	    }
	}
#endif
	private:
	static const fim_coo_t border_height_=1;

	int             vt_ ;
	//public:
	int32_t         lut_red_[256], lut_green_[256], lut_blue_[256];
	int             dither_ , pcd_res_ /*, steps_*/ ;
	private:
	float fbgamma_ ;

	/*static float fbgamma_ = 1;*/
	//public:


	// FS.C
	unsigned int       fs_bpp_, fs_black_, fs_white_;//STILL UNINITIALIZED
	int fs_init_fb(int white8);
	private:
	/* public */
	int visible_ ;

	/* private */
	//fim_char_t *x11_font_ ;

	int ys_ ;
	int xs_ ;

	//public:
	void (*fs_setpixel)(void *ptr, unsigned int color);
	private:

	static void setpixel1(void *ptr, unsigned int color)
	{
	    fim_byte_t *p = (fim_byte_t *) ptr;
	    *p = color;
	}

	static void setpixel2(void *ptr, unsigned int color)
	{
	    short unsigned int *p = (short unsigned int*) ptr;
	    *p = color;
	}

	static void setpixel3(void *ptr, unsigned int color)
	{
	    fim_byte_t *p = (fim_byte_t *) ptr;
	    *(p++) = (color >> 16) & 0xff;
	    *(p++) = (color >>  8) & 0xff;
	    *(p++) =  color        & 0xff;
	}

	static void setpixel4(void *ptr, unsigned int color)
	{
	    unsigned int *p = (unsigned int*) ptr;
	    *p = color;
	}

	/* framebuffer */
	fim_char_t                       *fbdev_;
	fim_char_t                       *fbmode_;

	public:
	/*
	 * FIXME : should be a static string, or troubles will come!
	 * */
	fim_err_t set_fbdev(fim_char_t *fbdev_)
	{
		/* only possible before init() */
		if(fb_mem_)
			return FIM_ERR_GENERIC;
		if(fbdev_)
			this->fbdev_=fbdev_;
		return FIM_ERR_NO_ERROR;
	}

	/*
	 * FIXME : should be a static string, or troubles will come!
	 * */
	fim_err_t set_fbmode(fim_char_t *fbmode_)
	{
		/* only possible before init() */
		if(fb_mem_)
			return FIM_ERR_GENERIC;
		if(fbmode_)
			this->fbmode_=fbmode_;
		return FIM_ERR_NO_ERROR;
	}

	int set_default_vt(int default_vt)
	{
		/* only possible before init() */
		if(fb_mem_)
			return FIM_ERR_GENERIC;
		if(default_vt)
			this->vt_=default_vt;
		return FIM_ERR_NO_ERROR;
	}

	int set_default_fbgamma(float fbgamma_)
	{
		/* only possible before init() */
		if(fb_mem_)
			return FIM_ERR_GENERIC;
		if(fbgamma_)
			this->fbgamma_=fbgamma_;
		return FIM_ERR_NO_ERROR;
	}

	//private:
	int                        fd_, switch_last_;

	unsigned short red_[256],  green_[256],  blue_[256];
	struct fb_cmap cmap_;


	//were static ..
	struct fb_cmap            ocmap_;
	unsigned short            ored_[256], ogreen_[256], oblue_[256];


	struct DEVS devs_default_;
	struct DEVS devs_devfs_;

#ifdef FIM_BOZ_PATCH
	int with_boz_patch_;
#endif /* FIM_BOZ_PATCH */

#ifndef FIM_WANT_NO_OUTPUT_CONSOLE
	FramebufferDevice(MiniConsole & mc_);
#else /* FIM_WANT_NO_OUTPUT_CONSOLE */
	FramebufferDevice(void);
#endif /* FIM_WANT_NO_OUTPUT_CONSOLE */


/* -------------------------------------------------------------------- */
	/* exported stuff                                                       */
	public:
	struct fb_fix_screeninfo   fb_fix_;
	struct fb_var_screeninfo   fb_var_;
	//private:
	fim_byte_t             *fb_mem_;
	int			   fb_mem_offset_;
	int                        fb_switch_state_;

/* -------------------------------------------------------------------- */
	/* internal variables                                                   */

	int                       fb_,tty_;
	#if 0
	static int                       bpp,black,white;
	#endif
	private:

	int                       orig_vt_no_;
	struct vt_mode            vt_mode_;
	int                       kd_mode_;
	struct vt_mode            vt_omode_;
	struct termios            term_;
	struct fb_var_screeninfo  fb_ovar_;


	public:
	fim_err_t framebuffer_init(void);

	struct DEVS *devices_;



	void dev_init(void);
	private:
	int fb_init(const fim_char_t *device, fim_char_t *mode, int vt_
			, int try_boz_patch=0
			);
	public:

	void fb_memset (void *addr, int c, size_t len);
	void fb_setcolor(int c) { fb_memset(fb_mem_+fb_mem_offset_,c,fb_fix_.smem_len); }



	void fb_setvt(int vtno);
	int fb_setmode(fim_char_t *name);
	int fb_activate_current(int tty_);

	void console_switch(fim_bool_t is_busy);

	int  fb_font_width(void);
	int  fb_font_height(void);

	int status_line(const fim_char_t *msg);

	//void fb_edit_line(fim_char_t *str, int pos);

	void fb_text_box(int x, int y, fim_char_t *lines[], unsigned int count);

	void fb_line(int x1, int x2, int y1,int y2);


	void fb_rect(int x1, int x2, int y1,int y2);

	void fb_setpixel(int x, int y, unsigned int color);
	fim_err_t fs_puts(struct fs_font *f, fim_coo_t x, fim_coo_t y, const fim_char_t *str);

	void fb_clear_rect(int x1, int x2, int y1,int y2);
	fim_err_t clear_rect(fim_coo_t x1, fim_coo_t x2, fim_coo_t y1,fim_coo_t y2)
	{
		fb_clear_rect(x1, x2, y1,y2);
		return 0;
	}

	void clear_screen(void);
	void cleanup(void);

	fim_err_t initialize (sym_keys_t &sym_keys){/*still unused : FIXME */ ;return FIM_ERR_NO_ERROR;}
	void finalize (void);
	struct fs_font * fb_font_get_current_font(void)
	{
	    return f_;
	}

	void switch_if_needed(void)
	{
		//fim's
		if (switch_last_ != fb_switch_state_)
		    console_switch(true);
	}

#define FIM_FBI_TRUE            1
#define FIM_FBI_FALSE           0
#define ARRAY_SIZE(x)   (sizeof(x)/sizeof(x[0]))

/*
 * framebuffer memory offset for x pixels left and y right from the screen
 * (by dez)
 */
#define FB_BPP  (((fb_var_.bits_per_pixel+7)/8))
#define FB_MEM_LINE_OFFSET  ((FB_BPP*fb_var_.xres))
#define FB_MEM_OFFSET(x,y)  (( FB_BPP*(x) + FB_MEM_LINE_OFFSET * (y) ))
#define FB_MEM(x,y) ((fb_mem_+FB_MEM_OFFSET((x),(y))))



//void svga_display_image_new(struct ida_image *img, int xoff, int yoff,unsigned int bx,unsigned int bw,unsigned int by,unsigned int bh,int mirror,int flip);
//void svga_display_image_new(struct ida_image *img, int xoff, int yoff,unsigned int bx,unsigned int bw,unsigned int by,unsigned int bh,int mirror,int flip);

fim_err_t display(
	//struct ida_image *img,
	void *ida_image_img, // source image structure
	fim_coo_t yoff,
	fim_coo_t xoff,
	fim_coo_t irows,fim_coo_t icols,// rows and columns in the input image
	fim_coo_t icskip,	// input columns to skip for each line
	fim_coo_t by,
	fim_coo_t bx,
	fim_coo_t bh,
	fim_coo_t bw,
	fim_coo_t ocskip,// output columns to skip for each line
	fim_flags_t flags);


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
inline fim_byte_t * clear_line(int bpp, int line, int owidth, fim_byte_t *dst);
fim_byte_t * convert_line(int bpp, int line, int owidth, fim_byte_t *dst, fim_byte_t *buffer, int mirror);/*dez's mirror patch*/
fim_byte_t * convert_line_8(int bpp, int line, int owidth, fim_byte_t *dst, fim_byte_t *buffer, int mirror);/*dez's mirror patch*/







void init_dither(int shades_r, int shades_g, int shades_b, int shades_gray);
inline void dither_line(fim_byte_t *src, fim_byte_t *dst, int y, int width,int mirror);

void dither_line_gray(fim_byte_t *src, fim_byte_t *dst, int y, int width);


void fb_switch_release(void);

void fb_switch_acquire(void);

int fb_switch_init(void);

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
	red_[i]   = calc_gamma(rs * ((i / (g * b)) % r), 255);
	green_[i] = calc_gamma(gs * ((i / b) % g),       255);
	blue_[i]  = calc_gamma(bs * ((i) % b),           255);
    }
}


unsigned short calc_gamma(int n, int max)
{
    int ret =(int)(65535.0 * pow((float)n/(max), 1 / fbgamma_)); 
    if (ret > 65535) ret = 65535;
    if (ret <     0) ret =     0;
    return ret;
}

void linear_palette(int bit)
{
    int i, size = 256 >> (8 - bit);
    
    for (i = 0; i < size; i++)
        red_[i] = green_[i] = blue_[i] = calc_gamma(i,size);
}

void lut_init(int depth)
{
    if (fb_var_.red.length   &&
	fb_var_.green.length &&
	fb_var_.blue.length) {
	/* fb_var_.{red|green|blue} looks sane, use it */
	init_one(lut_red_,   fb_var_.red.length,   fb_var_.red.offset);
	init_one(lut_green_, fb_var_.green.length, fb_var_.green.offset);
	init_one(lut_blue_,  fb_var_.blue.length,  fb_var_.blue.offset);
    } else {
	/* fallback */
	int i;
	switch (depth) {
	case 15:
	    for (i = 0; i < 256; i++) {
		lut_red_[i]   = (i & 0xf8) << 7;	/* bits -rrrrr-- -------- */
		lut_green_[i] = (i & 0xf8) << 2;	/* bits ------gg ggg----- */
		lut_blue_[i]  = (i & 0xf8) >> 3;	/* bits -------- ---bbbbb */
	    }
	    break;
	case 16:
	    for (i = 0; i < 256; i++) {
		lut_red_[i]   = (i & 0xf8) << 8;	/* bits rrrrr--- -------- */
		lut_green_[i] = (i & 0xfc) << 3;	/* bits -----ggg ggg----- */
		lut_blue_[i]  = (i & 0xf8) >> 3;	/* bits -------- ---bbbbb */
	    }
	    break;
	case 24:
	    for (i = 0; i < 256; i++) {
		lut_red_[i]   = i << 16;	/* byte -r-- */
		lut_green_[i] = i << 8;	/* byte --g- */
		lut_blue_[i]  = i;		/* byte ---b */
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

	int width(void)
	{
		return fb_var_.xres;
	}

	int height(void)
	{
		return fb_var_.yres;
	}

	int get_chars_per_column(void)
	{
		return fb_var_.yres / fb_font_height();
	}

	int get_chars_per_line(void)
	{
		return fb_var_.xres / fb_font_width();
	}

	fim_bool_t handle_console_switch(void)
	{
		if (switch_last_ == fb_switch_state_)return false;

		console_switch(true);
		return true;
        }

	//void status_screen(const fim_char_t *msg, int draw);
	void fs_render_fb(fim_byte_t *ptr, int pitch, FSXCharInfo *charInfo, fim_byte_t *data);
	fim_bpp_t get_bpp(void){return fb_var_.bits_per_pixel; };
	virtual ~FramebufferDevice(void);
	virtual fim_coo_t status_line_height(void)const;
};

}



#endif /* FIM_WITH_NO_FRAMEBUFFER */
#endif /* FIM_FRAMEBUFFER_DEVICE_H */

