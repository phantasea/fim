/* $LastChangedDate: 2013-11-06 19:33:57 +0100 (Wed, 06 Nov 2013) $ */
/*
 AADevice.h : aalib device Fim driver header file

 (c) 2008-2013 Michele Martone

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
#ifndef FIM_AADEVICE_H
#define FIM_AADEVICE_H
#ifdef FIM_WITH_AALIB

#include "DisplayDevice.h"
#include <aalib.h>

/*
 * Debugging only!
 * */
#define FIM_AALIB_DRIVER_DEBUG 0

class AADevice:public DisplayDevice 
{
	int allow_windowed;
	private:
	aa_context *ascii_context_;
	//struct aa_renderparams *ascii_rndparms;//we rely on aa_defrenderparams
	struct aa_hardware_params ascii_hwparms_;
	struct aa_savedata ascii_save_;
	fim_char_t name_[2];	/* For ascii_save_.name */
	public:
#ifndef FIM_WANT_NO_OUTPUT_CONSOLE
	AADevice(MiniConsole & mc_, fim::string opts);
#else /* FIM_WANT_NO_OUTPUT_CONSOLE */
	AADevice( fim::string opts );
#endif /* FIM_WANT_NO_OUTPUT_CONSOLE */

	virtual ~AADevice(void);

	virtual fim_err_t  display(
		void *ida_image_img, // source image structure (struct ida_image *)(but we refuse to include header files here!)
		//void* rgb,// destination gray array and source rgb array
		fim_coo_t iroff,fim_coo_t icoff, // row and column offset of the first input pixel
		fim_coo_t irows,fim_coo_t icols,// rows and columns in the input image
		fim_coo_t icskip,	// input columns to skip for each line
		fim_coo_t oroff,fim_coo_t ocoff,// row and column offset of the first output pixel
		fim_coo_t orows,fim_coo_t ocols,// rows and columns to draw in output buffer
		fim_coo_t ocskip,// output columns to skip for each line
		fim_flags_t flags// some flags
		);
	fim_err_t initialize(sym_keys_t &sym_keys);
	void finalize(void);

	int get_chars_per_line(void);
	int get_chars_per_column(void);
	fim_coo_t txt_width(void);
	fim_coo_t txt_height(void);
	fim_coo_t width(void);
	fim_coo_t height(void);
	fim_err_t status_line(const fim_char_t *msg);
	//void status_screen(int desc,int draw_output){}
	fim_bool_t handle_console_switch(void)
	{
		return false;
	}
	fim_err_t clear_rect_(
		void* dst,
		fim_coo_t oroff,fim_coo_t ocoff,
		fim_coo_t orows,fim_coo_t ocols,
		fim_coo_t ocskip);

	fim_err_t clear_rect(fim_coo_t x1, fim_coo_t x2, fim_coo_t y1, fim_coo_t y2);
	fim_err_t fs_puts(struct fs_font *f, fim_coo_t x, fim_coo_t y, const fim_char_t *str);
	void flush(void);
	fim_err_t init_console(void);
	fim_bpp_t get_bpp(void)
	{
		return 1;
	}
	fim_sys_int get_input(fim_key_t * c, bool want_poll=false);
	virtual fim_err_t reinit(const fim_char_t *rs);
	fim_err_t resize(fim_coo_t w, fim_coo_t h);
	virtual fim_coo_t status_line_height(void)const;
};

#endif /* FIM_WITH_AALIB  */
#endif /* FIM_AADEVICE_H*/
