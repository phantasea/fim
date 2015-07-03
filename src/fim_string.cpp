/* $LastChangedDate: 2015-02-11 17:51:25 +0100 (Wed, 11 Feb 2015) $ */
/*
 string.cpp : A reimplementation of string class

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

#define FIM_WANT_DEBUG_REGEXP 0

namespace fim
{
#ifndef _FIM_STRING_WRAPPER
	/*
	 *	FIX ME : 
	 *	this string should be dynamic as soon as possible,
	 *	without breaking the rest of the program.
	 */

	void string::_string_init(void)
	{	
		/* the string is initialized as unallocated and blank */
#ifdef _FIM_DYNAMIC_STRING
		s=NULL;
		len=0;
#else
		*s='\0';
#endif /* _FIM_DYNAMIC_STRING */
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

	string::string(void)
	{
		_string_init();
		/* no allocation is necessary for an empty string */
	}

	string::string(const string& s)
	{
		_string_init();
		this->assign(s.c_str());
	}

	string::string(const fim_char_t *str)
	{
		_string_init();
		this->assign(str);
	}

	string::~string(void)
	{
#ifdef _FIM_DYNAMIC_STRING
		if(s && len==0){std::cout <<"anomalia\n";exit(-1);}

		if(s)
		{
			fim_free(s);
		}
#else /* _FIM_DYNAMIC_STRING */
#endif /* _FIM_DYNAMIC_STRING */
	}

	string::string(const int i)
	{
		_string_init();
		fim_char_t buf[FIM_CHARS_FOR_INT];
		sprintf(buf,"%d",i);
		assign(buf);
	}

	string::string(const fim_int i)
	{
		_string_init();
		fim_char_t buf[FIM_CHARS_FOR_INT];
		if(sizeof(fim_int)==sizeof(int))
			sprintf(buf,"%d",(int)i);
		else
			sprintf(buf,"%lld",(long long int)i);
		assign(buf);
	}

	string::string(const unsigned int i)
	{
		_string_init();
		fim_char_t buf[FIM_CHARS_FOR_INT];
		sprintf(buf,"%u",i);
		assign(buf);
	}

	const fim_char_t*string::c_str(void)const
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

	bool string::operator==(const fim_char_t *  s)const
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

	bool string::operator >(const fim_char_t *s)const
	{
		if(this->isempty())return false;
		if(!s || !*s)return true;
		/* this is not an empty string */
		return (strcmp(this->s,s) >0);
	}

	bool string::operator <(const fim_char_t *s)const
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
		fim_char_t *ns;/* new string */
		ns=(fim_char_t*)realloc((void*)s,l);/* the terminator is our stuff */
		if(ns)
		{
			s=ns;
			if(l>len)fim_bzero(s+len,l-len);
			len=l;
		}
		//if realloc fails, we keep the old one
#else /* _FIM_DYNAMIC_STRING */
#endif /* _FIM_DYNAMIC_STRING */
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
	int  string::length(void)const
	{
		if(isempty())return 0;
		return strlen(s);
	}

	/* reports the effective allocated space */
	int  string::size(void)const
	{
#ifdef _FIM_DYNAMIC_STRING
		if(!s)return 0;
		return len;
#else /* _FIM_DYNAMIC_STRING */
		return sizeof(s);
#endif /* _FIM_DYNAMIC_STRING */
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
	int  string::assign(const fim_char_t *s)
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
	int  string::find(const fim_char_t*ss)const
	{
		if( this->isempty() && !ss      )return 0;
		if( this->isempty() && *ss!='\0')return -1;
		if(!this->isempty() && *ss=='\0')return 0;

		const fim_char_t*p=strstr(s,ss);if(!p)return -1;return p-s;
	}
 	
	std::ostream& string::print(std::ostream &os)const
	{
		if(this->isempty())return os;
		return os<<this->s;
	}

	operator string::int()const{return atoi(s);}
	operator string::fim_int()const{return fim_atoi(s);}
	operator string::float()const{return fim_atof(s);}

	/*
	 * a string is empty if NULL or allocated to zero bytes (if possible)
	 * */
	bool string::isempty(void)const
	{
#ifdef _FIM_DYNAMIC_STRING
		return (s==NULL || len==0 || *s=='\0');
#else /* _FIM_DYNAMIC_STRING */
		return *s=='\0';
#endif /* _FIM_DYNAMIC_STRING */
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
			fim_bzero(s,l+1);
		}else
		if(l++>0)
		{

#if 1
			// this is a sort of buffering
			#define BUFSIZE TOKSIZE
			l=(l<BUFSIZE?BUFSIZE:l);
#endif /* _FIM_DYNAMIC_STRING */

			s=(fim_char_t*)(fim_calloc(l));

			len=(s?l:0);	/* who knows .. */
		}
		else
		{
			/* we keep the string blanked  and we are happy */
		}
#else /* _FIM_DYNAMIC_STRING */
		*s='\0';
#endif /* _FIM_DYNAMIC_STRING */
		return (this->size());
	}
#else /* _FIM_STRING_WRAPPER */
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

	string::string(fim_char_t c)
	{
		fim_char_t buf[2];
		buf[0]=c;
		buf[1]='\0';
		append(buf);
	}

#if FIM_WANT_LONG_INT
	string::string(int i)
	{
		fim_char_t buf[FIM_CHARS_FOR_INT];
		snprintf(buf,FIM_CHARS_FOR_INT-1,"%d",i);
		buf[FIM_CHARS_FOR_INT-1]='\0';
		append(buf);
	}
#endif /* FIM_WANT_LONG_INT */

	string::string(fim_int i)
	{
		fim_char_t buf[FIM_CHARS_FOR_INT];
		if(sizeof(fim_int)==sizeof(int))
			snprintf(buf,FIM_CHARS_FOR_INT-1,"%d",(int)i);
		else
			snprintf(buf,FIM_CHARS_FOR_INT-1,"%lld",(long long int)i);
		buf[FIM_CHARS_FOR_INT-1]='\0';
		append(buf);
	}

	string::string(size_t i)
	{
		fim_char_t buf[FIM_CHARS_FOR_INT];
		snprintf(buf,FIM_CHARS_FOR_INT-1,"%zd",i);
		buf[FIM_CHARS_FOR_INT-1]='\0';
		append(buf);
	}

	string::string(int * i)
	{
		fim_char_t buf[FIM_CHARS_FOR_INT];
		snprintf(buf,FIM_CHARS_FOR_INT-1,"%p",(void*)i);
		buf[FIM_CHARS_FOR_INT-1]='\0';
		append(buf);
	}
	
	string::string(float i)
	{
		fim_char_t buf[FIM_ATOX_BUFSIZE];
		sprintf(buf,"%f",i);
		assign(buf);
	}

	string::string():std::string(""){}

	string string::operator+(const string s)const
	{
		string res(*this);
		res+=s.c_str();
		return string(res);
	}

	bool string::re_match(const fim_char_t*r)const
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

#if FIM_WANT_LONG_INT
 	string::operator int  (void)const{return atoi(this->c_str());}
#endif /* FIM_WANT_LONG_INT */
 	string::operator fim_int  (void)const{return fim_atoi(this->c_str());}
	string::operator float(void)const{return fim_atof(this->c_str());}

	int string::find_re(const fim_char_t*r, int *mbuf)const
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

	void string::substitute(const fim_char_t*r, const fim_char_t* s, int flags)
	{
		/*
		 * each occurrence of regular expression r will be substituted with s
		 *
		 * FIXME : return values could be more informative
		 * FIXME : not efficient
		 * */
		regex_t regex;
		const int nmatch=1;
		regmatch_t pmatch[nmatch];
		int off=0,sl=0;
		std::string rs =FIM_CNS_EMPTY_STRING;
		int ts=this->size();

		if( !r || !*r || !s )
			return;

		if( regcomp(&regex,r, 0 | REG_EXTENDED | REG_ICASE | flags ) != 0 )
			return;

		sl=strlen(s);
		//const int s_len=strlen(s);
		while(regexec(&regex,off+c_str(),nmatch,pmatch,REG_NOTBOL)==0)
		{
			/*
			 * please note that this is not an efficient subsitution implementation.
			 * */
			if(FIM_WANT_DEBUG_REGEXP)std::cerr << "pasting "<<off<< ":"<<off+pmatch->rm_so<<"\n";
			if(pmatch->rm_so>0)
			rs+=substr(off,off+pmatch->rm_so-1);
			if(FIM_WANT_DEBUG_REGEXP)std::cerr << "forward to "<<rs<<"\n";
			rs+=s;
			//rs+=substr(pmatch->rm_eo,ts);
			if(FIM_WANT_DEBUG_REGEXP)std::cerr << "match at "<<c_str() << " from " << off+pmatch->rm_so << " to " << off+pmatch->rm_eo<< ", off="<<off<<"\n";
			off+=pmatch->rm_eo;
			if(FIM_WANT_DEBUG_REGEXP)std::cerr << "forward to "<<rs<<"\n";
		}
		if(FIM_WANT_DEBUG_REGEXP)std::cerr << "forward no more\n";
		if(ts>off)
			rs+=substr(off,ts);
		if(FIM_WANT_DEBUG_REGEXP)std::cerr << "res is "<<rs<<", off= "<<off<<"\n";
		if(rs!=*this)
			*this=rs.c_str();
		regfree(&regex);
		return;
	}

	size_t string::lines(void)const
	{
		/*
		 * each empty line will be counted unless it is the last and not only.
		 * */
		const fim_char_t*s=c_str(),*f=s;
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
		const fim_char_t*s,*f;
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
			const fim_char_t *se=s;
			if(!*s)se=s+strlen(s);
			else se=strchr(s,'\n');
			fim::string rs;
			rs=substr(s-this->c_str(),se-s).c_str();
			return rs;
		}
		return "";
	}
#endif /* _FIM_STRING_WRAPPER */
}

