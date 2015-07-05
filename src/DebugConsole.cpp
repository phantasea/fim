/* $Id: DebugConsole.cpp 274 2010-01-21 02:11:06Z dezperado $ */
/*
 DebugConsole.cpp : Fim virtual console display.

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

#include "fim.h"

namespace fim
{

		#define min(x,y) ((x)<(y)?(x):(y))
		int MiniConsole::line_length(int li)
		{
			if(li<cline)
			{
				return li<cline?(line[li+1]-line[li]):(ccol);
			}
			else
			if(li<0)return 0;
			// in the case li==cline, ccol==bp-buffer will do the job:
			return ccol;
		}

		int MiniConsole::do_dump(int f, int l)const
		{
			/*
			 *
			 * if f <= l and f>= 0 
			 * lines from min(f,cline) to min(l,cline) will be dumped
			 *
			 * if f<0 and l>=0 and f+l<0 and -f<=cline,
			 * lines from cline+f to cline-l will be dumped
			 *
			 * FIXME : this function returns often with -1 !
			 * */
			int i;
			const int maxcols = cc.displaydevice->get_chars_per_line();
	
			if(f<0 && l>=0 && f+l<0 && -f<=cline) { f=cline+f; l=cline-l; }
			else
				if(f<=l && f>=0){f=f>cline?cline:f;l=l>cline?cline:l;}
			else
				return -1;// unsupported combination
			
			if(maxcols<1)return -1;

			char *buf = (char*) fim_malloc(maxcols+1); // ! we work on a stack, don't we ?! Fortran teaches us something here.
			if(!buf)return -1;

			if(*bp){fim_free(buf);return -1;}//if *bp then we could print garbage so we exit before damage is done

			int fh=cc.displaydevice->f ? cc.displaydevice->f->height:1; // FIXME : this is not clean
			l-=f; l%=(rows+1); l+=f;

			/* FIXME : the following line is redundant in fb, but not in SDL 
			 * moreover, it seems useless in AA (could be a bug)
			 * */
			cc.displaydevice->clear_rect(0, cc.displaydevice->width()-1, 0 ,fh*(l-f+1) );

			// fs_puts alone won't draw on screen, but in the back plance, so unlock/flip will be necessary
			cc.displaydevice->lock();

	    		for(i=f  ;i<=l   ;++i)
			{
				int t = (i<cline?(line[i+1]-line[i]):(ccol))%(min(maxcols,lwidth)+1);
				if( t<0 ){fim_free(buf);return -1;} // hmmm
				strncpy(buf,line[i],t);
				while(buf[t-1]=='\n' && t>0)--t;
				buf[maxcols]='\0';
				while(t<maxcols){buf[t++]=' ';}
				/*
				 * Since the user is free to set any value > 0 for the line width,
				 * we truncate the line for the interval exceeding the screen width.
				 * */
				buf[ maxcols ]='\0';
				cc.displaydevice->fs_puts(cc.displaydevice->f, 0, fh*(i-f), (unsigned char*)buf);
			}

			/* FIXME : THE FOLLOWING IS A FIXUP FOR AA!  */
	    		for(i=0  ;i<scroll ;++i)
			{
				int t = (min(maxcols,lwidth)+1);
				memset(buf,' ',t);
				buf[t-1]='\0';
				cc.displaydevice->fs_puts(cc.displaydevice->f, 0, fh*((l-f+1)+i), (unsigned char*)buf);
			}
			cc.displaydevice->unlock();

			cc.displaydevice->flush();
			fim_free(buf);
			return 0;
		#undef min
		}

		int MiniConsole::add(const char* cso)
		{
			char *s=NULL,*b=NULL;
			int nc;
			int nl,ol=cline;
			char *cs=NULL;/* using co would mean provoking the compiler */

			cs=dupstr(cso);

			if(!cs)goto rerr;
			nc=strlen(cs);
			if(!nc)goto rerr;
			nl=lines_count(cs,lwidth);
			// we count exactly the number of new entries needed in the arrays we have
			if((s=const_cast<char*>(strchr(cs,'\n')))!=NULL && s!=cs)nl+=(ccol+(s-cs-1))/lwidth;// single line with \n or multiline
			else nl+=(strlen(cs)+ccol)/lwidth;	// single line, with no terminators

			/*
			 * we check for room (please note that nl >= the effective new lines introduced , while
			 * nc amounts to the exact extra room needed )
			 * */
			if(nc+1+(int)(bp-buffer)>bsize || nl+1+cline>lsize)return -2;//no room : realloc needed ; 1 is for secur1ty
			scroll=scroll-nl<0?0:scroll-nl;

			// we copy the whole new string in our buffer
			strcpy(bp,cs);
			fim_free(cs); cs=NULL;
			sanitize_string_from_nongraph_except_newline(bp,0);
			s=bp-ccol;// we will work in our buffer space now on
			b=s;
			while(*s && (s=(char*)next_row(s,lwidth))!=NULL && *s)
			{
				line[++cline]=s;// we keep track of each new line
				ccol=0;
				bp=s;
			}// !s || !*s
			if(!*s && s-b==lwidth){line[++cline]=(bp=s);}// we keep track of the last line too
			

			if(ol==cline)
			{
				ccol=strlen(line[cline]);	// we update the current (right after last) column
				bp+=strlen(bp);	// the buffer points to the current column
			}
			else
			{
				ccol=strlen(bp);	// we update the current (right after last) column
				bp+=ccol;	// the buffer points to the current column
			}
			return 0;
rerr:
			fim_free(cs);
			return -1;
		}

		int MiniConsole::setRows(int nr)
		{
			/*
			 * We update the displayed rows, if this is physically possible
			 * If nr is negative, no correctness checks will occur.
			 * ( useful if calling this routine with NULL displaydevice.. )
			 * */
			int maxrows;
			if(nr<0)
			{
				rows=-nr;
				return 0;
			}
			maxrows = cc.displaydevice->get_chars_per_column();
			if(nr>0 && nr<=maxrows)
			{
				rows=nr;
				return 0;
			}
			return -1;
		}

		MiniConsole::MiniConsole(CommandConsole & cc_,int lw, int r)
		:
		cc(cc_),
		buffer(NULL),
		line(NULL),
		bp(NULL),
		bsize(0),
		lsize(0),
		ccol(0),
		cline(0),
		lwidth(0),
		rows(0)
		{
			/*
			 * We initialize the console
			 * */
			int BS=1024;	//block size of 1k

			bsize = BS * 128;
			lsize = BS *   8;

			lwidth=lw<=0?128:lw;
			rows=r<=0?24:r;

			cline =0;
			ccol  =0;
			buffer=(char*) fim_calloc(bsize,sizeof(char ));
			line  =(char**)fim_calloc(lsize,sizeof(char*));
			bp    =buffer;
			if(!buffer || !line)
			{
				bsize=0;
				lsize=0;
				if(buffer)fim_free(buffer);
				if(line  )fim_free(line);
			}
			else
			{
			
				line[cline]=buffer;
			}
		}

		int MiniConsole::do_dump(int amount)const
		{
			/*
			 * We dump:
			 * 	the last amount of lines if	amount >  0
			 * 	all the lines if		amount == 0
			 * 	the first ones if		amount <  0
			 * */
			if(amount > 0)
			{
				// dumps the last amount of lines
				amount=amount>cline?cline:amount;
				do_dump(cline-amount+1,cline);
			}
			else
			if(amount ==0)
			{
				// dumps all of the lines
				do_dump(0,cline);
			}
			else
			if(amount < 0)
			{
				// dumps the first amount of lines
				if(-amount>=cline)amount+=cline;
				do_dump(0,-amount);
			}
			return 0;
		}

		int MiniConsole::grow_lines(int glines)
		{
			/*
			 * grow of a specified amount of lines the lines array
			 *
			 * see the doc for grow() to get more
			 * */
			/* TEST ME AND FINISH ME */
			if(glines< 0)return -1;
			if(glines==0)return  0;
			char **p;
			p=line;
			line=(char**)realloc(line,bsize+glines*sizeof(char*));
			if(!line){line=p;return -1;/* no change */}
			lsize+=glines;
			return 0;
		}

		int MiniConsole::grow_buffer(int gbuffer)
		{
			/*
			 * grow of a specified amount of lines the buffer array
			 *
			 * see the doc for grow() to get more
			 * */
			/* TEST ME AND FINISH ME */
			if(gbuffer< 0)return -1;
			if(gbuffer==0)return  0;
			char *p;int i,d;
			p=buffer;
			buffer=(char*)realloc(buffer,(bsize+gbuffer)*sizeof(char));
			if(!buffer){buffer=p;return -1;/* no change */}
			if((d=(p-buffer))!=0)// in the case a shift is needed
			{
				for(i=0;i<cline;++i)line[i]-=d;
				bp-=d;
			}
			bsize+=gbuffer;
			return 0;
		}

		int MiniConsole::grow()
		{
			/*
			 * We grow a specified amount both the line count and the line buffer.
			 * */
			return grow(1024,8*1024);
		}

		int MiniConsole::grow(int glines, int gbuffer)
		{
			/*
			 * grow of a specified amount of lines or bytes the 
			 * current line and text buffers; i.e.: make room
			 * to support glines more lines and gbuffer more characters.
			 *
			 * grow values can be negative. in this case, the current 
			 * buffers will be shrunk of the specified amounts.
			 *
			 * consistency will be preserved by deleting a certain amount
			 * of lines: the older ones.
			 *
			 * a zero amount for a value imply the line or buffer arrays
			 * to remain untouched.
			 * */
			/* FINISH ME AND TEST ME */
			int gb,gl;
			gb=grow_buffer(gbuffer);
			gl=grow_lines (glines);
			return !( gb==0 && 0==gl );// 0 if both 0
		}

		int MiniConsole::reformat(int newlwidth)
		{
			/*
			 * This method reformats the whole buffer array; that is, it recomputes
			 * line information for it, thus updating the whole line array contents.
			 * It may fail, in the case a new line width (smaller) is desired, because
			 * more line information would be needed.
			 *
			 * If the new lines are longer than before, then it could not fail.
			 * Upon a successful execution, the width is updated.
			 * 
			 * */
			int nls;
			if(newlwidth==lwidth)return 0;//are we sure?
			if(newlwidth< lwidth)
			{
				// realloc needed
				if ( ( nls=lines_count(buffer, newlwidth) + 1 ) < lsize )
				if ( grow_lines( nls )!=0 )return -1;
			}
			if(newlwidth> lwidth || ( lines_count(buffer, newlwidth) + 1 < lsize ) )
			{
				// no realloc, no risk
				fim::string buf=buffer;
				if(((int)buf.size())==((int)(bp-buffer)))
				{
					ccol=0;cline=0;lwidth=newlwidth;*line=buffer;bp=buffer;
					// the easy way
					add(buf.c_str());// by adding a very big chunk of text, we make sure it gets formatted.
					return 0;
				}
				// if some error happened in buf string initialization, we return -1
				return -1;
			}
			return -1;
		}

		int MiniConsole::dump()
		{
			/*
			 * We dump on screen the textual console contents.
			 * We care about user set variables.
			 * */
			int co=getGlobalIntVariable(FIM_VID_CONSOLE_LINE_OFFSET);
			int lw=getGlobalIntVariable(FIM_VID_CONSOLE_LINE_WIDTH );
			int ls=getGlobalIntVariable(FIM_VID_CONSOLE_ROWS       );
			setGlobalVariable(FIM_VID_CONSOLE_BUFFER_TOTAL,bsize);
			setGlobalVariable(FIM_VID_CONSOLE_BUFFER_FREE,(int)bsize-(int)(bp-buffer));
			setGlobalVariable(FIM_VID_CONSOLE_BUFFER_USED,(int)(bp-buffer));
			// we eventually update internal variables now
			setRows(ls);
			if( lw > 0 && lw!=lwidth ) reformat(lw);
			if(co>=0)
			{
				scroll=scroll%(rows+1);
				if(scroll>0)
					return do_dump((cline-rows+1-co)>=0?(cline-(rows-scroll)+1-co):0,cline-co);
				else
					return do_dump((cline-rows+1-co)>=0?(cline-rows+1-co):0,cline-co);
			}
			else
				return do_dump(-co-1,cline);
			return -1;
		}

		int MiniConsole::do_dump()const
		{
			/*
			 * We dump on screen the textual console contents.
			 * */
			return do_dump((cline-rows+1)>=0?(cline-rows+1):0,cline);
		}

		int MiniConsole::clear()
		{
			scroll=rows;
			return 0;
		}

		int MiniConsole::scroll_down()
		{
			scroll=scroll<1?0:--scroll;
			return 0;
		}

		int MiniConsole::scroll_up()
		{
			scroll=scroll<rows?++scroll:scroll;
			return 0;
		}
}

