/* $Id: DebugConsole.h 268 2009-12-08 23:05:55Z dezperado $ */
/*
 DebugConsole.h : Fim virtual console display.

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
#ifndef FIM_CONSOLE_H
#define FIM_CONSOLE_H


#include <vector>
#include "fim.h"
#include "Var.h"
#include "Namespace.h"

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
#endif
	{
		char *buffer;	// the raw console buffer
		char **line;	// the (displayed) line pointers array

		char *bp;	// pointer to the top of the buffer

		int  bsize;	// the buffer size
		int  lsize;	// the lines array size

		int  ccol;	// the currently pointed column
		int  cline;	// the line on the top of the buffer
		
		int  lwidth;
		int  rows;
		int  scroll;

		public:
		CommandConsole & cc;	// temporarily

		MiniConsole(CommandConsole & cc_,int lw=48, int r=12);
		virtual ~MiniConsole(){}
		int dump();	// non const due to user variables reaction
		int grow();
		int setRows(int nr);
		int add(const char* cso);
		int reformat(int newlsize);
		int do_dump(int amount)const;
		int clear();
		int scroll_down();
		int scroll_up();

		private:
		MiniConsole& operator= (const MiniConsole&mc){return *this;/* a nilpotent assignation */}
		MiniConsole(const MiniConsole &mc) :
			buffer(NULL),
			line(NULL),
			bp(NULL),
			bsize(0),
			lsize(0),
			ccol(0),
			cline(0),
			lwidth(0),
			rows(0),
			scroll(0),
			cc(mc.cc)
			{/* this constructor should not be used */}

		int line_length(int li);
		int do_dump(int f, int l)const;
		int do_dump()const;

		int grow_lines(int glines);
		int grow_buffer(int gbuffer);
		int grow(int glines, int gbuffer);
	};
}

#endif

