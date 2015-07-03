/* $LastChangedDate: 2015-02-12 18:27:46 +0100 (Thu, 12 Feb 2015) $ */
/*
 Namespace.h : Namespace class headers

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

#ifndef FIM_NAMESPACE_H
#define FIM_NAMESPACE_H

#include "fim.h"

namespace fim
{
	typedef std::map<const fim::string,Var> variables_t;	//id->var
	class Namespace
	{
		protected:
#ifndef FIM_INDIPENDENT_NAMESPACE
		CommandConsole*rnsp_; // root Namespace pointer
#endif /* FIM_INDIPENDENT_NAMESPACE */
		variables_t variables_;	//id->var
		fim_char_t ns_char_; // ns_char_ ':' varname
	
		public:

		fim_int setVariable(const fim::string& varname,fim_int value);
		fim_float_t setVariable(const fim::string& varname,fim_float_t value);
		fim_int setVariable(const fim::string& varname,const fim_char_t*value);
		Var setVariable(const fim::string& varname,const Var&value);

		fim_int getIntVariable(const fim::string &varname)const;
		fim_float_t getFloatVariable(const fim::string &varname)const;
		fim::string getStringVariable(const fim::string &varname)const;
		Var getVariable(const fim::string &varname)const;

		fim_bool_t isSetVar(const fim::string &varname)const;

		fim_int  setGlobalVariable(const fim::string& varname,fim_int value);
	        fim_float_t setGlobalVariable(const fim::string& varname,fim_float_t value);
		fim_int setGlobalVariable(const fim::string& varname,const fim_char_t*value);

		fim_int getGlobalIntVariable(const fim::string &varname)const;
		fim_float_t getGlobalFloatVariable(const fim::string &varname)const;
		fim::string getGlobalStringVariable(const fim::string &varname)const;
		fim::string autocmd_exec(const fim::string &event,const fim::string &fname);
		fim::string get_variables_list(bool with_values=false)const;
		virtual size_t byte_size(void)const = 0;

		Namespace(
#ifndef FIM_INDIPENDENT_NAMESPACE
				CommandConsole *rnsp = NULL,
#endif /* FIM_INDIPENDENT_NAMESPACE */
			       	const fim_char_t ns_char = FIM_SYM_NULL_NAMESPACE_CHAR
			)
			:
#ifndef FIM_INDIPENDENT_NAMESPACE
		      rnsp_(rnsp),
#endif /* FIM_INDIPENDENT_NAMESPACE */
			variables_(variables_t())
			,ns_char_(ns_char)
	       	{}
		virtual ~Namespace(void){}
		fim_err_t find_matching_list(fim::string cmd, args_t & completions, bool prepend_ns)const;
		std::ostream& print(std::ostream &os)const;
	};
		std::ostream& operator<<(std::ostream &os, const Namespace & ns);
}

#endif /* FIM_NAMESPACE_H */

