/* $Id: readline.h 135 2008-05-06 20:41:10Z dezperado $ */
/*
 readline.h : Code dealing with the GNU readline library.

 (c) 2008 Michele Martone

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
#ifndef FIM_READLINE_H
#define FIM_READLINE_H

#include <readline/readline.h>	/*	the GNU readline library	*/
#include <readline/history.h> 	/*	the GNU readline library	*/
#include <readline/keymaps.h> 	/*	the GNU readline library	*/

namespace rl
{
	void initialize_readline ();
	char * command_generator (const char *text,int state);
}

#endif

