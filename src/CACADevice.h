/* $Id: CACADevice.h 203 2009-01-10 11:13:17Z dezperado $ */
/*
 CACADevice.h : cacalib device Fim driver header file

 (c) 2008-2009 Michele Martone

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
#ifndef FIM_CACADEVICE_H
#define FIM_CACADEVICE_H
#ifdef FIM_WITH_CACALIB

#include "DisplayDevice.h"
#include <caca.h>

/*  20080504 this CACA driver doesn't work yet */
class CACADevice:public DisplayDevice 
{
	private:
	unsigned int r[256], g[256], b[256], a[256];

	int XSIZ, YSIZ;
	struct caca_bitmap *caca_bitmap;
	char *bitmap;

	public:

	int  display(
		void *ida_image_img, // source image structure (struct ida_image *)(but we refuse to include header files here!)
		//void* rgb,// destination gray array and source rgb array
		int iroff,int icoff, // row and column offset of the first input pixel
		int irows,int icols,// rows and columns in the input image
		int icskip,	// input columns to skip for each line
		int oroff,int ocoff,// row and column offset of the first output pixel
		int orows,int ocols,// rows and columns to draw in output buffer
		int ocskip,// output columns to skip for each line
		int flags// some flags
		);
	int initialize(key_bindings_t &key_bindings);
	void finalize();

	int get_chars_per_line();
	int txt_width();
	int txt_height();
	int width();
	int height();
	int status_line(unsigned char *msg);
	void status_screen(int desc,int draw_output){}
	int console_control(int code){}
	int handle_console_switch(){}
	int clear_rect_(
		void* dst,
		int oroff,int ocoff,
		int orows,int ocols,
		int ocskip);
	int clear_rect(int x1, int x2, int y1,int y2)
	{
		/* FIXME : only if initialized !*/
		return -1;
	}
	int fs_puts(struct fs_font *f, unsigned int x, unsigned int y, unsigned char *str){return 0;}

};


#endif
#endif
