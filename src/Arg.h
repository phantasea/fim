/* $Id: Arg.h 203 2009-01-10 11:13:17Z dezperado $ */
/*
 Arg.h : Argument class

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
#ifndef FIM_ARG_H
#define FIM_ARG_H
#include "fim.h"
namespace fim
{
class Arg
{
	public:
	fim::string val;
	Arg(const fim::string &s):val(s){}
	Arg(const Arg &arg):val(arg.val){}
	bool operator==(int i){int v=atoi(val.c_str());return v==i;}
	bool operator<=(int i){int v=atoi(val.c_str());return v<=i;}
	bool operator>=(int i){int v=atoi(val.c_str());return v>=i;}
	bool operator <(int i){int v=atoi(val.c_str());return v <i;}
	bool operator >(int i){int v=atoi(val.c_str());return v >i;}
};


}
#endif
