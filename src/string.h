/* $Id: string.h 269 2009-12-08 23:45:10Z dezperado $ */
#ifndef FIM_STRING_H
#define FIM_STRING_H
/*
 string.h : Fim's own string implementation header file

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

/*
 * the rest of the program waits still for a proper adaptation of dynamic strings.
 */
//#define _FIM_DYNAMIC_STRING 1
#define _FIM_STRING_WRAPPER 1


#define FIM_CHARS_FOR_INT 32 /* should fit a pointer address printout */

namespace fim
{
#ifndef _FIM_STRING_WRAPPER

#define fim_free(x) {free(x);}
//#define fim_free(x) {std::cout<<"freeing "<<(int*)x<<"\n";free(x);x=NULL;std::cout<<"freeed!\n";}
#define fim_realloc(x,n) realloc((x),(n))
#define fim_empty_string(s) (!(s) || !(*(s)))

	/*
	 *	Allocation and duplication of a single string
	 */
	static char * fim_dupstr (const char* s);

	class string{
	/*
	 * this is a declaration of my love to the STL.. (i wrote this dumb code 
	 * in dumb 10 minutes after spending dumb hours messing with the original
	 * STL string template .. ) ( am i dumb ? ghghhh.. probably .. )
	 *
	 * FIX ME : all of this should be made dynamic, but with the right semantics.
	 */
        static const int TOKSIZE=128*8*4*2;	//max len.NUL included
#ifdef _FIM_DYNAMIC_STRING
	char*s;		/* the string : can be NULL */
	int len;	/* the allocated amount */
	std::string ss;
#else
	char s[TOKSIZE];
#endif
	public :
	void _string_init();

	int reallocate(int l);

	int reset(int l);

	bool isempty()const;

	virtual ~string();//virtual, as -Weffc++ suggests
	string();
	string(const string& s);
	string(const char *str);
	string(const int i);
	string(const unsigned int i);
	const char*c_str()const;
	bool operator==(const string& s)const;
	bool operator==(const char *  s)const;
	bool operator!=(const string& s)const;
	bool operator<=(const string& s)const;
	bool operator>=(const string& s)const;
	bool operator <(const string& s)const;
	bool operator >(const string& s)const;
	bool operator >(const char *s)const;
	bool operator <(const char *s)const;

	string& operator =(const string& s);
	string operator+=(const string& s);
	string operator+(const string& s)const;
	int  reinit(const int n)const;
	int  length()const;
	static int  max_string(){return TOKSIZE-1;}
	int  size()const;
	int  find(const string&str)const;
	int  assign(const string&str);
	int  assign(const char*str);
	int  find(const char*ss)const;
 	std::ostream& print(std::ostream &os)const;
//	int operator=(int &i,const string& s){i=-1;return i;}
	operator int()const{return atoi(s);}
	operator float()const{return fim_atof(s);}
	};
#else
	class string:public std::string
	{
		public:
		string();

		/* a virtual destructor will behave correctly when destroying this class
		 * objects with base pointers .. */
		~string(){}

		/*
			 if not, exception:
			 terminate called after throwing an instance of 'std::logic_error'
			 what():  basic_string::_S_construct NULL not valid
		*/
		string(const char*s):std::string(s?s:""){}

		string(int i);
		string(int * i);

/*
 		the following two operators are very nice to use but pose unexpected problems.		
*/
 		operator int  ()const{return atoi(this->c_str());}
		operator float()const{return fim_atof(this->c_str());}

		string operator+(const string s)const;
		/* copy constructor */
		string(const string& s);
		bool re_match(const char*r)const;
		void substitute(const char*r, const char* s, int flags=0);
		fim::string line(int ln)const;
		size_t lines()const;
		int find_re(const char*r,int *mbuf=NULL)const;
	};


#endif
}


#endif
