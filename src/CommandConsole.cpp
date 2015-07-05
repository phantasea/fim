/* $Id: CommandConsole.cpp 273 2009-12-21 17:26:56Z dezperado $ */
/*
 CommandConsole.cpp : Fim console dispatcher

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
 * TODO:
 * 	framebufferdevice -> device
 * 	output methods should be moved in some other, new class
 * */
#include "fim.h"
#ifdef FIM_DEFAULT_CONFIGURATION
#include "conf.h"
#endif
#include <sys/time.h>
#include <errno.h>

#ifdef FIM_USE_READLINE
#include "readline.h"
#endif

#include <sys/ioctl.h>

#include <signal.h>
#include <fstream>

#if HAVE_GET_CURRENT_DIR_NAME
#else
#if _BSD_SOURCE || _XOPEN_SOURCE >= 500
#include <unistd.h>		/* getcwd, as replacement for get_current_dir_name */
#endif
#endif

extern int yyparse();

namespace fim
{

	static int nochars(const char *s)
	{
		/*
		 * 1 if the string is null or empty, 0 otherwise
		 */
		if(s==NULL)return 1;
		while(*s && isspace(*s))++s;
		return *s=='\0'?1:0;
	}

	Command* CommandConsole::findCommand(fim::string cmd)const
	{
		/*
		 * is cmd a valid internal (registered) Fim command ?
		 */
		for(size_t i=0;i<commands.size();++i) 
			if(commands[i] && commands[i]->cmd==cmd)
				return commands[i];
		return NULL;
	}

	fim::string CommandConsole::bind(int c,const fim::string binding)
	{
		/*
		 * binds keycode c to the action specified in binding

		 * note : the binding translation map is used as a necessary
		 * indirection...
		 */
		if(bindings[c]!="")
		{
			bindings[c]=binding;
			fim::string rs("keycode ");
			rs+=string((int)c);
			rs+=" successfully reassigned to \"";
			rs+=bindings[c];
			rs+="\"\n";
			return rs;
		}
		else
		{
			bindings[c]=binding;
			fim::string rs("keycode ");
			rs+=string((int)c);
			rs+=" successfully assigned to \"";
			rs+=bindings[c];
			rs+="\"\n";
			return rs;
		}
	}

	fim::string CommandConsole::getBindingsList()const
	{
		/*
		 * collates all registered action bindings together in a single string
		 * */
		fim::string bindings_expanded;
		bindings_t::const_iterator bi;
		for( bi=bindings.begin();bi!=bindings.end();++bi)
		{
			//if(bi->second == "")continue;//FIX : THIS SHOULD NOT OCCUR
			bindings_expanded+="bind \"";
			inverse_key_bindings_t::const_iterator ikbi=inverse_key_bindings.find(((*bi).first));
			if(ikbi!=inverse_key_bindings.end()) bindings_expanded+=ikbi->second;
			bindings_expanded+="\" \"";
			bindings_expanded+=((*bi).second);
			bindings_expanded+="\"\n";
		}
		return bindings_expanded;
	}


	fim::string CommandConsole::unbind(const fim::string& key)
	{
		/*
		 * 	unbinds the action eventually bound to the first key name specified in args..
		 *	IDEAS : multiple unbindings ?
		 *	maybe you should made surjective the binding_keys mapping..
		 */
		return unbind(key_bindings[key]);
	}

	fim::string CommandConsole::unbind(int c)
	{
		/*
		 * unbinds the action eventually bound to the key combination code c
		 */
		fim::string rs("unbind ");
		if(bindings[c]!="")
		{
			bindings.erase(c);
			rs+=c;
			rs+=": successfully unbinded.\n";
		}
		else
		{
			rs+=c;
			rs+=": there were not such binding.\n";
		}
		return rs;
	}

	fim::string CommandConsole::aliasRecall(fim::string cmd)const
	{
		/*
		 * returns the alias command eventually specified by token cmd
		 *
		 * Note : return aliases[cmd] would create an entry associated to cmd 
		 * ( and this method would loose the chance to be const ).
		 */
		aliases_t::const_iterator ai=aliases.find(cmd);
		if(ai!=aliases.end()) return ai->second.first;
		return "";
	}

	fim::string CommandConsole::getAliasesList()const
	{
		/*
		 * collates all registered action aliases together in a single string
		 * */
		fim::string aliases_expanded;
		aliases_t::const_iterator ai;
		for( ai=aliases.begin();ai!=aliases.end();++ai)
		{
#if 0
			if(ai->second.first == "")continue;//FIX THIS : THIS SHOULD NOT OCCUR
			aliases_expanded+="alias ";
			aliases_expanded+=((*ai).first);
			aliases_expanded+="=\"";
			aliases_expanded+=((*ai).second.first);
			aliases_expanded+="\"\n";
#endif
			aliases_expanded+=get_alias_info((*ai).first);
		}
		return aliases_expanded;
	}

	fim::string CommandConsole::get_alias_info(const fim::string aname)const
	{
			string  r;
				r+=fim::string("alias \"");
				r+=aname;
				r+=fim::string("\" \"");
				aliases_t::const_iterator ai=aliases.find(aname);
				if(ai!=aliases.end())r+=ai->second.first;
				r+=fim::string("\"");
				if(ai!=aliases.end())
				if(ai->second.second!="")
				{
					r+=" # ";
					r+=ai->second.second;
				}
				r+=fim::string("\n");
				return r;
	}

	fim::string CommandConsole::alias(std::vector<Arg> args)
	{
		/*
		 * assigns to an alias some action
		 */
		fim::string cmdlist,desc;
		if(args.size()==0)
		{
			return getAliasesList();
		}
		if(args.size()<2)
		{
			return get_alias_info(args[0].val);
		}
		//for(size_t i=1;i<args.size();++i) cmdlist+=args[i].val;
		if(args.size()>=2)cmdlist+=args[1].val;
		if(args.size()>=3)desc   +=args[2].val;
		if(aliases[args[0].val].first!="")
		{
			aliases[args[0].val]=std::pair<fim::string,fim::string>(cmdlist,desc);
			string r;
			r+=fim::string("alias ");
			r+=args[0].val;
			r+=fim::string(" successfully replaced.\n");
			return r;
		}
		else
		{
			aliases[args[0].val].first=cmdlist;
			aliases[args[0].val].second=desc;
			string r;
			r+=fim::string("alias ");
			r+=args[0].val;
			r+=fim::string(" successfully added.\n");
			return r;
		}
	}

	fim::string CommandConsole::dummy(std::vector<Arg> args)
	{
		/*
		 * useful for test purposes
		 * */
		//std::cout << "dummy function : for test purposes :)\n";
		return "dummy function : for test purposes :)\n";
	}

	CommandConsole::CommandConsole(/*FramebufferDevice &_framebufferdevice*/):
#ifndef FIM_KEEP_BROKEN_CONSOLE
	mc(*this),
#endif
	fontserver(),
	browser(*this)
	//,framebufferdevice(_framebufferdevice)
	,return_code(0)
	,dummydisplaydevice(this->mc)
	,displaydevice(NULL)			/* the display device could be NULL ! (FIXME) */
#ifdef FIM_RECORDING
	,dont_record_last_action(false)		/* this variable is only useful in record mode */
	,recordMode(false)			/* we start not recording anything */
#endif
	,fim_stdin(0)
	,cycles(0)
	,show_must_go_on(1)
	{
//		addCommand(new Command(fim::string("type" ),fim::string("prints out the type of its arguments"),this,&CommandConsole::get_expr_type));
		addCommand(new Command(fim::string("prefetch" ),fim::string("prefetches"),&browser,&Browser::prefetch));
		addCommand(new Command(fim::string("no_image" ),fim::string("displays no image at all"),&browser,&Browser::no_image));
		addCommand(new Command(fim::string("next" ),fim::string("displays the next picture in the list"),&browser,&Browser::next));
		addCommand(new Command(fim::string("next_picture" ),fim::string("displays the next page or picture file"),&browser,&Browser::next_picture));
		addCommand(new Command(fim::string("prev_picture" ),fim::string("displays the previous page or picture file"),&browser,&Browser::prev_picture));
		addCommand(new Command(fim::string("prev" ),fim::string("displays the previous picture in the list"),&browser,&Browser::prev));
		addCommand(new Command(fim::string("next_page" ),fim::string("displays the next page"),&browser,&Browser::next_page));
		addCommand(new Command(fim::string("prev_page" ),fim::string("displays the previous page"),&browser,&Browser::prev_page));
		addCommand(new Command(fim::string("push" ),fim::string("pushes a file in the files list"),&browser,&Browser::push));
		addCommand(new Command(fim::string("display" ),fim::string("displays the current file"),&browser,&Browser::display));
		addCommand(new Command(fim::string("redisplay" ),fim::string("re-displays the current file"),&browser,&Browser::redisplay));
		addCommand(new Command(fim::string("list" ),fim::string("displays the files list"),&browser,&Browser::list));
		addCommand(new Command(fim::string("pop"  ),fim::string("pop the last file from the files list"),&browser,&Browser::pop));
		addCommand(new Command(fim::string("file" ),fim::string("displays the current file's name (UNFINISHED)"),&browser,&Browser::info));
		addCommand(new Command(fim::string("pan_ne" ),fim::string("pans the image north east"),&browser,&Browser::pan_ne));
		addCommand(new Command(fim::string("pan_nw" ),fim::string("pans the image north west"),&browser,&Browser::pan_nw));
		addCommand(new Command(fim::string("pan_sw" ),fim::string("pans the image south west"),&browser,&Browser::pan_sw));
		addCommand(new Command(fim::string("pan_se" ),fim::string("pans the image south east"),&browser,&Browser::pan_se));
		addCommand(new Command(fim::string("panup" ),fim::string("pans the image up"),&browser,&Browser::pan_up));
		addCommand(new Command("pandown" ,"pans the image down",&browser,&Browser::pan_down));
		addCommand(new Command(fim::string("panleft" ),fim::string("pans the image left"),&browser,&Browser::pan_left));
		addCommand(new Command(fim::string("panright" ),fim::string("pans the image right"),&browser,&Browser::pan_right));
		addCommand(new Command(fim::string("load" ),fim::string("load the image, if not yet loaded"),&browser,&Browser::load));
		addCommand(new Command(fim::string("reload" ),fim::string("loads the image into memory"),&browser,&Browser::reload));
		addCommand(new Command(fim::string("files"),fim::string("displays the number of files in the file list" ),&browser,&Browser::n));
		addCommand(new Command(fim::string("sort" ),fim::string("sorts the file list" ),&browser,&Browser::_sort));
		addCommand(new Command(fim::string("remove" ),fim::string("remove the current file or the selected ones from the list" ),&browser,&Browser::remove));
		addCommand(new Command(fim::string("info" ),fim::string("info about the current file" ),&browser,&Browser::info));
		addCommand(new Command(fim::string("regexp_goto" ),fim::string("jumps to the first image matching the given pattern"),&browser,&Browser::regexp_goto));
		addCommand(new Command(fim::string("regexp_goto_next" ),fim::string("jumps to the next image matching the last given pattern"),&browser,&Browser::regexp_goto_next));
		addCommand(new Command(fim::string("scale_increment" ),fim::string("increments the scale by a percentual amount"),&browser,&Browser::scale_increment));
		addCommand(new Command(fim::string("scale_multiply" ),fim::string("multiplies the scale by the specified amount"),&browser,&Browser::scale_multiply));
		addCommand(new Command(fim::string("scale_factor_grow" ),fim::string("multiply the scale factors reduce_factor and magnify_factor by scale_factor_multiplier"),&browser,&Browser::scale_factor_increase));
		addCommand(new Command(fim::string("scale_factor_shrink" ),fim::string("divide the scale factors reduce_factor and magnify_facto by scale_factor_multiplier"),&browser,&Browser::scale_factor_decrease));
		addCommand(new Command(fim::string("scale_factor_increase" ),fim::string("add scale_factor_delta to the scale factors reduce_factor and magnify_facto" ),&browser,&Browser::scale_factor_increase));
		addCommand(new Command(fim::string("scale_factor_decrease" ),fim::string( "subtract scale_factor_delta to the scale factors reduce_factor and magnify_factor" ),&browser,&Browser::scale_factor_decrease));
		addCommand(new Command(fim::string("rotate" ),fim::string( "rotate the image the specified amount of degrees" ),&browser,&Browser::rotate));
		addCommand(new Command(fim::string("magnify" ),fim::string("magnify [ARGS] : magnifies the displayed image by the magnify_factor variable or ARGS" ),&browser,&Browser::magnify));
		addCommand(new Command(fim::string("reduce"),fim::string("reduce [ARGS] ; reduces the displayed image by reduce_factor or ARGS" ),&browser,&Browser::reduce));
		addCommand(new Command(fim::string("return"),fim::string("returns from the program with a status code"),this,&CommandConsole::do_return));
		addCommand(new Command(fim::string("top_align"),fim::string("aligns to the upper side the image" ),&browser,&Browser::top_align));
		addCommand(new Command(fim::string("bottom_align"),fim::string("aligns to the lower side the image" ),&browser,&Browser::bottom_align));
		addCommand(new Command(fim::string("goto"),fim::string("goes to the index image" ),&browser,&Browser::goto_image));
		addCommand(new Command(fim::string("negate"),fim::string("negates the displayed image colors" ),&browser,&Browser::negate));
		addCommand(new Command(fim::string("status"),fim::string("sets the status line to the collation of the given arguments"),this,&CommandConsole::status));
		addCommand(new Command(fim::string("scrolldown" ),fim::string("scrolls down the image, going next if at bottom" ),&browser,&Browser::scrolldown));
		addCommand(new Command(fim::string("scrollforward" ),fim::string("scrolls the image as it were reading it :)" ),&browser,&Browser::scrollforward));
		addCommand(new Command(fim::string("scale" ),fim::string("scales the image according to a scale (ex.: 0.5,40%,..)" ),&browser,&Browser::scale));
		addCommand(new Command(fim::string("set" ),fim::string("manipulates the internal variables" ),this,&CommandConsole::set));
		addCommand(new Command(fim::string("auto_scale" ),fim::string("" ),&browser,&Browser::auto_scale));
		addCommand(new Command(fim::string("auto_width_scale" ),fim::string("scale the image so that it fits horizontally in the screen" ),&browser,&Browser::auto_width_scale));
		addCommand(new Command(fim::string("auto_height_scale" ),fim::string("scale the image so that it fits vertically in the screen" ),&browser,&Browser::auto_height_scale));
		addCommand(new Command(fim::string("bind" ),fim::string("binds some keyboard shortcut to an action"),this,&CommandConsole::bind));
		addCommand(new Command(fim::string("quit"  ),fim::string("terminates the program"),this,&CommandConsole::quit));
#ifndef FIM_NOSCRIPTING
		addCommand(new Command(fim::string("exec"  ),fim::string("executes script files"),this,&CommandConsole::executeFile));
#endif
		addCommand(new Command(fim::string("echo"  ),fim::string("echoes its arguments"),this,&CommandConsole::echo));
		//addCommand(new Command(fim::string("foo"   ),fim::string("a dummy command"),this,&CommandConsole::foo));
		//addCommand(new Command(fim::string("print"   ),fim::string("displays the value of a variable"),this,&CommandConsole::foo));
		addCommand(new Command(fim::string("if"    ),fim::string("if(expression){action;}[else{action;}]"),this,&CommandConsole::foo));
		addCommand(new Command(fim::string("else"    ),fim::string("if(expression){action;}[else{action;}]"),this,&CommandConsole::foo));
		addCommand(new Command(fim::string("while" ),fim::string("while(expression){action;}"),this,&CommandConsole::foo));
		addCommand(new Command(fim::string("alias" ),fim::string("alias ALIAS ACTION"),this,&CommandConsole::foo));
		addCommand(new Command(fim::string("getenv" ),fim::string("getenv IDENTIFIER"),this,&CommandConsole::do_getenv));
		addCommand(new Command(fim::string("unalias" ),fim::string("unalias {alias} | -a : deletes the alias {alias} or all aliases (use \"-a\", not -a)"),this,&CommandConsole::unalias));
		addCommand(new Command(fim::string("unbind"),fim::string("unbinds the action associated to KEYCODE"),this,&CommandConsole::unbind));
		addCommand(new Command(fim::string("sleep" ),fim::string("sleeps for n (default 1) seconds"),this,&CommandConsole::foo));
		addCommand(new Command(fim::string("mark" ),fim::string("marks the current file"),this,&CommandConsole::markCurrentFile));
		addCommand(new Command(fim::string("help"  ),fim::string("provides online help"),this,&CommandConsole::help));
#ifdef FIM_AUTOCMDS
		addCommand(new Command(fim::string("autocmd"  ),fim::string("usage: autocmd [Event] [Pattern] [Commands]"),this,&CommandConsole::autocmd));
		addCommand(new Command(fim::string("autocmd_del"  ),fim::string("usage: autocmd_del [Event] [Pattern] [Commands]"),this,&CommandConsole::autocmd_del));	/* this syntax is incompatible with vim ('autocmd!')*/
#endif
		addCommand(new Command(fim::string("set_interactive_mode"  ),fim::string("sets interactive mode"),this,&CommandConsole::set_interactive_mode));
		addCommand(new Command(fim::string("set_console_mode"  ),fim::string("sets console mode"),this,&CommandConsole::set_in_console));
#ifndef FIM_NO_SYSTEM
		addCommand(new Command(fim::string("system"  ),fim::string(FIM_CMD_HELP_SYSTEM),this,&CommandConsole::system));
#endif
		addCommand(new Command(fim::string("cd"      ),fim::string(FIM_CMD_HELP_CD  ),this,&CommandConsole::cd));
		addCommand(new Command(fim::string("pwd"     ),fim::string(FIM_CMD_HELP_PWD   ),this,&CommandConsole::pwd));
		addCommand(new Command(fim::string("popen"  ),fim::string("popen() invocation"),this,&CommandConsole::sys_popen));
		addCommand(new Command(fim::string("stdout"  ),fim::string("writes to stdout"),this,&CommandConsole::_stdout));
#ifdef FIM_PIPE_IMAGE_READ
		addCommand(new Command(fim::string("pread"  ),fim::string("executes the arguments as a shell command and reads the input as an image file (uses popen)"),this,&CommandConsole::pread));
#endif
#ifdef FIM_RECORDING
		addCommand(new Command(fim::string("start_recording"  ),fim::string("starts recording of commands"),this,&CommandConsole::start_recording));
		addCommand(new Command(fim::string("stop_recording"  ),fim::string("stops recording of commands"),this,&CommandConsole::stop_recording));
		addCommand(new Command(fim::string("dump_record_buffer"  ),fim::string("dumps on screen record buffer"),this,&CommandConsole::dump_record_buffer));
		addCommand(new Command(fim::string("execute_record_buffer"  ),fim::string("executes the record buffer"),this,&CommandConsole::execute_record_buffer));
		addCommand(new Command(fim::string("eval"),fim::string(FIM_CMD_HELP_EVAL),this,&CommandConsole::eval));
		addCommand(new Command(fim::string("repeat_last"  ),fim::string("repeats the last action"),this,&CommandConsole::repeat_last));
#endif
		addCommand(new Command(fim::string("variables"  ),fim::string("displays the associated variables"),this,&CommandConsole::variables_list));
		addCommand(new Command(fim::string("commands"  ),fim::string("displays the existing commands"),this,&CommandConsole::commands_list));
		addCommand(new Command(fim::string("dump_key_codes"  ),fim::string("dumps the key codes"),this,&CommandConsole::dump_key_codes));
		addCommand(new Command(fim::string("clear"  ),fim::string("clears the virtual console"),this,&CommandConsole::clear));
		addCommand(new Command(fim::string("scroll_console_up"  ),fim::string("scrolls up the virtual console"),this,&CommandConsole::scroll_up));
		addCommand(new Command(fim::string("scroll_console_down"),fim::string("scrolls down the virtual console"),this,&CommandConsole::scroll_down));
		/*
		 * This is not a nice choice, but it is clean regarding this file.
		 */
		#include "defaultConfiguration.cpp"
		setVariable(FIM_VID_PWD,pwd(args_t()).c_str());
		setVariable(FIM_VID_STEPS,50);
		setVariable(FIM_VID_TERM, fim_getenv("TERM"));		/* We read an environment variable */

		*prompt='\0';
		*(prompt+1)='\0';
	}

        bool CommandConsole::is_file(fim::string nf)const
        {
		/*
		 * note:
		 * this function is written a little bit unsafely,
		 * because the file could change between calls.
		 * improvements are possible.
		 */
                struct stat stat_s;
                /*      if the file doesn't exist, return */
                if(-1==stat(nf.c_str(),&stat_s))return false;
                if( S_ISDIR(stat_s.st_mode))return false;
                return true;
        }

	int CommandConsole::init(string device)
	{
		/*
		 * TODO : move most of this stuff to the constructor, some day.
		 */

		/* new : prevents atof, sprintf and such conversion mismatches! */
		setlocale(LC_ALL,"C");	/* portable (among Linux hosts) : should use dots for numerical radix separator */
		//setlocale(LC_NUMERIC,"en_US"); /* lame  */
		//setlocale(LC_ALL,""); /* just lame */

		displaydevice=NULL;	/* TODO : is this really necessary ? */
		int xres=0,yres=0;

#ifndef FIM_WITH_NO_FRAMEBUFFER
		if( device=="fb" )
		{
			extern char *default_fbdev,*default_fbmode;
			extern int default_vt;
			extern float default_fbgamma;
			FramebufferDevice * ffdp=NULL;

			displaydevice=new FramebufferDevice(mc);
			if(!displaydevice || ((FramebufferDevice*)displaydevice)->framebuffer_init()){cleanup();return -1;}
			ffdp=((FramebufferDevice*)displaydevice);
			setVariable(FIM_VID_DEVICE_DRIVER, "fbdev");
			if(default_fbdev)ffdp->set_fbdev(default_fbdev);
			if(default_fbmode)ffdp->set_fbmode(default_fbmode);
			if(default_vt!=-1)ffdp->set_default_vt(default_vt);
			if(default_fbgamma!=-1.0)ffdp->set_default_fbgamma(default_fbgamma);
		}
#endif	//#ifndef FIM_WITH_NO_FRAMEBUFFER


		#ifdef FIM_WITH_LIBSDL
		if(device=="sdl")
		{
			DisplayDevice *sdld=NULL;
			sdld=new SDLDevice(mc); if(sdld && sdld->initialize(key_bindings)!=0){delete sdld ; sdld=NULL;}
			if(sdld && displaydevice==NULL)
			{
				displaydevice=sdld;
				setVariable(FIM_VID_DEVICE_DRIVER, "sdl");
			}
		}
		#endif

		#ifdef FIM_WITH_CACALIB
		if(device=="caca")
		{
			DisplayDevice *cacad=NULL;
			cacad=new CACADevice(mc); if(cacad && cacad->initialize(key_bindings)!=0){delete cacad ; cacad=NULL;}
			if(cacad && displaydevice==NULL)
			{
				displaydevice=cacad;
				setVariable(FIM_VID_DEVICE_DRIVER, "cacalib");
			}
		}
		#endif

		#ifdef FIM_WITH_AALIB
		if(device=="aa")
		{
		aad=new AADevice(mc);

		if(aad && aad->initialize(key_bindings)!=0){delete aad ; aad=NULL;}
		if(aad && displaydevice==NULL)
		{
			displaydevice=aad;
			setVariable(FIM_VID_DEVICE_DRIVER, "aalib");

#if 1
			/*
			 * FIXME
			 *
			 * seems like the keymaps get shifted when running under screen
			 * weird, isn't it ?
			 * Regard this as a weird patch.
			 * */
			const char * term = fim_getenv("TERM");
			if(term && string(term).re_match("screen"))
			{
				key_bindings["Left"]-=3072;
				key_bindings["Right"]-=3072;
				key_bindings["Up"]-=3072;
				key_bindings["Down"]-=3072;
			}
#endif
		}
		}
		#endif
		tty_raw();// this inhibits unwanted key printout (raw mode), and saves the current tty state

		if( displaydevice==NULL)
		{
			displaydevice=&dummydisplaydevice;
			setVariable(FIM_VID_DEVICE_DRIVER, "dummy");
		}

		xres=displaydevice->width(),yres=displaydevice->height();

		// textual console reformatting (should go to displaydevice some day)
		displaydevice->init_console();
	

#ifdef FIM_WINDOWS
	
		/* true pixels if we are in framebuffer mode */
		/* fake pixels if we are in text (er.. less than!) mode */
		if( xres<=0 || yres<=0 ) return -1;

		try
		{
			window = new Window( *this, Rect(0,0,xres,yres) );

			if(window)window->setroot();
		}
		catch(FimException e)
		{
			if( e == FIM_E_NO_MEM || true ) quit(FIM_E_NO_MEM);
		}

		/*
		 * TODO: exceptions should be launched here in case ...
		 * */
		addCommand(new Command(fim::string("window" ),fim::string("manipulates the window system windows"), window,&Window::cmd));
#else
		try
		{
			viewport = new Viewport(*this);
		}
		catch(FimException e)
		{
			if( e == FIM_E_NO_MEM || true ) quit(FIM_E_NO_MEM);
		}
#endif
		setVariable(FIM_VID_SCREEN_WIDTH, xres);
		setVariable(FIM_VID_SCREEN_HEIGHT,yres);

		/*
		 *	Here the program loads initialization scripts :
		 * 	the default configuration file, and user invoked scripts.
		 */
//		executeFile("/etc/fim.conf");	//GLOBAL DEFAULT CONFIGURATION FILE
//		executeFile("/etc/fimrc");	//GLOBAL DEFAULT CONFIGURATION FILE

#ifndef FIM_NOFIMRC
  #ifndef FIM_NOSCRIPTING
		char rcfile[FIM_PATH_MAX];
		const char *e = fim_getenv("HOME");

		/* default, hard-coded configuration first */
		if(getIntVariable(FIM_VID_LOAD_DEFAULT_ETC_FIMRC)==1 )
		{
			if(is_file("/etc/fimrc"))
				if(-1==executeFile("/etc/fimrc"));
		}
		
		/* default, hard-coded configuration first */
		if(getIntVariable(FIM_VID_NO_DEFAULT_CONFIGURATION)==0 )
		{
    #ifdef FIM_DEFAULT_CONFIGURATION
			/* so the user could inspect what goes in the default configuration */
			setVariable(FIM_VID_FIM_DEFAULT_CONFIG_FILE_CONTENTS,FIM_DEFAULT_CONFIG_FILE_CONTENTS);

			execute(FIM_DEFAULT_CONFIG_FILE_CONTENTS,0,1);
    #endif		
		}

		if(e && strlen(e)<FIM_PATH_MAX-8)//strlen("/.fimrc")+2
		{
			strcpy(rcfile,e);
			strcat(rcfile,"/.fimrc");
			if(getIntVariable(FIM_VID_NO_RC_FILE)==0 )
			{
				if(
					(!is_file(rcfile) || -1==executeFile(rcfile))
				&&	(!is_file("/etc/fimrc") || -1==executeFile("/etc/fimrc"))
				  )
  #endif
#endif
				{
					/*
					 if no configuration file is present, or fails loading,
					 we use the default configuration (raccomended !)  !	*/
  #ifdef FIM_DEFAULT_CONFIGURATION
					execute(FIM_DEFAULT_CONFIG_FILE_CONTENTS,0,1);
  #endif		
				}
#ifndef FIM_NOFIMRC
  #ifndef FIM_NOSCRIPTING
			}

		}
  #endif		
#endif		
#ifndef FIM_NOSCRIPTING
		for(size_t i=0;i<scripts.size();++i) executeFile(scripts[i].c_str());
#endif		
#ifdef FIM_AUTOCMDS
		if(postInitCommand!=fim::string(""))
			autocmd_add("PreExecutionCycle","",postInitCommand.c_str());
		if(postExecutionCommand!=fim::string(""))
			autocmd_add("PostExecutionCycle","",postExecutionCommand.c_str());
#endif
		/*
		 *	FIXME : A TRADITIONAL /etc/fimrc LOADING WOULDN'T BE BAD..
		 * */
#ifdef FIM_USE_READLINE
		rl::initialize_readline( !displaydevice );
		load_history();
#endif
		if(getIntVariable(FIM_VID_SANITY_CHECK)==1 )
		{
			/* experimental */
			displaydevice->quickbench();
			quit(return_code);
			exit(return_code);
		}
		return 0;
	}

	int CommandConsole::addCommand(Command *c)
	{
		/*
		 * C is added to the commands list
		 */
		assert(c);	//FIXME : see the macro NDEBUG for this
		commands.push_back(c);
		return 0;
	}

	fim::string CommandConsole::alias(const fim::string& a,const fim::string& c)
	{
		/*
		 * an internal alias method
		 *
		 * FIXME: ERROR CHECKING NEEDED
		 * ( checks on arguments correctness ! )
		 */
		std::vector<fim::Arg> args;
		args.push_back(Arg(a));
		args.push_back(Arg(c));
		return alias(args);
	}

	char * CommandConsole::command_generator (const char *text,int state)const
	{
		/*
		 *	This is the reason why the commands should be kept
		 *	in a list or vector, rather than a map...  :(
		 *
		 *	TODO : INSTEAD OF USING commands[], make a new vector 
		 *	with completions!
		 *	FIX ME
		 *
		 *	DANGER : this method allocates memory
		 */
		static size_t list_index=0;
		if(state==0)list_index=0;
		while(isdigit(*text))text++;	//initial  repeat match
		const fim::string cmd(text);
		if(cmd=="")return NULL;
		args_t completions;
		aliases_t::const_iterator ai;
		variables_t::const_iterator vi;
		for(size_t i=0;i<commands.size();++i)
		{
			if(commands[i]->cmd.find(cmd)==0)
			completions.push_back(commands[i]->cmd);
		}
		for( ai=aliases.begin();ai!=aliases.end();++ai)
		{	
			if((ai->first).find(cmd)==0){
//			cout << ".." << ai->first << ".." << " matches " << cmd << "\n";
			completions.push_back((*ai).first);}
		}
		for( vi=variables.begin();vi!=variables.end();++vi)
		{
			if((vi->first).find(cmd)==0)
			completions.push_back((*vi).first);
		}
#ifndef FIM_COMMAND_AUTOCOMPLETION
		/* THIS DIRECTIVE IS MOTIVATED BY SOME STRANGE BUG!
		 */
		sort(completions.begin(),completions.end());
#endif 
		
/*		for(size_t i=list_index;i<completions.size();++i)
				cout << cmd << " matches with " << completions[i].c_str()<<  "\n";*/
		for(size_t i=list_index;i<completions.size();++i)
		{
			if(completions[i].find(cmd)==0)
			{
				list_index++;
				//readline will free this strings..
				return dupstr(completions[i].c_str());// is this malloc free ?
			}
			else
				;//std::cout << cmd << " no matches with " << commands[i]->cmd<<  "\n";
		}

/*		for(int i=list_index;i<aliases_keys.size();++i)
		{
			if(!commands[i])continue;
			if(commands[i]->cmd.find(cmd)==0)
			{
				list_index++;
				//std::cout << cmd << " matches with " << commands[i]->cmd<<  "\n";
				//readline will free this strings..
				return dupstr(commands[i]->cmd.c_str());
			}
			else
				;//std::cout << cmd << " no matches with " << commands[i]->cmd<<  "\n";
		}*/
		//TO DO : ADD VARIABLE AND ALIAS EXPANSION..
		return NULL;
	}

#define istrncpy(x,y,z) {strncpy(x,y,z-1);x[z-1]='\0';}
#define ferror(s) {/*fatal error*/FIM_FPRINTF(stderr, "%s,%d:%s(please submit this error as a bug!)\n",__FILE__,__LINE__,s);}/* temporarily, for security reason : no exceptions launched */
//#define ferror(s) {/*fatal error*/FIM_FPRINTF(stderr, "%s,%d:%s(please submit this error as a bug!)\n",__FILE__,__LINE__,s);throw FIM_E_TRAGIC;}

	fim::string CommandConsole::getBoundAction(const int c)const
	{
		/*
		 * returns the action assigned to key biding c
		 * */
		//return bindings[c];
		std::map<int,fim::string>::const_iterator bi=bindings.find(c);
		if(bi!=bindings.end()) return bi->second;
		else return "";
	}

	void CommandConsole::executeBinding(const int c)
	{
		/*
		 *	Executes the command eventually bound to c.
		 *	Doesn't log anything.
		 *	Just interpretates and executes the binding.
		 *	If the binding is inexistent, ignores silently the error.
		 */
		bindings_t::const_iterator bi=bindings.find(c);
		int status=0;
#define KEY_OFFSET 48

#ifdef FIM_ITERATED_COMMANDS
		static int it_buf=-1;
		if( c>=48 && c <=57 && (bi==bindings.end() || bi->second==""))//a number, not bound
		{
			if(it_buf>0){it_buf*=10;it_buf+=c - KEY_OFFSET;}
			else it_buf=c - KEY_OFFSET;
		}
#endif

		if(bi!=bindings.end() && bi->second!="")
		{
			fim::string cf=current();
#ifdef FIM_AUTOCMDS
			autocmd_exec("PreInteractiveCommand",cf);
#endif
#ifdef FIM_ITERATED_COMMANDS
			if(it_buf>1)
			{
				int m = getIntVariable(FIM_VID_MAX_ITERATED_COMMANDS);
				fim::string nc;
				/*
				 *  A non positive value of  _max_iterated_commands
				 *  will imply no limits on it_buf.
				 *
				 *  We can't assume the user is not so dumb to set a near 31 bits value..
				 */
				if(m>0 && m+1>0)
					it_buf=it_buf%(m+1);
				nc=it_buf;
				if(it_buf>1)
					nc+="{"+getBoundAction(c)+"}";
					/* adding ; before } can cause problems as long as ;; is not supported by the parser*/
				else
					nc = getBoundAction(c);
					
				cout << "about to execute " << nc << "\n";
				status=execute(nc.c_str(),1,0);
				it_buf=-1;
			}
			else
#endif
				status=execute(getBoundAction(c).c_str(),0,0);
#ifdef FIM_AUTOCMDS
			autocmd_exec("PostInteractiveCommand",cf);
#endif
		}

		if(status)
		{
			std::cerr << "error performing execute()\n";
			//show_must_go_on=0;	/* we terminate interactive execution */
		}
	}

	int CommandConsole::execute(const char *ss, int add_history_, int suppress_output_)
	{
		try{
		/*
		 *	This method executes a character string containing a script.
		 *	The second argument specifies whether the command is added or 
		 *	not to the command history buffer.
		 *
		 *	note : the pipe here opened shall be closed in the yyparse()
		 *	call, by the YY_INPUT macro (defined by me in lex.lex)
		 */
		char *s=dupstr(ss);//this malloc is free
		if(s==NULL)
		{
			std::cerr << "allocation problem!\n";
			//if(s==NULL){ferror("null command");return;}
			//assert(s);
			//this shouldn't happen
			//this->quit(0);
			return -1;
		}
		//we open a pipe with the lexer/parser
		int r = pipe(pipedesc);
		if(r!=0)
		{
			//strerror(errno);
			std::cerr << "error piping with the command interpreter ( pipe() gave "<< r<< " )\n";
			std::cerr << "the command was:\"" << ss << "\"\n";
			std::cerr << "we had : "<< aliases.size()<< " aliases\n";
//			std::exit(-1);
//			ferror("pipe error\n");
//   			cleanup();
			return -1;
		}
		//we write there our script or commands
		r=write(pipedesc[1],s,strlen(s));
		//we are done!
		if((size_t)r!=strlen(s))
		{
			ferror("write error");
    			cleanup();
			return -1;
		} 
		for(char*p=s;*p;++p)
			if(*p=='\n')*p=' ';
		close(pipedesc[1]);
		try	{	yyparse();		}
		catch	(FimException e)
		{
			if( e == FIM_E_TRAGIC || e == FIM_E_NO_MEM ) this->quit( FIM_E_NO_MEM );
			else ;	/* ]:-)> */
		}

#ifdef FIM_USE_READLINE
		if(add_history_)if(nochars(s)==0)add_history(s);
#endif
		fim_free(s);

		}
		catch	(FimException e)
		{
			if( e == FIM_E_TRAGIC || true ) this->quit( FIM_E_TRAGIC );
		}
		//we add to history only meaningful commands/aliases.
		return 0;
	}

        fim::string CommandConsole::execute(fim::string cmd, args_t args)
	{
		/*
		 *	This is the method where the tokenized commands are executed.
		 *	This method executes single commands with arguments.
		 */
		Command *c=NULL;
		/*
		 * we first determine if this is an alias
		 */

		fim::string ocmd=aliasRecall(cmd);
		if(ocmd!="")
		{
			//an alias should be expanded. arguments are appended.
			fim::string ex;
			cmd=ocmd;
			ex=ocmd;
			/*
			 * WARNING : i am not sure this is the best choice
			 */
			int r = pipe(pipedesc),sl;
			if(r!=0){ferror("pipe error\n");exit(-1);}
#ifndef			FIM_ALIASES_WITHOUT_ARGUMENTS
			for(size_t i=0;i<args.size();++i)
			{
				ex+=fim::string(" \""); ex+=args[i];
				ex+=fim::string("\""); 
			}
#endif
			sl=strlen(ex.c_str());
			r=write(pipedesc[1],ex.c_str(),sl);
			if(r!=sl){ferror("pipe write error");exit(-1);} 
			
			/*
			 * now the yyparse macro YY_INPUT itself handles the closing of the pipe.
			 *
			 * in this way nested commands could not cause harm, because the pipe
			 * is terminated BEFORE executing the command, and reusing pipedesc
			 * is harmless.
			 *
			 * before occurred multiple pipe creations, on the same descriptor buffer,
			 * resulting in a loss of the original descriptors on openings..
			 */
			close(pipedesc[1]);
			yyparse();
			return "";
		}
		if(cmd=="usleep")
		{
			unsigned int useconds;
			if(args.size()>0) useconds=atoi(args[0].c_str());
			else useconds=1;
			usleep((unsigned long)useconds);
			return "";
		}else
		if(cmd=="sleep")
		{
			int seconds;
			//sleeping for an amount of time specified in seconds.
			
			if(args.size()>0) seconds=atoi(args[0].c_str());
			else seconds=1;
#if 0
				sleep(seconds);
#else
				/*
				 * FIXME : we would like interruptible sleep.
				 */
				//while(seconds>0 && catchLoopBreakingCommand(seconds--))sleep(1);
				catchLoopBreakingCommand(seconds);
#endif
			return "";
		}else
		if(cmd=="alias")
		{
			//assignment of an alias
			std::vector<Arg> aargs;	//Arg args :P
			for(size_t i=0;i<args.size();++i)
			{
				aargs.push_back(Arg(args[i]));
			}
			cout << this->alias(aargs) << "\n";
			return "";
		}
		else
		{
			c=findCommand(cmd);

#ifdef FIM_COMMAND_AUTOCOMPLETION
			/*
			 * in case command autocompletion is enabled
			 */
			if(c==NULL)
			{
				char *match = this->command_generator(cmd.c_str(),0);
				if(match)
				{
					//cout << "but found :`"<<match<<"...\n";
					c=findCommand(match);
					fim_free(match);
				}
			}
#endif
			if(c==NULL)
			{
				/* FIXME : should stringify here and elsewhere
				 * see also http://gcc.gnu.org/onlinedocs/cpp/index.html
				 */
				cout << "sorry, no such command :`"<<cmd.c_str()<<"'\n";
				return "";
			}
			else
			{
				if(getIntVariable(FIM_VID_DBG_COMMANDS)!=0)
					std::cout << "in " << cmd << ":\n";
				
				cout << c->execute(args);

				return "";
			}
		}
		return "If you see this string, please report it to the program maintainer :P\n";
	}

	int CommandConsole::catchLoopBreakingCommand(int seconds)
	{
		/*	
		 *	This method is invoked during non interactive loops to
		 *	provide a method for interactive loop breaking.
		 *
		 *	The provided mechanism allows the user to press any key
		 *	during the loop, and the loop will continue its execution,
		 *	unless the pressed key is not exitBinding.
		 *
		 *	If not, and the key is bound to some action; this action
		 *	is executed.
		 *
		 *	FIXME : this could nest while loops !
		 *
		 *	returns 0 if no command was received.
		 */
		int c;

		//exitBinding = 10;
		if ( exitBinding == 0 ) return 1;	/* any key triggers an exit */

		c = displaydevice->catchInteractiveCommand(seconds);
	//	while((c = displaydevice.catchInteractiveCommand(seconds))!=-1)
		while(c!=-1)
		{
			/* while characters read */
			//if( c == -1 ) return 0;	/* no chars read */
			if( c != exitBinding )  /* some char read */
			{
				/*
				 * we give the user chance to issue commands
				 * and some times to realize this.
				 *
				 * is it a desirable behaviour ?
				 */
				executeBinding(c);
				c = displaydevice->catchInteractiveCommand(1);
//				return 0;/* could be a command key */
			}
			if(c==exitBinding) return 1; 		/* the user hit the exitBinding key */
			key_bindings_t::const_iterator ki;
//			if(c==key_bindings["Esc"]) return 1; 		/* the user hit the exitBinding key */
//			if(c==key_bindings[":"]) return 1; 		/* the user hit the exitBinding key */
			if((ki=key_bindings.find("Esc"))!=key_bindings.end() && c==ki->second)return 1;
			if((ki=key_bindings.find(":"  ))!=key_bindings.end() && c==ki->second)return 1;
		}
		return 0; 		/* no chars read  */

	}
		

#ifdef	FIM_USE_GPM
/*
	int gh(Gpm_Event *event, void *clientdata)
	{
		exit(0);
		quit();
		return 'n';
		return 0;
	}
*/
#endif

	int CommandConsole::executionCycle()
	{
		/*
		 * the cycle with fetches the instruction stream.
		 * */
#ifdef	FIM_USE_GPM
		//Gpm_PushRoi(0,0,1023,768,GPM_DOWN|GPM_UP|GPM_DRAG|GPM_ENTER|GPM_LEAVE,gh,NULL);
#endif
#ifdef FIM_AUTOCMDS
		fim::string initial=browser.current();
		autocmd_exec("PreExecutionCycle",initial);
		autocmd_exec("PreExecutionCycleArgs",initial);

#endif
		*prompt='\0';

	 	while(show_must_go_on)
		{
			cycles++;

#if 0
			/* dead code */
			// FIXME : document this
			fd_set          set;
			struct timeval  limit;
			FD_SET(0, &set);
			limit.tv_sec = -1;
			limit.tv_usec = 0;
#endif

#ifdef FIM_USE_READLINE
			if(ic==1)
			{
				ic=1;
				char *rl=readline(":");
				*prompt=':';
				if(rl==NULL)
				{
					/* FIXME : should exit ? */
					this->quit();
					/* empty line */
				}
				else if(*rl!='\0')
				{
					/*
					 * This code gets executed when the user is about to exit console mode, 
					 * having she pressed the 'Enter' key and expecting result.
					 * */
					fim::string cf=current();
#ifdef FIM_AUTOCMDS
					autocmd_exec("PreInteractiveCommand",cf);
#endif
#ifdef FIM_RECORDING
					if(recordMode)record_action(fim::string(rl));
#endif					
					//ic=0; // we 'exit' from the console for a while (WHY ? THIS CAUSES PRINTING PROBLEMS)
					execute(rl,1,0);	//execution of the command line with history
					ic=(ic==-1)?0:1; //a command could change the mode !
//					this->setVariable(FIM_VID_DISPLAY_CONSOLE,1);	//!!
//					execute("redisplay;",0,0);	//execution of the command line with history
#ifdef FIM_AUTOCMDS
					autocmd_exec("PostInteractiveCommand",cf);
#endif
#ifdef FIM_RECORDING
					memorize_last(rl);
#endif
					//p.s.:note that current() returns not necessarily the same in 
					//the two autocmd_exec() calls..
				}
				/*  else : *rl=="" : doesn't happen :) */

				if(rl && *rl=='\0')
				{
					/* happens when no command is issued and Enter key is pressed */
					ic=0;
					*(prompt)='\0';
					set_status_bar("",NULL);
				}
				if(rl)fim_free(rl);
			}
			else
#endif
			{
				*prompt='\0';
				unsigned int c;
				int r;char buf[64];
//				int c=getchar();
//				int c=fgetc(stdin);
				/*
				 *	problems :
				 *	 I can't read Control key and 
				 *	some upper case key together.
				 *	 I am not quite sure about portability..
				 *	... maybe a sample program which photograph
				 *	the keyboard is needed!.
				 */
				c=0;
				
				r=displaydevice->get_input(&c);
#ifdef	FIM_USE_GPM
/*
				Gpm_Event *EVENT;
				if(Gpm_GetEvent(EVENT)==1)quit();
				else cout << "...";*/
#endif
				if(r>0)
				{
					if(getIntVariable(FIM_VID_VERBOSE_KEYS))
					{
						/*
						 * <0x20 ? print ^ 0x40+..
						 * */
						sprintf(buf,"got : %x (%d)\n",c,c);
						cout << buf ;
					}
#ifndef FIM_USE_READLINE
					if(c==(unsigned int)getIntVariable(FIM_VID_CONSOLE_KEY) || 
					   c=='/')set_status_bar("compiled with no readline support!\n",NULL);
#else
					if(c==(unsigned int)getIntVariable(FIM_VID_CONSOLE_KEY)
					){ic=1;*prompt=':';}	//should be configurable..
					else if(c=='/')
					{
						/*
						 * this is a hack to handle vim-styled regexp searches
						 */
						ic=1;
						int tmp=rl_filename_completion_desired;
						rl_inhibit_completion=1;
						*prompt='/';
						char *rl=readline("/"); // !!
						// no readline ? no interactive searches !
						*prompt='\0';
						rl_inhibit_completion=tmp;
						ic=0;
						if(rl==NULL)
						{
							/* FIXME : should exit ? */
							this->quit();
							/* empty line */
						}
						/* 
						 * if using "" instead string("")
						 * warning: comparison with string literal results in unspecified behaviour */
						else if(rl!=string(""))
						{
							args_t args;
							args.push_back(rl);
							execute("regexp_goto",args);
						}
					}
					else
#endif
					{

						this->executeBinding(c);
#ifdef FIM_RECORDING
						if(recordMode) record_action(getBoundAction(c));
						memorize_last(getBoundAction(c));
#endif
					}
				}else
				{
					//cout<< "error reading key from keyboard\n";
					/*
					 * 	This happens when console switching, too.
					 * 	( switching out of the current! )
					 * 	So a redraw after is not bad.
					 * 	But it should work when stepping into the console,
					 * 	not out..
					 */
				}
			}
		}
#ifdef FIM_AUTOCMDS
		autocmd_exec("PostExecutionCycle",initial);
#endif
		return quit(return_code);
	}

	void CommandConsole::exit(int i)const
	{
		/*
		 *	This method will exit the program as a whole.
		 *      If various object destructors are set to destroy device
		 *	contexts, it should do no harm to the console.
		 *      (it will call statically declared object's destructors )
		 */
		std::exit(i);
	}

	int CommandConsole::quit(int i)
	{
		/*
		 * the method to be called to exit from the program safely.
		 * it is used mainly for safe and exit after severe errors.
		 * TODO : get rid of it.
		 */
    		cleanup();
		return i;/* is should be used in return */
	}

	CommandConsole::~CommandConsole()
	{
		/*
		 * NOTE:
		 * as long as this class is a singleton, we couldn't care less about memory freeing :)
		 */
		if(!marked_files.empty())
		{
			std::cerr << "The following files were marked by the user :\n";
			std::cout << "\n";
			for(std::set<fim::string>::iterator i=marked_files.begin();i!=marked_files.end();++i)
			std::cout << *i << "\n";
		}
		
		fim::string sof=getStringVariable(FIM_VID_SCRIPTOUT_FILE);
		if(sof!="")
		{
        		if(is_file(sof))
			{
				std::cerr << "Warning : the "<<sof<<" file exists and will not be overwritten!\n";
			}
			else
			{
				std::ofstream out(sof.c_str());
				if(!out)
				{
					std::cerr << "Warning : The "<<sof<<" file could not be opened for writing!\n";
					std::cerr << "check output directory permissions and retry!\n";
				}
				else
				{
					out << dump_record_buffer(args_t()) << "\n";
					out.close();
				}
			}
		}
		for(size_t i=0;i<commands.size();++i)
			if(commands[i])
				delete commands[i];

	#ifdef FIM_WITH_AALIB
		if(aad && !displaydevice)
			delete aad;	/* aad is an alias */
		else
	#endif
		{
			if(displaydevice && displaydevice != &dummydisplaydevice)delete displaydevice;
		}

#ifdef FIM_WINDOWS
		if(window) delete window;
#else
		if(viewport) delete viewport;
#endif
	}

	int CommandConsole::toggleStatusLine()
	{
		/*
		 * toggles the display of the status line
		 *
		 * FIX ME
		 */
		int sl=getIntVariable(FIM_VID_STATUS_LINE)?0:1;
		setVariable(FIM_VID_STATUS_LINE,sl);
		return 0;
	}

	fim::string CommandConsole::readStdFileDescriptor(FILE* fd)
	{
		/*
		 * TODO : catch exceptions
		 */

		int r;
		char buf[4096];	// TODO : buffer too small
		fim::string cmds;
		if(fd==NULL)return -1;
		while((r=fread(buf,1,sizeof(buf)-1,fd))>0){buf[r]='\0';cmds+=buf;}
		if(r==-1)return -1;
		return cmds;
	}
	
	int CommandConsole::executeStdFileDescriptor(FILE* fd)
	{
		/*
		 * FIX ME  HORRIBLE : FILE DESCRIPTOR USED AS A FILE HANDLE..
		 *
		 * TODO : catch exceptions
		 */

		int r;
		char buf[4096];
		fim::string cmds;
		if(fd==NULL)return -1;
		while((r=fread(buf,1,sizeof(buf)-1,fd))>0){buf[r]='\0';cmds+=buf;}
		if(r==-1)return -1;
		execute(cmds.c_str(),0,1);
		return 0;
	}

	int CommandConsole::executeFile(const char *s)
	{
		/*
		 * executes a file denoted by filename
		 *
		 * TODO : catch exceptions
		 * */
		execute(slurp_file(s).c_str(),0,1);
		return 0;
	}

	int CommandConsole::getVariableType(const fim::string &varname)const
	{
		/*
		 * returns the [internal] type of a variable
		 * FIXME : eradicate this cancer
		 * */
		variables_t::const_iterator vi=variables.find(varname);
		if(vi!=variables.end())
			return vi->second.getType();
		else return 0;
	}

	bool CommandConsole::isVariable(const fim::string &varname)const
	{
		const char * s;
		s = getStringVariable(varname).c_str();
		return (s && *s);
	}

	int CommandConsole::printVariable(const fim::string &varname)const
	{	
		/*
		 * a variable is taken and converted to a string and printed
		 *
		 * FIXME: should stringify ?
		 * */
		fim::cout<<getStringVariable(varname);
		return 0;
	}

	int CommandConsole::drawOutput(const char *s)const
	{
		/*
		 * whether the console should draw or not itself upon the arrival of textual output
		 * */
		//std::cout << s << " : " << (this->inConsole() )<< ( (s&&*s) ) << "\n";
		return(	(	this->inConsole()	/* in the command line */
				&& (s&&*s) 		/* actually some text to add */
			) 
			|| this->getIntVariable(FIM_VID_DISPLAY_CONSOLE)	/* or user requested for showing console */
			);
	}

	fim::string CommandConsole::get_aliases_list()const
	{
		/*
		 * returns the list of set action aliases
		 */
		fim::string aliases_list;
		aliases_t::const_iterator ai;
		for( ai=aliases.begin();ai!=aliases.end();++ai)
		{	
			aliases_list+=((*ai).first);
			aliases_list+=" ";
		}
		return aliases_list;
	}

	fim::string CommandConsole::get_commands_list()const
	{
		/*
		 * returns the list of registered commands
		 */
		fim::string commands_list;
		for(size_t i=0;i<commands.size();++i)
		{
			if(i)commands_list+=" ";
			commands_list+=(commands[i]->cmd);
		}
		return commands_list;
	}

	fim::string CommandConsole::get_variables_list()const
	{
		/*
		 * returns the list of set variables
		 */
		fim::string acl;
		variables_t::const_iterator vi;
		for( vi=variables.begin();vi!=variables.end();++vi)
		{
			acl+=((*vi).first);
			acl+=" ";
		}
		return acl;
	}

#ifdef FIM_AUTOCMDS
	fim::string CommandConsole::autocmds_list(const fim::string event, const fim::string pattern)const
	{
		/*
		 * as of now, lists the events for which an autocmd could be assigned.
		 *
		 * FIX ME
		 */
		fim::string acl;
//		std::map<fim::string,std::map<fim::string,fim::string> >  autocmds;
		autocmds_t::const_iterator ai;
		if(event=="" && pattern=="")
		//for each autocommand event registered
		for( ai=autocmds.begin();ai!=autocmds.end();++ai )
		//for each file pattern registered, display the list..
		for(	autocmds_p_t::const_iterator api=((*ai)).second.begin();
				api!=((*ai)).second.end();++api )
		//.. display the list of autocommands...
		for(	args_t::const_iterator aui=((*api)).second.begin();
				aui!=((*api)).second.end();++aui )
		{
			acl+="autocmd \""; 
			acl+=(*ai).first; 
			acl+="\" \""; 
			acl+=(*api).first; 
			acl+="\" \""; 
			acl+=(*aui); 
			acl+="\"\n"; 
		}
		else
		if(pattern==""){
		autocmds_t::const_iterator ai=autocmds.find(event);
		//for each autocommand event registered
		//for each file pattern registered, display the list..
		if(ai!=autocmds.end())
		for(	autocmds_p_t::const_iterator api=(*ai).second.begin();
				api!=(*ai).second.end();++api )
		//.. display the list of autocommands...
		for(	args_t::const_iterator aui=((*api)).second.begin();
				aui!=((*api)).second.end();++aui )
		{
			acl+="autocmd \""; 
			acl+=(*ai).first; 
			acl+="\" \""; 
			acl+=(*api).first; 
			acl+="\" \""; 
			acl+=(*aui); 
			acl+="\"\n"; 
		}}
		else
		{
		autocmds_t::const_iterator ai=autocmds.find(event);
		//for each autocommand event registered
		//for each file pattern registered, display the list..
		if(ai!=autocmds.end())
		{
		autocmds_p_t::const_iterator api=(*ai).second.find(pattern);
		//.. display the list of autocommands...
		if(api!=(*ai).second.end())
		{
		for(	args_t::const_iterator aui=((*api)).second.begin();
				aui!=((*api)).second.end();++aui )
		{
			acl+="autocmd \""; 
			acl+=(*ai).first; 
			acl+="\" \""; 
			acl+=(*api).first; 
			acl+="\" \""; 
			acl+=(*aui); 
			acl+="\"\n"; 
		}}}}
		
		if(acl=="")acl="no autocommands loaded\n";
		return acl;
	}

	fim::string CommandConsole::autocmd_del(const fim::string event, const fim::string pattern, const fim::string action)
	{
		/*
		 */
		autocmds_t::iterator ai;
		size_t n = 0;

		if(event=="" && pattern=="" && action == "" )
		{
			/* deletion of all autocmd's */
			n = autocmds.size();
			for( ai=autocmds.begin();ai!=autocmds.end();++ai )
				autocmds.erase(ai);
		}
		else
		if(action=="" && pattern=="" )
		{
			/* deletion of all autocmd's for given event */
			ai=autocmds.find(event);
			if(ai==autocmds.end())return "";
			n = (*ai).second.size();
			for(	autocmds_p_t::iterator api=((*ai)).second.begin();
				api!=((*ai)).second.end();++api )
				(*ai).second.erase(api);
		}
		else
		if(action=="" )
		{
			/* deletion of all autocmd's for given event and pattern */
			ai=autocmds.find(event);
			if(ai==autocmds.end())return "";
			autocmds_p_t::iterator api=((*ai)).second.find(pattern);
			n = (*api).second.size();
			for(	args_t::iterator aui=((*api)).second.begin();
					aui!=((*api)).second.end();++aui )
						(*api).second.erase(aui);
		}
		if(n)
			return n+" autocmd's removed\n";
		else
			return "no autocmd's removed\n";
	}

	fim::string CommandConsole::autocmd_add(const fim::string &event,const fim::string &pat,const fim::string &cmd)
	{
		/*
		 * the internal autocommand add function
		 *
		 * TODO : VALID VS INVALID EVENTS?
		 */
		if(cmd=="")
		{
			cout << "can't add empty autocommand\n";return "";
		}
		for(size_t i=0;i<autocmds[event][pat].size();++i)
		if((autocmds[event][pat][i])==cmd)
		{
			cout << "autocommand "<<cmd<<" already specified for event \""<<event<<"\" and pattern \""<<pat<<"\"\n";
			return "";
		}
		autocmds[event][pat].push_back(cmd);
		return "";
	}

	fim::string CommandConsole::pre_autocmd_add(const fim::string &cmd)
	{
		/*
		 * this autocommand will take argument related autocommands
		 */
	    	return autocmd_add("PreExecutionCycleArgs","",cmd);
	}

	fim::string CommandConsole::autocmd_exec(const fim::string &event,const fim::string &fname)
	{
		/*
		 *	WARNING : maybe there is the need of a sandbox, for
		 *	any command could do harm to the iterators themselves!
		 *
		 *	SO THE FIRST MATCHING SHOULD RETURN!
		 */
		autocmds_p_t::const_iterator api;
		/*
		 *	we want to prevent from looping autocommands, so this rudimentary
		 *	mechanism should avoid the majority of them.
		 */
		autocmds_loop_frame_t frame(event,fname);
		if(! autocmd_in_stack( frame ))
		{
			autocmd_push_stack( frame );
			for( api=autocmds[event].begin();api!=autocmds[event].end();++api )
			{
				autocmd_exec(event,(*api).first,fname);
			}
			autocmd_pop_stack( frame );
		}
		else
		{
			cout << "WARNING : there is a loop for "
			     << "(event:" << event << ",filename:" << fname << ")";
		}
		return "";
	}

	fim::string CommandConsole::autocmd_exec(const fim::string &event,const fim::string &pat,const fim::string &fname)
	{
		/*
		 * executes all the actions associated to the current event, if the current 
		 * file name matches the individual patterns
		 */
//		cout << "autocmd_exec_cmd...\n";
//		cout << "autocmd_exec_cmd. for pat '" << fname <<  "'\n";

		if(getIntVariable(FIM_VID_DBG_AUTOCMD_TRACE_STACK)!=0)
			autocmd_trace_stack();
			
		if(regexp_match(fname.c_str(),pat.c_str()))
		{
			for (size_t i=0;i<autocmds[event][pat].size();++i)
			{
				autocmds_frame_t frame(autocmds_loop_frame_t(event,fname),(autocmds[event][pat][i]).c_str());
//				cout << "should exec '"<<event<<"'->'"<<autocmds[event][pat][i]<<"'\n";
				autocmds_stack.push_back(frame);
				execute((autocmds[event][pat][i]).c_str(),0,1);
				autocmds_stack.pop_back();
			}
		}
		return "";
	}

	void CommandConsole::autocmd_push_stack(const autocmds_loop_frame_t& frame)
	{
		//WARNING : ERROR DETECTION IS MISSING
		autocmds_loop_stack.push_back(frame);
	}

	void CommandConsole::autocmd_pop_stack(const autocmds_loop_frame_t& frame)
	{
		//WARNING : ERROR DETECTION IS MISSING
		autocmds_loop_stack.pop_back();
	}
	
	void CommandConsole::autocmd_trace_stack()
	{
		/*
		 * this is mainly a debug function: it will write to stdout
		 * the current autocommands stack trace.
		 * set the FIM_VID_DBG_AUTOCMD_TRACE_STACK variable
		 */
		size_t indent=0,i;
		if(autocmds_stack.end()==autocmds_stack.begin()) std::cout << "<>\n";
		for(
			autocmds_stack_t::const_iterator citer=autocmds_stack.begin();
			citer!=autocmds_stack.end();++citer,++indent )
			{
				for(i=0;i<indent+1;++i) std::cout << " ";
				std::cout
					<< citer->first.first << " "
					<< citer->first.second << " "
					<< citer->second << "\n";
			}
	}
	
	int CommandConsole::autocmd_in_stack(const autocmds_loop_frame_t& frame)const
	{
		/*
		 * this function prevents a second autocommand triggered against 
		 * the same file to execute
		 */
		//return  autocmds_loop_stack.find(frame)!=autocmds_loop_stack.end();
		return  find(autocmds_loop_stack.begin(),autocmds_loop_stack.end(),frame)!=autocmds_loop_stack.end();
	}
#endif
	
	bool CommandConsole::regexp_match(const char*s, const char*r)const
	{
		/*
		 *	given a string s, and a Posix regular expression r, this
		 *	method returns true if there is match. false otherwise.
		 */
		regex_t regex;		//should be static!!!
		const int nmatch=1;	// we are satisfied with the first match, aren't we ?
		regmatch_t pmatch[nmatch];

		/*
		 * we allow for the default match, in case of null regexp
		 */
		if(!r || !strlen(r))return true;

		/* fixup code for a mysterious bug
		 */
		if(*r=='*')return false;

		//if(regcomp(&regex,"^ \\+$", 0 | REG_EXTENDED | REG_ICASE )==-1)
		if(regcomp(&regex,r, 0 | REG_EXTENDED | (getIntVariable(FIM_VID_IGNORECASE)==0?0:REG_ICASE) )!=0)
		{
			/* error calling regcomp (invalid regexp?)! (should we warn the user ?) */
			//cout << "error calling regcomp (invalid regexp?)!" << "\n";
			return false;
		}
		else
		{
//			cout << "done calling regcomp!" << "\n";
		}
		//if(regexec(&regex,s+0,nmatch,pmatch,0)==0)
		if(regexec(&regex,s+0,nmatch,pmatch,0)!=REG_NOMATCH)
		{
//			cout << "'"<< s << "' matches with '" << r << "'\n";
/*			cout << "match : " << "\n";
			cout << "\"";
			for(int m=pmatch[0].rm_so;m<pmatch[0].rm_eo;++m)
				cout << s[0+m];
			cout << "\"\n";*/
			regfree(&regex);
			return true;
		}
		else
		{
			/*	no match	*/
		};
		regfree(&regex);
		return false;
		return true;
	}

	bool CommandConsole::redisplay()
	{
		/*
		 * quick and dirty display function
		 */
#ifdef FIM_WINDOWS
		bool needed_redisplay=false;
		try
		{
			if(window)
				needed_redisplay=window->recursive_redisplay();
		}
		catch	(FimException e)
		{
			// well, we should to something : FIXME
			std::cerr << "fatal error" << __FILE__ << ":" << __LINE__ << "\n";
		}
		return needed_redisplay;
#else
		browser.redisplay();
		return true;
#endif
	}

	bool CommandConsole::display()
	{
		/*
		 * quick and dirty display function
		 */
#ifdef FIM_WINDOWS
		bool needed_redisplay=false;
		try
		{
			if(window )
				needed_redisplay=window->recursive_display();
#if 0
			else
				printf("%s : here should go image rendering code.\n",__LINE__);
#endif
		}
		catch	(FimException e)
		{
			// well, we should to something : FIXME
			std::cerr << "fatal error" << __FILE__ << ":" << __LINE__ << "\n";
		}
		return needed_redisplay;
#else
		browser.redisplay();
		return true;
#endif
	}

#ifdef FIM_RECORDING
	void CommandConsole::record_action(const fim::string &cmd)
	{
		/*	(action,millisleeps waitingbefore) is registered	*/
		/*
		 * PROBLEM:
		  clock_gettime() clock() times() getrusage() time() asctime() ctime() 
		  are NOT suitable

		 * clock_gettime() needs librealtime, and segfaults
		 * clock() gives process time, with no sense
		 * times() gives process time
		 * getrusage() gives process time
		 * time() gives time in seconds..
		 * asctime(),ctime() give time in seconds..
		 *
		 * gettimeofday was suggested by antani, instantaneously (thx antani)
		 *
		 * NOTE: recording the start_recording command itself is not harmful,
		 * as it only sets a flag.
		 * */
		static int pt=0;int t,d,err;//t,pt in ms; d in us
	        struct timeval tv;
		if(cmd==""){pt=0;return;}
	        if(!pt){err=gettimeofday(&tv, NULL);pt=tv.tv_usec/1000+tv.tv_sec*1000;}
	        err=gettimeofday(&tv, NULL);t=tv.tv_usec/1000+tv.tv_sec*1000;
		d=(t-pt)*1000;
		pt=t;
		recorded_actions.push_back(recorded_action_t(sanitize_action(cmd),d));
	}
#endif

	void CommandConsole::markCurrentFile()
	{
		/*
		 * the current file will be added to the list of filenames
		 * which will be printed upon the program termination.
		 * */
		if(browser.current()!=FIM_STDIN_IMAGE_NAME)
		{
			marked_files.insert(browser.current());
			cout<<"Marked file \""<<browser.current()<<"\"\n";
		}
	}

	void CommandConsole::printHelpMessage(char *pn)const
	{
		/*
		 * a prompty help message is pretty printed in the console
		 * */
		std::cout<<" Usage: "<<pn<<" [OPTIONS] [FILES]\n";
		/*  printf("\nThe help will be here soon!\n");*/
	}

#ifdef FIM_RECORDING
	fim::string CommandConsole::memorize_last(const fim::string &cmd)
	{
		/*
		 * the last executed command is appended in the buffer.
		 * of course, there are exceptions to these.
		 * and are quite intricated...
		 */
		if(dont_record_last_action==false)
		{
			last_action=cmd;
		}
		dont_record_last_action=false;	//from now on we can memorize again
		return "";
	}

	fim::string CommandConsole::sanitize_action(const fim::string &cmd)const
	{
		/*
		 * the purpose of this method is to sanitize the action token
		 * in order to gain a dumpable and self standing action
		 */
		if(cmd.c_str()[strlen(cmd.c_str())-1]!=';')
			return cmd+fim::string(";");
		return cmd;
	}
#endif
	void CommandConsole::appendPostInitCommand(const char* c)
	{
		/*
		 * the supplied command is applied right before a normal execution of Fim
		 * but after the configuration file loading
		 * */
		postInitCommand+=c;
	}

	void CommandConsole::appendPostExecutionCommand(const fim::string &c)
	{
		/*
		 * the supplied command is applied right before a normal termination of Fim
		 * */
		postExecutionCommand+=c;
	}
	
	bool CommandConsole::appendedPostInitCommand()const
	{
		/*
		 * whether some command will be executed right after initialization
		 * */
		return postInitCommand!=fim::string("");
	}

	Viewport* CommandConsole::current_viewport()const
	{
		/*
		 * returns a reference to the current viewport.
		 *
		 * TODO : did we catch all exceptions ?
		 * */
#ifdef FIM_WINDOWS
		return current_window().current_viewportp();
#else
		return viewport;
#endif
	}

#ifdef FIM_WINDOWS
	const Window & CommandConsole::current_window()const
	{
		/*
		 * returns a reference to the current window.
		 * there should be one :)
		 * if not, consider the situation TRAGIC
		 * */
		if(!window)
		{
//			temporarily, for security reasons
//			throw FIM_E_TRAGIC;
		}
		return *window;
	}

#endif
	bool CommandConsole::push(const fim::string nf)
	{
		/*
		 * returns true if push was ok
		 * */
		return browser.push(nf);
	}

#ifndef FIM_NOSCRIPTING
	bool CommandConsole::push_scriptfile(const fim::string ns)
	{
		/*
		 * pushes a script up in the pre-execution scriptfile list
		 * */
	    	scripts.push_back(ns);
		return true; /* for now a fare return code */
	}
	bool CommandConsole::with_scriptfile()const
	{
		return scripts.size() !=0;
	}
#endif

	void CommandConsole::dumpDefaultFimrc()const
	{
#ifdef FIM_DEFAULT_CONFIGURATION
		std::cout << FIM_DEFAULT_CONFIG_FILE_CONTENTS << "\n";
#endif
	}

	fim::string CommandConsole::print_commands()const
	{
		cout << "VARIABLES : "<<get_variables_list()<<"\n";
		cout << "COMMANDS : "<<get_commands_list()<<"\n";
		cout << "ALIASES : "<<get_aliases_list()<<"\n";
		return "";
	}

	/*
	 *	Setting the terminal in raw mode means:
	 *	 - setting the line discipline
	 *	 - setting the read rate
	 *	 - disabling the echo
	 */
	void CommandConsole::tty_raw()
	{
		struct termios tattr;
		//we set the terminal in raw mode.
		    
		fcntl(0,F_GETFL,saved_fl);
		tcgetattr (0, &saved_attributes);
		    
		//fcntl(0,F_SETFL,O_BLOCK);
		memcpy(&tattr,&saved_attributes,sizeof(struct termios));
		tattr.c_lflag &= ~(ICANON|ECHO);
		tattr.c_cc[VMIN] = 1;
		tattr.c_cc[VTIME] = 0;
		tcsetattr (0, TCSAFLUSH, &tattr);
	}
	
	void CommandConsole::tty_restore()
	{	
		//POSIX.1 compliant:
		//"a SIGIO signal is sent whenever input or output becomes possible on that file descriptor"
		fcntl(0,F_SETFL,saved_fl);
		//the Terminal Console State Attributes will be set right NOW
		tcsetattr (0, TCSANOW, &saved_attributes);
	}

	int CommandConsole::save_history()
	{
#ifndef FIM_NOFIMRC
  #ifndef FIM_NOSCRIPTING
    #ifdef FIM_USE_READLINE
		/* default, hard-coded configuration first */
		if(getIntVariable(FIM_VID_SAVE_FIM_HISTORY)==1 )
		{
			char hfile[FIM_PATH_MAX];
			const char *e = fim_getenv("HOME");
			if(e && strlen(e)<FIM_PATH_MAX-14)//strlen(".fim_history")+2
			{
				strcpy(hfile,e);
				strcat(hfile,"/.fim_history");
				bool need_chmod=!is_file(hfile);		// will try to chmod if already non existent
				write_history(hfile);
				if(need_chmod)chmod(hfile,S_IRUSR|S_IWUSR);	// we write the first .fim_history in mode -rw------- (600)
			}
			/* else : /home/useeeeeeeeeeeeeeeeeeeeeee.....eeeeeeeer ? :) */
			
		}
    #endif
  #endif
#endif
		return 0;
	}

	int CommandConsole::load_history()
	{
#ifndef FIM_NOFIMRC
  #ifndef FIM_NOSCRIPTING
    #ifdef FIM_USE_READLINE
		/* default, hard-coded configuration first */
		if(getIntVariable(FIM_VID_LOAD_FIM_HISTORY)==1 )
		{
			char hfile[FIM_PATH_MAX];
			const char *e = fim_getenv("HOME");
			if(e && strlen(e)<FIM_PATH_MAX-14)//strlen(".fim_history")+2
			{
				strcpy(hfile,e);
				strcat(hfile,"/.fim_history");
				read_history(hfile);
			}
		}
    #endif
  #endif
#endif
		return 1;
	}

	/*
	 * This routine terminates the program as cleanly as possible.
	 * It should be used whenever useful.
	 */
	void CommandConsole::cleanup()
	{
		/*
		 * the display device should exit cleanly to avoid cluttering the console
		 * ... or the window system
		 * used by : fb_catch_exit_signals() : should this matter ?
		 * */

		tty_restore();	
		if(displaydevice) displaydevice->finalize();
#ifdef FIM_USE_READLINE
		save_history();
#endif
	}

	/*
	 * inserts the desc text into the textual console,
	 * and eventually displays it
	 */
	void CommandConsole::status_screen(const char *desc)
	{
		if(!displaydevice)
			return;

		displaydevice->fb_status_screen_new(desc,drawOutput(desc),0);
	}

	void CommandConsole::set_status_bar(fim::string desc, const char *info)
	{
		set_status_bar(desc.c_str(), info);
	}
	
	/*
	 *	Set the 'status bar' of the program.
	 *	- desc will be placed on the left corner
	 *	- info on the right
	 *	pointers are not freed
	 *
	 *	TODO: a printf-like general functionality
	 */
	void CommandConsole::set_status_bar(const char *desc, const char *info)
	{
		/*
		 * pointers are not freed, by any means
		 */
		//FIX ME : does this function always draw ?
		int chars, ilen;
		char *str;

		prompt[1]='\0';
	
		if( ! displaydevice   ) return;
	
		chars = displaydevice->get_chars_per_line();
		if(chars<1)return;
		str = (char*) fim_calloc(chars+1,1);//this malloc is free
		if(!str)return;
		//sprintf(str, "");
		*str='\0';
		if (info && desc)
		{
			/* non interactive print */
			/*
			 * FIXME : and what if chars < 11 ? :)
			 * */
			ilen = strlen(info);
			if(chars-14-ilen>0)
			{
				sprintf(str, "%s%-*.*s [ %s ] H - Help",prompt,
				chars-14-ilen, chars-14-ilen, desc, info);//here above there is the need of 14+ilen chars
			}
			else
			if(chars>5) sprintf(str, "<-!->");
			else
			if(chars>0) sprintf(str, "!");	/* :D */
		}
#ifdef FIM_USE_READLINE
		else
		if(chars>=12 && desc) /* would be a nonsense :) */
		{
			/* interactive print */
			static int statusline_cursor=0;
			int offset=0,coffset=0;
			statusline_cursor=rl_point;	/* rl_point is readline stuff */
			ilen = strlen(desc);
			chars-=11+(*prompt=='\0'?0:1);	/* displayable, non-service chars  */
			/* 11 is strlen(" | H - Help")*/
			offset =(statusline_cursor/(chars))*(chars);
			coffset=(*prompt!='\0')+(statusline_cursor%(chars));
		
			sprintf(str, "%s%-*.*s | H - Help",prompt, chars, chars, desc+offset);
			str[coffset]='_';
		}
#endif

		displaydevice->status_line((unsigned char*)str);
		fim_free(str);
	}

	int  CommandConsole::inConsole()const
	{
#ifdef FIM_USE_READLINE
		return ic==1;
#else
		return 0;
#endif
	}

	fim::string CommandConsole::get_variables_reference()const
	{
		/*
		 * returns the reference of registered functions
		 */
		fim::string s;
		/*variables_t::const_iterator vi;
		for( vi=variables.begin();vi!=variables.end();++vi)
		{
			s+=vi->first;
			s+=" : ";
			s+=Var::var_help_db_query(vi->first);
			s+="\n";
		}*/
		s+= Var::get_variables_reference();
		return s;
	}

	fim::string CommandConsole::get_commands_reference()const
	{
		/*
		 * returns the reference of registered commands
		 * TODO : should write better help messages
		 */
		fim::string s;
		for(size_t i=0;i<commands.size();++i)
		{
			s+=(commands[i]->cmd);
			s+=" : ";
			s+=(commands[i])->getHelp();
			s+="\n";
		}
		return s;
	}

}

