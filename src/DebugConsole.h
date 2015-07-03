/* $LastChangedDate: 2014-08-30 10:31:56 +0200 (Sat, 30 Aug 2014) $ */
/*
 DebugConsole.h : Fim virtual console display.

 (c) 2008-2014 Michele Martone

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
#ifndef FIM_CONSOLE_H
#define FIM_CONSOLE_H


#include <vector>
#include "fim.h"
#include "Var.h"
#include "Namespace.h"

#ifndef FIM_WANT_NO_OUTPUT_CONSOLE
namespace fim
{
	class FimConsole
	{
		public:
		//virtual void add(string s)=0;
	//	virtual int do_dump(int f, int l)const;
	};

	class MiniConsole
#ifdef FIM_NAMESPACES
	:public Namespace
#endif /* FIM_NAMESPACES */
	{
		fim_char_t *buffer_;	// the raw console buffer
		fim_char_t **line_;	// the (displayed) line_ pointers array

		fim_char_t *bp_;	// pointer to the top of the buffer

		int  bsize_;	// the buffer size
		int  lsize_;	// the lines array size

		int  ccol_;	// the currently pointed column
		int  cline_;	// the line_ on the top of the buffer
		
		int  lwidth_;
		int  rows_;
		int  scroll_;

		public:
		CommandConsole & cc_;	// temporarily

		MiniConsole(CommandConsole & cc,int lw=48, int r=12); /* FIXME: shall get rid of numerical constants! */
		virtual ~MiniConsole(void){}
		fim_err_t dump(void);	// non const due to user variables reaction
		fim_err_t grow(void);
		fim_err_t setRows(int nr);
		fim_err_t add(const fim_char_t * cso);
		fim_err_t add(const fim_byte_t* cso);
		fim_err_t reformat(int newlsize);
		fim_err_t do_dump(int amount)const;
		fim_err_t clear(void);
		fim_err_t scroll_down(void);
		fim_err_t scroll_up(void);
		virtual size_t byte_size(void)const;

		private:
		MiniConsole& operator= (const MiniConsole&mc);
		MiniConsole(const MiniConsole &mc) :
			Namespace(mc),
			buffer_(NULL),
			line_(NULL),
			bp_(NULL),
			bsize_(0),
			lsize_(0),
			ccol_(0),
			cline_(0),
			lwidth_(0),
			rows_(0),
			scroll_(0),
			cc_(mc.cc_)
			{/* this constructor should not be used */}

		int line_length(int li);
		fim_err_t do_dump(int f, int l)const;
		fim_err_t do_dump(void)const;

		fim_err_t grow_lines(int glines);
		fim_err_t grow_buffer(int gbuffer);
		fim_err_t grow(int glines, int gbuffer);
	};
}
#endif /* FIM_WANT_NO_OUTPUT_CONSOLE */

#endif /* FIM_CONSOLE_H */

