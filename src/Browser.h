/* $LastChangedDate: 2015-02-12 18:27:46 +0100 (Thu, 12 Feb 2015) $ */
/*
 Browser.h : Image browser header file

 (c) 2007-2015 Michele Martone

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
#ifndef FIM_BROWSER_H
#define FIM_BROWSER_H
#include "fim.h"

#define FIM_FLAG_DEFAULT 0
#define FIM_FLAG_PUSH_REC 1

namespace fim
{
	extern CommandConsole cc;
/*
 * A Browser object oversees image browsing.
 */
#ifdef FIM_NAMESPACES
class Browser:public Namespace
#else /* FIM_NAMESPACES */
class Browser
#endif /* FIM_NAMESPACES */
{
	private:
	args_t flist_; /* the names of files in the slideshow.  */

	const fim::string nofile_; /* a dummy empty filename */

	/*
	 * cf_ is zero only when there are no files in the list.
	 * the current file index is in current_n()
	 */
	fim_int cf_;

#ifndef FIM_WINDOWS
	/* when compiled with no multiple windowing support, one viewport only will last. */
	Viewport *only_viewport_;
#endif /* FIM_WINDOWS */
	CommandConsole &commandConsole_;
	Image *image(void)const;

#ifdef FIM_READ_STDIN_IMAGE
	Image *default_image_;	// experimental
#endif /* FIM_READ_STDIN_IMAGE */
	Viewport* viewport(void)const;

	int current_n(void)const;
	int current_n(int ccp)const;
	const fim::string pop(fim::string filename=FIM_CNS_EMPTY_STRING);
	fim::string get_next_filename(int n)const;
	
	fim_int current_image(void)const;
	public:
	fim::string last_regexp_; // was private
	int last_src_dir_;
	Cache cache_;	// was private
#ifdef FIM_READ_STDIN_IMAGE
	void set_default_image(Image *stdin_image);
#endif /* FIM_READ_STDIN_IMAGE */
	const Image *c_image(void)const;	// was private
	int empty_file_list(void)const;

	Browser(CommandConsole &cc);
	~Browser(void) { }
	private:
	Browser& operator= (const Browser &b){return *this;/* a nilpotent assignation */}
	Browser(const Browser &b):
		Namespace(b),
		flist_(args_t()),
		nofile_(""),
		cf_(0),
		commandConsole_(cc),
#ifdef FIM_READ_STDIN_IMAGE
		default_image_(NULL),
#endif /* FIM_READ_STDIN_IMAGE */
		last_regexp_(fim::string()),
		last_src_dir_(0),
		cache_(Cache())
		{}
	public:
	fim::string current(void)const;
	fim::string regexp_goto(const args_t &args, int src_dir=1);
	fim::string fcmd_prefetch(const args_t &args);
	fim::string fcmd_goto_image(const args_t &args);
	fim::string goto_image_internal(const fim_char_t *s, fim_xflags_t xflags);
	fim::string goto_image(int n, bool isfg=false);
	fim::string fcmd_align(const args_t &args);
	fim::string pan(const args_t &args);
	fim::string fcmd_scrolldown(const args_t &args);
	fim::string fcmd_scrollforward(const args_t &args);
	fim::string fcmd_scale(const args_t &args);
#if FIM_WANT_FAT_BROWSER
	fim::string fcmd_reduce(const args_t &args);
	fim::string fcmd_magnify(const args_t &args);
	fim::string fcmd_next(const args_t &args);
	fim::string fcmd_prev(const args_t &args);
	fim::string fcmd_no_image(const args_t &args);
#endif /* FIM_WANT_FAT_BROWSER */
	fim::string fcmd_rotate(const args_t &args);/* FIXME : UNFINISHED */
	fim::string fcmd_display(const args_t &args);
	fim::string display_status(const fim_char_t *l);
	fim::string fcmd_negate(const args_t &args);
	fim::string fcmd_desaturate(const args_t &args);

	fim::string fcmd_reload(const args_t &args);
	fim::string fcmd_list(const args_t &args);
	fim::string do_push(const args_t &args);
	fim::string prev(int n=1);
	fim::string do_remove(const args_t &args);
	fim::string fcmd_info(const args_t &args);
	fim::string info(void);
	std::ostream& print(std::ostream &os)const;
	void redisplay(void);
	fim::string fcmd_redisplay(const args_t &args);
	fim::string fcmd_load(const args_t &args);
	const fim::string pop_current(void);
	fim::string pop_current(const args_t &args);
	bool present(const fim::string nf)const;
	fim_int find_file_index(const fim::string nf)const;
#ifdef FIM_READ_DIRS
	bool push_dir(fim::string nf, fim_flags_t pf=FIM_FLAG_PUSH_REC);
#endif /* FIM_READ_DIRS */
	bool push(fim::string nf, fim_flags_t pf=FIM_FLAG_PUSH_REC);

	fim::string display(void);
	fim::string _random_shuffle(bool dts=true);
	fim::string _sort(const fim_char_t sc='f');
	fim::string _clear_list(void);
	private:
	fim_err_t loadCurrentImage(void);
	fim::string reload(void);

	fim_int n_files(void)const;
	fim_int n_pages(void)const;
	fim::string _reverse(void);
	fim::string next(int n=1);

	void free_current_image(void);
	int load_error_handle(fim::string c);
	public:
	fim_int c_page(void)const;
	virtual size_t byte_size(void)const;
};
}

#endif /* FIM_BROWSER_H */
