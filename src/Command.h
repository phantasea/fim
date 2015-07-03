/* $LastChangedDate: 2013-11-06 19:33:57 +0100 (Wed, 06 Nov 2013) $ */
/*
 Command.h : Fim Command class header file

 (c) 2007-2013 Michele Martone

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
#ifndef FIM_COMMAND_H
#define FIM_COMMAND_H
#include "fim.h"
namespace fim
{

class Command
{
	public:
	fim::string cmd_,
		    help_ ;
	Command(fim::string cmd,fim::string help,Browser *b=NULL,fim::string(Browser::*bf)(const std::vector<fim::string>&)=NULL) :cmd_(cmd),help_(help),browserf(bf),browser(b),type(0) { type=BrowserT;}
	Command(fim::string cmd,fim::string help,CommandConsole *c=NULL,fim::string(CommandConsole::*cf)(const std::vector<fim::string>&)=NULL) :cmd_(cmd),help_(help),consolef(cf),console(c),type(0) { type=CommandConsoleT;}
#ifdef FIM_WINDOWS
	Command(fim::string cmd,fim::string help,FimWindow *w=NULL,fim::string(FimWindow::*cf)(const std::vector<fim::string>&)=NULL) :cmd_(cmd),help_(help),windowf(cf),window(w),type(0) { type=WindowT;}
#endif /* FIM_WINDOWS */

	fim::string getHelp(void)const{return help_;}
	private:
	enum
	{
		BrowserT,
#ifdef FIM_WINDOWS
		WindowT,
#endif /* FIM_WINDOWS */
		CommandConsoleT 
	};
	union{
		fim::string (Browser::*browserf)(const std::vector<fim::string>&) ;
		fim::string (CommandConsole::*consolef)(const std::vector<fim::string>&) ;
#ifdef FIM_WINDOWS
		fim::string (FimWindow::*windowf)(const std::vector<fim::string>&) ;
#endif /* FIM_WINDOWS */
	};
	union{
		Browser *browser;
		CommandConsole *console;
#ifdef FIM_WINDOWS
		FimWindow *window;
#endif /* FIM_WINDOWS */
	};
	fim_cmd_type_t type;

	public:
	
	~Command(void) { }
	
	fim::string execute(const std::vector<fim::string> &args)
	{
		/*
		 * FIXME 
		 */
		assert(browser && browserf);

		//std::cerr <<  "about to execute '"<<cmd_.c_str();for(int i=0;i<args.size();++i)std::cerr << " " << args[i].c_str(); std::cerr << "'\n";
/*		if(!browser || !browserf)
		{
			//std::cout << cmd_.c_str() << " : "<< "\n";
			return fim::string("problems executing command..");
		}else*/
		return (browser->*browserf)(args);
	}

	bool operator < (Command c)const{return cmd_< c.cmd_;}
	bool operator <=(Command c)const{return cmd_<=c.cmd_;}
};
}
#endif /* FIM_COMMAND_H */
