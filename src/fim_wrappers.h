/* $LastChangedDate: 2014-11-17 19:22:17 +0100 (Mon, 17 Nov 2014) $ */
/*
 fim_wrappers.h : Some wrappers

 (c) 2011-2014 Michele Martone

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

#ifndef FIM_WRAPPERS_H
#define FIM_WRAPPERS_H
namespace fim
{
/* symbolic wrappers for memory handling calls */
#define fim_calloc(x,y) calloc((x),(y)) /* may make this routine aligned in the future */
#define fim_stralloc(x) (fim_char_t*) calloc((x),(1)) /* ensures that first char is NUL */
#define fim_malloc(x) malloc(x)
#define fim_free(x) free(x)
#define fim_memset(x,y,z) memset(x,y,z)
#define fim_bzero(x,y) fim_memset(x,0,y)
//#define fim_bzero(x,y) bzero(x,0,y)
}
#endif /* FIM_WRAPPERS_H */
