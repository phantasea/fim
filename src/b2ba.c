/*
 (c) 2011-2013 Michele Martone

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
#include <stdio.h>
int main(int argc,char *argv[])
{
	/* read binary data from stdin, dump a C array with byte data on stdout */
	unsigned int byte=0,bc=0;
	printf("{\n");
	while(read(0,&byte,1)==1)
		printf("0x%02x,%s",byte,(++bc)%16?"":"\n");
	printf("\n};\n");
	return 0;
}

