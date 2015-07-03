/* $LastChangedDate: 2015-02-13 00:24:59 +0100 (Fri, 13 Feb 2015) $ */
/*
 CommandConsole-var.h : CommandConsole variables store

 (c) 2013-2015 Michele Martone

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

	fim::string CommandConsole::rnid(const fim::string & varname)const
	{
		fim::string id;
#ifdef FIM_NAMESPACES
		if( varname[1]==FIM_SYM_NAMESPACE_SEP && varname.length() > 2 )
		{
			if( varname.length() > 2 )
				id = varname.c_str()+2;
		}
		else
#endif /* FIM_NAMESPACES */
			id = varname;
		return id;
	}

	Namespace * CommandConsole::rns(const fim::string varname)
	{
		Namespace * nsp = NULL;
#ifdef FIM_NAMESPACES
		if( varname[1]==FIM_SYM_NAMESPACE_SEP )
		{
			try
			{
			//a specific namespace was selected!
			fim_char_t ns = varname[0];
			fim::string id = varname.c_str()+2;

			if( ns == FIM_SYM_NAMESPACE_WINDOW_CHAR )
#ifdef FIM_WINDOWS
			{
				//window variable
				nsp = window_;
				goto err;
			}
			else
			if( ns == FIM_SYM_NAMESPACE_VIEWPORT_CHAR )
			{
				//viewport variable
				if(window_)
					nsp = window_->current_viewportp();
				goto err;
			}
			else
#endif /* FIM_WINDOWS */
			if( ns == FIM_SYM_NAMESPACE_IMAGE_CHAR )
			{
				//image variable
				nsp = (Image*) browser_.c_image(); /* FIXME: porcata assurda */
				goto err;
			}
			else
			if( ns == FIM_SYM_NAMESPACE_BROWSER_CHAR )
			{
				//browser variable
				nsp = & browser_;
				goto err;
			}
			else
			if( ns == FIM_SYM_NAMESPACE_GLOBAL_CHAR )
			{
				nsp = (Namespace*) this;
				goto err;
			}
			else
			if( ns != FIM_SYM_NAMESPACE_GLOBAL_CHAR )
			{
				//invalid namespace
				goto err;
			}
			}
			catch(FimException e){}
		}
#endif /* FIM_NAMESPACES */
		nsp = this;
err:
		return nsp;
	}

	const Namespace * CommandConsole::c_rns(const fim::string varname)const
	{
		const Namespace * nsp = NULL;
#ifdef FIM_NAMESPACES
		if( varname[1]==FIM_SYM_NAMESPACE_SEP )
		{
			try
			{
			//a specific namespace was selected!
			fim_char_t ns = varname[0];
			fim::string id = varname.c_str()+2;

			if( ns == FIM_SYM_NAMESPACE_WINDOW_CHAR )
#ifdef FIM_WINDOWS
			{
				//window variable
				nsp = window_;
				goto err;
			}
			else
			if( ns == FIM_SYM_NAMESPACE_VIEWPORT_CHAR )
			{
				//viewport variable
				if(window_)
					nsp = window_->current_viewportp();
				goto err;
			}
			else
#endif /* FIM_WINDOWS */
			if( ns == FIM_SYM_NAMESPACE_IMAGE_CHAR )
			{
				//image variable
				nsp = (Image*) browser_.c_image(); /* FIXME: porcata assurda */
				goto err;
			}
			else
			if( ns == FIM_SYM_NAMESPACE_BROWSER_CHAR )
			{
				//browser variable
				nsp = & browser_;
				goto err;
			}
			else
			if( ns == FIM_SYM_NAMESPACE_GLOBAL_CHAR )
			{
				nsp = (Namespace*) this;
				goto err;
			}
			else
			if( ns != FIM_SYM_NAMESPACE_GLOBAL_CHAR )
			{
				//invalid namespace
				goto err;
			}
			}
			catch(FimException e){}
		}
#endif /* FIM_NAMESPACES */
		nsp = this;
err:
		return nsp;
	}

	fim_int CommandConsole::setVariable(const fim::string& varname,fim_int value)
	{
		fim_int retval = 0;
		Namespace *nsp = rns(varname);

		if(nsp)
			retval = nsp->setVariable(rnid(varname),value);
err:
		return retval;
	}

	fim_float_t CommandConsole::setVariable(const fim::string& varname,fim_float_t value)
	{
		fim_float_t retval = FIM_CNS_EMPTY_FP_VAL;
		Namespace *nsp = rns(varname);

		if(nsp)
			retval = nsp->setVariable(rnid(varname),value);
err:
		return retval;
	}

	fim_int CommandConsole::setVariable(const fim::string& varname,const fim_char_t*value)
	{
		fim_int retval = 0;
		Namespace *nsp = rns(varname);

		if(nsp)
			retval = nsp->setVariable(rnid(varname),value);
err:
		return retval;
	}

	Var CommandConsole::setVariable(const fim::string varname,const Var&value)
	{
		Var retval = (fim_int)0;
		Namespace *nsp = rns(varname);

		if(nsp)
			retval = nsp->setVariable(rnid(varname),value);
err:
		return retval;
	}

	fim_int CommandConsole::getIntVariable(const fim::string &varname)const
	{
		fim_int retval = 0;
		const Namespace *nsp = c_rns(varname);

		if(nsp)
			retval = nsp->getIntVariable(rnid(varname));
err:
		return retval;
	}

	fim_float_t CommandConsole::getFloatVariable(const fim::string &varname)const
	{
		fim_float_t retval = FIM_CNS_EMPTY_FP_VAL;
		const Namespace *nsp = c_rns(varname);

		if(nsp)
			retval = nsp->getFloatVariable(rnid(varname));
		return retval;
	}

	fim::string CommandConsole::getStringVariable(const fim::string &varname)const
	{
		fim::string retval = FIM_CNS_EMPTY_RESULT;
		const Namespace *nsp = c_rns(varname);

		if(nsp)
			retval = nsp->getStringVariable(rnid(varname));
		return retval;
	}

	Var CommandConsole::getVariable(const fim::string &varname)const
	{
		const Namespace *nsp = c_rns(varname);

		if(nsp)
			return nsp->getVariable(rnid(varname));
		else
			return Var();
	}
}
