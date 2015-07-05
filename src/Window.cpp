/* $Id: Window.cpp 232 2009-03-29 17:32:02Z dezperado $ */
/*
 Window.cpp : Fim's own windowing system

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

/*
 *	This code is still experimental and programmed in great hurry.
 *	FIXME : there are bugs.
 */


#include "fim.h"

#ifdef FIM_WINDOWS

namespace fim
{
        fim::string Window::cmd(const std::vector<fim::string> &args)
        {
		unsigned int i=0;
		int rc=0;/*return code*/
#ifdef FIM_AUTOCMDS
		fim::string c=getGlobalIntVariable(FIM_VID_FILENAME);
		// note that an autocommand on a transient object is lethal
		if(amroot)autocmd_exec("PreWindow",c);
#endif
		try
		{
		while(i<args.size())
                {
			string cmd=args[i];
			if(cmd == "split" || cmd == "hsplit")
			{
				hsplit();
			}
			else if(cmd == "vsplit")
			{
				vsplit();
			}
/*			else if(cmd == "draw")
			{
				draw();
				return "\n";
			}*/
			else if(cmd == "normalize")
			{
				normalize();
				return "\n";
			}
			else if(cmd == "enlarge")
			{
				/*
				 * NOTE : this is not yet RECURSIVE !
				 * */
				enlarge(10);
				return "\n";
			}
			else if(cmd == "venlarge")
			{
				/*
				 * NOTE : this is not yet RECURSIVE !
				 * */
				venlarge(10);
				return "\n";
			}
			else if(cmd == "henlarge")
			{
				henlarge(10);
				return "\n";
			}
			else if(cmd == "up"   ) { move_focus(Up   ); }
			else if(cmd == "down" ) { move_focus(Down ); }
			else if(cmd == "left" ) { move_focus(Left ); }
			else if(cmd == "right") { move_focus(Right); }
			else if(cmd == "close") { close(); }
			else if(cmd == "swap") { swap(); }
			else rc=-1;

			/*
			 * FIXME
			 * */
			if(rc!=0) return "window : bad command\n";

			/*
			else if(cmd == "test")
			{
				fim::cout << "test ok!!\n";
				fb_clear_rect(10, 610, 100*4,100);
				return "test ok!!!\n";
			}*/
			++i;
                }
                }
		catch(FimException e)
		{
			cout << "please note some problems occurred in the window subsystem\n";
		}
#ifdef FIM_AUTOCMDS
		// note that an autocommand on a transient object is lethal
		if(amroot)autocmd_exec("PostWindow",c);
#endif
                return "";
        }

	Window::Window(CommandConsole &c,const Rect& corners_, Viewport* vp):corners(corners_),focus(0),first(NULL),second(NULL),amroot(false),
	viewport(NULL),
	commandConsole(c)
	{
		/*
		 *  A new leave Window is created with a specified geometry.
		 *  An exception is launched upon memory errors.
		 */
		focus=0;
		if(vp)
		{
			viewport=new Viewport(*vp );
			if(viewport)viewport->reassignWindow(this);

		}
		else
			viewport=new Viewport( commandConsole,  this );

		if( viewport == NULL ) throw FIM_E_NO_MEM;
	}

//#ifdef FIM_UNDEFINED
	Window::Window(const Window & root):corners(root.corners),focus(root.focus),first(root.first),second(root.second),amroot(false), viewport(NULL),commandConsole(root.commandConsole)
	{
		/*
		 *  A new leave Window is created with a specified geometry.
		 *  An exception is launched upon memory errors.
		 *
		 *  Note : this method is useless, and should be kept private :D
		 */
		viewport=new Viewport( commandConsole, this );

		if( viewport == NULL ) throw FIM_E_NO_MEM;
	}
//#endif

	bool Window::issplit()const
	{
		/*
		 * return whether this window is split in some way
		 * */
		return ( first && second ) ;
	}

	bool Window::isleaf()const
	{
		/*
		 * +----------+
		 * |____|_____|
		 * |+--+|     |
		 * ||L ||     |
		 * |+--+|     |
		 * +----------+
		 *   NON LEAF
		 * +----------+
		 *
		 * +----------+
		 * |          |
		 * |  LEAF    |
		 * |          |
		 * |          |
		 * +----------+
		 */
		return ( ! first && ! second ) ;
	}

	bool Window::isvalid()const
	{	
		/*
		 * return whether this window is split right, if it is
		 * */
		return !(( first && ! second ) || ( ! first && second ) );
	}

	bool Window::ishsplit()const
	{
		/*
		 * +----------+
		 * |          |
		 * |__________|
		 * |          |
		 * |          |
		 * +----------+
		 */
		return ( issplit() && focused().corners.x==shadowed().corners.x ) ;
	}
	
	bool Window::isvsplit()const
	{
		/*
		 * +----------+
		 * |    |     |
		 * |    |     |
		 * |    |     |
		 * |    |     |
		 * +----------+
		 */
		return ( issplit() && focused().corners.y==shadowed().corners.y ) ;
	}
	
	const Window & Window::c_focused()const
	{
		/*
		 * return a const reference to the focused window
		 * throws an exception in case the window is not split!
		 * */
		if(isleaf())/* temporarily, for security reasons */throw FIM_E_WINDOW_ERROR;

		if(focus==0)return first->c_focused();
		else return second->c_focused();
	}

	Window & Window::focused()const
	{
		/*
		 * return a reference to the focused window
		 * throws an exception in case the window is not split!
		 * */
		if(isleaf())/* temporarily, for security reasons */throw FIM_E_WINDOW_ERROR;

		if(focus==0)return *first;
		else return *second;
	}

	Window & Window::upper()
	{
		/*
		 * return a reference to the upper window
		 * throws an exception in case the window is not split!
		 * */
		if(!ishsplit())/* temporarily, for security reasons */throw FIM_E_WINDOW_ERROR;
		return *first;
	}

	Window & Window::lower()
	{
		/*
		 * return a reference to the lower window
		 * throws an exception in case the window is not split!
		 * */
		if(!ishsplit())/* temporarily, for security reasons */throw FIM_E_WINDOW_ERROR;
		return *second;
	}

	Window & Window::left()
	{
		/*
		 * return a reference to the left window
		 * throws an exception in case the window is not split!
		 * */
		if(!isvsplit())/* temporarily, for security reasons */throw FIM_E_WINDOW_ERROR;
		return *first;
	}

	Window & Window::right()
	{
		/*
		 * return a reference to the right window
		 * throws an exception in case the window is not split!
		 * */
		if(!isvsplit())/* temporarily, for security reasons */throw FIM_E_WINDOW_ERROR;
		return *second;
	}

	Window & Window::shadowed()const
	{
		/*
		 * return a const reference to the right window
		 * throws an exception in case the window is not split!
		 * */		
		if(isleaf())/* temporarily, for security reasons */throw FIM_E_WINDOW_ERROR;

		if(focus!=0)return *first;
		else return *second;
	}

	const Window & Window::c_shadowed()const
	{
		/*
		 * return a const reference to the shadowed window
		 * throws an exception in case the window is not split!
		 * */		
		if(isleaf())/* temporarily, for security reasons */throw FIM_E_WINDOW_ERROR;

		if(focus!=0)return first->c_shadowed();
		else return second->c_shadowed();
	}

	void Window::setroot()
	{
		/*
		 * FIXME
		 * */
		amroot=true;
	}

	void Window::split()
	{
		/*
		 * an alias for hsplit()
		 * */
		hsplit();
	}

#if 0
	void Window::print_focused()
	{
		if(isleaf())
		{
			std::cout << "F:" ;
			corners.print();
		}
		else focused().print_focused();
	}

	void Window::print()
	{
		if(amroot)std::cout<<"--\n";
		if(amroot)print_focused();
		if(isleaf())std::cout<<"L:";
		corners.print();
		if(!isleaf())first ->print();
		if(!isleaf())second->print();
	}
#endif
	
	void Window::hsplit()
	{
		/*
		 * splits the window with a horizontal separator
		 * */
		if(   ! isvalid() ) return;

		/*
		 * we should check if there is still room to split ...
		 * */
		if(isleaf())
		{
			first  = new Window( commandConsole, this->corners.hsplit(Rect::Upper),viewport);
			second = new Window( commandConsole, this->corners.hsplit(Rect::Lower),viewport);
			if(viewport && first && second)
			{
#define FIM_COOL_WINDOWS_SPLITTING 0
#if     FIM_COOL_WINDOWS_SPLITTING
				first ->current_viewport().pan_up  ( second->current_viewport().viewport_height() );
#endif
				delete viewport;
				viewport = NULL;
			}
		}
		else focused().hsplit();
	}

	void Window::vsplit()
	{
		/*
		 * splits the window with a vertical separator
		 * */
		if(   !isvalid() ) return;

		/*
		 * we should check if there is still room to split ...
		 * */
		if(isleaf())
		{
			first  = new Window( commandConsole, this->corners.vsplit(Rect::Left ),viewport);
			second = new Window( commandConsole, this->corners.vsplit(Rect::Right),viewport);
			if(viewport && first && second)
			{
#if     FIM_COOL_WINDOWS_SPLITTING
				second->current_viewport().pan_right( first->current_viewport().viewport_width() );
#endif
				delete viewport;
				viewport = NULL;
			}
		}
		else focused().vsplit();
	}

	bool Window::swap()
	{
		/*
		 * swap window content
		 *
		 * FIXME : unfinished
		 *
		 * +-----+-----+   +-----+-----+
		 * |     |     |   |     |     |
		 * | S   | F   |-->| F   | S   |
		 * |     |     |   |     |     |
		 * |b.jpg|a.jpg|   |a.jpg|b.jpg|
		 * +-----+-----+   +-----+-----+
		 */
		if(   !isvalid() ) return false;

		if(isleaf())
		{
			// no problem
			return false;
		}
		else if(focused().isleaf())
		{
			Viewport *vf,*vs;
			vf = focused().viewport;
			vs = shadowed().viewport;
			// WARNING : dangerous
			if(vf && vs)
			{
				vf ->reassignWindow(&(shadowed()));
				vs ->reassignWindow(&( focused()));
				focused().viewport  = vs;
				shadowed().viewport = vf;
			}
			else
			{
				// an error should be spawned
				// FIXME
				return false;
			}
		}
		else return focused().swap();
		return true;
	}


	bool Window::close()
	{
		/*
		 * closing a leaf window implies its rejoining with the parent one
		 *
		 * FIXME : unfinished
		 *
		 * +----+-----+   +----------+
		 * |    |     |   |          |
		 * |    |     |-->|          |
		 * |    |     |   |          |
		 * |    |     |   |          |
		 * +----+-----+   +----------+
		 */
		if(   !isvalid() ) return false;

		if(isleaf())
		{
			// no problem
			return false;
		}
		else if(focused().isleaf())
		{
			/*if(ishsplit())
			this->corners=Rect(focused().corners.x,focused().corners.y,shadowed().corners.w,focused().corners.h+shadowed().corners.h);
			else if(isvsplit())
			this->corners=Rect(focused().corners.x,focused().corners.y,shadowed().corners.w+focused().corners.w,shadowed().corners.h);
			else ;//error
			*/
			/*
			 * some inheritance operations needed here!
			 */

			// WARNING : dangerous
			if(viewport)
			{
				cout << "viewport should be NULL!\n";
				// an error should be spawned
			}
			if( ( viewport = focused().viewport ) )
			{
				viewport ->reassignWindow(this);
				focused().viewport=NULL;
			}
			else
			{
				// error action
				return false;
			}
			delete first;  first  = NULL;
			delete second; second = NULL;
		}
		else return focused().close();
//		print();
		return true;
	}

	void Window::balance()
	{
		/*
		 * FIXME
		 * +---+-------+   +-----+-----+
		 * |---+-------+   |     |     |
		 * |           |-->|     |     |
		 * |           |   +-----+-----+
		 * |           |   |     |     |
		 * |           |   |     |     |
		 * +-----------+   +-----+-----+
		 */
	}

	Window::Moves Window::reverseMove(Moves move)
	{
		/*
		 * returns the complementary window move
		 *
		 * ( > )^-1 = <
		 * ( < )^-1 = >
		 * ( ^ )^-1 = v
		 * ( v )^-1 = ^
		 * */
		if(move==Left )return Right;
		if(move==Right)return Left;
		if(move==Up   )return Down;
		if(move==Down )return Up;
		return move;
	}

	Window::Moves Window::move_focus(Moves move)
	{
		/*
		 * shifts the focus from a window to another, 
		 * unfortunately not always adjacent (a better algorithm would is needed for this)
		 *
		 * maybe more abstractions is needed here..
		 * */
		Moves m;
		if( isleaf() || move==NoMove )return NoMove;
		else
		if( isvsplit() )
		{
			if(  move != Left  &&  move != Right )
				return focused().move_focus(move);

			if( focused().isleaf() )
			{
				if( ( right() == focused() && move == Left ) || ( left() == focused() && move == Right ) )
				{
					chfocus();
					return move;
				}
				else
				{
					return NoMove;
				}
			}
			else
			{
				if((m=focused().move_focus(move)) == NoMove)
				{
					chfocus();
					focused().move_focus(reverseMove(move));
				}
				return move;
			}
		}
		else
		if( ishsplit() )
		{
			if(  move != Up  &&  move != Down )
				return focused().move_focus(move);

			if( focused().isleaf() )
			{
				if( ( upper() == focused() && move == Down ) || ( lower() == focused() && move == Up ) )
				{
					chfocus();
					return move;
				}
				else
				{
					return NoMove;
				}
			}
			else
			{
				if((m=focused().move_focus(move)) == NoMove)
				{
					chfocus();
					focused().move_focus(reverseMove(move));
				}
				return move;
			}
		}
		return move;
	}

	int Window::chfocus()
	{
		/*
		 * this makes sense if issplit().
		 *
		 * swaps the focus only.
		 *
		 * +----+----+   +----+----+
		 * |    |    |   |    |    |
		 * |    |    |   |    |    |
		 * | F  | S  |-->| S  | F  |
		 * |    |    |   |    |    |
		 * +----+----+   +----+----+
		 */
		return focus = ~focus;
	}

	int Window::height()const
	{
		/*
		 * +---+ +
		 * |   | |
		 * +---+ +
		 */
		return corners.h ;
	}

	int Window::setwidth(int w)
	{
		/*
		 * +---+
		 * +---+
		 * |   |
		 * +---+
		 */
		return corners.w=w;
	}

	int Window::setheight(int h)
	{
		/*
		 * +---+ +
		 * |   | |
		 * +---+ +
		 */
		return corners.h=h;
	}

	int Window::width()const
	{
		/*
		 * +---+
		 * +---+
		 * |   |
		 * +---+
		 */
		return corners.w ;
	}

	int Window::setxorigin(int x)
	{
		/*
		 * o---+
		 * |   |
		 * +---+
		 */
		return corners.x=x ;
	}

	int Window::setyorigin(int y)
	{
		/*
		 * o---+
		 * |   |
		 * +---+
		 */
		return corners.y=y ;
	}

	int Window::xorigin()const
	{
		/*
		 * o---+
		 * |   |
		 * +---+
		 */
		return corners.x ;
	}

	int Window::yorigin()const
	{
		/*
		 * o---+
		 * |   |
		 * +---+
		 */
		return corners.y ;
	}

	bool Window::can_vgrow(const Window & window, int howmuch)
	{
		/*
		 * Assuming that the argument window is a contained one, 
		 * can this window grow the specified amount and assure the
		 * minimum spacing is respected ?
		 *
		 * +--------+
		 * +-^-+this|
		 * | ? |    |
		 * +-v-+    |
		 * +--------+
		 */
		return window.height() + howmuch + vspacing  < height();
	}

	bool Window::can_hgrow(const Window & window, int howmuch)
	{
		/*
		 * Assuming that the argument window is a contained one, 
		 * can this window grow the specified amount and assure the
		 * minimum spacing is respected ?
		 *
		 * +--------+
		 * +---+this|
		 * |<?>|    |
		 * +---+    |
		 * +--------+
		 */		return window.width() + howmuch + hspacing   < width();
	}


	bool Window::operator==(const Window&window)const
	{
		/*
		 * #===#
		 * #   #
		 * #===#
		 */
		return corners==window.corners;
	}

	int Window::count_hdivs()const
	{
		/*
		 * how many horizontal divisions ?
		 *
		 * +----------+
		 * |          |
		 * |          | hdivs = 3
		 * +----------+
		 * +----------+
		 * +----------+
		 * */
		return (isleaf()|| !ishsplit())?1: first->count_hdivs()+ second->count_hdivs();
	}

	int Window::count_vdivs()const
	{
		/*
		 * how many vertical divisions ?
		 * */
		return (isleaf()|| !isvsplit())?1: first->count_vdivs()+ second->count_vdivs();
	}

	int Window::normalize()
	{
		/*
		 * FIXME vs balance
		 *
		 * +---+-------+   +-----+-----+
		 * |---+-------+   |     |     |
		 * |           |-->|     |     |
		 * |           |   +-----+-----+
		 * |           |   |     |     |
		 * |           |   |     |     |
		 * +-----------+   +-----+-----+
		 */
		return
//		(hnormalize(xorigin(), width() )!= -1);
		(hnormalize(xorigin(), width() )!= -1) &&
		(vnormalize(yorigin(), height())!= -1);
	}

	int Window::vnormalize(int y, int h)
	{
		/*
		 * balances the horizontal divisions height
		 *
		 * FIXME
		 *
		 * +---+-------+   +---+-------+
		 * |---+-------+   |   |       |
		 * |           |-->|   |       |
		 * |           |   +---+-------+
		 * |           |   |   |       |
		 * |           |   |   |       |
		 * +-----------+   +---+-------+
		 */
		if(isleaf())
		{
			setyorigin(y);
			setheight(h);
			return 0;
		}
		else
		{
			int fhdivs,shdivs,hdivs,upd;
			fhdivs=first ->count_hdivs();
			shdivs=second->count_hdivs();
			hdivs=count_hdivs();
			upd=h/hdivs;
			if(hdivs>h)return -1;// no space left
			//...
			setyorigin(y);
			setheight(h);

			if(ishsplit())
			{
				first-> vnormalize(y,upd*fhdivs);
				second->vnormalize(y+upd*fhdivs,h-upd*fhdivs);
			}
			else
			{
				first-> vnormalize(y,h);
				second->vnormalize(y,h);
			}
			return 0;
		}
	}

	int Window::hnormalize(int x, int w)
	{
		/*
		 * balances the vertical divisions width
		 *
		 * FIXME
		 *
		 * +---+-------+   +-----+-----+
		 * |   |       |   |     |     |
		 * |   |       |-->|     |     |
		 * |---+-------+   +-----+-----+
		 * |   |       |   |     |     |
		 * |   |       |   |     |     |
		 * +---+-------+   +-----+-----+
		 */
		if(isleaf())
		{
			setxorigin(x);
			setwidth(w);
			return 0;
		}
		else
		{
			int fvdivs,svdivs,vdivs,upd;
			fvdivs=first ->count_vdivs();
			svdivs=second->count_vdivs();
			vdivs=count_vdivs();
			upd=w/vdivs;
			if(vdivs>w)return -1;// no space left
			//...
			setxorigin(x);
			setwidth(w);

			if(isvsplit())
			{
				first-> hnormalize(x,upd*fvdivs);
				second->hnormalize(x+upd*fvdivs,w-upd*fvdivs);
			}
			else
			{
				first-> hnormalize(x,w);
				second->hnormalize(x,w);
			}
			return 0;
		}
	}

	int Window::venlarge(int units=1)
	{
#if FIM_BUGGED_ENLARGE
		return -1;
#endif
		/*
		 * SEEMS BUGGY:
		 * */
		 // make && src/fim media/* -c 'split;vsplit;6henlarge;wd;7henlarge;wu;4henlarge'
		 // make && src/fim media/* -c 'split;vsplit;window "venlarge";wd; window "venlarge";'
			/*
			 * +----------+
			 * |    |     |
			 * |   >|     |
			 * | F  |  S  |
			 * |    |     |
			 * +----------+
			 */
			if( isleaf() )
			{
				if(viewport)commandConsole.displaydevice->redraw=1;// no effect
				return 0;
			}

			if(isvsplit())
			{
				/*
				 * +-+-+
				 * + | +
				 * +-+-+
				 * */
				if(focused()==left()) focused().hrgrow(units);
				if(focused()==right())focused().hlgrow(units);
				focused().normalize();  // i think there is a more elegant way to this but hmm..
				
			}
			focused().venlarge(units); //regardless the split status
			if(isvsplit())
			{
				if(focused()==left())  shadowed().hlshrink(units);
				if(focused()==right()) shadowed().hrshrink(units);
				shadowed().normalize(); 
			}
			return 0;
	}

	int Window::henlarge(int units=1)
	{
		/*
		 * SEEMS BUGGY:
		 * */
		 // make && src/fim media/* -c 'split;vsplit;6henlarge;wd;7henlarge;wu;4henlarge'
#if FIM_BUGGED_ENLARGE
		return -1;
#endif
			/*
			 * this operation doesn't change the outer bounds of the called window
			 *
			 * +----------+
			 * |   S      |
			 * |__________|
			 * |     ^    |
			 * |   F      |
			 * +----------+
			 */
			if( isleaf() )
			{
				if(viewport)commandConsole.displaydevice->redraw=1;// no effect
				return 0;
			}

			if(ishsplit())
			{
				/*
				 * +---+
				 * +---+
				 * +---+
				 * */
				if(focused()==upper()) focused().vlgrow(units);
				if(focused()==lower()) focused().vugrow(units);
				focused().normalize();  // i think there is a more elegant way to thism but hmm..
				
			}
			focused().henlarge(units); //regardless the split status
			if(ishsplit())
			{
				if(focused()==upper()) shadowed().vushrink(units);
				if(focused()==lower()) shadowed().vlshrink(units);

				shadowed().normalize(); 
			}
			return 0;
	}

	int Window::enlarge(int units=1)
	{
		/*
		 * FIXME : ???
		 */
#if FIM_BUGGED_ENLARGE
			return -1;
#endif
		/*
		 * complicato ...
		 */
//			std::cout << "enlarge\n";
			if(ishsplit() && can_vgrow(focused(),units))
			{
				return henlarge(units);
			}else
			if(isvsplit() && can_hgrow(focused(),units))
			{
				return venlarge(units);
			}else
			// isleaf()
			return 0;
	}


	int Window::vlgrow(int units=1)   {  return corners.vlgrow(  units); } 
	int Window::vlshrink(int units=1) {  return corners.vlshrink(units); }
	int Window::vugrow(int units=1)   {  return corners.vugrow(  units); } 
	int Window::vushrink(int units=1) {  return corners.vushrink(units); }

	int Window::hlgrow(int units=1)   {  return corners.hlgrow(  units); } 
	int Window::hlshrink(int units=1) {  return corners.hlshrink(units); }
	int Window::hrgrow(int units=1)   {  return corners.hrgrow(  units); } 
	int Window::hrshrink(int units=1) {  return corners.hrshrink(units); }

#if 0
	void Window::draw()const
	{
		/*
		 * 
		 * */
		if(isleaf())
		{
			// we draw
			int OFF=100,K=4;
			OFF=40;
			fb_clear_rect(corners.x+OFF, corners.x+(corners.w-OFF)*K, (corners.y+OFF),(corners.y+(corners.h-OFF)));
		}
		else
		{
			focused().draw();
			shadowed().draw();
		}
	}
#endif

	// WARNING : SHOULD BE SURE VIEWPORT IS CORRECTLY INITIALIZED
	bool Window::recursive_redisplay()const
	{
		/*
		 * whole, deep, window redisplay
		 * */
		bool re=false;//really redisplayed ? sometimes fim guesses it is not necessary
		try
		{
		if(isleaf())
		{
			if(viewport)re=viewport->redisplay();
		}
		else
		{
			re |= focused().recursive_redisplay();
			re |= shadowed().recursive_redisplay();
		}
		}
		catch(FimException e)
		{
			if( e != FIM_E_WINDOW_ERROR) ;// this would be bad..
		}
		return re;
	}

	// WARNING : SHOULD BE SURE VIEWPORT IS CORRECTLY INITIALIZED
	bool Window::recursive_display()const
	{
		/*
		 * whole, deep, window display
		 * */
		bool re=false;//really displayed ? sometimes fim guesses it is not necessary
		try
		{
		if(isleaf())
		{
			if(viewport)re=viewport->display();
		}
		else
		{
			re |= focused().recursive_display();
			re |= shadowed().recursive_display();
		}
		}
		catch(FimException e)
		{
			if( e != FIM_E_WINDOW_ERROR) ;// this would be bad..
		}
		return re;
	}

	Viewport * Window::current_viewportp()const
	{
		/*
		 * returns a pointer to the current window's viewport.
		 *
		 * +#===#+-----+
		 * ||   ||     |
		 * ||   ||     |
		 * ||FVP||     |
		 * ||   ||     |
		 * +#===#+-----+
		 */
		if(!isleaf()) return focused().current_viewportp();

		return viewport;
	}	

	Viewport & Window::current_viewport()const
	{
		/*
		 * returns a reference to the current window's viewport.
		 * throws an exception if this window is a leaf.
		 *
		 * +#===#+-----+
		 * ||   ||     |
		 * ||   ||     |
		 * ||FVP||     |
		 * ||   ||     |
		 * +#===#+-----+
		 */
		if(!isleaf()) return focused().current_viewport();

		if(!viewport)/* temporarily, for security reasons throw FIM_E_TRAGIC*/; // isleaf()

		return *viewport;
	}	

	const Image *Window::getImage()const
	{
		if( current_viewportp() )
			return current_viewportp()->getImage();
		else
			return NULL;
	}

	Window::~Window()
	{
		if(viewport) delete viewport;
		if(first)delete first;
		if(second)delete second; 
	}
}
#if 0
/*
 *	A test main program.
 */
int main()
{
	Window w(Rect(0,0,1024,768));
	w.setroot();
	w.vsplit();
	w.hsplit();
	w.normalize();
	w.print();
	std::cout << "move_focus:\n";
	w.move_focus(Window::Down);
	w.move_focus(Window::Right);
	w.move_focus(Window::Left);
	w.move_focus(Window::Down);
	w.print();
	std::cout << "move_focus:\n";
	w.move_focus(Window::Up);
	w.print();
/*	w.enlarge();
	w.enlarge();
	w.enlarge();
	w.enlarge();
	w.enlarge();
	std::cout << "normalized:\n";

	w.print();
	std::cout << "enlarged:\n";*/
//	w.hnormalize(w.xorigin(),w.width());
	w.close();
	w.close();
	w.close();
}
#endif
#endif

