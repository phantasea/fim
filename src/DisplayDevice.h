/* $Id: DisplayDevice.h 255 2009-08-20 14:18:47Z dezperado $ */
/*
 DisplayDevice.h : virtual device Fim driver header file

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
#ifndef FIM_DISPLAY_DEVICE_H
#define FIM_DISPLAY_DEVICE_H

#include "DebugConsole.h"

class DisplayDevice
{
	protected:
	bool finalized;
#ifndef FIM_KEEP_BROKEN_CONSOLE
	public:
	MiniConsole & mc;
#endif
	/*
	 * The generalization of a Fim output device.
	 */
	public:
	struct fs_font *f;
	const char* fontname;
	int                        debug;// really, only for making happy fbdev
	public:
	DisplayDevice(MiniConsole &mc_);
	virtual int initialize(key_bindings_t &key_bindings)=0;
	virtual void  finalize()=0;

	virtual int  display(
		void *ida_image_img, // source image structure
		int iroff,int icoff, // row and column offset of the first input pixel
		int irows,int icols,// rows and columns in the input image
		int icskip,	// input columns to skip for each line
		int oroff,int ocoff,// row and column offset of the first output pixel
		int orows,int ocols,// rows and columns to draw in output buffer
		int ocskip,// output columns to skip for each line
		int flags// some flags
		)=0;

	virtual ~DisplayDevice();

	virtual void flush(){};
	virtual void lock(){}
	virtual void unlock(){}
	virtual int get_chars_per_line()=0;
	virtual int get_chars_per_column()=0;
	virtual int width()=0;
	virtual int get_bpp()=0;
	virtual int height()=0;
	virtual int status_line(const unsigned char *msg)=0;
	int console_control(int code);
	virtual int handle_console_switch()=0;
	virtual int clear_rect(int x1, int x2, int y1,int y2)=0;
	virtual int get_input(unsigned int * c);
	virtual int catchInteractiveCommand(int seconds)const;
	virtual int init_console();
	virtual void switch_if_needed(){}// really, only for making happy fbdev
	virtual void cleanup(){}// really, only for making happy fbdev

	int redraw;
	virtual int fs_puts(struct fs_font *f, unsigned int x, unsigned int y, const unsigned char *str)=0;
	void fb_status_screen_new(const char *msg, int draw, int flags);//experimental
	void quickbench();
	private:
	virtual void console_switch(int is_busy){}// really, only for making happy fbdev
};

#endif
