/* $LastChangedDate: 2015-03-01 11:49:22 +0100 (Sun, 01 Mar 2015) $ */
/*
 CommandConsole.cpp : Fim console dispatcher

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

/*
 * TODO:
 * 	framebufferdevice -> device
 * 	output methods should be moved in some other, new class
 * */
#include "fim.h"
#include <sys/time.h>
#include <errno.h>

#ifdef FIM_USE_READLINE
#include "readline.h"
#endif /* FIM_USE_READLINE */

#include <sys/ioctl.h>

#include <signal.h>
#include <fstream>

#if HAVE_GET_CURRENT_DIR_NAME
#else /* HAVE_GET_CURRENT_DIR_NAME */
#if _BSD_SOURCE || _XOPEN_SOURCE >= 500
#include <unistd.h>		/* getcwd, as replacement for get_current_dir_name */ /* STDIN_FILENO */
#endif /* _BSD_SOURCE || _XOPEN_SOURCE >= 500 */
#endif /* HAVE_GET_CURRENT_DIR_NAME */

#if FIM_WANT_RAW_KEYS_BINDING
#define FIM_CNS_RAW_KEYS_MESG "; if " FIM_CNS_EX_KSY_STRING " is at least two characters long and begins with 0 (zero), the integer number after the 0 will be treated as a raw keycode to bind the specified " FIM_CNS_EX_KSY_STRING " to. activate the " FIM_VID_VERBOSE_KEYS " variable to discover (display device dependent) raw keys." 
#else /* FIM_WANT_RAW_KEYS_BINDING */
#define FIM_CNS_RAW_KEYS_MESG 
#endif /* FIM_WANT_RAW_KEYS_BINDING */
#define FIM_INVALID_IDX -1

#define FIM_KEY_OFFSET '0'

extern fim_sys_int yyparse();

namespace fim
{

	static  bool nochars(const fim_char_t *s)
	{
		/*
		 * true if the string is null or empty, false otherwise
		 */
		if(s==NULL)
			return true;
		while(*s && isspace(*s))
			++s;
		return *s=='\0'?true:false;
	}

	int CommandConsole::findCommandIdx(fim::string cmd)const
	{
		/*
		 * check whether cmd is a valid internal (registered) Fim command and returns index
		 */
		for(size_t i=0;i<commands_.size();++i) 
			if(commands_[i] && commands_[i]->cmd_==cmd)
				return i;
		return FIM_INVALID_IDX;
	}

	Command* CommandConsole::findCommand(fim::string cmd)const
	{
		/*
		 * check whether cmd is a valid internal (registered) Fim command and returns pointer
		 */
		int idx=findCommandIdx(cmd);

		if(idx!=FIM_INVALID_IDX)
			return commands_[idx];
		return NULL;
	}

	fim::string CommandConsole::bind(fim_key_t c,const fim::string binding)
	{
		/*
		 * binds keycode c to the action specified in binding

		 * note : the binding translation map is used as a necessary
		 * indirection...
		 */
		bindings_t::const_iterator bi=bindings_.find(c);
		fim::string rs("keycode ");
		string ksym = key_syms_[c];
		if( ksym != FIM_CNS_EMPTY_STRING )
			ksym = " (keysym \"" + ksym + "\")";

		bindings_[c]=binding; /* this is the operation; what follows is only debug info */

		rs+=string((int)c);
		rs+=ksym;
		if(bi!=bindings_.end())
			rs+=" successfully reassigned to \"";
		else
			rs+=" successfully assigned to \"";
		rs+=bindings_[c];
		rs+="\"\n";
		return rs;
	}

	fim::string CommandConsole::getBindingsList(void)const
	{
		/*
		 * collates all registered action bindings_ together in a single string
		 * */
		fim::string bindings_expanded;
		bindings_t::const_iterator bi;

		for( bi=bindings_.begin();bi!=bindings_.end();++bi)
		{
			//if(bi->second == FIM_CNS_EMPTY_STRING)continue;//FIX : THIS SHOULD NOT OCCUR
			bindings_expanded+=FIM_FLT_BIND" \"";
			key_syms_t::const_iterator ikbi=key_syms_.find(((*bi).first));
			if(ikbi!=key_syms_.end())
			       	bindings_expanded+=ikbi->second;
			bindings_expanded+="\" \"";
			bindings_expanded+=((*bi).second);
			if( bindings_help_.find((*bi).first) != bindings_help_.end() )
				bindings_expanded+="\" # ",
				bindings_expanded+=string(bindings_help_.find((*bi).first) -> second),
				bindings_expanded+="\n";
			else
				bindings_expanded+="\"\n";
		}
		return bindings_expanded;
	}

	fim::string CommandConsole::unbind(const fim::string& kfstr)
	{
		/*
		 * 	unbinds the action eventually bound to the first key name specified in args..
		 *	IDEAS : multiple unbindings ?
		 *	maybe you should made surjective the binding_keys mapping..
		 */
		fim_key_t key=FIM_SYM_NULL_KEY;
#ifdef FIM_WANT_RAW_KEYS_BINDING
		const fim_char_t*kstr=kfstr.c_str();

		if(strlen(kstr)>=2 && isdigit(kstr[0]) && isdigit(kstr[1]))
		{
			key=atoi(kstr+1);
		}
		else
#endif /* FIM_WANT_RAW_KEYS_BINDING */
		{
			sym_keys_t::const_iterator kbi=sym_keys_.find(kfstr);
			if(kbi!=sym_keys_.end())
				key=sym_keys_[kfstr];
		}
		return unbind(key);
	}

	fim_key_t CommandConsole::find_keycode_for_bound_cmd(fim::string binding)
	{
		/*
		 * looks for a binding to 'cmd' and returns a string description for its bound key 
		 */
		bindings_t::const_iterator bi;
		fim_key_t key=FIM_SYM_NULL_KEY;

		for( bi=bindings_.begin();bi!=bindings_.end();++bi)
		{
			/* FIXME: should move this functionality to an ad-hoc search routine */
			if(bi->second==binding)
			{
				key = bi->first;	
				goto ret;
			}
		}
ret:		return key;
	}

	fim::string CommandConsole::find_key_for_bound_cmd(fim::string binding)
	{
		/*
		 * looks for a binding to 'cmd' and returns a string description for its bound key 
		 */
		fim_key_t key = find_keycode_for_bound_cmd(binding);

		if( key != FIM_SYM_NULL_KEY)
		{
				return key_syms_[key];	
		}

		return FIM_CNS_EMPTY_RESULT;
	}

	fim::string CommandConsole::unbind(fim_key_t c)
	{
		/*
		 * unbinds the action eventually bound to the key combination code c
		 */
		fim::string rs(FIM_FLT_UNBIND" ");
		bindings_t::const_iterator bi=bindings_.find(c);

		if(bi!=bindings_.end())
		{
			bindings_.erase(c);
			rs+=c;
			rs+=": successfully unbound.\n";
			if( bindings_help_.find(c) != bindings_help_.end() )
				bindings_help_.erase(c);
		}
		else
		{
			rs+=c;
			rs+=": there is not such binding.\n";
		}
		return rs;
	}

	fim::string CommandConsole::aliasRecall(fim::string cmd)const
	{
		/*
		 * returns the alias command eventually specified by token cmd
		 *
		 * Note : return aliases_[cmd] would create an entry associated to cmd 
		 * ( and this method would loose the chance to be const ).
		 */
		aliases_t::const_iterator ai=aliases_.find(cmd);

		if(ai!=aliases_.end())
		       	return ai->second.first;
		return FIM_CNS_EMPTY_RESULT;
	}

	fim::string CommandConsole::getAliasesList(void)const
	{
		/*
		 * collates all registered action aliases together in a single string
		 * */
		fim::string aliases_expanded;
		aliases_t::const_iterator ai;

		for( ai=aliases_.begin();ai!=aliases_.end();++ai)
		{
#if 0
			if(ai->second.first == FIM_CNS_EMPTY_STRING)continue;//FIX THIS : THIS SHOULD NOT OCCUR
			aliases_expanded+=FIM_FLT_ALIAS" ";
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
		r+=fim::string(FIM_FLT_ALIAS" \"");
		r+=aname;
		r+=fim::string("\" \"");

		aliases_t::const_iterator ai=aliases_.find(aname);
		if(ai!=aliases_.end())
			r+=ai->second.first;
		r+=fim::string("\"");
		if(ai!=aliases_.end())
		if(ai->second.second!=FIM_CNS_EMPTY_STRING)
		{
			r+=" # ";
			r+=ai->second.second;
		}
		r+=fim::string("\n");
		return r;
	}

	fim::string CommandConsole::fcmd_alias(std::vector<Arg> args)
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
			return get_alias_info(args[0].val_);
		}
		//for(size_t i=1;i<args.size();++i) cmdlist+=args[i].val_;
		if(args.size()>=2)
			cmdlist+=args[1].val_;
		if(args.size()>=3)
			desc   +=args[2].val_;
		if(aliases_[args[0].val_].first!=FIM_CNS_EMPTY_STRING)
		{
			string r;
			aliases_[args[0].val_]=std::pair<fim::string,fim::string>(cmdlist,desc);
			r+=fim::string(FIM_FLT_ALIAS" ");
			r+=args[0].val_;
			r+=fim::string(" successfully replaced.\n");
			return r;
		}
		else
		{
			string r;
			aliases_[args[0].val_].first=cmdlist;
			aliases_[args[0].val_].second=desc;
			r+=fim::string(FIM_FLT_ALIAS" ");
			r+=args[0].val_;
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
#ifndef FIM_WANT_NO_OUTPUT_CONSOLE
#ifndef FIM_KEEP_BROKEN_CONSOLE
	mc_(*this),
#endif /* FIM_KEEP_BROKEN_CONSOLE */
#endif /* FIM_WANT_NO_OUTPUT_CONSOLE */
	fontserver_(),
	show_must_go_on_(1),
	return_code_(0)
	,mangle_tcattr_(false)
	,browser_(*this)
	//,framebufferdevice(_framebufferdevice)
	,cycles_(0)
#ifdef FIM_RECORDING
	,recordMode_(false)			/* we start not recording anything */
	,dont_record_last_action_(false)		/* this variable is only useful in record mode */
#endif /* FIM_RECORDING */
	,fim_stdin_(STDIN_FILENO)
#ifndef FIM_WANT_NO_OUTPUT_CONSOLE
	,dummydisplaydevice_(this->mc_)
#else /* FIM_WANT_NO_OUTPUT_CONSOLE */
	,dummydisplaydevice_()
#endif /* FIM_WANT_NO_OUTPUT_CONSOLE */
	,displaydevice_(NULL)			/* the display device could be NULL ! (FIXME) */
	{
		addCommand(new Command(fim::string(FIM_FLT_ALIAS),fim::string(FIM_FLT_ALIAS " [" FIM_CNS_EX_ID_STRING " [" FIM_CNS_EX_CMDS_STRING " [" FIM_CNS_EX_DSC_STRING "]]]"),this,&CommandConsole::fcmd_foo));
		addCommand(new Command(fim::string(FIM_FLT_ALIGN),fim::string(FIM_CMD_HELP_ALIGN),&browser_,&Browser::fcmd_align));
#ifdef FIM_AUTOCMDS
		addCommand(new Command(fim::string(FIM_FLT_AUTOCMD),fim::string(FIM_FLT_AUTOCMD " " FIM_CNS_EX_EVT_STRING " " FIM_CNS_EX_PAT_STRING " " FIM_CNS_EX_CMDS_STRING " : manipulate auto commands"),this,&CommandConsole::fcmd_autocmd));
		addCommand(new Command(fim::string(FIM_FLT_AUTOCMD_DEL),fim::string(FIM_FLT_AUTOCMD_DEL" : manipulate auto commands. usage: " FIM_FLT_AUTOCMD_DEL " " FIM_CNS_EX_EVT_STRING " " FIM_CNS_EX_PAT_STRING " " FIM_CNS_EX_CMDS_STRING),this,&CommandConsole::fcmd_autocmd_del));	/* this syntax is incompatible with vim ('autocmd!')*/
#endif /* FIM_AUTOCMDS */
		addCommand(new Command(fim::string(FIM_FLT_BASENAME),fim::string(FIM_FLT_BASENAME" {filename} : returns the basename of {filename}"),this,&CommandConsole::fcmd_basename));
		addCommand(new Command(fim::string(FIM_FLT_BIND),fim::string(FIM_FLT_BIND" [" FIM_CNS_EX_KSY_STRING " [" FIM_CNS_EX_CMDS_STRING "]] : bind some keyboard shortcut " FIM_CNS_EX_KSY_STRING " to " FIM_CNS_EX_CMDS_STRING "" FIM_CNS_RAW_KEYS_MESG "; binding is dynamical, so you can rebind keys even during program's execution"),this,&CommandConsole::fcmd_bind));
		addCommand(new Command(fim::string(FIM_FLT_CD),fim::string(FIM_CMD_HELP_CD  ),this,&CommandConsole::fcmd_cd));
#ifndef FIM_WANT_NO_OUTPUT_CONSOLE
		addCommand(new Command(fim::string(FIM_FLT_CLEAR),fim::string(FIM_FLT_CLEAR" : clear the virtual console"),this,&CommandConsole::fcmd_clear));
		// 20110507 we other means for scrolling the console, now
		//addCommand(new Command(fim::string("scroll_console_up"  ),fim::string("scrolls up the virtual console"),this,&CommandConsole::scroll_up));
		//addCommand(new Command(fim::string("scroll_console_down"),fim::string("scrolls down the virtual console"),this,&CommandConsole::scroll_down));
#endif /* FIM_WANT_NO_OUTPUT_CONSOLE */
		addCommand(new Command(fim::string(FIM_FLT_COMMANDS),fim::string(FIM_FLT_COMMANDS " : display the existing commands"),this,&CommandConsole::fcmd_commands_list));

		addCommand(new Command(fim::string(FIM_FLT_DISPLAY),fim::string(FIM_FLT_HELP_DISPLAY),&browser_,&Browser::fcmd_display));
		addCommand(new Command(fim::string(FIM_FLT_REDISPLAY),fim::string(FIM_FLT_REDISPLAY " : re-display the current file contents"),&browser_,&Browser::fcmd_redisplay));
		addCommand(new Command(fim::string(FIM_FLT_DESC),fim::string(FIM_FLT_HELP_DESC),this,&CommandConsole::fcmd_desc));
		addCommand(new Command(fim::string(FIM_FLT_DUMP_KEY_CODES),fim::string(FIM_FLT_DUMP_KEY_CODES " : dump the active key codes (unescaped, for inspection)"),this,&CommandConsole::fcmd_dump_key_codes));
		addCommand(new Command(fim::string(FIM_FLT_ECHO),fim::string(FIM_FLT_ECHO " " FIM_CNS_EX_ARGS_STRING ": print the " FIM_CNS_EX_ARGS_STRING " on console"),this,&CommandConsole::fcmd_echo));
#ifndef FIM_WANT_NOSCRIPTING
		addCommand(new Command(fim::string(FIM_FLT_EXEC),fim::string(FIM_FLT_EXEC " " FIM_CNS_EX_FNS_STRING " : execute script " FIM_CNS_EX_FNS_STRING ""),this,&CommandConsole::fcmd_executeFile));
#endif /* FIM_WANT_NOSCRIPTING */
		addCommand(new Command(fim::string(FIM_FLT_GETENV),fim::string(FIM_FLT_GETENV " " FIM_CNS_EX_ID_STRING " : display the value of the " FIM_CNS_EX_ID_STRING " environment variable"),this,&CommandConsole::fcmd_do_getenv));
		addCommand(new Command(fim::string(FIM_FLT_GOTO),fim::string(FIM_CMD_HELP_GOTO),&browser_,&Browser::fcmd_goto_image));
		addCommand(new Command(fim::string(FIM_FLT_HELP),fim::string(FIM_CMD_HELP_HELP),this,&CommandConsole::fcmd_help));
		addCommand(new Command(fim::string(FIM_FLT_IF),fim::string("if(expression){action;}['else'{action;}]"),this,&CommandConsole::fcmd_foo));// FIXME: need a special "help grammar" command !
		addCommand(new Command(fim::string(FIM_FLT_ELSE),fim::string("if(expression){action;}['else'{action;}]"),this,&CommandConsole::fcmd_foo));// FIXME: need a special "help grammar" command !
#ifdef FIM_RECORDING
		addCommand(new Command(fim::string(FIM_FLT_EVAL),fim::string(FIM_CMD_HELP_EVAL),this,&CommandConsole::fcmd_eval));
#endif /* FIM_RECORDING */
		addCommand(new Command(fim::string(FIM_FLT_INFO),fim::string(FIM_FLT_INFO" : display information about the current file" ),&browser_,&Browser::fcmd_info));
		addCommand(new Command(fim::string(FIM_FLT_LIST),fim::string(FIM_CMD_HELP_LIST),&browser_,&Browser::fcmd_list));
		addCommand(new Command(fim::string(FIM_FLT_LOAD),fim::string(FIM_FLT_LOAD" : load the image, if not yet loaded"),&browser_,&Browser::fcmd_load));
		addCommand(new Command(fim::string(FIM_FLT_RELOAD),fim::string(FIM_FLT_RELOAD" [{arg}] : load the image into memory; if {arg} is present, will force reloading, bypassing the cache"),&browser_,&Browser::fcmd_reload));
		addCommand(new Command(fim::string(FIM_FLT_NEGATE),fim::string(FIM_FLT_NEGATE" : negate the displayed image colors." ),&browser_,&Browser::fcmd_negate));
		addCommand(new Command(fim::string(FIM_FLT_DESATURATE),fim::string(FIM_FLT_DESATURATE" : desaturate the displayed image colors. To get back the original you will have to reload the image." ),&browser_,&Browser::fcmd_desaturate));
		addCommand(new Command(fim::string(FIM_FLT_PAN),fim::string( FIM_FLT_PAN" {'down'|'up'|'left'|'right'|'ne'|'nw'|'se'|'sw'} [{steps}['%']] pan the image {steps} pixels in the desired direction;" " if the '%' specifier is present, {steps} will be treated as a percentage of current screen dimensions;" " if {steps} is not specified, the \"" FIM_VID_STEPS "\" variable will be used;" " if present, the \"" FIM_VID_HSTEPS "\" variable will be considered for horizontal panning;" " if present, the \"" FIM_VID_VSTEPS "\" variable will be considered for vertical panning;" " the variables may be terminated by the \'%\' specifier" " "),&browser_,&Browser::pan));
		addCommand(new Command(fim::string(FIM_FLT_POPEN),fim::string(FIM_FLT_POPEN " " FIM_CNS_EX_SYSC_STRING " : pipe a command, invoking popen(): spawns a shell, invoking " FIM_CNS_EX_SYSC_STRING " and executing as fim commands the output of " FIM_CNS_EX_SYSC_STRING),this,&CommandConsole::fcmd_sys_popen));
#ifdef FIM_PIPE_IMAGE_READ
		addCommand(new Command(fim::string(FIM_FLT_PREAD),fim::string(FIM_FLT_PREAD " " FIM_CNS_EX_ARGS_STRING " : execute " FIM_CNS_EX_ARGS_STRING " as a shell command and read the output as an image file (using " FIM_FLT_POPEN ")"),this,&CommandConsole::fcmd_pread));
#endif /* FIM_PIPE_IMAGE_READ */
		addCommand(new Command(fim::string(FIM_FLT_PWD),fim::string(FIM_CMD_HELP_PWD   ),this,&CommandConsole::fcmd_pwd));
		addCommand(new Command(fim::string(FIM_FLT_PREFETCH),fim::string(FIM_FLT_PREFETCH " : prefetch two nearby image files, for a faster subsequent opening"),&browser_,&Browser::fcmd_prefetch));
		addCommand(new Command(fim::string(FIM_FLT_QUIT),fim::string(FIM_FLT_QUIT " [{number}] : terminate the program; if {number} is specified, use it as the program return status"),this,&CommandConsole::fcmd_quit));
#ifdef FIM_RECORDING
		addCommand(new Command(fim::string(FIM_FLT_RECORDING),fim::string(
FIM_FLT_RECORDING " 'start' : start recording the executed commands; " FIM_FLT_RECORDING " 'stop' : stop  recording the executed commands; " FIM_FLT_RECORDING " 'dump' : dump in the console the record buffer; " FIM_FLT_RECORDING " 'execute' : execute the record buffer; " FIM_FLT_RECORDING " 'repeat_last' : repeat the last performed action; "),this,&CommandConsole::fcmd_recording));
#endif /* FIM_RECORDING */
		addCommand(new Command(fim::string(FIM_FLT_ROTATE),fim::string(FIM_FLT_ROTATE " " FIM_CNS_EX_NUM_STRING ": rotate the image the specified amount of degrees [undocumented]" ),&browser_,&Browser::fcmd_rotate));
		addCommand(new Command(fim::string(FIM_FLT_SCALE),fim::string(FIM_CMD_HELP_SCALE),&browser_,&Browser::fcmd_scale));
		addCommand(new Command(fim::string(FIM_FLT_SCROLLDOWN),fim::string(FIM_FLT_SCROLLDOWN " : scroll down the image, going next if at bottom" ),&browser_,&Browser::fcmd_scrolldown));
		addCommand(new Command(fim::string(FIM_FLT_SCROLLFORWARD),fim::string(FIM_FLT_SCROLLFORWARD " : scroll the image as it were reading it" ),&browser_,&Browser::fcmd_scrollforward));
		addCommand(new Command(fim::string(FIM_FLT_SET),fim::string(FIM_CMD_HELP_SET),this,&CommandConsole::fcmd_set));
		addCommand(new Command(fim::string(FIM_FLT_SET_CONSOLE_MODE),fim::string(FIM_FLT_SET_CONSOLE_MODE " : set console mode"),this,&CommandConsole::fcmd_set_in_console));
		addCommand(new Command(fim::string(FIM_FLT_SET_INTERACTIVE_MODE),fim::string(FIM_FLT_SET_INTERACTIVE_MODE " : set interactive mode"),this,&CommandConsole::fcmd_set_interactive_mode));
		addCommand(new Command(fim::string(FIM_FLT_SLEEP),fim::string(FIM_FLT_SLEEP " [" FIM_CNS_EX_NUM_STRING "=1] : sleep for the specified (default 1) number of seconds"),this,&CommandConsole::fcmd_foo));
		addCommand(new Command(fim::string(FIM_FLT_STATUS),fim::string(FIM_FLT_STATUS " : set the status line to the collation of the given arguments"),this,&CommandConsole::fcmd_status));
		addCommand(new Command(fim::string(FIM_FLT_STDOUT),fim::string(FIM_FLT_STDOUT " " FIM_CNS_EX_ARGS_STRING " : writes to stdout its arguments " FIM_CNS_EX_ARGS_STRING ""),this,&CommandConsole::fcmd__stdout));
#ifndef FIM_NO_SYSTEM
		addCommand(new Command(fim::string(FIM_FLT_SYSTEM),fim::string(FIM_CMD_HELP_SYSTEM),this,&CommandConsole::fcmd_system));
#endif /* FIM_NO_SYSTEM */
		addCommand(new Command(fim::string(FIM_FLT_VARIABLES),fim::string(FIM_FLT_VARIABLES " : display the existing variables"),this,&CommandConsole::fcmd_variables_list));
		addCommand(new Command(fim::string(FIM_FLT_UNALIAS),fim::string(FIM_FLT_UNALIAS " " FIM_CNS_EX_ID_STRING " | '-a' : delete the alias " FIM_CNS_EX_ID_STRING " or all aliases (use '-a', not -a)"),this,&CommandConsole::fcmd_unalias));
		addCommand(new Command(fim::string(FIM_FLT_UNBIND),fim::string(FIM_FLT_UNBIND " " FIM_CNS_EX_KSY_STRING " : unbind the action associated to a specified " FIM_CNS_EX_KSY_STRING FIM_CNS_RAW_KEYS_MESG),this,&CommandConsole::fcmd_unbind));
		addCommand(new Command(fim::string(FIM_FLT_WHILE),fim::string("while(expression){action;}  A conditional cycle construct. May be interrupted by hitting the " FIM_KBD_ESC " or the " FIM_KBD_COLON " key"),this,&CommandConsole::fcmd_foo));/* may introduce a special "help grammar" command */
#ifdef FIM_WINDOWS
		/* this is a stub for the manual generation (actually, the FimWindow object gets built later) */
		addCommand(new Command(fim::string(FIM_FLT_WINDOW),fim::string(FIM_CMD_HELP_WINDOW),this,&CommandConsole::fcmd_foo));
#endif /* FIM_WINDOWS */
//		addCommand(new Command(fim::string("type" ),fim::string("prints out the type of its arguments"),this,&CommandConsole::get_expr_type));
//		addCommand(new Command(fim::string(FIM_FLT_NO_IMAGE),fim::string(FIM_FLT_NO_IMAGE" : displays no image at all (BROKEN)"),&browser_,&Browser::fcmd_no_image));/* FIXME: broken */
		//addCommand(new Command(fim::string("print"   ),fim::string("displays the value of a variable"),this,&CommandConsole::fcmd_foo));
		execDefaultConfiguration();
		fcmd_cd(args_t());
		setVariable(FIM_VID_VERSION,(fim_int)FIM_REVISION_NUMBER);
		setVariable(FIM_VID_STEPS,FIM_CNS_STEPS_DEFAULT);
		setVariable(FIM_VID_TERM, fim_getenv(FIM_CNS_TERM_VAR));
		setVariable(FIM_VID_LOAD_DEFAULT_ETC_FIMRC,(fim_int)1);
		setVariable(FIM_VID_DEFAULT_ETC_FIMRC,FIM_CNS_SYS_RC_FILEPATH);
		setVariable(FIM_VID_PRELOAD_CHECKS,(fim_int)1);
		*prompt_=*(prompt_+1)=FIM_SYM_CHAR_NUL;
	}

	fim_err_t CommandConsole::execDefaultConfiguration(void)
	{
		/* FIXME: #including a file not a clean practice, but it is clean regarding this file. */
		#include "defaultConfiguration.cpp"
		return FIM_ERR_NO_ERROR; 
	}

        bool CommandConsole::is_file(fim::string nf)const
        {
		/*
		 * note:
		 * this function is written a little bit unsafely,
		 * because the file could change between calls.
		 * improvements are possible.
		 * TODO: maybe access() should be used too (it checks file permissions, too)
		 */
                struct stat stat_s;

                if(-1==stat(nf.c_str(),&stat_s))
			goto err;
                if( S_ISDIR(stat_s.st_mode))
			goto err;
                return true;
err:
                /* if the file doesn't exist, return false */
		return false;
        }

	fim_err_t CommandConsole::addCommand(Command *c)
	{
		/*
		 * c is added to the commands list
		 */
		assert(c);
		int idx=findCommandIdx(c->cmd_);

		if(idx!=FIM_INVALID_IDX)
		{
			// here, we replace rather than add
			delete commands_[idx];
			commands_[idx]=c;
		}
		else
			commands_.push_back(c);
		return FIM_ERR_NO_ERROR; 
	}

	fim::string CommandConsole::alias(const fim::string& a, const fim::string& c, const fim::string& d)
	{
		/*
		 * an internal alias method
		 */
		std::vector<fim::Arg> args;
		args.push_back(Arg(a));
		args.push_back(Arg(c));
		args.push_back(Arg(d));
		return fcmd_alias(args);
	}

	fim_char_t * CommandConsole::command_generator (const fim_char_t *text,int state,int mask)const
	{
		/*
		 *	This is the reason why the commands should be kept
		 *	in a list or vector, rather than a map...  :(
		 *
		 *	TODO : INSTEAD OF USING commands_[], make a new vector 
		 *	TODO : the 'mask' mechanism is still a quick hack; it shall be adjusted more properly 
		 *	with completions!
		 *	FIXME
		 *	DANGER : this method allocates memory
		 */
		args_t completions;
		aliases_t::const_iterator ai;
		variables_t::const_iterator vi;
		static size_t list_index=0;
		fim_char_t nschar='\0';

		if(state==0)
			list_index=0;
		while(isdigit(*text))
			text++;	//initial  repeat match
		/*const*/ fim::string cmd(text);
		if(cmd==FIM_CNS_EMPTY_STRING)
			return NULL;
		if(cmd.re_match(FIM_SYM_NAMESPACE_REGEX)==true)
		{
			mask=4,
			nschar=cmd[0],
			cmd=cmd.substr(2,cmd.size());
		}
		if(mask==0 || (mask&1))
		for(size_t i=0;i<commands_.size();++i)
		{
			if(commands_[i]->cmd_.find(cmd)==0)
			completions.push_back(commands_[i]->cmd_);
		}
		if(mask==0 || (mask&2))
		for( ai=aliases_.begin();ai!=aliases_.end();++ai)
		{	
			if((ai->first).find(cmd)==0){
//			cout << ".." << ai->first << ".." << " matches " << cmd << "\n";
			completions.push_back((*ai).first);}
		}
		if(mask==0 || (mask&4))
		{
			if(nschar==FIM_SYM_NULL_NAMESPACE_CHAR || nschar==FIM_SYM_NAMESPACE_GLOBAL_CHAR)
			for( vi=variables_.begin();vi!=variables_.end();++vi)
			{
				if((vi->first).find(cmd)==0)
					completions.push_back((*vi).first);
			}
#if 1
			if(browser_.c_image())
			if(nschar==FIM_SYM_NULL_NAMESPACE_CHAR || nschar==FIM_SYM_NAMESPACE_IMAGE_CHAR)
				browser_.c_image()->find_matching_list(cmd,completions,true);
			if(nschar==FIM_SYM_NULL_NAMESPACE_CHAR || nschar==FIM_SYM_NAMESPACE_BROWSER_CHAR)
				browser_.find_matching_list(cmd,completions,true);
#ifdef FIM_WINDOWS
			if(current_window().current_viewportp())
			if(nschar==FIM_SYM_NULL_NAMESPACE_CHAR || nschar==FIM_SYM_NAMESPACE_VIEWPORT_CHAR)
				current_window().current_viewportp()->find_matching_list(cmd,completions,true);
			if(nschar==FIM_SYM_NULL_NAMESPACE_CHAR || nschar==FIM_SYM_NAMESPACE_WINDOW_CHAR)
				current_window().find_matching_list(cmd,completions,true);
#endif
#endif
		}
#ifndef FIM_COMMAND_AUTOCOMPLETION
		/* THIS DIRECTIVE IS MOTIVATED BY SOME STRANGE BUG!
		 */
		sort(completions.begin(),completions.end());
#endif /* FIM_COMMAND_AUTOCOMPLETION */
		
/*		for(size_t i=list_index;i<completions.size();++i)
				cout << cmd << " matches with " << completions[i].c_str()<<  "\n";*/
		for(size_t i=list_index;i<completions.size();++i)
		{
			//if(completions[i].find(cmd)==0)
			if(1)
			{
				list_index++;
				//readline will free this string..
				return dupstr(completions[i].c_str());// is this malloc free ?
			}
			else
				;//std::cout << cmd << " no matches with " << commands_[i]->cmd_<<  "\n";
		}

/*		for(int i=list_index;i<aliases_keys.size();++i)
		{
			if(!commands_[i])
				continue;
			if(commands_[i]->cmd_.find(cmd)==0)
			{
				list_index++;
				//std::cout << cmd << " matches with " << commands_[i]->cmd_<<  "\n";
				//fim_readline will free this strings..
				return dupstr(commands_[i]->cmd_.c_str());
			}
			else
				;//std::cout << cmd << " no matches with " << commands_[i]->cmd_<<  "\n";
		}*/
		//TO DO : ADD VARIABLE AND ALIAS EXPANSION..
		return NULL;
	}

#define istrncpy(x,y,z) {strncpy(x,y,z-1);x[z-1]='\0';}
#define ferror(s) {/*fatal error*/FIM_FPRINTF(stderr, "%s,%d:%s(please submit this error as a bug!)\n",__FILE__,__LINE__,s);}/* temporarily, for security reason : no exceptions launched */
//#define ferror(s) {/*fatal error*/FIM_FPRINTF(stderr, "%s,%d:%s(please submit this error as a bug!)\n",__FILE__,__LINE__,s);throw FIM_E_TRAGIC;}

	fim::string CommandConsole::getBoundAction(const fim_key_t c)const
	{
		/*
		 * returns the action assigned to key biding c
		 * */
		bindings_t::const_iterator bi=bindings_.find(c);

		if(bi!=bindings_.end()) 
			return bi->second;
		else
		       	return FIM_CNS_EMPTY_RESULT;
	}

	void CommandConsole::executeBinding(const fim_key_t c)
	{
		/*
		 *	Executes the command eventually bound to c.
		 *	Doesn't log anything.
		 *	Just interpretates and executes the binding.
		 *	If the binding is inexistent, ignores silently the error.
		 */
		bindings_t::const_iterator bi=bindings_.find(c);
		fim_err_t status=FIM_ERR_NO_ERROR;
#ifdef FIM_ITERATED_COMMANDS
		static fim_int it_buf=-1; /* FIXME: make this it_buf_ instead. */

		if( c>='0' && c <='9' && (bi==bindings_.end() || bi->second==FIM_CNS_EMPTY_STRING))//a number, not bound
		{
			if(it_buf>0)
			{
				fim_int nit_buf = it_buf;
				it_buf*=10;
				it_buf+=c - FIM_KEY_OFFSET;
				if( it_buf < nit_buf )
					it_buf = nit_buf;
			}
			else
			       	it_buf=c - FIM_KEY_OFFSET;
			goto ret;
		}
		/* FIXME 20110515 this prevents infinite recursion on iterated commands in SDL mode. this is probably a bug in the sdl input handling code and shall be solved */
		if(c==FIM_SYM_NULL_KEY)
			goto ret;
#endif /* FIM_ITERATED_COMMANDS */

		if(bi!=bindings_.end() && bi->second!=FIM_CNS_EMPTY_STRING)
		{
			fim::string cf=current();
			FIM_AUTOCMD_EXEC(FIM_ACM_PREINTERACTIVECOMMAND,cf);
#ifdef FIM_ITERATED_COMMANDS
			if(it_buf>1)
			{
				fim_int mit = getIntVariable(FIM_VID_MAX_ITERATED_COMMANDS);
				fim::string nc;

				if(mit>0 && it_buf > mit)
				{
					cout << "Command repeat parameter of " << it_buf << " exceeds the maximum allowed value of " << mit << ". You can adjust " FIM_VID_MAX_ITERATED_COMMANDS " to raise this limit.\n";
					it_buf = FIM_MIN(mit,it_buf);
				}
				nc=it_buf;
				if(it_buf>1)
					nc+="{"+getBoundAction(c)+"}";
					/* adding ; before } can cause problems as long as ;; is not supported by the parser*/
				else
					nc = getBoundAction(c);
					
				cout << "About to execute " << nc << " .\n";
				status=execute_internal(nc.c_str(),FIM_X_HISTORY);
				it_buf=-1;
			}
			else
#endif /* FIM_ITERATED_COMMANDS */
				status=execute_internal(getBoundAction(c).c_str(),FIM_X_NULL);
			FIM_AUTOCMD_EXEC(FIM_ACM_POSTINTERACTIVECOMMAND,cf);
		}

		if(status)
		{
			std::cerr << "error performing execute()\n";
			//show_must_go_on_=0;	/* we terminate interactive execution */
		}
ret:
		return;
	}

	fim_err_t CommandConsole::execute_internal(const fim_char_t *ss, fim_xflags_t xflags)
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
		fim_bool_t add_history_=(xflags&FIM_X_HISTORY)?true:false;
		/* fim_bool_t suppress_output_=(xflags&FIM_X_QUIET)?true:false; */
		fim_char_t *s=dupstr(ss);//this malloc is free
		int iret=0;
		int r =0;

		if(s==NULL)
		{
			std::cerr << "allocation problem!\n";
			//if(s==NULL){ferror("null command");return;}
			//assert(s);
			//this shouldn't happen
			//this->quit(0);
			return FIM_ERR_GENERIC;
		}
		if(errno)
		{
			//fim_perror("before pipe(fim_pipedesc)");
			//goto ret;
			fim_perror(NULL);// we need to clear errno
		}
		//we open a pipe with the lexer/parser
		r = pipe(fim_pipedesc);
		if(r!=0)
		{
			//strerror(errno);
			std::cerr << "error piping with the command interpreter ( pipe() gave "<< r<< " )\n";
			std::cerr << "the command was:\"" << ss << "\"\n";
			std::cerr << "we had : "<< aliases_.size()<< " aliases_\n";
//			std::exit(-1);
//			ferror("pipe error\n");
//   			cleanup();
			if(errno)
			{
				fim_perror("in pipe(fim_pipedesc)");
				goto ret;
			}
			return FIM_ERR_GENERIC;
		}
		//we write there our script or commands
		r=write(fim_pipedesc[1],s,strlen(s));
		if(errno)
		{
			fim_perror("in write(fim_pipedesc[1])");
			goto ret;
		}
		//we are done!
		if((size_t)r!=strlen(s))
		{
			ferror("write error");
    			cleanup();
			return FIM_ERR_GENERIC;
		} 
		for(fim_char_t *p=s;*p;++p)
			if(*p=='\n')
				*p=' ';
		iret=close(fim_pipedesc[1]);
		if(iret || errno)
		{
			fim_perror("in close(fim_pipedesc[1])");
			goto ret;
		}
		try	{	iret=yyparse();		}
		catch	(FimException e)
		{
			if( e == FIM_E_TRAGIC || e == FIM_E_NO_MEM )
			       	this->quit( FIM_E_NO_MEM );
			else
			       	;	/* ]:-)> */
		}
		close(fim_pipedesc[0]);
		if(iret!=0 || errno!=0)
		{
			if(getIntVariable(FIM_VID_VERBOSE_ERRORS)==1)
			{
				// FIXME; the pipe descriptor is used in a bad way.
				std::cout << "When parsing: " << FIM_MSG_CONSOLE_LONG_LINE   << s << FIM_MSG_CONSOLE_LONG_LINE  << "\n";
				fim_perror("in yyparse()");
			}
			else
				fim_perror(NULL);
			// ignoring yyparse's errno: it may originate from any command!
			//goto ret;
		}

#ifdef FIM_USE_READLINE
		if(add_history_)
			if(nochars(s)==false)
				add_history(s);
#endif /* FIM_USE_READLINE */
ret:
		fim_free(s);

		}
		catch	(FimException e)
		{
			if( e == FIM_E_TRAGIC || true )
			       	this->quit( FIM_E_TRAGIC );
		}
		//we add to history only meaningful commands/aliases.
		return FIM_ERR_NO_ERROR;
	}

        fim::string CommandConsole::execute(fim::string cmd, args_t args)
	{
		/*
		 *	This is the method where the tokenized commands are executed.
		 *	This method executes single commands with arguments.
		 */
		Command *c=NULL;
		/* first determine whether cmd is an alias */
		fim::string ocmd=aliasRecall(cmd);

		if(ocmd!=FIM_CNS_EMPTY_STRING)
		{
			//an alias should be expanded. arguments are appended.
			fim::string ex;
			cmd=ocmd;
			ex=ocmd;
			/*
			 * WARNING : i am not sure this is the best choice
			 */
			fim_sys_int r = pipe(fim_pipedesc),sl;
			if(r!=0)
			{ferror("pipe error\n");exit(-1);}
#ifndef			FIM_ALIASES_WITHOUT_ARGUMENTS
			for(size_t i=0;i<args.size();++i)
			{
				ex+=fim::string(" \"");
				ex+=args[i];
				ex+=fim::string("\""); 
			}
#endif			/* FIM_ALIASES_WITHOUT_ARGUMENTS */
			sl=strlen(ex.c_str());
			r=write(fim_pipedesc[1],ex.c_str(),sl);
			if(r!=sl)
			{ferror("pipe write error");exit(-1);} 
			
			/*
			 * now the yyparse macro YY_INPUT itself handles the closing of the pipe.
			 *
			 * in this way nested commands could not cause harm, because the pipe
			 * is terminated BEFORE executing the command, and reusing fim_pipedesc
			 * is harmless.
			 *
			 * before occurred multiple pipe creations, on the same descriptor buffer,
			 * resulting in a loss of the original descriptors on openings..
			 */
			close(fim_pipedesc[1]);
			fim_perror(NULL);//FIXME: shall use only one yyparse-calling function!
			yyparse();
			fim_perror(NULL);//FIXME: shall use only one yyparse-calling function!
			close(fim_pipedesc[0]);
			goto ok;
		}
		if(cmd==FIM_FLT_USLEEP)
		{
			fim_tus_t useconds=1;
			
			if(args.size()>0)
				useconds=atoi(args[0].c_str());
			usleep(useconds);
			goto ok;
		}
		else
		if(cmd==FIM_FLT_SLEEP)
		{
			fim_ts_t seconds=1;

			if(args.size()>0)
				seconds=atoi(args[0].c_str());
#if 0
			sleep(seconds);
#else
			/* we want an interruptible sleep.  */
			//while(seconds>0 && catchLoopBreakingCommand(seconds--))sleep(1);
			catchLoopBreakingCommand(seconds);
#endif
			goto ok;
		}
		else
		if(cmd==FIM_FLT_ALIAS)
		{
			//assignment of an alias
			std::vector<Arg> aargs;	//Arg args :P

			for(size_t i=0;i<args.size();++i)
			{
				aargs.push_back(Arg(args[i]));
			}
			cout << this->fcmd_alias(aargs) << "\n";
			goto ok;
		}
		else
		{
			c=findCommand(cmd);

#ifdef FIM_COMMAND_AUTOCOMPLETION
			if(getIntVariable(FIM_VID_CMD_EXPANSION)==1)
			if(c==NULL)
			{
				fim_char_t *match = this->command_generator(cmd.c_str(),0,0);

				if(match)
				{
					//cout << "but found :`"<<match<<"...\n";
					c=findCommand(match);
					fim_free(match);
				}
			}
#endif /* FIM_COMMAND_AUTOCOMPLETION */
			if(c==NULL)
			{
				/* FIXME : should stringify here and elsewhere
				 * see also http://gcc.gnu.org/onlinedocs/cpp/index.html
				 */
				cout << "sorry, no such command :`"<<cmd.c_str()<<"'\n";
				goto ok;
			}
			else
			{
				if(getIntVariable(FIM_VID_DBG_COMMANDS)!=0)
					std::cout << "in " << cmd << ":\n";
				
				cout << c->execute(args);
				goto ok;
			}
		}
		return "If you see this string, please report it to the program maintainer :P\n";
ok:
		return FIM_CNS_EMPTY_RESULT;
	}

	fim_int CommandConsole::catchLoopBreakingCommand(fim_ts_t seconds)
	{
		/*	
		 *	This method is invoked during non interactive loops to
		 *	provide a method for interactive loop breaking.
		 *
		 *	The provided mechanism allows the user to press any key
		 *	during the loop, and the loop will continue its execution,
		 *	unless the pressed key is not exitBinding_.
		 *
		 *	If not, and the key is bound to some action; this action
		 *	is executed.
		 *
		 *	NOTE: this could nest while loops !
		 *
		 *	returns 0 if no command was received.
		 */
		fim_key_t c;

		if ( exitBinding_ == 0 )
		       	goto err;	/* any key triggers an exit */

		c = displaydevice_->catchInteractiveCommand(seconds);
	//	while((c = displaydevice_.catchInteractiveCommand(seconds))!=-1)
		while(c!=-1)
		{
			/* while characters read */
			//if( c == -1 ) return 0;	/* no chars read */
			sym_keys_t::const_iterator ki;

//			if(c==sym_keys_[FIM_KBD_ESC]) return 1; 		/* the user hit the exitBinding_ key */
//			if(c==sym_keys_[FIM_KBD_COLON]) return 1; 		/* the user hit the exitBinding_ key */
//			// 20110601 need some string variables with these two keys (see while() interruption documentation) 
			if((ki=sym_keys_.find(FIM_KBD_ESC))!=sym_keys_.end() && c==ki->second)
				goto err;
			if((ki=sym_keys_.find(FIM_KBD_COLON))!=sym_keys_.end() && c==ki->second)
				goto err;
			if( c != exitBinding_ )  /* characters read */
			{
				/*
				 * we give the user chance to issue commands
				 * and some times to realize this.
				 *
				 * is it a desirable behaviour ?
				 */
				executeBinding(c);
				if(!show_must_go_on_)
					goto err;
				c = displaydevice_->catchInteractiveCommand(1);
//				return 0;/* could be a command key */
			}
			if(c==exitBinding_)
			       	goto err; 		/* the user hit the exitBinding_ key */
		}
		return 0; 		/* no chars read  */
err:
		return 1;
	}
		

#ifdef	FIM_USE_GPM
	static int gh(Gpm_Event *event, void *clientdata)
	{
		std::cout << "GPM event captured.\n";
		exit(0);
		//quit();
		return 'n';
		return 0;
	}
#endif	/* FIM_USE_GPM */

	fim_perr_t CommandConsole::executionCycle(void)
	{
		/*
		 * the cycle with fetches the instruction stream.
		 * */
#ifdef	FIM_USE_GPM
		Gpm_PushRoi(0,0,1023,768,GPM_DOWN|GPM_UP|GPM_DRAG|GPM_ENTER|GPM_LEAVE,gh,NULL);
#endif	/* FIM_USE_GPM */
		fim::string initial = browser_.current();
#ifdef FIM_AUTOCMDS

		FIM_AUTOCMD_EXEC(FIM_ACM_PREEXECUTIONCYCLE,initial);
		FIM_AUTOCMD_EXEC(FIM_ACM_PREEXECUTIONCYCLEARGS,initial);
#endif /* FIM_AUTOCMDS */
		*prompt_=FIM_SYM_PROMPT_NUL;

	 	while(show_must_go_on_)
		{
			cycles_++;
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
			if(ic_==1)
			{
				ic_=1;
				fim_char_t *rl = fim_readline(FIM_KBD_COLON);
				*prompt_=FIM_SYM_PROMPT_CHAR;
				if(rl==NULL)
				{
					goto rlnull;
				}
				else if(*rl!=FIM_SYM_CHAR_NUL)
				{
					/*
					 * This code gets executed when the user is about to exit console mode, 
					 * having she pressed the 'Enter' key and expecting result.
					 * */
					fim::string cf=current();
					FIM_AUTOCMD_EXEC(FIM_ACM_PREINTERACTIVECOMMAND,cf);
#ifdef FIM_RECORDING
					if(recordMode_)
						record_action(fim::string(rl));
#endif /* FIM_RECORDING */
					//ic_=0; // we 'exit' from the console for a while (WHY ? THIS CAUSES PRINTING PROBLEMS)
					execute_internal(rl,FIM_X_HISTORY);	//execution of the command line with history
					ic_=(ic_==-1)?0:1; //a command could change the mode !
//					this->setVariable(FIM_VID_DISPLAY_CONSOLE,1);	//!!
//					execute_internal("redisplay;",FIM_X_NULL);	//execution of the command line with history
					FIM_AUTOCMD_EXEC(FIM_ACM_POSTINTERACTIVECOMMAND,cf);
#ifdef FIM_RECORDING
					memorize_last(rl);
#endif /* FIM_RECORDING */
					//p.s.:note that current() returns not necessarily the same in 
					//the two FIM_AUTOCMD_EXEC() calls..
				}
				/*  else : *rl==FIM_CNS_EMPTY_STRING : doesn't happen :) */

				if(rl && *rl==FIM_SYM_CHAR_NUL)
				{
					/* happens when no command is issued and Enter key is pressed */
					ic_=0;
					*(prompt_)=FIM_SYM_PROMPT_NUL;
					set_status_bar(FIM_CNS_EMPTY_STRING,NULL);
				}
				if(rl)
					fim_free(rl);
			}
			else
#endif /* FIM_USE_READLINE */
			{
#ifdef	FIM_USE_GPM
				Gpm_Event *EVENT = NULL;
#endif	/* FIM_USE_GPM */
				fim_key_t c;
				fim_sys_int r;
				fim_char_t buf[FIM_VERBOSE_KEYS_BUFSIZE];

				*prompt_ = FIM_SYM_PROMPT_NUL;
//				fim_sys_int c=getchar();
//				fim_sys_int c=fgetc(stdin);
				/*
				 *	problems :
				 *	 I can't read Control key and 
				 *	some upper case key together.
				 *	 I am not quite sure about portability..
				 *	... maybe a sample program which photograph
				 *	the keyboard is needed!.
				 */
				c=0;
				r=displaydevice_->get_input(&c);
#ifdef	FIM_USE_GPM
				if(Gpm_GetEvent(EVENT)==1)
					quit();
				else
					cout << "...";
#endif	/* FIM_USE_GPM */
				if(r>0)
				{
					if(getIntVariable(FIM_VID_VERBOSE_KEYS)==1)
					{
						/*
						 * <0x20 ? print ^ 0x40+..
						 * */
						sprintf(buf,"got : 0x%x (%d)\n",c,c);
						cout << buf ;
					}
#ifndef FIM_USE_READLINE
					if(c==(fim_key_t)getIntVariable(FIM_VID_CONSOLE_KEY) || 
						c == FIM_SYM_FW_SEARCH_KEY || c == FIM_SYM_BW_SEARCH_KEY )
						set_status_bar("compiled with no readline support!\n",NULL);
#else /* FIM_USE_READLINE */
					if(c==(fim_key_t)getIntVariable(FIM_VID_CONSOLE_KEY))
					{
						ic_=1;
						*prompt_ = FIM_SYM_PROMPT_CHAR;
					}
					else
					if( c == FIM_SYM_FW_SEARCH_KEY || c == FIM_SYM_BW_SEARCH_KEY )
					{
						/* a hack to handle vim-style regexp searches */
						fim_sys_int tmp=rl_filename_completion_desired;
						rl_hook_func_t *osh=rl_startup_hook;
						rl_startup_hook=rl::fim_search_rl_startup_hook;
						fim_char_t *rl = NULL;
						const fim_char_t *rlp = FIM_CNS_SLASH_STRING;
						*prompt_=FIM_SYM_PROMPT_SLASH;
						if(c == FIM_SYM_BW_SEARCH_KEY)
							rlp=FIM_CNS_QU_MA_STRING,
							*prompt_='?';
						rl=fim_readline(rlp); // !!
						ic_=1;
						rl_inhibit_completion=1;
						rl_startup_hook=osh;
						// no readline ? no interactive searches !
						*prompt_=FIM_SYM_PROMPT_NUL;
						rl_inhibit_completion=tmp;
						ic_=0;
						if(rl==NULL)
						{
							goto rlnull;
						}
						/* 
						 * if using "" instead string("")
						 * warning: comparison with string literal results in unspecified behaviour */
						else if(rl!=string(FIM_CNS_EMPTY_STRING))
						{
							args_t args;
							std::string rls("");

							if(c == FIM_SYM_BW_SEARCH_KEY)
								rls+="-";
							rls+="/";
							rls+=rl;
							rls+="/";
							args.push_back(rls);
							execute(FIM_FLT_GOTO,args);
						}
					}
					else
#endif /* FIM_USE_READLINE */
					{

						this->executeBinding(c);
#ifdef FIM_RECORDING
						if(recordMode_)
						       	record_action(getBoundAction(c));
						memorize_last(getBoundAction(c));
#endif /* FIM_RECORDING */
					}
				}
				else
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
			continue;
#ifdef FIM_USE_READLINE
rlnull:
			{
				ic_=0;
				*prompt_=FIM_SYM_CHAR_NUL;
				*prompt_=*(prompt_+1)=FIM_SYM_CHAR_NUL;
				const fim_char_t * msg = " Warning: readline failed ! Probably no stdin is available, right ? Unfortunately fim is not yet ready for this case.\n";
				cout << msg;
				set_status_bar(msg,NULL);
			}
			/* this->quit(); */
#endif /* FIM_USE_READLINE */
		}
		FIM_AUTOCMD_EXEC(FIM_ACM_POSTEXECUTIONCYCLE,initial);
		return quit(return_code_);
	}

	void CommandConsole::exit(fim_perr_t i)const
	{
		/*
		 *	This method will exit the program as a whole.
		 *      If various object destructors are set to destroy device
		 *	contexts, it should do no harm to the console.
		 *      (it will call statically declared object's destructors )
		 */
		std::exit(i);
	}

	fim_perr_t CommandConsole::quit(fim_perr_t i)
	{
		/*
		 * the method to be called to exit from the program safely.
		 * it is used mainly for safe and exit after severe errors.
		 * TODO : get rid of it.
		 */
    		cleanup();
		return i;/* is should be used in return */
	}

#if FIM_WANT_FILENAME_MARK_AND_DUMP
	fim::string CommandConsole::marked_files_list(void)const
	{
		fim::string res;
		for(std::set<fim::string>::iterator i=marked_files_.begin();i!=marked_files_.end();++i)
			res += *i,
			res += "\n";
		return res;
	}
#endif /* FIM_WANT_FILENAME_MARK_AND_DUMP */

	CommandConsole::~CommandConsole(void)
	{
		/*
		 * NOTE:
		 * as long as this class is a singleton, we couldn't care less about memory freeing :)
		 */
		fim::string sof=getStringVariable(FIM_VID_SCRIPTOUT_FILE);

#if FIM_WANT_FILENAME_MARK_AND_DUMP
		if(!marked_files_.empty())
		{
			std::cerr << "The following files were marked by the user :\n";
			std::cerr << "\n";
			std::cout << marked_files_list();
		}
#endif /* FIM_WANT_FILENAME_MARK_AND_DUMP */
		if(sof!=FIM_CNS_EMPTY_STRING)
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
		for(size_t i=0;i<commands_.size();++i)
			if(commands_[i])
				delete commands_[i];

	#ifdef FIM_WITH_AALIB
		if(aad_ && !displaydevice_)
			delete aad_;	/* aad_ is an alias */
		else
	#endif /* FIM_WITH_AALIB */
		{
			if(displaydevice_ && displaydevice_ != &dummydisplaydevice_)
				delete displaydevice_;
		}

#ifdef FIM_WINDOWS
		if(window_)
			delete window_;
#else /* FIM_WINDOWS */
		if(viewport_)
		       	delete viewport_;
#endif /* FIM_WINDOWS */
	}

	fim::string CommandConsole::readStdFileDescriptor(FILE* fd, int*rp)
	{
		/*
		 * TODO : catch exceptions/interruptions
		 */

		fim_sys_int r;
		fim_char_t buf[FIM_STREAM_BUFSIZE];	// NOTE: a larger buffer would be ok (e.g.: user configurable)...
		fim::string cmds;

		if(fd==NULL)
		{
			/* return FIM_ERR_GENERIC; */
		       	cmds = FIM_ERR_GENERIC;
		       	goto ret;
	       	}

		while((r=fread(buf,1,sizeof(buf)-1,fd))>0)
		{
			buf[r]='\0';
			cmds+=buf;
	/*		if(displaydevice_->catchInteractiveCommand(0)!=-1) goto ret; */
		}
		if(rp)
			*rp=r;
ret:
		return cmds;
	}
	
	fim_err_t CommandConsole::executeStdFileDescriptor(FILE* fd)
	{
		/*
		 * FIX ME  HORRIBLE : FILE DESCRIPTOR USED AS A FILE HANDLE..
		 *
		 * TODO : catch exceptions
		 */
		fim_sys_int r;
		fim_err_t errv = FIM_ERR_NO_ERROR;
		/* fim_char_t buf[FIM_STREAM_BUFSIZE]; */
		fim::string cmds = CommandConsole::readStdFileDescriptor(fd,&r);

		if(r==-1)
		{
			errv = FIM_ERR_GENERIC;
			goto ret;
		}

		execute_internal(cmds.c_str(),FIM_X_QUIET);
ret:
		return errv;
	}

	fim_err_t CommandConsole::executeFile(const fim_char_t *s)
	{
		/*
		 * executes a file denoted by filename
		 * */
		execute_internal(slurp_file(s).c_str(),FIM_X_QUIET);
		return FIM_ERR_NO_ERROR;
	}

	fim_var_t CommandConsole::getVariableType(const fim::string &varname)const
	{
		/*
		 * returns the [internal] type of a variable
		 * FIXME : eradicate this cancer
		 * */
		variables_t::const_iterator vi=variables_.find(varname);

		if(vi!=variables_.end())
			return vi->second.getType();
		else
		       	return FIM_ERR_NO_ERROR;
	}

	bool CommandConsole::isVariable(const fim::string &varname)const
	{
		const fim_char_t* s=NULL;

		s = getStringVariable(varname).c_str();
		return (s && *s);
	}

	fim_err_t CommandConsole::printVariable(const fim::string &varname)const
	{	
		/*
		 * a variable is taken and converted to a string and printed
		 *
		 * FIXME: should escape (possibly optionally) ?
		 * */
		fim::cout<<getStringVariable(varname);
		return FIM_ERR_NO_ERROR;
	}

	fim_bool_t CommandConsole::drawOutput(const fim_char_t *s)const
	{
		/*
		 * whether the console should draw or not itself upon the arrival of textual output
		 * */
		//std::cout << s << " : " << (this->inConsole() )<< ( (s&&*s) ) << "\n";
		fim_bool_t sd=(	(	this->inConsole()	/* in the command line */
				&& (s&&*s) 		/* actually some text to add */
			) 
			|| this->getIntVariable(FIM_VID_DISPLAY_CONSOLE)	/* or user requested for showing console */
			);
		return sd;
	}

	fim::string CommandConsole::get_aliases_list(void)const
	{
		/*
		 * returns the list of set action aliases
		 */
		fim::string aliases_list;
		aliases_t::const_iterator ai;

		for( ai=aliases_.begin();ai!=aliases_.end();++ai)
		{	
			aliases_list+=((*ai).first);
			aliases_list+=" ";
		}
		return aliases_list;
	}

	fim::string CommandConsole::get_commands_list(void)const
	{
		/*
		 * returns the list of registered commands
		 */
		fim::string commands_list;

		for(size_t i=0;i<commands_.size();++i)
		{
			if(i)
				commands_list+=" ";
			commands_list+=(commands_[i]->cmd_);
		}
		return commands_list;
	}

	fim::string CommandConsole::get_variables_list(void)const
	{
		/*
		 * returns the list of set variables
		 */
		fim::string acl,sep=" ";
		variables_t::const_iterator vi;

		for( vi=variables_.begin();vi!=variables_.end();++vi)
		{
			acl+=((*vi).first);
			acl+=" ";
		}

#ifdef FIM_NAMESPACES
		acl+=browser_.get_variables_list();
		acl+=sep;
		if(browser_.c_image())
		{
			acl+=browser_.c_image()->get_variables_list();
			acl+=sep;
		}
#endif /* FIM_NAMESPACES */
#ifdef FIM_WINDOWS
		if(window_)
		{
			acl+=window_->get_variables_list();
			acl+=sep;
		}
		if(current_viewport())
		{
			acl+=current_viewport()->get_variables_list();
			acl+=sep;
		}
#endif /* FIM_WINDOWS */
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
//		std::map<fim::string,std::map<fim::string,fim::string> >  autocmds_;
		autocmds_t::const_iterator ai;

		if(event==FIM_CNS_EMPTY_STRING && pattern==FIM_CNS_EMPTY_STRING)
		//for each autocommand event registered
		for( ai=autocmds_.begin();ai!=autocmds_.end();++ai )
		//for each file pattern registered, display the list..
		for(	autocmds_p_t::const_iterator api=((*ai)).second.begin();
				api!=((*ai)).second.end();++api )
		//.. display the list of autocommands...
		for(	args_t::const_iterator aui=((*api)).second.begin();
				aui!=((*api)).second.end();++aui )
		{
			acl+=FIM_FLT_AUTOCMD" \""; 
			acl+=(*ai).first; 
			acl+="\" \""; 
			acl+=(*api).first; 
			acl+="\" \""; 
			acl+=(*aui); 
			acl+="\"\n"; 
		}
		else
		if(pattern==FIM_CNS_EMPTY_STRING)
		{
			autocmds_t::const_iterator ai=autocmds_.find(event);
			//for each autocommand event registered
			//for each file pattern registered, display the list..
			if(ai!=autocmds_.end())
			for(	autocmds_p_t::const_iterator api=(*ai).second.begin();
					api!=(*ai).second.end();++api )
			//.. display the list of autocommands...
			for(	args_t::const_iterator aui=((*api)).second.begin();
					aui!=((*api)).second.end();++aui )
			{
				acl+=FIM_FLT_AUTOCMD" \""; 
				acl+=(*ai).first; 
				acl+="\" \""; 
				acl+=(*api).first; 
				acl+="\" \""; 
				acl+=(*aui); 
				acl+="\"\n"; 
			}
		}
		else
		{
			autocmds_t::const_iterator ai=autocmds_.find(event);

			//for each autocommand event registered
			//for each file pattern registered, display the list..
			if(ai!=autocmds_.end())
			{
				autocmds_p_t::const_iterator api=(*ai).second.find(pattern);

				//.. display the list of autocommands...
				if(api!=(*ai).second.end())
				{
					for(	args_t::const_iterator aui=((*api)).second.begin();
					aui!=((*api)).second.end();++aui )
					{
						acl+=FIM_FLT_AUTOCMD" \""; 
						acl+=(*ai).first; 
						acl+="\" \""; 
						acl+=(*api).first; 
						acl+="\" \""; 
						acl+=(*aui); 
						acl+="\"\n"; 
					}
				}
			}
		}
		
		if(acl==FIM_CNS_EMPTY_STRING)
			acl="no autocommands loaded\n";
		return acl;
	}

	fim::string CommandConsole::autocmd_del(const fim::string event, const fim::string pattern, const fim::string action)
	{
		/*
		 */
		autocmds_t::iterator ai;
		size_t n = 0;

		if(event==FIM_CNS_EMPTY_STRING && pattern==FIM_CNS_EMPTY_STRING  && action == FIM_CNS_EMPTY_STRING  )
		{
			/* deletion of all autocmd's */
			n = autocmds_.size();
			for( ai=autocmds_.begin();ai!=autocmds_.end();++ai )
				autocmds_.erase(ai);
		}
		else
		if(action==FIM_CNS_EMPTY_STRING   && pattern==FIM_CNS_EMPTY_STRING    )
		{
			/* deletion of all autocmd's for given event */
			ai=autocmds_.find(event);
			if(ai==autocmds_.end())
				return FIM_CNS_EMPTY_RESULT;
			n = (*ai).second.size();
			for(	autocmds_p_t::iterator api=((*ai)).second.begin();
				api!=((*ai)).second.end();++api )
				(*ai).second.erase(api);
		}
		else
		if(action==FIM_CNS_EMPTY_STRING)
		{
			/* deletion of all autocmd's for given event and pattern */
			ai=autocmds_.find(event);
			if(ai==autocmds_.end())
				return FIM_CNS_EMPTY_RESULT;
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
		if(cmd==FIM_CNS_EMPTY_STRING)
		{
			cout << "can't add empty autocommand\n";
			goto ok;
		}
		for(size_t i=0;i<autocmds_[event][pat].size();++i)
		if((autocmds_[event][pat][i])==cmd)
		{
			cout << "autocommand "<<cmd<<" already specified for event \""<<event<<"\" and pattern \""<<pat<<"\"\n";
			goto ok;
		}
		autocmds_[event][pat].push_back(cmd);
ok:
		return FIM_CNS_EMPTY_RESULT;
	}

	fim::string CommandConsole::pre_autocmd_add(const fim::string &cmd)
	{
		/*
		 * this autocommand will take argument related autocommands
		 */
	    	//return autocmd_add(FIM_ACM_PREEXECUTIONCYCLEARGS,"",cmd);
	    	//return autocmd_add(FIM_ACM_POSTFIMRC,"",cmd);
	    	return autocmd_add(FIM_ACM_POSTHFIMRC,"",cmd);
	}

	fim::string CommandConsole::pre_autocmd_exec(void)
	{
		/*
		 */
	    	//return FIM_AUTOCMD_EXEC(FIM_ACM_POSTFIMRC,"");
	    	return FIM_AUTOCMD_EXEC(FIM_ACM_POSTHFIMRC,"");

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
			for( api=autocmds_[event].begin();api!=autocmds_[event].end();++api )
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
		return FIM_CNS_EMPTY_RESULT;
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
			
		if(regexp_match(fname.c_str(),pat.c_str(),getIntVariable(FIM_VID_IGNORECASE)))
		{
			for (size_t i=0;i<autocmds_[event][pat].size();++i)
			{
				autocmds_frame_t frame(autocmds_loop_frame_t(event,fname),(autocmds_[event][pat][i]).c_str());
//				cout << "should exec '"<<event<<"'->'"<<autocmds_[event][pat][i]<<"'\n";
				autocmds_stack.push_back(frame);
				execute_internal((autocmds_[event][pat][i]).c_str(),FIM_X_QUIET);
				autocmds_stack.pop_back();
			}
		}
		return FIM_CNS_EMPTY_RESULT;
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
	
	void CommandConsole::autocmd_trace_stack(void)
	{
		/*
		 * this is mainly a debug function: it will write to stdout
		 * the current autocommands stack trace.
		 * set the FIM_VID_DBG_AUTOCMD_TRACE_STACK variable
		 */
		size_t indent=0,i;

		if(autocmds_stack.end()==autocmds_stack.begin())
			std::cout << "<>\n";
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
	
	fim_bool_t CommandConsole::autocmd_in_stack(const autocmds_loop_frame_t& frame)const
	{
		/*
		 * this function prevents a second autocommand triggered against 
		 * the same file to execute
		 */
		//return  autocmds_loop_stack.find(frame)!=autocmds_loop_stack.end();
		return  find(autocmds_loop_stack.begin(),autocmds_loop_stack.end(),frame)!=autocmds_loop_stack.end();
	}
#endif /* FIM_AUTOCMDS */
	
	bool CommandConsole::regexp_match(const fim_char_t*s, const fim_char_t*r, int rsic)const
	{
		/*
		 *	given a string s, and a Posix regular expression r, this
		 *	method returns true if there is match. false otherwise.
		 */
		regex_t regex;		//should be static!!!
		const fim_size_t nmatch=1;	// we are satisfied with the first match, aren't we ?
		regmatch_t pmatch[nmatch];

		/*
		 * we allow for the default match, in case of null regexp
		 */
		if(!r || !strlen(r))
			return true;

		/* fixup code for a mysterious bug
		 */
		if(*r=='*')
			return false;

		//if(regcomp(&regex,"^ \\+$", 0 | REG_EXTENDED | REG_ICASE )==-1)
		if(regcomp(&regex,r, 0 | REG_EXTENDED | (rsic==0?0:REG_ICASE) )!=0)
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
		}
		regfree(&regex);
		return false;
		//return true;
	}

	bool CommandConsole::redisplay(void)
	{
		/*
		 * quick and dirty display function
		 */
#ifdef FIM_WINDOWS
		bool needed_redisplay=false;

		try
		{
			if(window_)
				needed_redisplay=window_->recursive_redisplay();
		}
		catch	(FimException e)
		{
			// well, we should to something : FIXME
			std::cerr << "fatal error" << __FILE__ << ":" << __LINE__ << "\n";
		}
		return needed_redisplay;
#else /* FIM_WINDOWS */
		//browser_.redisplay();
		if(cc.viewport_)
			return cc.viewport_->redisplay();
		return true;
#endif /* FIM_WINDOWS */
	}

	bool CommandConsole::display(void)
	{
		/*
		 * quick and dirty display function
		 */
#ifdef FIM_WINDOWS
		bool needed_redisplay=false;

		try
		{
			if(window_ )
				needed_redisplay=window_->recursive_display();
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
#else /* FIM_WINDOWS */
		browser_.redisplay();
		return true;
#endif /* FIM_WINDOWS */
	}

#ifdef FIM_RECORDING
	void CommandConsole::record_action(const fim::string &cmd)
	{
		/*	(action,millisleeps waitingbefore) is registered	*/
		/*
		 * PROBLEM:
		  clock_gettime() clock() times() getrusage() time() asctime() ctime(void)
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
		static time_t pt=0;
		fim_tms_t t,d,err;//t,pt in ms; d in us
	        struct timeval tv;

		if(cmd==FIM_CNS_EMPTY_STRING)
		{
			pt=0;
			return;
		}
	        if(!pt)
		{
			err=gettimeofday(&tv, NULL);
			pt=tv.tv_usec/1000+tv.tv_sec*1000;
		}
	        err=gettimeofday(&tv, NULL);t=tv.tv_usec/1000+tv.tv_sec*1000;
		if(err != 0)
		{
			/* TODO: error handling ... */
		}
		d=(t-pt)*1000;
		pt=t;
		recorded_actions_.push_back(recorded_action_t(sanitize_action(cmd),d));
	}
#endif /* FIM_RECORDING */

#if FIM_WANT_FILENAME_MARK_AND_DUMP
	void CommandConsole::markCurrentFile(void)
	{
		/*
		 * the current file will be added to the list of filenames
		 * which will be printed upon the program termination.
		 * */
		if(browser_.current()!=FIM_STDIN_IMAGE_NAME)
		{
			marked_files_t::iterator mfi=marked_files_.find(browser_.current());
			if(mfi==marked_files_.end())
			{
				marked_files_.insert(browser_.current());
				cout<<"Marked file \""<<browser_.current()<<"\"\n";
			}
			else
				cout<<"File \""<<browser_.current()<<"\" was already marked\n";
		}
	}

	void CommandConsole::unmarkCurrentFile(void)
	{
		/*
		 * the current file will be added to the list of filenames
		 * which will be printed upon the program termination.
		 * */
		if(browser_.current()!=FIM_STDIN_IMAGE_NAME)
		{
			marked_files_t::iterator mfi=marked_files_.find(browser_.current());
			if(mfi!=marked_files_.end())
			{
				marked_files_.erase(mfi);
				cout<<"Unmarked file \""<<browser_.current()<<"\"\n";
			}
			else
				cout<<"File \""<<browser_.current()<<"\" was not marked\n";
		}
	}
#endif /* FIM_WANT_FILENAME_MARK_AND_DUMP */

	void CommandConsole::printHelpMessage(const fim_char_t *pn)const
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
		if(dont_record_last_action_==false)
		{
			last_action_=cmd;
		}
		dont_record_last_action_=false;	//from now on we can memorize again
		return FIM_CNS_EMPTY_RESULT;
	}

	fim::string CommandConsole::sanitize_action(const fim::string &cmd)const
	{
		/*
		 * the purpose of this method is to sanitize the action token
		 * in order to gain a dumpable and self standing action
		 */
		if(cmd.c_str()[strlen(cmd.c_str())-1]!=FIM_SYM_SEMICOLON)
			return cmd+fim::string(FIM_SYM_SEMICOLON_STRING);
		return cmd;
	}
#endif /* FIM_RECORDING */
	void CommandConsole::appendPostInitCommand(const fim_char_t* c)
	{
		/*
		 * the supplied command is applied right before a normal execution of Fim
		 * but after the configuration file loading
		 * */
		postInitCommand_+=c;
	}

	void CommandConsole::appendPreConfigCommand(const fim_char_t* c)
	{
		/*
		 * */
		preConfigCommand_+=c;
	}

	void CommandConsole::appendPostExecutionCommand(const fim::string &c)
	{
		/*
		 * the supplied command is applied right before a normal termination of Fim
		 * */
		postExecutionCommand_+=c;
	}
	
	bool CommandConsole::appendedPostInitCommand(void)const
	{
		/*
		 * whether some command will be executed right after initialization
		 * */
		return postInitCommand_!=fim::string("");
	}

	bool CommandConsole::appendedPreConfigCommand(void)const
	{
		/*
		 * */
		return preConfigCommand_!=fim::string("");
	}

	Viewport* CommandConsole::current_viewport(void)const
	{
		/*
		 * returns a reference to the current viewport.
		 *
		 * TODO : did we catch all exceptions ?
		 * */
#ifdef FIM_WINDOWS
		return current_window().current_viewportp();
#else /* FIM_WINDOWS */
		return viewport_;
#endif /* FIM_WINDOWS */
	}

#ifdef FIM_WINDOWS
	const FimWindow & CommandConsole::current_window(void)const
	{
		/*
		 * returns a reference to the current window_.
		 * there should be one :)
		 * if not, consider the situation TRAGIC
		 * */
		if(!window_)
		{
//			temporarily, for security reasons
//			throw FIM_E_TRAGIC;
		}
		return *window_;
	}
#endif /* FIM_WINDOWS */

	bool CommandConsole::push(const fim::string nf, fim_flags_t pf)
	{
		/*
		 * returns true if push was ok
		 * */
		return browser_.push(nf,pf);
	}

#ifndef FIM_WANT_NOSCRIPTING
	bool CommandConsole::push_scriptfile(const fim::string ns)
	{
		/*
		 * pushes a script up in the pre-execution scriptfile list
		 * */
	    	scripts_.push_back(ns);
		return true; /* for now a fare return code */
	}
	bool CommandConsole::with_scriptfile(void)const
	{
		return scripts_.size() !=0 ;
	}
#endif /* FIM_WANT_NOSCRIPTING */

	/*
	 *	Setting the terminal in raw mode means:
	 *	 - setting the line discipline
	 *	 - setting the read rate
	 *	 - disabling the echo
	 */
	void CommandConsole::tty_raw(void)
	{
		struct termios tattr;
		//we set the terminal in raw mode.
		    
		fcntl(0,F_GETFL,saved_fl_);
		tcgetattr (0, &saved_attributes_);
		    
		//fcntl(0,F_SETFL,O_BLOCK);
		memcpy(&tattr,&saved_attributes_,sizeof(struct termios));
		tattr.c_lflag &= ~(ICANON|ECHO);
		tattr.c_cc[VMIN] = 1;
		tattr.c_cc[VTIME] = 0;
		tcsetattr (0, TCSAFLUSH, &tattr);
	}
	
	void CommandConsole::tty_restore(void)
	{	
		//POSIX.1 compliant:
		//"a SIGIO signal is sent whenever input or output becomes possible on that file descriptor"
		fcntl(0,F_SETFL,saved_fl_);
		//the Terminal Console State Attributes will be set right NOW
		tcsetattr (0, TCSANOW, &saved_attributes_);
	}

	fim_err_t CommandConsole::load_or_save_history(bool load_or_save)
	{
#if FIM_WANT_HISTORY
#ifndef FIM_NOHISTORY
  #ifndef FIM_WANT_NOSCRIPTING
    #ifdef FIM_USE_READLINE
		bool do_load = (  load_or_save  && getIntVariable(FIM_VID_LOAD_FIM_HISTORY)==1 );
		bool do_save = ((!load_or_save) && getIntVariable(FIM_VID_SAVE_FIM_HISTORY)==1 );

		if( do_load || do_save )
		{
			fim_char_t hfile[FIM_PATH_MAX];
			const fim_char_t *e = fim_getenv(FIM_CNS_HOME_VAR);

			if(e && strlen(e)<FIM_PATH_MAX-14)//strlen(FIM_CNS_HIST_FILENAME)+2
			{
				strcpy(hfile,e);
				strcat(hfile,"/" FIM_CNS_HIST_FILENAME);

				if( do_load )
					read_history(hfile);
				else
				{
					bool need_chmod=!is_file(hfile);		// will try to chmod if already non existent
					write_history(hfile);
					if(need_chmod)
						chmod(hfile,S_IRUSR|S_IWUSR);	// we write the first .fim_history in mode -rw------- (600)
				}
			}
		}

		return FIM_ERR_NO_ERROR;
    #endif /* FIM_USE_READLINE */
  #endif /* FIM_WANT_NOSCRIPTING  */
#endif /* FIM_NOHISTORY */
#endif /* FIM_WANT_HISTORY */
		return FIM_ERR_GENERIC;
	}

	/*
	 * This routine terminates the program as cleanly as possible.
	 * It should be used whenever useful.
	 */
	void CommandConsole::cleanup(void)
	{
		/*
		 * the display device should exit cleanly to avoid cluttering the console
		 * ... or the window system
		 * used by : fb_catch_exit_signals() : should this matter ?
		 * */

		if(mangle_tcattr_)
			tty_restore();	
		if(displaydevice_)
		       	displaydevice_->finalize();
		load_or_save_history(false);
	}

	/*
	 * inserts the desc text into the textual console,
	 * and eventually displays it
	 */
	void CommandConsole::status_screen(const fim_char_t *desc)
	{
		if(!displaydevice_)
			return;

		displaydevice_->fb_status_screen_new((fim_char_t*)desc,drawOutput(desc),0);
	}

	void CommandConsole::set_status_bar(fim::string desc, const fim_char_t *info)
	{
		set_status_bar(desc.c_str(), info);
	}
	

	bool CommandConsole::set_wm_caption(const fim_char_t *str)
	{
		bool wcs = true;
#if FIM_WANT_CAPTION_CONTROL
		fim_err_t rc=FIM_ERR_NO_ERROR;
		string wcss = getStringVariable(FIM_VID_WANT_CAPTION_STATUS);

		if( wcss.c_str() && *wcss.c_str() && browser_.c_image())
		{
			fim::string clb = browser_.c_image()->getInfoCustom(wcss.c_str());

			rc = displaydevice_->set_wm_caption(clb.c_str());
			wcs = false; /* caption + status */
		}
		else
			if( str && *str )
				rc = displaydevice_->set_wm_caption(str);

		if(rc==FIM_ERR_UNSUPPORTED)
			wcs = false; /* revert */
#endif /* FIM_WANT_CAPTION_CONTROL */
		return wcs;
	}

	void CommandConsole::set_status_bar(const fim_char_t *desc, const fim_char_t *info)
	{
		/*
		 *	Set the 'status bar' of the program.
		 *	- desc will be placed on the left corner
		 *	- info on the right
		 *	Pointers are meant to be freed by the caller.
		 */
		int chars, ilen;
		fim_char_t *str = NULL;
		fim::string hk=FIM_CNS_EMPTY_STRING;	/* help key string */
		int hkl=0;		/* help key string length */
		const int mhkl=5,eisl=9;
		const fim_char_t *hp=" - Help";
		int hpl=fim_strlen(hp);
		prompt_[1]=FIM_SYM_CHAR_NUL;
		fim_bool_t wcs = isSetVar(FIM_VID_WANT_CAPTION_STATUS);
	
		if( ! displaydevice_   )
		       	goto ret;

		hk=this->find_key_for_bound_cmd(FIM_FLT_HELP);/* FIXME: this is SLOW, and should be replaced */
		hkl=fim_strlen(hk.c_str());
		/* FIXME: can we guarantee a bound on its length in some way ? */
		if(hkl>mhkl)
			{hk=FIM_CNS_EMPTY_STRING;hkl=0;/* fix */}
		else
		{
			if(hkl>0)
				{hk+=hp;hkl=hpl;/* help message append*/}
			else
				{hpl=0;/* no help key ? no message, then */}
		}
	
		chars = displaydevice_->get_chars_per_line();
		if(chars<1)
			goto ret;

		str = fim_stralloc(chars+1);
		if(!str)
			goto ret;

		if (desc && info)
		{
			/* non interactive print */
			/*
			 * FIXME : and what if chars < 11 ? :)
			 * */
			ilen = fim_strlen(info);
			if(chars-eisl-ilen-hkl>0)
			{
#if 0
				// sprintf(str, "%s%-*.*s [ %s ] %s",prompt_,
				sprintf(str, "%s%-*.*s %s %s",prompt_,
				chars-eisl-ilen-hkl, chars-eisl-ilen-hkl, desc, info, hk.c_str());//here above there is the need of 14+ilen chars
#else
				snprintf(str, chars-1, "%s%-*.*s %s %s",prompt_,
				chars-eisl-ilen-hkl, chars-eisl-ilen-hkl, desc, info, hk.c_str());//here above there is the need of 14+ilen chars
#endif
			}
			else
			{
				if(chars>5)
				{
					if(chars>10)
					{
				       		snprintf(str, chars-3, "%s%s", prompt_, info);
				       		strcat(str, "...");
					}
					else
				       		sprintf(str, "<-!->");
				}
				else
				{
				       	if(chars>0)
				       	sprintf(str, "!");
			       	}	/* :D */
			}
		}
#ifdef FIM_USE_READLINE
		else
		if(chars>=6+hkl && desc) /* would be a nonsense :) */
		{
			/* interactive print */
			static int statusline_cursor=0;
			int offset=0,coffset=0;

			statusline_cursor=rl_point;	/* rl_point is readline stuff */
			ilen = fim_strlen(desc);
			chars-=6+hpl+(*prompt_=='\0'?0:1);	/* displayable, non-service chars  */
			if(!chars)
				goto done;
			/* 11 is strlen(" | H - Help")*/
			offset =(statusline_cursor/(chars))*(chars);
			coffset=(*prompt_!='\0')+(statusline_cursor%(chars));
		
			sprintf(str, "%s%-*.*s | %s",prompt_, chars, chars, desc+offset, hk.c_str());
			str[coffset]='_';
		}
#endif /* FIM_USE_READLINE */

#if FIM_WANT_CAPTION_CONTROL
		if(wcs)
			wcs = set_wm_caption(str);
		if(!wcs)
#endif /* FIM_WANT_CAPTION_CONTROL */
			displaydevice_->status_line((const fim_char_t*)str); /* one may check the return value.... */
done:
		fim_free(str);
ret:
		return;
	}

	fim_bool_t CommandConsole::inConsole(void)const
	{
#ifdef FIM_USE_READLINE
		return ic_==1;
#else /* FIM_USE_READLINE */
		return false;
#endif /* FIM_USE_READLINE */
	}

	fim_err_t CommandConsole::resize(fim_coo_t w, fim_coo_t h)
	{
		if(!displaydevice_)
			return FIM_ERR_GENERIC;

		if(FIM_ERR_NO_ERROR!=displaydevice_->resize(w,h))
			return FIM_ERR_GENERIC;

		w=displaydevice_->width();
		h=displaydevice_->height();

#ifdef FIM_WINDOWS
		if(window_)
	       	{
		       	Rect nr(0,0,w,h);
			cc.window_->update(nr);
		}
#endif

		displaydevice_->init_console();

		// FIXME: this is a hack
		setVariable("i:" FIM_VID_FRESH,(fim_int)1);//FIXME: bad practice
		browser_.fcmd_redisplay(args_t());

		if(getGlobalIntVariable(FIM_VID_DISPLAY_BUSY))
		{
			fim::string msg="resized window to ";
			msg+=fim::string(w);
		       	msg+=" x ";
		       	msg+=fim::string(h);
			cc.set_status_bar(msg.c_str(),NULL);
		}

		return FIM_ERR_NO_ERROR;
	}

	fim_err_t CommandConsole::display_reinit(const fim_char_t *rs)
	{

		if(!displaydevice_)
			goto err;
		return displaydevice_->reinit(rs);
err:
		return FIM_ERR_GENERIC;
	}

	fim_bool_t CommandConsole::key_syms_update(void)
	{
		sym_keys_t::const_iterator ki;

		for( ki=sym_keys_.begin();ki!=sym_keys_.end();++ki)
			key_syms_[(((*ki).second))]=((*ki).first);
		return true;
	}

	size_t CommandConsole::byte_size(void)const
	{
		size_t bs = 0;
		bs += browser_.byte_size();
		/* NOTE: lots is missing here */
#if FIM_WANT_PIC_CMTS
		bs += id_.byte_size();
#endif /* FIM_WANT_PIC_CMTS */
		return bs;
	}

	fim::string CommandConsole::fcmd_variables_list(const args_t& args)
	{
		return get_variables_list();
	}

	fim::string CommandConsole::fcmd_commands_list(const args_t& args)
	{
		return get_commands_list();
	}

	fim::string CommandConsole::current()const
	{
	       	return browser_.current();
	}

	CommandConsole& CommandConsole::operator= (const CommandConsole&cc)
	{
		return *this;/* a nilpotent assignment */
	}
}

