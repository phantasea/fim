/* $LastChangedDate: 2015-04-18 21:37:25 +0200 (Sat, 18 Apr 2015) $ */
/*
 CommandConsole-help.cpp : Fim console dispatcher--help methods

 (c) 2011-2015 Michele Martone

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
	fim::string CommandConsole::get_variables_reference(FimDocRefMode refmode)const
	{
		/*
		 * returns the reference of registered functions
		 */
		fim::string s;
		fim::string sep=" ";
		s+= fim_get_variables_reference(refmode);

		// FIXME: shall fix the following to work
#if 0
#ifdef FIM_NAMESPACES
		s+=browser_.get_variables_list();
		s+=sep;
		if(browser_.c_image())
		{
			s+=browser_.c_image()->get_variables_list();
			s+=sep;
		}
#endif /* FIM_NAMESPACES */
#endif
#if 0
#ifdef FIM_WINDOWS
		if(window_)
		{
			s+=window_->get_variables_list();
			s+=sep;
		}
		if(current_viewport())
		{
			s+=current_viewport()->get_variables_list();
			s+=sep;
		}
#endif /* FIM_WINDOWS */
#endif
		// FIXME: incomplete
		return s;
	}

	fim::string CommandConsole::get_commands_reference(FimDocRefMode refmode)const
	{
		/*
		 * returns the reference of registered commands_
		 * TODO : should write better help messages
		 */
		fim::string s;
		if(refmode==Man)
			goto manmode;
		for(size_t i=0;i<commands_.size();++i)
		{
			s+=(commands_[i]->cmd_);
			s+=" : ";
			s+=(commands_[i])->getHelp();
			s+="\n";
		}
		return s;
manmode:
		for(size_t i=0;i<commands_.size();++i)
		{
			s+=".B\n";
			s+=(commands_[i]->cmd_);
			s+="\n.fi\n";
			s+=(commands_[i])->getHelp();
			s+="\n";
			s+=".fi\n";
			s+="\n";
		}
		return s;
	}

	fim::string CommandConsole::print_commands(void)const
	{
		cout << "VARIABLES : "<<get_variables_list()<<"\n";
		cout << "COMMANDS : "<<get_commands_list()<<"\n";
		cout << "ALIASES : "<<get_aliases_list()<<"\n";
		return FIM_CNS_EMPTY_RESULT;
	}

	fim::string CommandConsole::get_reference_manual(const args_t& args)
	{
		/*
		 * dump textually a reference manual from all the available fim language features.
		 */
#include "grammar.h"
#include "examples.h"
#include "conf.h"
#include "help-acm.cpp"
		FimDocRefMode refmode=DefRefMode;
		if(args.size()==1 && args[0]=="man")
		{
			refmode=Man;
			return
			string(".\\\"\n"
			".\\\" $Id""$\n"
			".\\\"\n"
			".TH fimrc 5 \"(c) 2011-" FIM_CNS_LCY " " FIM_AUTHOR_NAME "\"\n"
			".SH NAME\n"
			"fimrc - \\fB fim \\fP configuration file and language reference\n"
			"\n"
			".SH SYNOPSIS\n"
			".B " FIM_CNS_USR_RC_COMPLETE_FILEPATH "\n.fi\n"
			".B " FIM_CNS_SYS_RC_FILEPATH "\n.fi\n"
			".B fim --" FIM_OSW_SCRIPT_FROM_STDIN " [ {options} ] < {scriptfile}\n.fi\n"
			".B fim --" FIM_OSW_EXECUTE_SCRIPT " {scriptfile} [ {options} ]\n.fi\n"
			".B fim --" FIM_OSW_EXECUTE_COMMANDS " {commands} [ {options} ]\n.fi\n"
			".B fim --" FIM_OSW_FINAL_COMMANDS " {commands} [ {options} ]\n.fi\n"
			".B fim --" FIM_OSW_DUMP_SCRIPTOUT " {scriptfile} [ {options} ]  \n.fi\n"
			".B fim --" FIM_OSW_DUMP_SCRIPTOUT " " FIM_LINUX_STDOUT_FILE " [ {options} ]\n.fi\n"
			"\n"
			".SH DESCRIPTION\n"
			"This page explains the \n.B fim\nlanguage, which is used for the \n.B fimrc\nconfiguration files, {scriptfile}s, or {commands} passed via command line {options}.\n"
			"This language can be used to issue commands (or programs) from the internal program command line accessed interactively through the \"" FIM_SYM_CONSOLE_KEY_STR "\" key (or rather, the key code specified by the \"" FIM_VID_CONSOLE_KEY "\" variable).\n"
			"One may exit from command line mode by pressing the " FIM_KBD_ENTER " key on an empty line (a non empty command line would be submitted for execution), or the " FIM_KBD_ESC " key ")+
#if FIM_WANT_DOUBLE_ESC_TO_ENTER
		       	string(" (in non-SDL mode, it is required to press the " FIM_KBD_ESC " key twice).\n")+
#else /* FIM_WANT_DOUBLE_ESC_TO_ENTER */
		       	string(" (only in SDL mode).\n")+
#endif /* FIM_WANT_DOUBLE_ESC_TO_ENTER */
			string("The general form of a fim command/program is shown in the next section.\n")+
#ifndef FIM_COMMAND_AUTOCOMPLETION
			string("\nInterpretation of commands or aliases may use autocompletion (if enabled; see the " FIM_VID_CMD_EXPANSION " variable description), in a way to allow the user to type only the beginning of the command of interest.\n")+
#endif /* FIM_COMMAND_AUTOCOMPLETION */
			string("\n"
			"\n"
			".SH FIM LANGUAGE GRAMMAR\n"
//			"Incomplete section.\n"
			"This section specifies the grammar of the \n.B fim\nlanguage.\n\n"
			"Language elements surrounded by a single quote (\"'\") are literals.\n\n"
			"Warning: at the present state, this grammar has conflicts. A future release shall fix them.\n"
			"\n")+
			string(FIM_DEFAULT_GRAMMAR_FILE_CONTENTS)+
			string("\n"
			"A STRING can be either a single quoted string or a double quoted string.\n"
			"A FLOAT is a floating point number.\n"
			"A QUOTED_FLOAT is a floating point number, either single (\"'\") or double (\"\"\") quoted.\n"
			"An INTEGER shall be an unsigned integer number.\n"
			"An IDENTIFIER shall be one of the valid fim commands (see \n.B COMMANDS REFERENCE\n) or a valid alias.\n"
			"A VARIABLE shall be an already declared or undeclared variable identifier (see \n.B VARIABLES REFERENCE\n) or a valid alias, created using the \n.B alias\ncommand.\n"
			"The \"=~\" operator treats the right expression as a STRING, and uses is as a regular expression for matching purposes.\n"
			"The SLASH_AND_REGEXP is a slash (\"/\") followed by a STRING, interpreted as a regular expression.\n"
			"See ""\\fR\\fI""regex""\\fR""(1) for regular expression syntax.\n"
			"\n"
			"The way some one-line statements are evaluated:\n\n")+
			string(FIM_INTERNAL_LANGUAGE_SHORTCUT_SHORT_HELP)+
			string("\n")+
			// TODO: shall specify the identifier form
			// TODO: shall specify min and max ranges, signedness
			// TODO: place some working example here.
			// TODO: shall write about the conversion rules.
			string("\n")+
			string(".SH COMMANDS REFERENCE\n")+
			string("\n")+
			get_commands_reference(refmode)+
			string(".SH AUTOCOMMANDS REFERENCE\n"
			"Available autocommands are: "
			FIM_AUTOCOMMANDS_LIST
			" and they are triggered on actions as indicated by their name.\n"
			".SH VARIABLES REFERENCE\n"
			"If undeclared, a variable will evaluate to 0.\n\n"
			"There namespaces in which variables may exist are: " FIM_SYM_NAMESPACE_PREFIXES_DSC ". A namespace is specified by a prefix, which is one of: " FIM_SYM_NAMESPACE_PREFIXES ", which shall be prepended to the variable name. The global namespace is equivalent to the empty one:''. The special variable " FIM_SYM_NAMESPACE_IMAGE_ALL_STR " expands to the collation of all the name-value pairs for the current image.\n"
			"\nIn the following, the [internal] variables are the ones referenced in the source code (not including the hardcoded configuration, which may be inspected and/or invalidated by the user at runtime).\n"
			"\n")+
		       	get_variables_reference(refmode)+
			string(".SH USAGE EXAMPLES\n"
			".nf\n")+
			string(FIM_DEFAULT_EXAMPLE_FILE_CONTENTS)+
			string("\n"
			".SH CONFIGURATION EXAMPLES\n"
			"This is the default configuration, as contained in the " FIM_VID_FIM_DEFAULT_CONFIG_FILE_CONTENTS " variable.\n"
			"\n.nf\n")+
			string(FIM_DEFAULT_CONFIG_FILE_CONTENTS)+
			string("\n")+
//			string("Incomplete section.\n")+
			string(".SH NOTES\n")+
			string("This manual page is incomplete: a number of topics, as type conversions, or operator precedence, or exact command usage is left unspecified.\n")+
			string("The conditions for autocommands triggering are not specified as they should.\n")+
			string("A formal description of the various one-line commands, as well a more extensive example list is needed.\n")+
			string("Many of the listed variables are only valid within a namespace, and this shall be documented clearly.\n")+
			string(".SH BUGS\n"
"The\n.B fim\nlanguage has a number of problems that shall be first documented, then fixed.\n"
			".SH SEE ALSO\n"
			"""\\fR\\fI""fim""\\fR""(1), ""\\fR\\fI""regex""\\fR""(1).\n"
			".SH AUTHOR\n"
			FIM_AUTHOR
			"\n"
			".SH COPYRIGHT\n"
			"See copyright notice in ""\\fR\\fI""fim""\\fR""(1).\n"
			"\n"
			"\n")
			;
		}
		else
		return
			string("commands:\n")+
			get_commands_reference()+
			string("variables:\n")+
			get_variables_reference();
	}

	fim::string CommandConsole::dump_reference_manual(const args_t& args)
	{
		/*
		 * dump textually a reference manual from all the available fim language features.
		 */
		std::cout << get_reference_manual(args);
		return FIM_CNS_EMPTY_RESULT;
	}
}

