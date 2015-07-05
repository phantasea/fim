/* $Id: SDLDevice.h 236 2009-04-03 22:26:03Z dezperado $ */
/*
 SDLDevice.h : sdllib device Fim driver header file

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
#ifndef FIM_SDLDEVICE_H
#define FIM_SDLDEVICE_H
#ifdef FIM_WITH_LIBSDL

#include "DisplayDevice.h"
#include <SDL.h>

class SDLDevice:public DisplayDevice 
{
	private:

	SDL_Surface *screen;
	SDL_Event event;
	const SDL_VideoInfo* vi;

	int keypress ;
	int h;

	int current_w;
	int current_h;
	int Bpp,bpp;

	public:

	SDLDevice(MiniConsole & mc_);

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
	void finalize() ;

	int get_chars_per_line() ;
	int get_chars_per_column();
	int txt_width() ;
	int txt_height() ;
	int width();
	int height();
	int status_line(const unsigned char *msg);
	void status_screen(int desc,int draw_output){ return ; }
	int handle_console_switch(){return 0;}
	int clear_rect_(
		void* dst,
		int oroff,int ocoff,
		int orows,int ocols,
		int ocskip);
	int clear_rect(int x1, int x2, int y1,int y2);


	/* TEMPORARY */
	inline void setpixel(SDL_Surface *screen, int x, int y, Uint8 r, Uint8 g, Uint8 b);

	int get_input(unsigned int * c);
	int catchInteractiveCommand(int seconds);
	
	void fs_render_fb(int x, int y, FSXCharInfo *charInfo, unsigned char *data);
	int fs_puts(struct fs_font *f, unsigned int x, unsigned int y, const unsigned char *str);
	int fill_rect(int x1, int x2, int y1,int y2, int color);
	void flush();
	void lock();
	void unlock();
	int get_bpp(){return bpp; };
};


#endif
#endif
