/* $LastChangedDate: 2015-02-11 08:49:13 +0100 (Wed, 11 Feb 2015) $ */
/*
 fim_stream.cpp : Textual output facility

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


#include "fim.h"
#include "fim_stream.h"

namespace fim
{
		fim_stream::fim_stream(fim_str_t fd):fd_(fd)
		{
		}

		fim_stream& fim_stream::operator<<(const fim_byte_t* s)
		{
			*this<<(const fim_char_t*)s;
			return *this;
		}


		fim_stream& fim_stream::operator<<(const  fim::string&s)
		{
			*this<<(const  fim_char_t*)(s.c_str());
			return *this;
		}


		fim_stream& fim_stream::operator<<(float f)
		{
			fim_char_t s[FIM_ATOX_BUFSIZE];sprintf(s,"%f",f);
			*this<<(const fim_char_t*)s;
			return *this;
		}

		fim_stream& fim_stream::operator<<(int i)
		{
			fim_char_t s[FIM_ATOX_BUFSIZE];sprintf(s,"%d",i);
			*this<<s;
			return *this;
		}

#if FIM_WANT_LONG_INT
		fim_stream& fim_stream::operator<<(fim_int i)
		{
			/* FIXME */
			fim_char_t s[FIM_ATOX_BUFSIZE];
			if(sizeof(fim_int)==sizeof(int))
				sprintf(s,"%d",(int)i);
			else
				sprintf(s,"%lld",(long long int)i);
			*this<<s;
			return *this;
		}
#endif /* FIM_WANT_LONG_INT */

		fim_stream& fim_stream::operator<<(const  fim_char_t* s)
		{
			if(s)
			{
				if(fd_<=-1)
					cc.status_screen(s);
				else
				{
					// 0 == dumb (no output)
					if(fd_==1)
						std::cout << s ;
					else
					{
						if(fd_>=2)
							std::cerr << s ;
					}
				}
			}
			//else if(s)printf("%s",s);

			return *this;
		}
}

