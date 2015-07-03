/* $LastChangedDate: 2013-11-06 18:43:59 +0100 (Wed, 06 Nov 2013) $ */
/*
 Var.cpp : 

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
#include "fim.h"
namespace fim
{
	typedef std::map<fim::string, fim::string> fim_var_help_t;//variable id -> variable help
	static fim_var_help_t fim_var_help_db;	/* this is the global help db for fim variables */

	void fim_var_help_db_init(void)
	{
		/* The inclusion of the next file is not essential : it serves only to populate the variables help database. */
		#define FIM_WANT_INLINE_HELP 1
		#include "help.cpp"
		#undef  FIM_WANT_INLINE_HELP
		;/* freebsd 7.2 cc dies without */
	}

	fim::string fim_var_help_db_query(const fim::string &id)
	{
		string hs = fim_var_help_db[id];
		if(hs==FIM_CNS_EMPTY_STRING)
			return "the help system for variables is still incomplete";
		else
			return hs;
	}

	fim::string fim_get_variables_reference(FimDocRefMode refmode)
	{
		string s =FIM_CNS_EMPTY_STRING;
		fim_var_help_t::const_iterator vi;
		if(refmode==Man)
			goto manmode;
		for( vi=fim_var_help_db.begin();vi!=fim_var_help_db.end();++vi)
		{
			s+=vi->first;
			s+=" : ";
			s+=fim_var_help_db_query(vi->first);
			s+="\n";
		}
		return s;
manmode:
		for( vi=fim_var_help_db.begin();vi!=fim_var_help_db.end();++vi)
		{
			s+=".B\n";
			s+=vi->first;
			s+="\n";
			s+=fim_var_help_db_query(vi->first);
			s+="\n";
			s+=".fi\n";
		}
		return s;
	}

	std::ostream& Var::print(std::ostream &os)const
	{
		return os << this->getString();
	}

	std::ostream& operator<<(std::ostream &os, const Var & var)
	{
		return var.print(os);
	}
}
