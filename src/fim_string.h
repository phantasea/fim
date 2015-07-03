/* $LastChangedDate: 2015-02-11 08:43:28 +0100 (Wed, 11 Feb 2015) $ */
#ifndef FIM_STRING_H
#define FIM_STRING_H
/*
 string.h : Fim's own string implementation header file

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
	static fim_char_t * fim_dupstr (const fim_char_t* s);

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
	fim_char_t*s;		/* the string : can be NULL */
	int len;	/* the allocated amount */
	std::string ss;
#else /* _FIM_DYNAMIC_STRING */
	fim_char_t s[TOKSIZE];
#endif /* _FIM_DYNAMIC_STRING */
	public :
	void _string_init(void);

	int reallocate(int l);

	int reset(int l);

	bool isempty(void)const;

	virtual ~string(void);//virtual, as -Weffc++ suggests
	string(void);
	string(const string& s);
	string(const fim_char_t *str);
	string(const int i);
	string(const unsigned int i);
	const fim_char_t*c_str(void)const;
	bool operator==(const string& s)const;
	bool operator==(const fim_char_t *  s)const;
	bool operator!=(const string& s)const;
	bool operator<=(const string& s)const;
	bool operator>=(const string& s)const;
	bool operator <(const string& s)const;
	bool operator >(const string& s)const;
	bool operator >(const fim_char_t *s)const;
	bool operator <(const fim_char_t *s)const;

	string& operator =(const string& s);
	string operator+=(const string& s);
	string operator+(const string& s)const;
	int  reinit(const int n)const;
	int  length(void)const;
	static int  max_string(void){return TOKSIZE-1;}
	int  size(void)const;
	int  find(const string&str)const;
	int  assign(const string&str);
	int  assign(const fim_char_t*str);
	int  find(const fim_char_t*ss)const;
 	std::ostream& print(std::ostream &os)const;
//	int operator=(int &i,const string& s){i=-1;return i;}
	operator int(void)const;
	operator float(void)const;
	};
#else /* _FIM_STRING_WRAPPER */
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
		string(const std::string&s):std::string(s){}
		string(const fim_char_t*s):std::string(s?s:""){}

		string(fim_char_t c);
#if FIM_WANT_LONG_INT
		string(int i);
#endif /* FIM_WANT_LONG_INT */
		string(fim_int i);
		string(float i);
		string(int * i);
		string(size_t i);

/*
 		the following two operators are very nice to use but pose unexpected problems.		
*/
 		operator fim_int  (void)const;
#if FIM_WANT_LONG_INT
 		operator int  (void)const;
#endif /* FIM_WANT_LONG_INT */
		operator float(void)const;

		string operator+(const string s)const;
		/* copy constructor */
		string(const string& s);
		bool re_match(const fim_char_t*r)const;
		void substitute(const fim_char_t*r, const fim_char_t* s, int flags=0);
		fim::string line(int ln)const;
		size_t lines(void)const;
		int find_re(const fim_char_t*r,int *mbuf=NULL)const;
	};


#endif /* _FIM_STRING_WRAPPER */
}


#endif /* FIM_STRING_H */
