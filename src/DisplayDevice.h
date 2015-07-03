/* $LastChangedDate: 2013-11-06 19:33:57 +0100 (Wed, 06 Nov 2013) $ */
/*
 DisplayDevice.h : virtual device Fim driver header file

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
#ifndef FIM_DISPLAY_DEVICE_H
#define FIM_DISPLAY_DEVICE_H

#include "DebugConsole.h"

class DisplayDevice
#if FIM_WANT_BENCHMARKS
: public Benchmarkable
#endif /* FIM_WANT_BENCHMARKS */
{
	protected:
	fim_bool_t finalized_;
#ifndef FIM_WANT_NO_OUTPUT_CONSOLE
#ifndef FIM_KEEP_BROKEN_CONSOLE
	public:
	MiniConsole & mc_;
#endif /* FIM_KEEP_BROKEN_CONSOLE */
#endif /* FIM_WANT_NO_OUTPUT_CONSOLE */
	/*
	 * The generalization of a Fim output device.
	 */
	public:
	struct fs_font *f_;
	const fim_char_t* fontname_;
	fim_bool_t debug_;// really, only for making happy fbdev
	public:
#ifndef FIM_WANT_NO_OUTPUT_CONSOLE
	DisplayDevice(MiniConsole &mc_);
#else /* FIM_WANT_NO_OUTPUT_CONSOLE */
	DisplayDevice(void);
#endif /* FIM_WANT_NO_OUTPUT_CONSOLE */
	virtual fim_err_t initialize(sym_keys_t &sym_keys)=0;
	virtual void  finalize(void)=0;

	virtual fim_err_t display(
		void *ida_image_img, // source image structure
		fim_coo_t iroff,fim_coo_t icoff, // row and column offset of the first input pixel
		fim_coo_t irows,fim_coo_t icols,// rows and columns in the input image
		fim_coo_t icskip,	// input columns to skip for each line
		fim_coo_t oroff,fim_coo_t ocoff,// row and column offset of the first output pixel
		fim_coo_t orows,fim_coo_t ocols,// rows and columns to draw in output buffer
		fim_coo_t ocskip,// output columns to skip for each line
		fim_flags_t flags// some flags
		)=0;

	virtual ~DisplayDevice(void);

	virtual void flush(void){};
	virtual void lock(void){}
	virtual void unlock(void){}
	virtual int get_chars_per_line(void)=0;
	virtual int get_chars_per_column(void)=0;
	virtual fim_coo_t width(void)=0;
	virtual fim_bpp_t get_bpp(void)=0;
	virtual fim_coo_t height(void)=0;
	virtual fim_coo_t status_line_height(void)const=0;
	/* virtual fim_coo_t font_width(void)const{return 1;} */
	virtual fim_err_t status_line(const fim_char_t *msg)=0;
	fim_err_t console_control(fim_cc_t code);
	virtual fim_bool_t handle_console_switch(void)=0;
	virtual fim_err_t clear_rect(fim_coo_t x1, fim_coo_t x2, fim_coo_t y1,fim_coo_t y2)=0;
	virtual fim_sys_int get_input(fim_key_t * c, bool want_poll=false);
	virtual fim_key_t catchInteractiveCommand(fim_ts_t seconds)const;
	virtual fim_err_t init_console(void);
	virtual void switch_if_needed(void){}// really, only for making happy fbdev
	virtual void cleanup(void){}// really, only for making happy fbdev

	fim_redraw_t redraw_;
	virtual fim_err_t fs_puts(struct fs_font *f, fim_coo_t x, fim_coo_t y, const fim_char_t *str)=0;
	void fb_status_screen_new(const fim_char_t *msg, fim_bool_t draw, fim_flags_t flags);//experimental
#if FIM_WANT_BENCHMARKS
	virtual fim_int get_n_qbenchmarks(void)const;
	virtual string get_bresults_string(fim_int qbi, fim_int qbtimes, fim_fms_t qbttime)const;
	virtual void quickbench_init(fim_int qbi);
	virtual void quickbench_finalize(fim_int qbi);
	virtual void quickbench(fim_int qbi);
#endif /* FIM_WANT_BENCHMARKS */
	private:
	virtual void console_switch(fim_bool_t is_busy){}// really, only for making happy fbdev
	public:
	virtual fim_err_t resize(fim_coo_t w, fim_coo_t h);
	virtual fim_err_t reinit(const fim_char_t *rs);
	virtual fim_err_t set_wm_caption(const fim_char_t *msg);
};

#endif /* FIM_DISPLAY_DEVICE_H */
