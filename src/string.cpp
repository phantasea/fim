/* $Id: string.cpp 245 2009-04-28 21:28:38Z dezperado $ */
/*
 string.cpp : A reimplementation of string class

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
#include "fim.h"

namespace fim
{
#ifndef _FIM_STRING_WRAPPER
	/*
	 *	FIX ME : 
	 *	this string should be dynamic as soon as possible,
	 *	without breaking the rest of the program.
	 */

	void string::_string_init()
	{	
		/* the string is initialized as unallocated and blank */
#ifdef _FIM_DYNAMIC_STRING
		s=NULL;
		len=0;
#else
		*s='\0';
#endif
	}

	std::ostream& operator<<(std::ostream &os,const string& s)
	{
		return s.print(os);
	}

	std::ostream& operator<<(std::ostream &os, const std::vector<fim::string> & v)
	{
		os<<"{";
		for(size_t i=0;i<v.size();++i)
			os<<v[i]<<",";
		os<<"}";
		return os;
	}

	std::ostream& operator<<(std::ostream &os, const Browser & b)
	{
		return b.print(os);
	}

	string::string()
	{
		_string_init();
		/* no allocation is necessary for an empty string */
	}

	string::string(const string& s)
	{
		_string_init();
		this->assign(s.c_str());
	}

	string::string(const char *str)
	{
		_string_init();
		this->assign(str);
	}

	string::~string()
	{
#ifdef _FIM_DYNAMIC_STRING
		if(s && len==0){std::cout <<"anomalia\n";exit(-1);}

		if(s)
		{
			fim_free(s);
		}
#else
#endif
	}

	string::string(const int i)
	{
		_string_init();
		char buf[FIM_CHARS_FOR_INT];
		sprintf(buf,"%d",i);
		assign(buf);
	}

	string::string(const unsigned int i)
	{
		_string_init();
		char buf[FIM_CHARS_FOR_INT];
		sprintf(buf,"%u",i);
		assign(buf);
	}

	const char*string::c_str()const
	{
		if(  this->isempty() == true ) return "";
		return s;	/* yes, a heap allocated reference */
	}

	/*
	 * null or empty is the same for us
	 */
	bool string::operator==(const string& s)const
	{
		return ((*this) == s.c_str());
	}

	bool string::operator==(const char *  s)const
	{
		/* both empty ? */
		if(fim_empty_string(s) && this->isempty())return true;
		/* if one only is empty then are not equal */
		if(fim_empty_string(s) || this->isempty())return false;
		return strcmp(this->s,  s)==0;
	}

	bool string::operator!=(const string& s)const
	{
		return !((*this)==s);
	}

	bool string::operator<=(const string& s)const
	{	
		return ((*this)<s)||((*this)==s);
	}

	bool string::operator>=(const string& s)const
	{
		return ((*this)>s)||((*this)==s);
	}

	bool string::operator <(const string& s)const
	{
		return ((*this)<s.c_str());
	}

	bool string::operator >(const string& s)const
	{
		return ((*this)>s.c_str());
	}

	bool string::operator >(const char *s)const
	{
		if(this->isempty())return false;
		if(!s || !*s)return true;
		/* this is not an empty string */
		return (strcmp(this->s,s) >0);
	}

	bool string::operator <(const char *s)const
	{
		if(this->isempty())
		{
			if( s && *s )return true;
			else return false;
		}
		/* this is not an empty string */
		if(s && !*s)return false;
		return (strcmp(this->s,s) <0);
	}

	string& string::operator =(const string& s)
	{
		assign(s);
		return *this;
	} 

	/*
	 * essentially, realloc for a l chars long string
	 *
	 * returns the new allocated size
	 * in case of allocation error, returns the available allocated space (could be >= 0)
	 * */
	int string::reallocate(int l=0)
	{
		if(l<=0)return reset(0);
#ifdef _FIM_DYNAMIC_STRING
		++l;
		if(l<size())return size();
		char *ns;/* new string */
		ns=(char*)realloc((void*)s,l);/* the terminator is our stuff */
		if(ns)
		{
			s=ns;
			if(l>len)memset(s+len,0,l-len);
			len=l;
		}
		//if realloc fails, we keep the old one
#else
#endif
		return this->size();
	}

	string string::operator+=(const string& s)
	{

		int r,n;// returned, needed
		n = ( this->length() + s.length() );
		r = this->reallocate( n );
		if(r < n || r==0)
		{
			return *this; // ! FIXME !
		}
		int tlen=strlen(this->s),
		    slen=strlen(s.c_str()),
		    flen=this->size()-1-slen-tlen;
		strncat(this->s+tlen,s.c_str(),slen);
		return string(*this);
	} 

	/* allocates a new object and returns it */
	string string::operator+(const string& s)const
	{
		string res(*this);
		res+=s;
		return res;
	}

	/* reports the effective used space */
	int  string::length()const
	{
		if(isempty())return 0;
		return strlen(s);
	}

	/* reports the effective allocated space */
	int  string::size()const
	{
#ifdef _FIM_DYNAMIC_STRING
		if(!s)return 0;
		return len;
#else
		return sizeof(s);
#endif
	}

	int  string::find(const string&str)const
	{
		return find(str.c_str());
	}

	/*
	 * returns the new length
	 * */
	int  string::assign(const string&str)
	{
		return assign(str.c_str());
	}

	/*
	 * returns the new length
	 * */
	int  string::assign(const char *s)
	{
		int l,r;
		if(!s || !*s)	// length is zero in these cases
		{
			return this->reset(0);
		}
		l=strlen(s);//at least 1, as the string is not empty

		/*
		 * the string is reset, and should allocate some bytes for us (or have already allocated)..
		 * */
		if((r=reset(l))<1 || l<1 )
		{
			return this->reset(0);
		}

		strncpy(this->s,s,r);	//r can be longer than strlen(s)

		this->s[max((min(r-1,l)),0)]='\0';	/* remember r is the size of the allocated space */

		return (this->size());	/*  the effective allocated memory */
	}
	
	/*
	 * empty or null string is always found
	 * */
	int  string::find(const char*ss)const
	{
		if( this->isempty() && !ss      )return 0;
		if( this->isempty() && *ss!='\0')return -1;
		if(!this->isempty() && *ss=='\0')return 0;

		const char*p=strstr(s,ss);if(!p)return -1;return p-s;
	}
 	
	std::ostream& string::print(std::ostream &os)const
	{
		if(this->isempty())return os;
		return os<<this->s;
	}

	/*
	 * a string is empty if NULL or allocated to zero bytes (if possible)
	 * */
	bool string::isempty()const
	{
#ifdef _FIM_DYNAMIC_STRING
		return (s==NULL || len==0 || *s=='\0');
#else
		return *s=='\0';
#endif
	}

	/* 
	 * make room for a string long at most l
	 *
	 * moreover, the allocated space is cleared and the maximum
	 * returns the new allocated size
	 * in case of allocation error, returns the available allocated space (could be >= 0)
	 */
	int string::reset(int l)
	{
		l=min(l,max_string());	/* etica professionale */

#ifdef _FIM_DYNAMIC_STRING
		//blanking
		if(s){fim_free(s); s=NULL;len=0;}
		//do we need some allocation? only if l was already > 0
		if(l+1<len && s)
		{
			memset(s,0,l+1);
		}else
		if(l++>0)
		{

#if 1
			// this is a sort of buffering
//			#define BUFSIZE 64
			#define BUFSIZE TOKSIZE
			l=(l<BUFSIZE?BUFSIZE:l);
#endif

			s=(char*)(fim_calloc(l));

			len=(s?l:0);	/* who knows .. */
		}
		else
		{
			/* we keep the string blanked  and we are happy */
		}
#else
		*s='\0';
#endif
		return (this->size());
	}
#else
	std::ostream& operator<<(std::ostream &os,const string& s)
	{
		return os << s.c_str();
	}

	std::ostream& operator<<(std::ostream &os, const std::vector<fim::string> & v)
	{
		std::cout<<"{";
		for(size_t i=0;i<v.size();++i)
			std::cout<<v[i]<<",";
		std::cout<<"}";
		return os;
	}

	std::ostream& operator<<(std::ostream &os, const Browser & b)
	{
		return os;
	}

	//string::string(const string& s):std::string(s.c_str())
	string::string(const string& s):std::string(s)
	{
	}

	string::string(int i)
	{
		char buf[FIM_CHARS_FOR_INT];
		snprintf(buf,FIM_CHARS_FOR_INT-1,"%d",i);
		buf[FIM_CHARS_FOR_INT-1]='\0';
		append(buf);
	}

	string::string(int * i)
	{
		char buf[FIM_CHARS_FOR_INT];
		snprintf(buf,FIM_CHARS_FOR_INT-1,"%p",i);
		buf[FIM_CHARS_FOR_INT-1]='\0';
		append(buf);
	}
	
	/*
	 * see the next commented declaration ? it is pure evil, do not use it !
	 *
	#define	FIM_STRING_INITIAL_LENGTH 64
	string::string():std::string(FIM_STRING_INITIAL_LENGTH,'\0'){}
	*/
	string::string():std::string(""){}

	string string::operator+(const string s)const
	{
		string res(*this);
		res+=s.c_str();
		return string(res);
	}

	bool string::re_match(const char*r)const
	{
		/*
		 * each occurrence of regular expression r will be substituted with t
		 *
		 * FIXME : return values could be more informative
		 * */
		regex_t regex;
		const int nmatch=1;
		regmatch_t pmatch[nmatch];

		if( !r || !*r )
			return false;

		if( regcomp(&regex,r, 0 | REG_EXTENDED | REG_ICASE ) != 0 )
			return false;

		if(regexec(&regex,c_str(),nmatch,pmatch,0)==0)
		{
			regfree(&regex);
			return true;
		}
		regfree(&regex);
		return false;
	}

	int string::find_re(const char*r, int *mbuf)const
	{
		/*
		 * each occurrence of regular expression r will be substituted with t
		 *
		 * FIXME : return values could be more informative
		 * NOTE: mbuf is int, but pmatch->rm_so and pmatch->rm_eo are regoff_t from regex.h
		 * */
		regex_t regex;
		const size_t nmatch=1;
		regmatch_t pmatch[nmatch];

		if( !r || !*r )
			return -1;

		if( regcomp(&regex,r, 0 | REG_EXTENDED | REG_ICASE ) != 0 )
			return -1;

		if(regexec(&regex,c_str(),nmatch,pmatch,0)==0)
		{
			regfree(&regex);
			if(mbuf)
			{
				mbuf[0]=pmatch->rm_so;
				mbuf[1]=pmatch->rm_eo;
			}
			return pmatch->rm_so;
		}
		regfree(&regex);
		return -1;
	}

	void string::substitute(const char*r, const char* s, int flags)
	{
		/*
		 * each occurrence of regular expression r will be substituted with t
		 *
		 * FIXME : return values could be more informative
		 * */
		regex_t regex;
		const int nmatch=1;
		regmatch_t pmatch[nmatch];

		if( !r || !*r || !s )
			return;

		if( regcomp(&regex,r, 0 | REG_EXTENDED | REG_ICASE | flags ) != 0 )
			return;

		int off=0;
		//const int s_len=strlen(s);
		while(regexec(&regex,off+c_str(),nmatch,pmatch,0)==0)
		{
			/*
			 * please note that this is not an efficient subsitution implementation.
			 * */
			std::string ss = substr(0,pmatch->rm_so);
			ss+=s;
			ss+=substr(pmatch->rm_eo,this->size());
			*this=ss.c_str();

		}
		regfree(&regex);
		return;
	}

	size_t string::lines()const
	{
		/*
		 * each empty line will be counted unless it is the last and not only.
		 * */
		const char*s=c_str(),*f=s;
		size_t c=0;
		if(!s)return 0;
		while((s=strchr(s,'\n'))!=NULL){++c;f=++s;}
		return c+(strlen(f)>0);
	}

	fim::string string::line(int ln)const
	{
		/*
		 * returns the ln'th line of the string, if found, or ""
		 * */
		const char*s,*f;
		s=this->c_str();
		f=s;
		if(ln< 0 || !s)return "";
		while( ln && ((f=strchr(s,'\n'))!=NULL ) )
		{
			--ln;
			s=f+1;
		}
		if(!ln)
		{
			const char *se=s;
			if(!*s)se=s+strlen(s);
			else se=strchr(s,'\n');
			fim::string rs;
			rs=substr(s-this->c_str(),se-s).c_str();
			return rs;
		}
		return "";
	}
#endif
}

