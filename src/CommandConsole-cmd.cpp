/* $LastChangedDate: 2015-04-18 21:28:34 +0200 (Sat, 18 Apr 2015) $ */
/*
 CommandConsole-cmd.cpp : Fim console commands

 (c) 2009-2015 Michele Martone

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
#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif /* HAVE_LIBGEN_H */

#define FIM_WANT_SYSTEM_CALL_DEBUG 0

namespace fim
{
	fim::string CommandConsole::fcmd_bind(const args_t& args)
	{
		/*
		 *	this is the interactive bind command
		 *	the user supplies a string with the key combination, and if valid, its keycode
		 *	is associated to the user supplied action (be it a command, alias, etc..)
		 *	FIX ME
		 */
		const fim_char_t *kerr=FIM_FLT_BIND" : invalid key argument (should be one of : k, C-k, K, <Left..> }\n";
		const fim_char_t*kstr=NULL;
		int l;
		fim_key_t key=FIM_SYM_NULL_KEY;

		if(args.size()==0)
			return getBindingsList();

		kstr=(args[0].c_str());
		if(!kstr)
			return kerr;
		l=strlen(kstr);
		if(!l)
			return kerr;

#ifdef FIM_WANT_RAW_KEYS_BINDING
		if(l>=2 && isdigit(kstr[0]) && isdigit(kstr[1]))
		{
			/* special syntax for raw keys */
			key=atoi(kstr+1);
		}
#endif /* FIM_WANT_RAW_KEYS_BINDING */
		else
		{
			if(sym_keys_.find(kstr)!= sym_keys_.end())
				key=sym_keys_[args[0]];
		}
		if(key==FIM_SYM_NULL_KEY)
		{
			return "supplied key "+args[0]+" is invalid.\n";
		}
		if(args.size()==1 && bindings_.find(key)==bindings_.end())
		{
			return "no command bound to keycode "+string(key)+" (keysym "+args[0]+").\n";
		}

		if(args.size()==1)
		{
			//first arg should be a valid key code
			fim::string binding_expanded;
			binding_expanded+=FIM_FLT_BIND" '";
			binding_expanded+=args[0];
			binding_expanded+="' '";
			binding_expanded+=bindings_[key];
			binding_expanded+="'\n";
			return binding_expanded;
		}
		if(args.size()<2)
		       	return kerr;
		if(args[1]==FIM_CNS_EMPTY_STRING)
		       	return unbind(args[0]);

		if(args.size()>=3)
			bindings_help_[key]=args[2]; /* TODO: shall move to bind() */
		return bind(key,args[1]);
	}

	fim::string CommandConsole::fcmd_unbind(const args_t& args)
	{
		/*
		 * 	unbinds the action eventually bound to the first key name specified in args..
		 *	IDEAS : multiple unbindings ?
		 *	maybe you should made surjective the binding_keys mapping..
		 */
		if(args.size()!=1)
			return FIM_FLT_UNBIND" : specify the key to unbind\n";
		return unbind(args[0]);
	}

#if 0
	fim::string CommandConsole::fcmd_setenv(const args_t& args)
	{
		/*
		 *	
		 */
		if(args.size()<2) return FIM_CNS_EMPTY_RESULT;
		setenv(args[0].c_str(),args[1].c_str(),1);
		setenv("DISPLAY","",1);
		return FIM_CNS_EMPTY_RESULT;
	}
#endif

	fim::string CommandConsole::fcmd_help(const args_t &args)
	{	
		Command *cmd = NULL;

		if(!args.empty() && args[0].length()>0 )
		{
			if((args[0].at(0))==FIM_CNS_SLASH_CHAR)
			{
				/* FIXME: this shall be regexp-based. */
				fim::string hstr,cstr,astr,bstr,ptn;

				ptn = args[0].substr(1,args[0].size());
				for(size_t i=0;i<commands_.size();++i) 
					if(commands_[i] && commands_[i]->help_.find(ptn) != commands_[i]->help_.npos)
					{
						cstr += commands_[i]->cmd_;
						cstr += " ";
					}

				for( aliases_t::const_iterator ai=aliases_.begin();ai!=aliases_.end();++ai)
				{
					if(ai->first.find(ptn) != ai->first.npos)
					{	
						astr +=((*ai).first);
						astr += " ";
					}
					else
					if(ai->second.first.find(ptn) != ai->second.first.npos)
					{	
						astr +=((*ai).first);
						astr += " ";
					}
					else
					if(ai->second.second.find(ptn) != ai->second.second.npos)
					{	
						astr +=((*ai).first);
						astr += " ";
					}
				}

				for(bindings_t::const_iterator  bi=bindings_.begin();bi!=bindings_.end();++bi)
				{
					key_syms_t::const_iterator ikbi=key_syms_.find(((*bi).first));
					bindings_help_t::const_iterator  bhi=bindings_help_.find((*bi).first);

					if(ikbi!=key_syms_.end())
					{
						if(ikbi->second.find(ptn) != ikbi->second.npos)
						{
							bstr += ikbi->second;
						       	bstr += " ";
							// std::cout << "key : " << ikbi->second << "\n";
						}
						else
						if(bi->second.find(ptn) != bi->second.npos)
						{
							bstr += ikbi->second;
						       	bstr += " ";
							// std::cout << "def : " << ikbi->second << "\n";
						}
						if(bhi != bindings_help_.end() )
						if(bhi->second.find(ptn) != bhi->second.npos)
						{
							bstr += ikbi->second;
						       	bstr += " ";
							// std::cout << "dsc : " << ikbi->second << "\n";
						}
					}
				}
				/* FIXME: may extend to autocmd's */

				if(!cstr.empty())
					hstr+="Commands (type 'help' and one quoted command name to get more info):\n " + cstr + "\n";
				if(!astr.empty())
					hstr+="Aliases (type 'alias' and one quoted alias to get more info):\n " + astr + "\n";
				if(!bstr.empty())
					hstr+="Bindings (type 'bind' and one quoted binding to get more info):\n " + bstr + "\n";
				if(!hstr.empty())
					return "The following help items matched \"" + ptn + "\":\n" + hstr;
				else
					return "No item matched \"" + ptn + "\"\n";
			}

		{
			fim::string sws;
			sws = fim_help_opt(args[0].c_str());
			if(sws != FIM_CNS_EMPTY_STRING)
				return sws+string("\n");
		}
			cmd=findCommand(args[0]);
			if(cmd)
				return
					string("\"")+(args[0]+string("\" is a command, documented:\n"))+
				      	cmd->getHelp()+string("\n");
			else
			if(aliasRecall(fim::string(args[0]))!=FIM_CNS_EMPTY_STRING)
				return
					string("\"")+(args[0]+string("\" is an alias, and was declared as:\n"))+
					get_alias_info(args[0]);
			else
			{
				if(isVariable(args[0]))
				{
					fim::string hs;
					hs+=fim::string("\"");
					hs+=args[0] + fim::string( "\" is a variable, with value:\n" );
					hs+=getStringVariable(args[0]);
					hs+=fim::string("\nand description:\n");
					hs+=fim_var_help_db_query(args[0]);
					hs+=fim::string("\n");
					return hs;
				}
				else
					cout << args[0] << " : no such command, alias, or variable.\n";
			}

		}
		this->setVariable(FIM_VID_DISPLAY_CONSOLE,(fim_int)1);
		return "" FIM_FLT_HELP " " FIM_CNS_EX_ID_STRING ": provides help for " FIM_CNS_EX_ID_STRING ", if it is a variable, alias, or command. Use " FIM_KBD_TAB " in commandline mode to get a list of commands. Command line mode can be entered with the default key '" FIM_SYM_CONSOLE_KEY_STR "', and left pressing " FIM_KBD_ENTER ".\n";
	}

	fim::string CommandConsole::fcmd_quit(const args_t &args)
	{
		/*
		 * now the postcycle execution autocommands are enabled !
		 * */
		show_must_go_on_=0;
		if( args.size() < 1 )
			return_code_=0;
		else
			return_code_=(int)args[0];
		return FIM_CNS_EMPTY_RESULT;
	}

#ifndef FIM_WANT_NOSCRIPTING
	fim::string CommandConsole::fcmd_executeFile(const args_t &args)
	{
		/*
		 * TODO : catch exceptions
		 * */
		for(size_t i=0;i<args.size();++i)
			executeFile(args[i].c_str());
		return FIM_CNS_EMPTY_RESULT;
	}
#endif /* FIM_WANT_NOSCRIPTING */
	
	fim::string CommandConsole::fcmd_foo(const args_t &args)
	{
		/*
		 * useful function for bogus commands, but autocompletable (like language constructs)
		 * */
		return FIM_CNS_EMPTY_RESULT;
	}

#if 0
	fim::string CommandConsole::get_expr_type(const args_t &args);
	{
		/*
		 * a command to echo arguments types, for debug and learning purposes
		 */
		if(args.size()==0)fim::cout<<"type command\n";
		for(size_t i=0;i<args.size();++i)fim::cout << (args[i].c_str()) << "\n";
		return FIM_CNS_EMPTY_RESULT;

	}
#endif

	fim::string CommandConsole::fcmd_echo(const args_t &args)
	{
		return do_echo(args);
	}

	fim::string CommandConsole::do_echo(const args_t &args)const
	{
		/*
		 * a command to echo arguments, for debug and learning purposes
		 */
		if(args.size()==0)
			fim::cout<<FIM_FLT_ECHO" command\n";
		for(size_t i=0;i<args.size();++i)
			fim::cout << (args[i].c_str()) << "\n";
		return FIM_CNS_EMPTY_RESULT;
	}

	fim::string CommandConsole::fcmd__stdout(const args_t &args)
	{
		/*
		 * a command to echo to stdout arguments, for debug and learning purposes
		 */
		if(args.size()==0)
			std::cout<<"echo command\n";
		for(size_t i=0;i<args.size();++i)
			std::cout << (args[i].c_str()) << "\n";
		return FIM_CNS_EMPTY_RESULT;
	}

#ifdef FIM_AUTOCMDS
	fim::string CommandConsole::fcmd_autocmd(const args_t& args)
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
		return FIM_CNS_EMPTY_RESULT;
	}

	fim::string CommandConsole::fcmd_autocmd_del(const args_t& args)
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
		return FIM_CNS_EMPTY_RESULT;
	}
#endif /* FIM_AUTOCMDS */

	fim::string CommandConsole::fcmd_set_in_console(const args_t& args)
	{
		/*
		 * EXPERIMENTAL !!
		 * */
#ifdef FIM_USE_READLINE
		ic_ = 1;
#endif /* FIM_USE_READLINE */
		return FIM_CNS_EMPTY_RESULT;
	}

	fim::string CommandConsole::fcmd_set_interactive_mode(const args_t& args)
	{
#ifdef FIM_USE_READLINE
		ic_=-1;set_status_bar("",NULL);
#endif /* FIM_USE_READLINE */
		/*
		 *
		 * */
		return FIM_CNS_EMPTY_RESULT;
	}

	fim::string CommandConsole::fcmd_sys_popen(const args_t& args)
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
		return FIM_CNS_EMPTY_RESULT;
	}

	fim_err_t CommandConsole::fpush(FILE *tfd)
	{
			/* todo : read errno in case of error and print some report.. */
	
			/* pipes are not seekable : this breaks down all the Fim file handling mechanism */
			/*
			while( (rc=read(0,buf,FIM_PIPE_BUFSIZE))>0 ) fwrite(buf,rc,1,tfd);
			rewind(tfd);
			*/
			/*
			 * Note that it would be much nicer to do this in another way,
			 * but it would require to rewrite much of the file loading stuff
			 * (which is quite fbi's untouched stuff right now)
			 * */
			Image* stream_image=NULL;
			if(!tfd)
				return FIM_ERR_GENERIC;
			try{ stream_image=new Image(FIM_STDIN_IMAGE_NAME,fim_fread_tmpfile(tfd)); }
			catch (FimException e){/* write me */}
#ifdef FIM_READ_STDIN_IMAGE
			// DANGEROUS TRICK!
			if(stream_image)
			{
				browser_.set_default_image(stream_image);
					if(!cc.browser_.cache_.setAndCacheStdinCachedImage(stream_image))
						std::cerr << FIM_EMSG_CACHING_STDIN;// FIXME
				browser_.push(FIM_STDIN_IMAGE_NAME);
			}
#else /* FIM_READ_STDIN_IMAGE */
			/* FIXME: this point should be better not reached */
#endif /* FIM_READ_STDIN_IMAGE */
			return FIM_ERR_NO_ERROR;
			//pclose(tfd);
	}

#ifdef FIM_PIPE_IMAGE_READ
	/*
	 * FBI/FIM FILE PROBING MECHANISMS ARE NOT THOUGHT WITH PIPES IN MIND!
	 * THEREFORE WE MUST FIND A SMARTER TRICK TO IMPLEMENT THIS
	 * */
	fim::string CommandConsole::fcmd_pread(const args_t& args)
	{
		/*
		 * we read a whole image file from pipe
		 * */
		size_t i;
		FILE* tfd;
		/* fim_char_t buf[FIM_PIPE_BUFSIZE]; int rc=0; */
		for(i=0;i<args.size();++i)
		if( (tfd=popen(args[i].c_str(),"r")) != NULL )
		{	
			fpush(tfd);
		}
		else
		{
			/*
			 * error handling
			 * */
		}
		return FIM_CNS_EMPTY_RESULT;
	}
#endif /* FIM_PIPE_IMAGE_READ */

	fim::string CommandConsole::fcmd_cd(const args_t& args)
	{
		/*
		 * change working directory
		 * */
		static fim::string oldpwd=fcmd_pwd(args_t());
		for(size_t i=0;i<args.size();++i)
		{
			fim::string dir=args[i];
			if(dir=="-")dir=oldpwd;
			oldpwd=fcmd_pwd(args_t());
#ifdef HAVE_LIBGEN_H
			if(!is_dir(dir))
			{
				dir=fim_dirname(dir);
			}
#endif /* HAVE_LIBGEN_H */
			int ret = chdir(dir.c_str());
#if 1
			if(ret)
			       	return (fim::string("cd error : ")+fim::string(strerror(errno)));
#else
			// deprecated
			if(ret)
			       	return (fim::string("cd error : ")+fim::string(sys_errlist[errno]));
#endif
		}
		setVariable(FIM_VID_PWD,fcmd_pwd(args_t()).c_str());
		return FIM_CNS_EMPTY_RESULT;
	}

	fim::string CommandConsole::fcmd_basename(const args_t& args)
	{
		/*
		 * */
		static fim::string res="";
		for(size_t i=0;i<args.size();++i)
		{
			fim::string arg=args[i];
			res+=basename((fim_char_t*)arg.c_str());//FIXME
		}
		setVariable(FIM_VID_LAST_SYSTEM_OUTPUT,res.c_str());
		return res;
	}

	fim::string CommandConsole::fcmd_pwd(const args_t& args)
	{
		/*
		 * print working directory
		 * */
		fim::string cwd=FIM_CNS_EMPTY_STRING;
#if HAVE_GET_CURRENT_DIR_NAME
		/* default */
		fim_char_t *p=get_current_dir_name();
		if(p)
			cwd=p;
		else
		       	cwd=FIM_CNS_EMPTY_STRING;
		if(p)
			fim_free(p);
#else /* HAVE_GET_CURRENT_DIR_NAME */
#if _BSD_SOURCE || _XOPEN_SOURCE >= 500
		{
			/* untested */
			fim_char_t *buf[PATH_MAX];
			getcwd(buf,PATH_MAX-1): 
			buf[PATH_MAX-1]=FIM_SYM_CHAR_NUL;
			cwd=buf;
		}
#endif /* _BSD_SOURCE || _XOPEN_SOURCE >= 500 */
#endif /* HAVE_GET_CURRENT_DIR_NAME */
		return cwd;
	}

#ifndef FIM_NO_SYSTEM
	fim::string CommandConsole::fcmd_system(const args_t& args)
	{
		/*
		 * executes the shell commands given in the arguments,
		 * one by one, and returns the (collated) standard output
		 * */
#if FIM_WANT_SYSTEM_CALL_DEBUG
		fim::string is=FIM_CNS_EMPTY_STRING;
#endif /* FIM_WANT_SYSTEM_CALL_DEBUG */
#if FIM_WANT_SINGLE_SYSTEM_INVOCATION
		/* 20110302 FIXME: inefficient */
		fim::string cc=FIM_CNS_EMPTY_STRING;
		for(size_t i=0;i<args.size();++i)
		{
			// FIXME: escaping the command (first argument) actually requires more than this
#define FIM_WANT_SIMPLE_SHELL_ESCAPING 1
#if FIM_WANT_SIMPLE_SHELL_ESCAPING
			cc+=fim_shell_arg_escape(args[i]);
#else /* FIM_WANT_SIMPLE_SHELL_ESCAPING */
			cc+=args[i];
#endif /* FIM_WANT_SIMPLE_SHELL_ESCAPING */
			cc+=" ";
#if FIM_WANT_SYSTEM_CALL_DEBUG
			is+=args[i];
			is+=" ";
#endif /* FIM_WANT_SYSTEM_CALL_DEBUG */
		}
#if FIM_WANT_SYSTEM_CALL_DEBUG
		std::cerr << "received string: " << is << FIM_SYM_ENDL;	
		std::cerr << "about to call popen on string: " << cc << FIM_SYM_ENDL;	
#endif /* FIM_WANT_SYSTEM_CALL_DEBUG */
		if(args.size())
		{
			FILE* fd=popen(cc.c_str(),"r");
		//	cout << readStdFileDescriptor(fd);
			setVariable(FIM_VID_LAST_SYSTEM_OUTPUT,readStdFileDescriptor(fd).c_str());
			cout << getStringVariable(FIM_VID_LAST_SYSTEM_OUTPUT);
		       	pclose(fd);
		}
#else /* FIM_WANT_SINGLE_SYSTEM_INVOCATION */
		for(size_t i=0;i<args.size();++i)
		{
			FILE* fd=popen(args[i].c_str(),"r");
			/*
			 * popen example:
			 *
			 * int fd=(int)popen("/bin/echo quit","r");
			 */
			//cout << readStdFileDescriptor(fd);
			setVariable(FIM_VID_LAST_SYSTEM_OUTPUT,readStdFileDescriptor(fd).c_str());
			pclose(fd);
		}
#endif /* FIM_WANT_SINGLE_SYSTEM_INVOCATION */
#if 0
		for(size_t i=0;i<args.size();++i)
		{
			std::system(args[i].c_str());
		}
#endif
		return FIM_CNS_EMPTY_RESULT;
	}
#endif /* FIM_NO_SYSTEM */
	
	fim::string CommandConsole::fcmd_status(const args_t &args)
	{
		/*
		 * the status bar is updated with the given arguments collated.
		 * */
		fim::string s;
		for(size_t i=0;i<args.size();++i)
			s+=args[i].c_str();
		browser_.display_status(s.c_str());
		return FIM_CNS_EMPTY_RESULT;
	}

	fim::string CommandConsole::fcmd_unalias(const args_t& args)
	{
		/*
		 * removes the actions assigned to the specified aliases,
		 */
		if(args.size()<1)
			return FIM_FLT_UNALIAS" : please specify an alias to remove or all (-a)!\n";

		if(args[0]==string("-a"))
		{
			/* FIXME : the lexer/parser is bugged and it takes -a as an expression if not between double quotes ("-a") */
			aliases_.clear();
			return FIM_CNS_EMPTY_RESULT;
		}

		for(size_t i=0;i<args.size();++i)
		if(aliases_[args[i]].first!=FIM_CNS_EMPTY_STRING)
		{
			aliases_.erase(args[i]);
			return FIM_CNS_EMPTY_RESULT;
			/* fim::string(FIM_FLT_UNALIAS" : \"")+args[i]+fim::string("\" successfully unaliased.\n"); */
		}
		else
		       	return fim::string(FIM_FLT_UNALIAS" : \"")+args[i]+fim::string("\" there is not such alias.\n");
		return FIM_CNS_EMPTY_RESULT;
	}

	fim::string CommandConsole::fcmd_dump_key_codes(const args_t& args)
	{
		return do_dump_key_codes(args);
	}

	fim::string CommandConsole::do_dump_key_codes(const args_t& args)const
	{
		/*
		 * all keyboard codes are dumped in the console.
		 * */
		fim::string acl;
		sym_keys_t::const_iterator ki;
		acl+="There are ";
		acl+=fim::string((int)(sym_keys_.size()));
		acl+=" bindings (dumping them here, unescaped).\n";
		for( ki=sym_keys_.begin();ki!=sym_keys_.end();++ki)
		{
			acl+=((*ki).first);
			acl+=" -> ";
			acl+=fim::string((int)(((*ki).second)));
			acl+="  ";
		}
		acl+="\n";
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
		for(size_t i=0;i<recorded_actions_.size();++i)
		{
			fim::string ss=(int)recorded_actions_[i].second;
			/*
			 * FIXME : fim::string+=<int> is bugful
			 * */
			res+=FIM_FLT_USLEEP" '";
//			res+=(int)recorded_actions_[i].second;
			res+=ss;
			res+="';\n";
			res+=recorded_actions_[i].first;
			res+="\n";
		}
		return res;
	}

	fim::string CommandConsole::execute_record_buffer(const args_t &args)
	{
		/*
		 * all of the commands in the record buffer are re-executed.
		 * */
		execute_internal(dump_record_buffer(args).c_str(),FIM_X_NULL);
		/* for unknown reasons, the following code gives problems : image resizes don't work..
		 * but the present (above) doesn't support interruptions ...
		 * */
/*		fim::string res;
		for(size_t i=0;i<recorded_actions_.size();++i)
		{
			res=recorded_actions_[i].first+(fim::string)recorded_actions_[i].second;
			execute_internal(res.c_str(),FIM_X_QUIET);
		}*/
		return FIM_CNS_EMPTY_RESULT;
	}

	fim::string CommandConsole::fcmd_eval(const args_t &args)
	{
		/*
		 * all of the commands given as arguments are executed.
		 * */
		for(size_t i=0;i<args.size();++i)
		{
			if(execute_internal(args[i].c_str(),FIM_X_NULL))
				return fim::string("problems executing ")+args[i]+fim::string("\n");
		}
		return FIM_CNS_EMPTY_RESULT;
	}
#endif /* FIM_RECORDING */

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
		 *   a dont_record_last_action_ flag after each detection of repeat_last, so we do not 
		 *   record the containing string.
		 */
		execute_internal(last_action_.c_str(),FIM_X_NULL);
		dont_record_last_action_=true;	//the issuing action will not be recorded
		return FIM_CNS_EMPTY_RESULT;
	}

	fim::string CommandConsole::fcmd_recording(const args_t &args)
	{
		if(args.size()<1)
		{
			goto nop;
		}
		else
		{
			if(args[0]=="start")
				return start_recording();
			if(args[0]=="stop")
				return stop_recording();
			if(args[0]=="dump")
			{
				args_t argsc(args);
				argsc.erase(argsc.begin());
				return dump_record_buffer(argsc);
			}
			if(args[0]=="execute")
			{
				args_t argsc(args);
				argsc.erase(argsc.begin());
				return execute_record_buffer(argsc);
			}
			if(args[0]=="repeat_last")
			{
				args_t argsc(args);
				argsc.erase(argsc.begin());
				return repeat_last(argsc);
			}
		}
nop:
		return FIM_CNS_EMPTY_RESULT;
	}

	fim::string CommandConsole::start_recording(void)
	{
		/*
		 * recording of commands starts here
		 * */
		recorded_actions_.clear();
		recordMode_=true;
		return FIM_CNS_EMPTY_RESULT;
	}

	fim::string CommandConsole::stop_recording(void)
	{
		/*
		 * since the last recorded action was stop_recording, we pop out the last command
		 */
		if(recorded_actions_.size()>0)recorded_actions_.pop_back();
		recordMode_=false;
		return FIM_CNS_EMPTY_RESULT;
	}

	fim::string CommandConsole::fcmd_set(const args_t &args)
	{
		/*
		 * with no arguments, prints out the variable names.
		 * with one identifier as argument, prints out its value.
		 * with two arguments, sets the first argument's value.
		 *
		 * NOTE : THIS IS NOT EXACTLY VIM'S BEHAVIOUR (FIXME)
		 * */
		if( ! args.size())
			return get_variables_list();
		if(1==args.size())
			return getStringVariable(args[0]);
		/*
		 * warning!
		 * */
		if(2==args.size())
			return setVariable(args[0],args [1].c_str());
		else
			return FIM_CMD_HELP_SET;
	}

#ifndef FIM_WANT_NO_OUTPUT_CONSOLE
	fim::string CommandConsole::scroll_up(const args_t& args)
	{
		if(!displaydevice_)
	       	{ }
		else
			displaydevice_->console_control(0x01);
		return FIM_CNS_EMPTY_RESULT;
	}

	fim::string CommandConsole::scroll_down(const args_t& args)
	{
		if(!displaydevice_)
	       	{ }
		else
			displaydevice_->console_control(0x02);
		return FIM_CNS_EMPTY_RESULT;
	}

	fim::string CommandConsole::fcmd_clear(const args_t& args)
	{
		displaydevice_->console_control(0x03);//experimental
		return FIM_CNS_EMPTY_RESULT;
	}
#endif /* FIM_WANT_NO_OUTPUT_CONSOLE */

	fim::string CommandConsole::fcmd_do_getenv(const args_t& args)
	{
		string help="usage : " FIM_FLT_GETENV " " FIM_CNS_EX_ID_STRING " will create a fim variable named " FIM_CNS_EX_ID_STRING " with value $" FIM_CNS_EX_ID_STRING " (if nonempty), from the current shell."
#ifndef HAVE_GETENV
		" (note that getenv call was not available at build time, so it won't work)\n"
#endif /* HAVE_GETENV */
		;
		if( ! args.size())
			return help;
#ifdef HAVE_GETENV
		if(1==args.size())
		{
			if(fim_getenv(args[0].c_str()))
				return setVariable( fim::string("ENV_")+args[0], fim_getenv(args[0].c_str()) );
			else
				return FIM_CNS_EMPTY_RESULT;
		}
		else
			return help;
#else /* HAVE_GETENV */
		return help;
#endif /* HAVE_GETENV */
	}

	fim::string CommandConsole::fcmd_desc(const args_t& args)
	{
#if FIM_WANT_PIC_CMTS
		fim_char_t sc = '\t';

		if(2 > args.size() || args[0] != "load" )
			goto err;
		if(2 < args.size())
			sc = *args[2].c_str();
		this->id_.fetch(args[1],sc);
		browser_.cache_.desc_update();
err:
#endif /* FIM_WANT_PIC_CMTS */
		return FIM_CNS_EMPTY_RESULT;
	}

}
