/* $Id$ */
/*
 CommandConsole-cmd.cpp : Fim console commands

 (c) 2009-2009 Michele Martone

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
	fim::string CommandConsole::bind(const args_t& args)
	{
		/*
		 *	this is the interactive bind command
		 *	the user supplies a string with the key combination, and if valid, its keycode
		 *	is associated to the user supplied actin (be it a command, alias, etc..)
		 *	FIX ME
		 */
		const char *kerr="bind : invalid key argument (should be one of : k, C-k, K, <Left..> }\n";
		if(args.size()==0)return getBindingsList();
		if(args.size()==1)
		{
			//first arg should be a valid key code
			fim::string binding_expanded;
			binding_expanded+="bind '";
			binding_expanded+=args[0];
			binding_expanded+="' '";
			binding_expanded+=bindings[key_bindings[args[0]]];
			binding_expanded+="'\n";
			return binding_expanded;
		}
		/*
		 * TODO: there will be room for the binding comment by the user
		 * */
		if(args.size()<2) return kerr;
		const char*key=(args[0].c_str());
		if(!key)return kerr;
		int l=strlen(key);
		if(!l)return kerr;
		if(args[1]=="") return unbind(args[0]);
		return bind(key_bindings[args[0]],args[1]);
	}

	fim::string CommandConsole::unbind(const args_t& args)
	{
		/*
		 * 	unbinds the action eventually bound to the first key name specified in args..
		 *	IDEAS : multiple unbindings ?
		 *	maybe you should made surjective the binding_keys mapping..
		 */
		if(args.size()!=1)return "unbind : specify the key to unbind\n";
		return unbind(args[0]);
	}

	fim::string CommandConsole::help(const args_t &args)
	{	
		/*
		 *	FIX ME:
		 *	the online help system still needs rework
	 	 * 	TODO : implement a regexp-based search, to give the user hints. (20090512) (like vim's helpgrep)
		 */
		Command *cmd;
		if(!args.empty())
		{
			cmd=findCommand(args[0]);
			if(cmd)
				return  cmd->getHelp()+string("\n");
			else
			if(aliasRecall(fim::string(args[0]))!="")
				return
					string("\"")+(args[0]+string("\" is an alias, and was declared:\n"))+
					get_alias_info(args[0]);
			else
			{
				if(isVariable(args[0]))
					return string("\"")+( args[0] + string( "\" is a variable, with value:\n" )+
					getStringVariable(args[0]));
				else
					cout << args[0] << " : no such command\n";
			}

		}
		this->setVariable(FIM_VID_DISPLAY_CONSOLE,1);
		return "usage : help CMD   (use TAB in commandline mode to get a list of commands :) )\n";
	}

	fim::string CommandConsole::quit(const args_t &args)
	{
		/*
		 * now the postcycle execution autocommands are enabled !
		 * */
		show_must_go_on=0;
		return "";
	}

#ifndef FIM_NOSCRIPTING
	fim::string CommandConsole::executeFile(const args_t &args)
	{
		/*
		 * TODO : catch exceptions
		 * */
		for(size_t i=0;i<args.size();++i)executeFile(args[i].c_str());
		return "";
	}
#endif
	
	fim::string CommandConsole::foo(const args_t &args)
	{
		/*
		 * useful function for bogus commands, but autocompletable (like language constructs)
		 * */
		return "";
	}

#if 0
	fim::string CommandConsole::get_expr_type(const args_t &args);
	{
		/*
		 * a command to echo arguments types, for debug and learning purposes
		 */
		if(args.size()==0)fim::cout<<"type command\n";
		for(size_t i=0;i<args.size();++i)fim::cout << (args[i].c_str()) << "\n";
		return "";

	}
#endif

	fim::string CommandConsole::echo(const args_t &args)
	{
		return do_echo(args);
	}

	fim::string CommandConsole::do_echo(const args_t &args)const
	{
		/*
		 * a command to echo arguments, for debug and learning purposes
		 */
		if(args.size()==0)fim::cout<<"echo command\n";
		for(size_t i=0;i<args.size();++i)fim::cout << (args[i].c_str()) << "\n";
		return "";
	}

	fim::string CommandConsole::do_stdout(const args_t &args)const
	{
		/*
		 * a command to echo to stdout arguments, for debug and learning purposes
		 */
		if(args.size()==0)std::cout<<"echo command\n";
		for(size_t i=0;i<args.size();++i)std::cout << (args[i].c_str()) << "\n";
		return "";
	}

	fim::string CommandConsole::_stdout(const args_t &args)
	{
		return do_stdout(args);
	}

	fim::string CommandConsole::autocmd(const args_t& args)
	{
		/*
		 * associates an action to a certain event in certain circumstances
		 */
		//cout << "autocmd '"<<args[0]<<"' '"<<args[1]<<"' '"<<args[2]<<"' added..\n";
		if(args.size()==0)
		{
			/* no args, returns autocmd's list */
			return autocmds_list("","");
		}
		if(args.size()==1)
		{
			/* autocmd Event : should list all autocmds for the given Event */
			return autocmds_list(args[0],"");
		}
		if(args.size()==2)
		{
			/* autocmd Event Pattern : should list all autocmds for the given Event Pattern */
			return autocmds_list(args[0],args[1]);
		}
		if(args.size()==3)
		{
			return autocmd_add(args[0],args[1],args[2]);
		}
		return "";
	}

	fim::string CommandConsole::autocmd_del(const args_t& args)
	{
		/*
		 * deletes one or more autocommands
		 */
		if(args.size()==0)
		{
			/* no args, returns autocmd's list */
			return autocmd_del("","","");
		}
		if(args.size()==1)
		{
			/* autocmd Event : should delete all autocmds for the given Event */
			return autocmd_del(args[0],"","");
		}
		if(args.size()==2)
		{
			/* autocmd Event Pattern : should delete all autocmds for the given Event Pattern */
			return autocmd_del(args[0],args[1],"");
		}
		if(args.size()==3)
		{
			return autocmd_del(args[0],args[1],args[2]);
		}
		return "";
	}

	fim::string CommandConsole::set_in_console(const args_t& args)
	{
		/*
		 * EXPERIMENTAL !!
		 * */
#ifdef FIM_USE_READLINE
		ic = 1;
#endif
		return "";
	}

	fim::string CommandConsole::set_interactive_mode(const args_t& args)
	{
#ifdef FIM_USE_READLINE
		ic=-1;set_status_bar("",NULL);
#endif
		/*
		 *
		 * */
		return "";
	}

	fim::string CommandConsole::sys_popen(const args_t& args)
	{
		/*
		 *
		 * */
		for(size_t i=0;i<args.size();++i)
		{
			FILE* fd=popen(args[i].c_str(),"r");
			/*
			 * example:
			 *
			 * int fd=(int)popen("/bin/echo quit","r");
			 */
			executeStdFileDescriptor(fd);
			pclose(fd);
		}
		return "";
	}

#ifdef FIM_PIPE_IMAGE_READ
	/*
	 * FBI/FIM FILE PROBING MECHANISMS ARE NOT THOUGHT WITH PIPES IN MIND!
	 * THEREFORE WE MUST FIND A SMARTER TRICK TO IMPLEMENT THIS
	 * */
	fim::string CommandConsole::pread(const args_t& args)
	{
		/*
		 * we read a whole image file from pipe
		 * */
		size_t i;
		FILE* tfd;
		char buf[1024];int rc=0;
		for(i=0;i<args.size();++i)
		if( (tfd=popen(args[i].c_str(),"r")) != NULL )
		{	
			/* todo : read errno in case of error and print some report.. */
	
			/* pipes are not seekable : this breaks down all the Fim file handling mechanism */
			/*
			while( (rc=read(0,buf,1024))>0 ) fwrite(buf,rc,1,tfd);
			rewind(tfd);
			*/
			/*
			 * Note that it would be much nicer to do this in another way,
			 * but it would require to rewrite much of the file loading stuff
			 * (which is quite fbi's untouched stuff right now)
			 * */
			Image* stream_image=new Image("/dev/stdin",tfd);
			// DANGEROUS TRICK!
			browser.set_default_image(stream_image);
			browser.push("");
			//pclose(tfd);
		}
		else
		{
			/*
			 * error handling
			 * */
		}
		return "";
	}
#endif

	fim::string CommandConsole::cd(const args_t& args)
	{
		/*
		 * change working directory
		 * */
		static fim::string oldpwd=pwd(args_t());
		for(size_t i=0;i<args.size();++i)
		{
			fim::string dir=args[i];
			if(dir=="-")dir=oldpwd;
			oldpwd=pwd(args_t());
			int ret = chdir(dir.c_str());
#if 1
			if(ret) return (fim::string("cd error : ")+fim::string(strerror(errno)));
#else
			// deprecated
			if(ret) return (fim::string("cd error : ")+fim::string(sys_errlist[errno]));
#endif
		}
		return "";
	}

	fim::string CommandConsole::pwd(const args_t& args)
	{
		/*
		 * yes, print working directory
		 * */
		fim::string cwd="";
#if HAVE_GET_CURRENT_DIR_NAME
		/* default */
		char *p=get_current_dir_name();
		if(p)cwd=p;
		else cwd="";
		if(p)fim_free(p);
#else
#if _BSD_SOURCE || _XOPEN_SOURCE >= 500
		{
			/* untested */
			char *buf[PATH_MAX];
			getcwd(buf,PATH_MAX-1): 
			buf[PATH_MAX-1]='\0';
			cwd=buf;
		}
#endif
#endif
		return cwd;
	}

#ifndef FIM_NO_SYSTEM
	fim::string CommandConsole::system(const args_t& args)
	{
		/*
		 * executes the shell commands given in the arguments,
		 * one by one, and returns the (collated) standard output
		 * */
		for(size_t i=0;i<args.size();++i)
		{
			FILE* fd=popen(args[i].c_str(),"r");
			/*
			 * popen example:
			 *
			 * int fd=(int)popen("/bin/echo quit","r");
			 */
			cout << readStdFileDescriptor(fd);
			pclose(fd);
		}
#if 0
		for(size_t i=0;i<args.size();++i)
		{
			std::system(args[i].c_str());
		}
#endif
		return "";
	}
#endif
	
	fim::string CommandConsole::do_return(const args_t &args)
	{
		/*
		 * returns immediately the program with an exit code
		 * */
/*		this in unclean
		if( args.size() < 0 ) this->quit(0);
		else	this->quit( (int) args[0] );*/
		/* this is clean */
		if( args.size() < 0 )
			return_code=0;
		else
			return_code=(int)args[0];
		show_must_go_on=0;
		return "";/* it shouldn' return, though :) */
	}

	fim::string CommandConsole::status(const args_t &args)
	{
		/*
		 * the status bar is updated with the given arguments collated.
		 * */
		fim::string s;
		for(size_t i=0;i<args.size();++i)
			s+=args[i].c_str();
		browser.display_status(s.c_str(),NULL);
		return "";
	}

	fim::string CommandConsole::unalias(const args_t& args)
	{
		/*
		 * removes the actions assigned to the specified aliases,
		 */
		if(args.size()<1)
			return "unalias : please specify an alias to remove or all (-a)!\n";

		if(args[0]==string("-a"))
		{
			/* FIXME : the lexer/parser is bugged and it takes -a as an expression if not between double quotes ("-a") */
			aliases.clear();
			return "";
		}

		for(size_t i=0;i<args.size();++i)
		if(aliases[args[i]].first!="")
		{
			aliases.erase(args[i]);
			return "";
			/* fim::string("unalias : \"")+args[i]+fim::string("\" successfully unaliased.\n"); */
		}
		else return fim::string("unalias : \"")+args[i]+fim::string("\" there is not such alias.\n");
		return "";
	}

	fim::string CommandConsole::dump_key_codes(const args_t& args)
	{
		return do_dump_key_codes(args);
	}

	fim::string CommandConsole::do_dump_key_codes(const args_t& args)const
	{
		/*
		 * all keyboard codes are dumped in the console.
		 * */
		fim::string acl;
		key_bindings_t::const_iterator ki;
		for( ki=key_bindings.begin();ki!=key_bindings.end();++ki)
		{
			acl+=((*ki).first);
			acl+=" -> ";
			acl+=(unsigned int)(((*ki).second));
			acl+=", ";
		}
		return acl;
	}

#ifdef FIM_RECORDING
	fim::string CommandConsole::dump_record_buffer(const args_t &args)
	{
		return do_dump_record_buffer(args);
	}

	fim::string CommandConsole::do_dump_record_buffer(const args_t &args)const
	{
		/*
		 * the recorded commands are dumped in the console
		 * */
		fim::string res;
		for(size_t i=0;i<recorded_actions.size();++i)
		{
			fim::string ss=(int)recorded_actions[i].second;
			/*
			 * FIXME : fim::string+=<int> is bugful
			 * */
			res+="usleep '";
//			res+=(int)recorded_actions[i].second;
			res+=ss;
			res+="';\n";
			res+=recorded_actions[i].first;
			res+="\n";
		}
		return res;
	}

	fim::string CommandConsole::execute_record_buffer(const args_t &args)
	{
		/*
		 * all of the commands in the record buffer are re-executed.
		 * */
		execute(dump_record_buffer(args).c_str(),0,0);
		/* for unknown reasons, the following code gives problems : image resizes don't work..
		 * but the present (above) doesn't support interruptions ...
		 * */
/*		fim::string res;
		for(size_t i=0;i<recorded_actions.size();++i)
		{
			res=recorded_actions[i].first+(fim::string)recorded_actions[i].second;
			execute(res.c_str(),0,1);
		}*/
		return "";
	}

	fim::string CommandConsole::eval(const args_t &args)
	{
		/*
		 * all of the commands given as arguments are executed.
		 * */
		for(size_t i=0;i<args.size();++i)
		{
			if(execute(args[i].c_str(),0,0))
				return fim::string("problems executing ")+args[i]+fim::string("\n");
		}
		return "";
	}
#endif

	fim::string CommandConsole::repeat_last(const args_t &args)
	{
		/*
		 * WARNING : there is an intricacy concerning the semantics of this command :
		 * - This command should NOT be registered as last_command, nor any alias 
		 *   triggering it. But this solution would require heavy parsing and very
		 *   complicated machinery and information propagation... 
		 * - A solution would be confining the repeat_last only to interactive commands,
		 *   but this would be a lot sorrowful too, and requires the non-registration 
		 *   of the 'repeat_last;' issuing..
		 * - So, since the recording is made AFTER the command was executed, we set
		 *   a dont_record_last_action flag after each detection of repeat_last, so we do not 
		 *   record the containing string.
		 */
		execute(last_action.c_str(),0,0);
		dont_record_last_action=true;	//the issuing action will not be recorded
		return "";
	}

	fim::string CommandConsole::start_recording(const args_t &args)
	{
		/*
		 * recording of commands starts here
		 * */
		recorded_actions.clear();
		recordMode=true;
		return "";
	}

	fim::string CommandConsole::stop_recording(const args_t &args)
	{
		/*
		 * since the last recorded action was stop_recording, we pop out the last command
		 */
		if(recorded_actions.size()>0)recorded_actions.pop_back();
		recordMode=false;
		return "";
	}

	fim::string CommandConsole::set(const args_t &args)
	{
		/*
		 * with no arguments, prints out the variable names.
		 * with one identifier as argument, prints out its value.
		 * with two arguments, sets the first argument's value.
		 *
		 * NOTE : THIS IS NOT EXACTLY VIM'S BEHAVIOUR (FIXME)
		 * */
		if( ! args.size())return get_variables_list();
		if(1==args.size())return getStringVariable(args[0]);
		/*
		 * warning!
		 * */
		if(2==args.size())return setVariable(args[0],args [1].c_str());
		else
		return "usage : set | set IDENTIFIER | set IDENTIFIER VALUE";
	}

	fim::string CommandConsole::scroll_up(const args_t& args)
	{
		if(!displaydevice) { } else
			displaydevice->console_control(0x01);
		return "";
	}

	fim::string CommandConsole::scroll_down(const args_t& args)
	{
		if(!displaydevice) { } else
			displaydevice->console_control(0x02);
		return "";
	}

	fim::string CommandConsole::clear(const args_t& args)
	{
		displaydevice->console_control(0x03);//experimental
		return "";
	}

	fim::string CommandConsole::markCurrentFile(const args_t& args)
	{
		markCurrentFile();
		return "";
	}

	fim::string CommandConsole::do_getenv(const args_t& args)
	{
		string help="usage : getenv IDENTIFIER  will create a fim variable named IDENTIFIER with value $IDENTIFIER (if nonempty), from the current shell."
#ifndef HAVE_GETENV
		" (note that getenv call was not available at build time, so it won't work)\n"
#endif
		;
		if( ! args.size())return help;
#ifdef HAVE_GETENV
		if(1==args.size())
		{
			if(fim_getenv(args[0].c_str()))
				return setVariable( fim::string("ENV_")+args[0], fim_getenv(args[0].c_str()) );
			else
				return "";
		}
		else
			return help;
#else
		return help;
#endif
	}

	fim::string CommandConsole::get_reference_manual(const args_t& args)
	{
		/*
		 * dump textually a reference manual from all the available fim language features.
		 */
		return
			string("commands:\n")+
			get_commands_reference()+
			string("variables:\n")+
			get_variables_reference()/*+
			get_commands_reference()*/;
	}

	fim::string CommandConsole::dump_reference_manual(const args_t& args)
	{
		/*
		 * dump textually a reference manual from all the available fim language features.
		 */
		std::cout << get_reference_manual(args);
		return "";
	}

}
