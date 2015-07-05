/* $Id: Var.cpp 245 2009-04-28 21:28:38Z dezperado $ */
/*
 Var.cpp : 

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
	typedef std::map<fim::string, fim::string> fim_var_help_t;//variable id -> variable help
	static fim_var_help_t fim_var_help_db;	/* this is the global help db for fim variables */

	void Var::var_help_db_init()
	{
		/* The inclusion of the next file is not essential : it serves only to populate the variables help database. */
		#define FIM_WANT_INLINE_HELP 1
		#include "help.cpp"
		#undef  FIM_WANT_INLINE_HELP
		;/* freebsd 7.2 cc dies without */
	}

	fim::string Var::var_help_db_query(const fim::string &id)
	{
		string hs = fim_var_help_db[id];
		if(hs=="")
			return "the help system for variables is still incomplete";
		else
			return hs;
	}

	fim::string Var::get_variables_reference()
	{
		string s ="";
		
		fim_var_help_t::const_iterator vi;
		for( vi=fim_var_help_db.begin();vi!=fim_var_help_db.end();++vi)
		{
			s+=vi->first;
			s+=" : ";
			s+=Var::var_help_db_query(vi->first);
			s+="\n";
		}
		return s;
	}
}
