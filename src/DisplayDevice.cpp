/* $Id: DisplayDevice.cpp 260 2009-10-08 17:53:46Z dezperado $ */
/*
 DisplayDevice.cpp : virtual device Fim driver file

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
#include "DisplayDevice.h"

	DisplayDevice::DisplayDevice(MiniConsole & mc_):fontname(NULL)
	,mc(mc_)
	,f(NULL)
	,debug(0)
	,redraw(0)
	,finalized(false)
	{}

	int DisplayDevice::get_input(unsigned int * c)
	{
		*c=0;
		/*
		 * It is sad to place this functionality here, but often the input subsystem 
		 * is tightly bound to the output device.
		 * */
			int r=0;
#ifdef  FIM_SWITCH_FIXUP
			/*
			 * this way the console switches the right way :
			 * the following code taken live from the original fbi.c
			 */

			/*
			 * patch: the following read blocks the program even when switching console
			 */
			//r=read(fim_stdin,&c,1); if(c==0x1b){read(0,&c,3);c=(0x1b)+(c<<8);}
			/*
			 * so the next coded shoul circumvent this behaviour!
			 */
			{
				fd_set set;
				int fdmax;
				struct timeval  limit;
				int timeout=1,rc,paused=0;
	
			        FD_ZERO(&set);
			        FD_SET(cc.fim_stdin, &set);
			        fdmax = 1;
#ifdef FBI_HAVE_LIBLIRC
				/*
				 * expansion code :)
				 */
			        if (-1 != lirc) {
			            FD_SET(lirc,&set);
			            fdmax = lirc+1;
			        }
#endif
			        limit.tv_sec = timeout;
			        limit.tv_usec = 0;
			        rc = select(fdmax, &set, NULL, NULL,
			                    (0 != timeout && !paused) ? &limit : NULL);
				if(handle_console_switch())	/* this may have side effects, though */
				{
					return 0;	/* warning : originally a 'continue' in a loop ! */
				}
				
				if (FD_ISSET(cc.fim_stdin,&set))rc = read(cc.fim_stdin, c, 4);
				r=rc;
				*c=int2msbf(*c);
			}
#else	
			/*
			 * this way the console switches the wrong way
			 */
			r=read(fim_stdin,&c,4);	//up to four chars should suffice
#endif
			//std::cout << (int)*c<<"\n";

			return r;
	}

	int DisplayDevice::catchInteractiveCommand(int seconds)const
	{
		/*	
		 *
		 *	THIS DOES NOT WORK, BECAUSE IT IS A BLOCKING READ.
		 *	MAKE THIS READ UNBLOCKING AN UNCOMMENT. <- ?
		 *	
		 *	FIX ME
		 *
		 *	NOTE : this call should 'steal' circa 1/10 of second..
		 */
		fd_set          set;
		FD_SET(0, &set);
		size_t rc=0;

		struct termios tattr;
		struct termios sattr;
		//we set the terminal in raw mode.
		    
	//	fcntl(0,F_GETFL,&saved_fl);
		tcgetattr (0, &sattr);

		//fcntl(0,F_SETFL,O_BLOCK);
		memcpy(&tattr,&sattr,sizeof(struct termios));
		tattr.c_lflag &= ~(ICANON|ECHO);
		tattr.c_cc[VMIN]  = 0;
		tattr.c_cc[VTIME] = 1 * (seconds==0?1:(seconds*10)%256);
		tcsetattr (0, TCSAFLUSH, &tattr);
		
		int c,r;//char buf[64];
		//r=read(fim_stdin,&c,4);
		r=read(cc.fim_stdin,&c,1); if(r>0&&c==0x1b){rc=read(0,&c,3);c=(0x1b)+(c<<8);/* we should do something with rc now */}

		//we restore the previous console attributes
		tcsetattr (0, TCSAFLUSH, &sattr);

		if( r<=0 ) return -1;	/*	-1 means 'no character pressed	*/

		return c;		/*	we return the read key		*/
	}

#ifndef FIM_KEEP_BROKEN_CONSOLE
void DisplayDevice::fb_status_screen_new(const char *msg, int draw, int flags)//experimental
{
	int r;
	
	if(flags==0x03)
	{
		/* clear screen sequence */
		mc.clear();
		return;
	}

	if( flags==0x01 ) { mc.scroll_down(); return; }
	if( flags==0x02 ) { mc.scroll_up(); return; }

	r=mc.add(msg);
	if(r==-2)
	{
		r=mc.grow();
		if(r==-1)return;
		r=mc.add(msg);
		if(r==-1)return;
	}

	if(!draw )return;//CONVENTION!
	
	//fb_memset(fb_mem ,0,fb_fix.line_length * (fb_var.yres/2)*(fs_bpp));
	cc.displaydevice->lock();
	clear_rect(0, width()-1, 0,height()/2);
	cc.displaydevice->unlock();
	mc.dump();
//	mc.dump(0,1000000);
	return;
}
#endif

int DisplayDevice::console_control(int arg)//experimental
{
	if(arg==0x01)fb_status_screen_new(NULL,0,arg);
	if(arg==0x02)fb_status_screen_new(NULL,0,arg);
	if(arg==0x03)fb_status_screen_new(NULL,0,arg);
	return 0;
}

int DisplayDevice::init_console()
{
	if(f)
	{	
		mc.setRows ((height()/f->height)/2);
		mc.reformat( width() /f->width    );
	}
	else
	{
		mc.setRows ( height()   );
		mc.reformat( width()    );
	}
	return 0;
}

DisplayDevice::~DisplayDevice()
{
}

void DisplayDevice::quickbench()
{
	/*
		a quick draw benchmark and sanity check.
		currently performs only the clear function.
	*/
	double tbtime,btime;// ms
	size_t times=1;
	string msg="fim check";

	std::cout << msg << " : " << "please be patient\n";

	times=1;
	tbtime=1000.0,btime=0.0;// ms

	do
	{
		btime=-getmilliseconds();
		clear_rect(0, width()-1, 0,height()/2);
		btime+=getmilliseconds();
		++times;
		tbtime-=btime;
	}
	while(btime>=0.0 && tbtime>0.0 && times>0);
	--times;
	tbtime=1000.0-tbtime;

	std::cout << msg << " : " << ((double)times)/((tbtime)*1.e-3) << " clears/s\n";
}

