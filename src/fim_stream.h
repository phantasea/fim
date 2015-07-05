/* $Id: fim_stream.h 203 2009-01-10 11:13:17Z dezperado $ */
/*
 fim_stream.h : Textual output facility

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
#ifndef FIM_FIM_STREAM_H
#define FIM_FIM_STREAM_H
#include "fim.h"
namespace fim
{
	/*
	 * this class is a stream used in Fim and should output to an internal console.
	 *
	 * TODO: error and to file dump. maybe, some day.
	 *	 move here the console handling functionalities.
	 * */

	class fim_stream
	{
		public:
		fim_stream(){}

		fim_stream& operator<<(const  char* s);

		fim_stream& operator<<(const unsigned char* s);

		fim_stream& operator<<(const  fim::string&s);

		fim_stream& operator<<(float f);

		fim_stream& operator<<(int i);

	};
}
#endif
