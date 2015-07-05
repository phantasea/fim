/* $Id: common.cpp 272 2009-12-21 17:10:21Z dezperado $ */
/*
 common.cpp : Miscellaneous stuff..

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

// including fim.h poses incompatibilities with <fstream>
//#include "fim.h"

#include <iostream>
#include "fim.h"
#include "string.h"
#include "common.h"
#include <string>
#include <fstream>
#include <sys/time.h>	/* gettimeofday */

#ifdef HAVE_GETLINE
#include <stdio.h>	/* getline : _GNU_SOURCE  */
#endif

void trhex(char *str)
{
	/*	
	 * 	translates C-like hexcodes (e.g.: \xFF) to chars, in place.
	 * 	if \x is not followed by two hexadecimal values, it is ignored and silently copied.
	 * 
	 *
	 *	this function could be optimized.
	 *
	 * 	FIXME : UNUSED
	 */
	const char *fp;//fast pointer
	char *sp;//slow pointer
	char hb[3];
	if(!str)return;

	hb[2]=0;
	fp=sp=str;
	while(*fp)
	{
			if(
				    fp[0] =='\\'
				 && fp[1] && fp[1]=='x'
				 && fp[2] && isxdigit(toupper(fp[2])) 
				 && fp[3] && isxdigit(toupper(fp[3]))  )
			{
				unsigned int hc;
				hb[0]=toupper(fp[2]);
				hb[1]=toupper(fp[3]);
				hc=(unsigned char)strtol(hb,NULL,16);
				*sp=hc;
				fp+=3;
			}
			else
				*sp=*fp;
			++fp;
			++sp;
	}
	*sp=0;
ret:
	return;
}

void trec(char *str,const char *f,const char*t)
{
	/*	this function translates escaped characters at index i in 
	 *	f into the characters at index i in t.
	 *
	 *	order is not important for the final effect.
	 * 
	 *	this function could be optimized.
	 *	20090520 hex translation in
	 */
	if(!str || !f || !t || strlen(f)-strlen(t))return;
	int tl=strlen(f);//table length
	char*_p=str;
	const char *fp;
	const char *tp;
	while(*_p && _p[1])//redundant ?
	{
		fp=f;
		tp=t;

#if 1
		// NEW
		if(
			    _p[0] =='\\'
			 && _p[1] && _p[1]=='x'
			 && _p[2] && isxdigit(toupper(_p[2])) 
			 && _p[3] && isxdigit(toupper(_p[3]))  )
		{
			unsigned int hc;
			char hb[3];
			char *pp;
			hb[2]=0;
			hb[0]=toupper(_p[2]);
			hb[1]=toupper(_p[3]);
			hc=(unsigned char)strtol(hb,NULL,16);
			*_p=hc;
			/*	
				\xFF
				^-_p^-_p+4
			*/
			pp=_p+4;
			while(*pp){pp[-3]=*pp;++pp;}
			pp[-3]='\0';
		}
		else
#endif
		while(*fp)
		{
			//  if the following char is backslash-escaped and is in our from-list ..
			if( *_p == '\\' && *(_p+1) == *fp )
			{
				char*pp;
				*_p = *tp;//translation	
				++_p;  //new focus
				pp=_p+1;
				while(*pp){pp[-1]=*pp;++pp;}//!*pp means we are done :)
				pp[-1]='\0';
				//if(*_p=='\\')++_p;//we want a single pass
//				if(*_p)++_p;//we want a single pass // ! BUG
				fp=f+tl;// in this way  *(fp) == '\0' (single translation pass) as soon as we continue
				if(!*_p)return;
				--_p;//note that the outermost loop will increment this anyway
				continue;//we jump straight to while(NUL)
			}
			++fp;++tp;
		}
		++_p;
	} 
}

	char* slurp_binary_FD(FILE* fd,int *rs)
	{
			/*
			 * ripped off quickly from slurp_binary_fd
			 * FIXME : it is not throughly tested
			 * */
			char	*buf=NULL;
			int	inc=FIM_FILE_BUF_SIZE,rb=0,nrb=0;
			buf=(char*)fim_calloc(inc,1);
			if(!buf) return buf;
			while((nrb=fread(buf+rb,1,inc,fd))>0)
			{
				char *tb;
				// if(nrb==inc) a full read. let's try again
				// else we assume this is the last read (could not be true, of course)
				tb=(char*)realloc(buf,rb+=nrb);
				if(tb!=NULL)
					buf=tb;
				else
					{rb-=nrb;continue;}
			}
			if(rs){*rs=rb;}
			return buf;
	}

	char* slurp_binary_fd(int fd,int *rs)
	{
			/*
			 * If badly tuned, this code is a true allocator grinder :)
			 *
			 * slurps a binary file (possibly a stream) and returns the allocated memory.
			 *
			 * FIXME : use stat if possible.
			 * FIXME : it is not throughly tested
			 * */
			char	*buf=NULL;
			int	inc=FIM_FILE_BUF_SIZE,rb=0,nrb=0;
			buf=(char*)fim_calloc(inc,1);
			if(!buf) return buf;
			while((nrb=read(fd,buf+rb,inc))>0)
			{
				char *tb;
				// if(nrb==inc) a full read. let's try again
				// else we assume this is the last read (could not be true, of course)
				tb=(char*)realloc(buf,rb+=nrb);
				if(tb!=NULL)
					buf=tb;
				else
					{rb-=nrb;continue;}
			}
			if(rs){*rs=rb;}
			return buf;
	}

	fim::string slurp_file(fim::string filename)
	{
		std::string file;
		std::ifstream fs;
		fs.open(filename.c_str(),std::ios::binary);
		if(fs.is_open())
		{
			std::string tmp;
			/* FIXME : this is not efficient */
			while(!fs.eof())
			{
				getline(fs,tmp);
				tmp+="\n" ;
				file+=tmp;
			}
			//	printf("%s\n",file.c_str());
		}
		fs.close();
		return fim::string(file.c_str());
	}

	void append_to_file(fim::string filename, fim::string lines)
	{
		std::ofstream fs;
		fs.open(filename.c_str(),std::ios::app|std::ios::binary);
		if(fs.is_open())
		{
			fs << "\""<< lines.c_str()<<"\"" ;
		}
		fs.close();
		sync();
	}

/*
 * Turns newline characters in NULs.
 * Does stop on the first NUL encountered.
 */
void chomp(char *s)
{
	for(;*s;++s)if(*s=='\n')*s='\0';
}

/*
 * cleans the input string terminating it when some non printable character is encountered
 * (except newline)
 * */
void sanitize_string_from_nongraph_except_newline(char *s, int c)
{	
	int n=c;
	if(s)
	while(*s && (c--||!n))if(!isgraph(*s)&&*s!='\n'){*s=' ';++s;}else ++s;
	return;
}

/*
 * cleans the input string terminating it when some non printable character is encountered
 * */
void sanitize_string_from_nongraph(char *s, int c)
{	
	int n=c;
	if(s)
	while(*s && (c--||!n))if(!isgraph(*s)||*s=='\n'){*s=' ';++s;}else ++s;
	return;
}

/*
 *	Allocation of a small string for storing the 
 *	representation of a double.
 */
char * dupnstr (double n)
{
	//allocation of a single string
	char *r = (char*) fim_malloc (16);
	if(!r){/*assert(r);*/throw FIM_E_NO_MEM;}
	sprintf(r,"%f",n);
	return (r);
}

/*
 *	Allocation of a small string for storing the *	representation of an integer.
 */
char * dupnstr (int n)
{
	//allocation of a single string
	char *r = (char*) fim_malloc (16);
	if(!r){/*assert(r);*/throw FIM_E_NO_MEM;}
	sprintf(r,"%d",n);
	return (r);
}

/*
 *	Allocation and duplication of a single string
 */
char * dupstr (const char* s)
{
	char *r = (char*) fim_malloc (strlen (s) + 1);
	if(!r){/*assert(r);*/throw FIM_E_NO_MEM;}
	strcpy (r, s);
	return (r);
}

/*
 *	Allocation and duplication of a single string (not necessarily terminating)
 */
static char * dupstrn (const char* s, size_t l)
{
	char *r = (char*) fim_malloc (l + 1);
	strncpy(r,s,l);
	r[l]='\0';
	return (r);
}

static int pick_word(const char *f, unsigned int *w)
{
	/*
		FIXME : what is this ? :)
	*/
	int fd = open(f,O_RDONLY);
	if(fd==-1) return -1;
	if(read(fd,w,sizeof(int))==sizeof(int))return 0;
	return -1;
}

/*
 * Will be improved, if needed.
 * */
int fim_rand()
{
	/*
	 * Please don't use Fim for cryptographical purposes ;)
	 * Note that we use /dev/urandom because it will never block on reading.
	 * Reading from     /dev/random could instead block.
	 * */
	unsigned int w;
	if(pick_word("/dev/urandom",&w)==0) return (w%RAND_MAX);
	
	srand(clock()); return rand();

}

	bool regexp_match(const char*s, const char*r, int ignorecase, int ignorenewlines)
	{
		/*
		 *	given a string s, and a Posix regular expression r, this
		 *	method returns true if there is match. false otherwise.
		 */
		regex_t regex;		//should be static!!!
		const int nmatch=1;	// we are satisfied with the first match, aren't we ?
		regmatch_t pmatch[nmatch];

		/*
		 * we allow for the default match, in case of null regexp
		 */
		if(!r || !strlen(r))return true;

		/* fixup code for a mysterious bug
		 */
		if(*r=='*')return false;

		fim::string aux;
		if(ignorenewlines)
		{
			aux=s;
		}

		//if(regcomp(&regex,"^ \\+$", 0 | REG_EXTENDED | REG_ICASE )!=0)
		if(regcomp(&regex,r, 0 | REG_EXTENDED | (ignorecase==0?0:REG_ICASE) )!=0)
		{
			/* error calling regcomp (invalid regexp?)! (should we warn the user ?) */
			//cout << "error calling regcomp (invalid regexp?)!" << "\n";
			return false;
		}
		else
		{
//			cout << "done calling regcomp!" << "\n";
		}
		//if(regexec(&regex,s+0,nmatch,pmatch,0)==0)
		if(regexec(&regex,s+0,nmatch,pmatch,0)!=REG_NOMATCH)
		{
//			cout << "'"<< s << "' matches with '" << r << "'\n";
/*			cout << "match : " << "\n";
			cout << "\"";
			for(int m=pmatch[0].rm_so;m<pmatch[0].rm_eo;++m)
				cout << s[0+m];
			cout << "\"\n";*/
			regfree(&regex);
			return true;
		}
		else
		{
			/*	no match	*/
		};
		regfree(&regex);
		return false;
		return true;
	}

int strchr_count(const char*s, int c)
{
	int n=0;
	if(!s)return 0;
	while((s=strchr(s,c))!=NULL && *s){++n;++s;}
	return n;
}

int newlines_count(const char*s)
{
	/*
	 * "" 0
	 * "aab" 0
	 * "aaaaba\nemk" 1
	 * "aaaaba\nemk\n" 2
	 * */
	int c=strchr_count(s,'\n');
	if(s[strlen(s)-1]=='\n')++c;
	return c;
}

const char* next_row(const char*s, int cols)
{
	/*
	 * returns a pointer to the first char *after*
	 * the newline or the last one of the string.
	 *
	 * for cols=3:
	 * next_row("123\n")  -> \0
	 * next_row("123\n4") ->  4
	 * next_row("12")     -> \0
	 * next_row("1234")   ->  4
	 * */
	const char *b=s;int l=strlen(s);
	if(!s)return NULL;
	if((s=strchr(s,'\n'))!=NULL)
	{
		// we have a newline marking the end of line:
		// with newline-column merge (*s==\n and s+1 is after)
		if((s-b)<=cols) return s+1;
		// ... or without merge (b[cols]!=\n and belongs to the next line)
		else return b+=cols;
	}
	return b+(l>=cols?cols:l);// no newlines in this string; we return the cols'th char or the NUL
}

int lines_count(const char*s, int cols)
{
	/* for cols=6
	 *
	 * "" 0
	 * "aab" 0
	 * "aaaaba\nemk" 1
	 * "aaaaba\nemk\n" 2
	 * "aaaabaa\nemk\n" 3
	 * */
	if(cols<=0)return -1;
	if(cols==0)return newlines_count(s);

	int n=0;
	const char*b;
	if(!s)return 0;
	b=s;
	while((s=strchr(s,'\n'))!=NULL && *s)
	{
		/*
		 * we want a cols long sequence followed by \n
		 * to be counted as one line, just as cols chars alone.
		 *
		 * moreover, we want to be able to enter in this body
		 * even if *++s is NUL, just to perform this computation.
		 */
		n+=s>b?(s-1-b)/cols:0;	/* extra lines due to the excessive line width (if s==b we can't expect any wrapping, of course )	*/
		++n;	// the \n is counted as a new line
		b=++s;	// if now *s==NUL, strchr simply will fail
	};
	//printf("n:%d\n",n);
	s=b;//*b==NUL or *b points to the last substring non newline terminated
	n+=(strlen(s))/cols;	// we count the last substring chars (with no wrapping exceptions)
	return n;
}

int fim_common_test()
{	
	/*
	 * this function should test the correctness of the functions in this file.
	 * it should be used for debug purposes, for Fim maintainance.
	 * */
	printf("%d\n",0==lines_count("" ,6));
	printf("%d\n",0==lines_count("aab" ,6));
	printf("%d\n",1==lines_count("aaaaba\nemk" ,6));
	printf("%d\n",2==lines_count("aaaaba\nemk\n" ,6));
	printf("%d\n",3==lines_count("aaaabaa\nemk\n" ,6));
	printf("%d\n",*next_row("123\n",3)=='\0');
	printf("%d\n",*next_row("123\n4",3)=='4');
	printf("%d\n",*next_row("12",3)=='\0');
	printf("%d\n",*next_row("1234",3)=='4');
	return 0;
}

int swap_bytes_in_int(int in)
{
	// to Most Significant Byte First
	// FIXME : this function should be optimized
	int out=0;
	int b=sizeof(int),i=-1;
	while(i++<b/2)
	{
	((char*)&out)[i]=((char*)&in)[b-i-1];
	((char*)&out)[b-i-1]=((char*)&in)[i];
	}
	return out;
}

int int2lsbf(int in)
{
	int one=0x01;
	if( 0x01 & (*(char*)(&one)) )/*true on msbf (like ppc), false on lsbf (like x86)*/
		return swap_bytes_in_int(in);
	return in;
}

int int2msbf(int in)
{
	int one=0x01;
	if( 0x01 & (*(char*)(&one)) )/*true on msbf (like ppc), false on lsbf (like x86)*/
		return in;
	return swap_bytes_in_int(in);
}

double getmilliseconds()
{
	/*
         * For internal usage: returns with milliseconds precision the current clock time.
         * NOTE : this function is NOT essential.
         */
	int err;//t,pt in ms; d in us
	double dt=0.0;
        struct timeval tv;
        err=gettimeofday(&tv, NULL);
	dt+=tv.tv_usec/1000;
	dt+=tv.tv_sec *1000;
	// note : we ignore err!
	return dt;
}

const char * fim_getenv(const char * name)
{
	/*
	*  A getenv() wrapper function.
	*/
#ifdef HAVE_GETENV
	return getenv(name);
#else
	return NULL;
#endif
}

FILE * fim_fread_tmpfile(FILE * fp)
{
	/*
	*  We transfer a stream contents in a tmpfile()
	* NEW
	*/
	FILE *tfd=NULL;
	if( ( tfd=tmpfile() )!=NULL )
	{	
		/* todo : read errno in case of error and print some report.. */
		const size_t buf_size=4096;
		char buf[buf_size];size_t rc=0,wc=0;/* on some systems fwrite has attribute warn_unused_result */
		while( (rc=fread(buf,1,buf_size,fp))>0 )
		{
			wc=fwrite(buf,1,rc,tfd);
			if(wc!=rc)
			{/* FIXME : this error condition should be handled, as this mechanism is very brittle */}
		}
		rewind(tfd);
		/*
		 * Note that it would be much nicer to do this in another way,
		 * but it would require to rewrite much of the file loading stuff
		 * (which is quite fbi's untouched stuff right now)
		 * */
		return tfd;
	}
	return NULL;
}

double fim_atof(const char *nptr)
{
	/* the original atof suffers from locale 'problems', like non dotted radix representations */
	/* although, atof can be used if one calls setlocale(LC_ALL,"C");  */
	double n=0.0;
	double d=1.0;
	bool sign=false;
	while( *nptr == '-' ){++nptr;sign=!sign;}
	if(!nptr)return n;
	while( isdigit(*nptr) )
	{
		n+=.1*((double)(*nptr-'0'));
		n*=10.0;
		++nptr;
	}
	if(*nptr!='.')return sign?-n:n;
	++nptr;
	while( isdigit(*nptr) )
	{
		d/=10.0;
		n+=d*((double)(*nptr-'0'));
		++nptr;
	}
	return sign?-n:n;
}

ssize_t fim_getline(char **lineptr, size_t *n, FILE *stream)
{
	/*
	 * WARNING : untested!
	 */
#ifdef HAVE_GETLINE
	return getline(lineptr,n,stream);
#endif
#ifdef HAVE_FGETLN
	{	
		/* for BSD (in stdlib.h) */
		char *s,*ns;
		size_t len=0;
		s=fgetln(stream,&len);
		if(!s)return EINVAL;
		*lineptr=dupstrn(s,len);
		*n=len;
		return len;
	}
#endif
	return EINVAL;
}

	bool is_dir(const fim::string nf)
	{
		struct stat stat_s;
		/*	if the directory doesn't exist, return */
		if(-1==stat(nf.c_str(),&stat_s))return false;
		if( ! S_ISDIR(stat_s.st_mode))return false;
		return true;
	}

	bool is_file(const fim::string nf)
	{
		/* FIXME */
		return !is_dir(nf);
	}


