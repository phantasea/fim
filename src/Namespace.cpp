/* $Id: Namespace.cpp 229 2009-03-28 15:31:59Z dezperado $ */
/*
 Namespace.h : a class for local variables storage

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

#include "fim.h"

namespace fim
{

	/*
	 * WARNING : TEMPORARILY, THIS FILE WILL HOST SOME CommandConsole's METHODS
	 * */

	int CommandConsole::setVariable(const fim::string& varname,int value)
	{
		/*
		 * an internal function to set a user variable
		 */
#ifdef FIM_NAMESPACES
		if( varname[1]==':' )
		{
			try
			{
			//a specific namespace was selected!
			char ns = varname[0];
			fim::string id=varname.c_str()+2;
			if( ns == 'w' )
#ifdef FIM_WINDOWS
			{
				//window variable
				if(window)
					return window->setVariable(id,value);
				else
					return 0;
			}
			else
			if( ns == 'v' )
			{
				//viewport variable
				if(window && window->current_viewportp())
					return window->current_viewportp()->setVariable(id,value);
				else
					return 0;
			}
			else
#endif
			if( ns == 'i' )
			{
				//image variable
				return
					browser.c_image()?
					( (Image*) (browser.c_image()))->setVariable(id,value):
					0;
			}
			else
			if( ns == 'b' )
			{
				//browser variable
				return browser.setVariable(id,value);
			}
			else
			if( ns != 'g' )
			{
				//invalid namespace
				return 0;
			}
			}
			catch(FimException e){}
		}
#endif
		return variables[varname].setInt(value);
	}

	float CommandConsole::setVariable(const fim::string& varname,float value)
	{
		/*
		 * an internal function to set a user variable
		 */
//		cout << "setVariable " << variables[varname].setFloat(value) << "\n"; 
#ifdef FIM_NAMESPACES
		if( varname[1]==':' )
		{
			try
			{
			//a specific namespace was selected!
			char ns = varname[0];
			fim::string id=varname.c_str()+2;
#ifdef FIM_WINDOWS
			if( ns == 'w' )
			{
				//window variable
				if(window)
					return window->setVariable(id,value);
				else
					return 0;
			}
			else
			if( ns == 'v' )
			{
				//viewport variable
				if(window && window->current_viewportp())
					return window->current_viewportp()->setVariable(id,value);
				else
					return 0;
			}
			else
#endif
			if( ns == 'i' )
			{
				//image variable
				return
					browser.c_image()?
					( (Image*) (browser.c_image()))->setVariable(id,value):
					0;
			}
			else
			if( ns == 'b' )
			{
				//browser variable
				return browser.setVariable(id,value);
			}
			else
			if( ns != 'g' )
			{
				//invalid namespace
				return 0;
			}
			}
			catch(FimException e){}
		}
#endif
		return variables[varname].setFloat(value);
	}

	int CommandConsole::setVariable(const fim::string& varname,const char*value)
	{
		/*
		 * an internal function to set a user variable
		 */
		fim::string s(value);
#ifdef FIM_NAMESPACES
		if( varname[1]==':' )
		{
			//a specific namespace was selected!
			try
			{
			char ns = varname[0];
			fim::string id=varname.c_str()+2;
#ifdef FIM_WINDOWS
			if( ns == 'w' )
			{
				//window variable
				if(window)
					return window->setVariable(id,s.c_str());
				else
					return 0;
			}
			else
			if( ns == 'v' )
			{
				//viewport variable
				if(window && window->current_viewportp())
					return window->current_viewportp()->setVariable(id,s.c_str());
				else
					return 0;
			}
			else
#endif
			if( ns == 'i' )
			{
				//image variable
				return
					browser.c_image()?
					( (Image*) (browser.c_image()))->setVariable(id,s.c_str()):
					0;
			}
			else
			if( ns == 'b' )
			{
				//browser variable
				return browser.setVariable(id,s.c_str());
			}
			else
			if( ns != 'g' )
			{
				//invalid namespace
				return 0;
			}
			}
			catch(FimException e){}
		}
#endif
		return (int)(variables[varname].setString(s));
	}

	Var CommandConsole::setVariable(const fim::string varname,const Var&value)
	{
		/*
		 * an internal function to set a user variable
		 */
//		std::cout << "CCSV\n";
#ifdef FIM_NAMESPACES
		if( varname[1]==':' )
		{
			//a specific namespace was selected!
			try
			{
			char ns = varname[0];
			fim::string id=varname.c_str()+2;
#ifdef FIM_WINDOWS
			if( ns == 'w' )
			{
				//window variable
				if(window)
					return window->setVariable(id,value);
				else
					return 0;
			}
			else
			if( ns == 'v' )
			{
				//viewport variable
				if(window && window->current_viewportp())
					return window->current_viewportp()->setVariable(id,value);
				else
					return 0;
			}
			else
#endif
			if( ns == 'i' )
			{
				//image variable
				if(browser.c_image())
				DBG(varname<<" : "<<value.getType()<<"\n");
				return
					browser.c_image()?
					( (Image*) (browser.c_image()))->setVariable(id,value):
					Var(0);
			}
			else
			if( ns == 'b' )
			{
				//browser variable
				return browser.setVariable(id,value);
			}
			else
			if( ns != 'g' )
			{
				//invalid namespace
				return 0;
			}
			}
			catch(FimException e){}
		}
#endif
		variables[varname]=value;	/* FIXME : this does not work */
		variables[varname].set(value);	/* FIXME : this works */
		//std::cout <<"CCGS["<<varname<<"]"<<variables[varname].getString()<<"\n";
		//std::cout <<"CCGS["<<varname<<"]"<<value.getString()<<"\n";
		return value;;
	}


	int CommandConsole::getIntVariable(const fim::string &varname)const
	{
#ifdef FIM_NAMESPACES
		if( varname[1]==':' )
		{
			try
			{
			//a specific namespace was selected!
			char ns = varname[0];
			fim::string id=varname.c_str()+2;
#ifdef FIM_WINDOWS
			if( ns == 'w' )
			{
				//window variable
				if(window)
					return window->getIntVariable(id);
				else
					return 0;
			}
			else
			if( ns == 'v' )
			{
				//viewport variable
				if(window && window->current_viewportp())
					return window->current_viewportp()->getIntVariable(id);
				else
					return 0;
			}
			else
#endif
			if( ns == 'i' )
			{
				//image variable
				return
					browser.c_image()?
					( (Image*) (browser.c_image()))->getIntVariable(id):
					0;
			}
			else
			if( ns == 'b' )
			{
				//browser variable
				return browser.getIntVariable(id);
			}
			else
			if( ns != 'g' )
			{
				//invalid namespace
				return 0;
			}
			}
			catch(FimException e){}
		}
#endif
		// this scope was selected
		/*
		 * the variable name supplied is used as a key to the variables hash
		 *
		 * FIXME : now the random stuff is done at interpreter level.
		 * AND IT SHOULD NOT BE USED INTERNALY BY FIM!
		 *
		 * BEWARE!
		 * */
		variables_t::const_iterator vi=variables.find(varname);
		if(vi!=variables.end()) return vi->second.getInt();
		else return 0;
		//return ((c_variables_t)(variables))[varname].getInt();
	}

	float CommandConsole::getFloatVariable(const fim::string &varname)const
	{
		/*
		 * the variable name supplied is used as a key to the variables hash
		 * */
//		cout << "getVariable " << varname  << " : " << variables[varname].getFloat()<< "\n";
//		cout << "getVariable " << varname  << ", type : " << variables[varname].getType()<< "\n";
#ifdef FIM_NAMESPACES
		if( varname[1]==':' )
		{
			try{	
			//a specific namespace was selected!
			char ns = varname[0];
			fim::string id=varname.c_str()+2;
#ifdef FIM_WINDOWS
			if( ns == 'w' )
			{
				//window variable
				if(window)
					return window->getFloatVariable(id);
				else
					return 0.0;
			}
			else
			if( ns == 'v' )
			{
				//viewport variable
				if(window && window->current_viewportp())
					return window->current_viewportp()->getFloatVariable(id);
				else
					return 0.0;
			}
			else
#endif
			if( ns == 'i' )
			{
				//image variable
				return
					browser.c_image()?
					( (Image*) (browser.c_image()))->getFloatVariable(id):
					0.0;
			}
			else
			if( ns == 'b' )
			{
				//browser variable
				return browser.getFloatVariable(id);
			}
			else
			if( ns != 'g' )
			{
				//invalid namespace
				return 0.0;
			}
			}
			catch(FimException e){}
		}
#endif
		variables_t::const_iterator vi=variables.find(varname);
		if(vi!=variables.end()) return vi->second.getFloat();
		else return 0.0;
//		return variables[varname].getFloat();
	}

	fim::string CommandConsole::getStringVariable(const fim::string &varname)const
	{
		/*
		 * the variable name supplied is used as a key to the variables hash
		 * */
#ifdef FIM_NAMESPACES
		if( varname[1]==':' )
		{
			try
			{
			//a specific namespace was selected!
			char ns = varname[0];
			fim::string id=varname.c_str()+2;
#ifdef FIM_WINDOWS
			if( ns == 'w' )
			{
				//window variable
				if(window)
					return window->getStringVariable(id);
				else
					return "";
			}
			else
			if( ns == 'v' )
			{
				//viewport variable
				if(window && window->current_viewportp())
					return window->current_viewportp()->getStringVariable(id);
				else
					return "";
			}
			else
#endif
			if( ns == 'i' )
			{
				//image variable
				return
					browser.c_image()?
					( (Image*) (browser.c_image()))->getStringVariable(id):
					"";
			}
			else
			if( ns == 'b' )
			{
				//browser variable
//				std::cout << "brbbbr\n";
				return browser.getStringVariable(id);
			}
			else
			if( ns != 'g' )
			{
				//invalid namespace
				return "";
			}
			}
			catch(FimException e){}
		}
#endif
//		return variables[varname].getString();
		variables_t::const_iterator vi=variables.find(varname);
		if(vi!=variables.end()) 
		{
			return vi->second.getString();
		}
		else return "";
	}

		int Namespace::setVariable(const fim::string& varname,int value)
		{
			return variables[varname].setInt(value);
		}

		float Namespace::setVariable(const fim::string& varname,float value)
		{
			/*
			 * an internal function to set a user variable
			 */
	//		cout << "setVariable " << variables[varname].setFloat(value) << "\n"; 
			return variables[varname].setFloat(value);
		}

		Var Namespace::setVariable(const fim::string& varname,const Var&value)
		{
			/*
			 * an internal function to set a user variable
			 */
			/* FIXME ! */
//			std::cout << "NSSV:"<<varname<<"\n";
//			std::cout << __FILE__<<":"<<__LINE__<<"\n";
			variables[varname]=Var(value);
			variables[varname].set(value);/* FIXME : it seems necessary (see tests) */
//			std::cout << "NSSV:"<<variables[varname].getString()<<"\n";
			return variables[varname].getString();
		}

		int Namespace::setVariable(const fim::string& varname,const char*value)
		{
			/*
			 * an internal function to set a user variable
			 */
			fim::string s(value);
			return (int)(variables[varname].setString(s));
		}
	
		int Namespace::getIntVariable(const fim::string &varname)const
		{
			// this scope was selected
			//return variables[varname];
			variables_t::const_iterator vi=variables.find(varname);
			if(vi!=variables.end()) return vi->second.getInt();
			else return 0;
		}

		Var CommandConsole::getVariable(const fim::string &varname)const
		{
		/*
		 * the variable name supplied is used as a key to the variables hash
		 * */
#ifdef FIM_NAMESPACES
		if( varname[1]==':' )
		{
			try{	
			//a specific namespace was selected!
			char ns = varname[0];
			fim::string id=varname.c_str()+2;
#ifdef FIM_WINDOWS
			if( ns == 'w' )
			{
				//window variable
				if(window)
					return window->getVariable(id);
				else
					return Var();
			}
			else
			if( ns == 'v' )
			{
				//viewport variable
				if(window && window->current_viewportp())
					return window->current_viewportp()->getVariable(id);
				else
					return Var();
			}
			else
#endif
			if( ns == 'i' )
			{
				//image variable
				return
					browser.c_image()?
					( (Image*) (browser.c_image()))->getVariable(id):
					Var();
			}
			else
			if( ns == 'b' )
			{
				//browser variable
				return browser.getVariable(id);
			}
			else
			if( ns != 'g' )
			{
				//invalid namespace
				return Var();
			}
			}
			catch(FimException e){}
		}
#endif
		variables_t::const_iterator vi=variables.find(varname);
		if(vi!=variables.end()) return vi->second;
		else return Var();
		}

		Var Namespace::getVariable(const fim::string &varname)const
		{
			// this scope was selected
			//return variables[varname];
			variables_t::const_iterator vi=variables.find(varname);
			if(vi!=variables.end()) return vi->second;
			else return Var(0);
		}

		float Namespace::getFloatVariable(const fim::string &varname)const
		{
			/*
			 * the variable name supplied is used as a key to the variables hash
			 * */
//			cout << "getVariable " << varname  << " : " << variables[varname].getFloat()<< "\n";
//			cout << "getVariable " << varname  << ", type : " << variables[varname].getType()<< "\n";
//			return variables[varname].getFloat();
			variables_t::const_iterator vi=variables.find(varname);
			if(vi!=variables.end()) return vi->second.getString();
			else return 0.0;
		}

		fim::string Namespace::getStringVariable(const fim::string &varname)const
		{
			/*
			 * the variable name supplied is used as a key to the variables hash
			 * */
//			std::cout << "NSGSV:"<<varname<<"\n";
			//return variables[varname].getString();
			variables_t::const_iterator vi=variables.find(varname);
			if(vi!=variables.end())
			{
//				std::cout << "NSGSV:"<<vi->second.getString()<<"\n";
				return vi->second.getString();
			}
			else return "";
		}

	        float Namespace::setGlobalVariable(const fim::string& varname,float value)
		{
			return cc.setVariable(varname,value);
		}

		int Namespace::setGlobalVariable(const fim::string& varname,int value)
		{
			return cc.setVariable(varname,value);
		}

		int Namespace::setGlobalVariable(const fim::string& varname,const char*value)
		{
			return cc.setVariable(varname,value);
		}

		int Namespace::getGlobalIntVariable(const fim::string &varname)const
		{
			return cc.getIntVariable(varname);
		}

		float Namespace::getGlobalFloatVariable(const fim::string &varname)
		{
			return cc.getFloatVariable(varname);
		}

		fim::string Namespace::getGlobalStringVariable(const fim::string &varname)
		{
			return cc.getStringVariable(varname);
		}

		fim::string Namespace::autocmd_exec(const fim::string &event,const fim::string &fname)
		{
#ifdef FIM_AUTOCMDS
			return cc.autocmd_exec(event,fname);
#else
			return "";
#endif
		}
}

