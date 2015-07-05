/* $Id: Browser.h 272 2009-12-21 17:10:21Z dezperado $ */
/*
 Browser.h : Image browser header file

 (c) 2007-2009 Michele Martone

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
namespace fim
{
/*
 * A Browser object oversees image browsing.
 */
#ifdef FIM_NAMESPACES
class Browser:public Namespace
#else
class Browser
#endif
{
	private:
	/*
	 * A file browser holds the names of files in the slideshow.
	 */
	args_t flist;

	/*
	 * It has a dummy empty filename for technical reasons
	 */
	const fim::string nofile;

	/*
	 * And it keeps a numerical index of the current file, too.
	 *
	 * cp is zero only when there are no files in the list.
	 * the current file index is in current_n()
	 */
	int cp;

#ifndef FIM_WINDOWS
	/*
	 * When compiled with no multiple windowing support, one viewport only will last.
	 * */
	Viewport *only_viewport;
#endif
	CommandConsole &commandConsole;
	Image *image()const;

#ifdef FIM_READ_STDIN_IMAGE
	Image *default_image;	// experimental
#endif

	Viewport* viewport()const;

	int current_n()const;
	int current_n(int ccp)const;
	const fim::string pop(fim::string filename="");
	fim::string get_next_filename(int n)const;
	
	fim::string last_regexp;
	int current_image()const;
	int current_images()const{ return n(); }
	public:
	Cache cache;	// was private
#ifdef FIM_READ_STDIN_IMAGE
	void set_default_image(Image *stdin_image);
#endif
	const Image *c_image()const;	// was private
	int empty_file_list()const;

	Browser(CommandConsole &cc);
	~Browser() { }
	private:
	Browser& operator= (const Browser &b){return *this;/* a nilpotent assignation */}
	Browser(const Browser &b):
		flist(args_t()),
		nofile(""),
		cp(0),
		commandConsole(cc),
#ifdef FIM_READ_STDIN_IMAGE
		default_image(NULL),
#endif
		last_regexp(fim::string()),
		cache(Cache())
		{}
	public:
	fim::string current()const;
	fim::string regexp_goto(const args_t &args);
	fim::string prefetch(const args_t &args);
	fim::string regexp_goto_next(const args_t &args);
	fim::string goto_image(const args_t &args);
	fim::string goto_image(int n);
	fim::string top_align(const args_t &args);
	fim::string bottom_align(const args_t &args);
	fim::string pan_ne(const args_t &args);
	fim::string pan_nw(const args_t &args);
	fim::string pan_sw(const args_t &args);
	fim::string pan_se(const args_t &args);
	fim::string pan_up(const args_t &args);
	fim::string pan_down(const args_t &args);
	fim::string pan_right(const args_t &args);
	fim::string pan_left(const args_t &args);
	fim::string scrolldown(const args_t &args);
	fim::string scrollforward(const args_t &args);
	fim::string scale_increment(const args_t &args);
	fim::string scale_multiply(const args_t &args);
	fim::string auto_scale(const args_t &args);
	fim::string auto_width_scale(const args_t &args);
	fim::string scale(const args_t &args);
	fim::string auto_height_scale(const args_t &args);
	fim::string reduce(const args_t &args);
	fim::string magnify(const args_t &args);
	fim::string scale_factor_increase(const args_t &args);
	fim::string scale_factor_decrease(const args_t &args);
	fim::string rotate(const args_t &args);/* FIXME : UNFINISHED */
	fim::string scale_factor_grow(const args_t &args);
	fim::string scale_factor_shrink(const args_t &args);
	fim::string display(const args_t &args);
	fim::string display_status(const char *l,const char*r);
	fim::string negate(const args_t &args);

	fim::string reload(const args_t &args);
	fim::string list(const args_t &args){return list();}
	fim::string push(const args_t &args);

	fim::string n(const args_t &args){return n();}
	fim::string _sort(const args_t &args){return _sort();}
	fim::string next(const args_t &args);
	fim::string next_picture(const args_t &args);
	fim::string prev_picture(const args_t &args);
	fim::string next_page(const args_t &args);
	fim::string prev_page(const args_t &args);
	fim::string prev(int n=1);
	fim::string prev(const args_t &args);
	fim::string remove(const args_t &args);
	fim::string info(const args_t &args);
	fim::string info();
	std::ostream& print(std::ostream &os)const;
	void redisplay();
	fim::string redisplay(const args_t &args);


	fim::string load(const args_t &args);
	fim::string pop(const args_t &args);
	const fim::string pop_current();
	fim::string pop_current(const args_t &args);
	fim::string no_image(const args_t &args);
	bool present(const fim::string nf);
#ifdef FIM_READ_DIRS
	bool push_dir(fim::string nf);
#endif
	bool push(fim::string nf);

	fim::string display();
	private:
	fim::string loadCurrentImage();
	fim::string reload();
	fim::string list()const;

	int n_files()const;
	const fim::string n()const;
	fim::string _sort();
	fim::string next(int n);
	fim::string do_next(int n);

	void free_current_image();
	int load_error_handle(fim::string c);
	public:
};
}

#endif
