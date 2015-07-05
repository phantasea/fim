/* $Id: DummyDisplayDevice.h 236 2009-04-03 22:26:03Z dezperado $ */
/*
 DummyDisplayDevice.h : virtual device Fim driver header file

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
#ifndef FIM_DUMMYDISPLAY_DEVICE_H
#define FIM_DUMMYDISPLAY_DEVICE_H


class DummyDisplayDevice:public DisplayDevice
{
	/*
	 * The generalization of a Fim output device.
	 */
	public:
	virtual int initialize(key_bindings_t &key_bindings){return 0;}
	virtual void  finalize(){}

	virtual int  display(
		void *ida_image_img, // source image structure
		int iroff,int icoff, // row and column offset of the first input pixel
		int irows,int icols,// rows and columns in the input image
		int icskip,	// input columns to skip for each line
		int oroff,int ocoff,// row and column offset of the first output pixel
		int orows,int ocols,// rows and columns to draw in output buffer
		int ocskip,// output columns to skip for each line
		int flags// some flags
		){return 0;}

	DummyDisplayDevice(MiniConsole & mc_):DisplayDevice(mc_){}
	virtual ~DummyDisplayDevice(){}

	virtual int get_chars_per_line(){return 1;/* 0 would be so cruel */}
	virtual int get_chars_per_column(){return 1;/* 0 would be so cruel */}
	virtual int width(){return 1;/* 0 would be so cruel */}
	virtual int height(){return 1;/* 0 would be so cruel */}
	virtual int status_line(const unsigned char *msg){return 0;}
	virtual int console_control(int code){return 0;}
	virtual int handle_console_switch(){return 0;}
	virtual int clear_rect(int x1, int x2, int y1,int y2){return 0;}
	int fs_puts(struct fs_font *f, unsigned int x, unsigned int y, const unsigned char *str){return 0;}
	virtual int get_bpp(){return 0;};

	private:
};

#endif
