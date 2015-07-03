/* $LastChangedDate: 2015-02-12 19:10:43 +0100 (Thu, 12 Feb 2015) $ */
/*
 common.cpp : Miscellaneous stuff..

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

// including fim.h poses incompatibilities with <fstream>
//#include "fim.h"

#include <iostream>
#include "fim.h"
#include "fim_string.h"
#include "common.h"
#include <string>
#include <fstream>
#include <sys/time.h>	/* gettimeofday */

#ifdef HAVE_GETLINE
#include <stdio.h>	/* getline : _GNU_SOURCE  */
#endif /* HAVE_GETLINE */
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif /* HAVE_WCHAR_H */
#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif /* HAVE_LIBGEN_H */
#include <zlib.h>

/*
void fim_tolowers(fim_char_t *s)
{
	if(!s)
		return;
	for(;*s;++s)
		*s=tolower(*s);
}

void fim_touppers(fim_char_t *s)
{
	if(!s)
		return;
	for(;*s;++s)
		*s=toupper(*s);
}
*/

fim::string fim_dirname(const fim::string & arg)
{
#ifdef HAVE_LIBGEN_H
	fim_char_t buf[FIM_PATH_MAX];
	strncpy(buf,arg.c_str(),FIM_PATH_MAX-1);
	buf[FIM_PATH_MAX-1]='\0';
	return dirname(buf);
#else /* HAVE_LIBGEN_H */
	return "";//FIXME
#endif /* HAVE_LIBGEN_H */
}
	fim::string fim_shell_arg_escape(const fim::string & arg)
	{
		// FIXME: this escaping function is NOT safe; this code shall only serve as a placeholder for a better one.
		fim::string ear=arg;
		fim::string res=FIM_CNS_EMPTY_STRING;
		res+="'";
		ear.substitute("'","'\\''");
		res+=ear;
		res+="'";
		return res;
	}

void fim_perror(const fim_char_t *s)
{
#if 1
	if(errno)
	{
		if(s)
			perror(s);
		errno=0; // shall reset the error status
	}
#endif
}

size_t fim_strlen(const fim_char_t *str)
{
	return strlen(str);
}

void trhex(fim_char_t *str)
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
	const fim_char_t *fp;//fast pointer
	fim_char_t *sp;//slow pointer
	fim_char_t hb[3];

	if(!str)
		goto ret;

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
				hc=(fim_byte_t)strtol(hb,NULL,FIM_PRINTINUM_BUFSIZE);
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

void trec(fim_char_t *str,const fim_char_t *f,const fim_char_t*t)
{
	/*	this function translates escaped characters at index i in 
	 *	f into the characters at index i in t.
	 *
	 *	order is not important for the final effect.
	 * 
	 *	this function could be optimized.
	 *	20090520 hex translation in
	 */
	int tl;
	fim_char_t*_p=NULL;
	const fim_char_t *fp;
	const fim_char_t *tp;

	if(!str || !f || !t || strlen(f)-strlen(t))
		goto ret;

	tl = strlen(f);//table length
	_p=str;

	while(*_p && _p[1])//redundant ?
	{
		fp=f;
		tp=t;

#if 1
		if(
			    _p[0] =='\\'
			 && _p[1] && _p[1]=='x'
			 && _p[2] && isxdigit(toupper(_p[2])) 
			 && _p[3] && isxdigit(toupper(_p[3]))  )
		{
			unsigned int hc;
			fim_char_t hb[3];
			fim_char_t *pp;
			hb[2]=0;
			hb[0]=toupper(_p[2]);
			hb[1]=toupper(_p[3]);
			hc=(fim_byte_t)strtol(hb,NULL,FIM_PRINTINUM_BUFSIZE);
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
			//  if the following charcter is backslash-escaped and is in our from-list ..
			if( *_p == '\\' && *(_p+1) == *fp )
			{
				fim_char_t*pp;
				*_p = *tp;//translation	
				++_p;  //new focus
				pp=_p+1;
				while(*pp)
				{
					pp[-1]=*pp;++pp;
				}//!*pp means we are done :)
				pp[-1]='\0';
				//if(*_p=='\\')++_p;//we want a single pass
//				if(*_p)++_p;//we want a single pass // ! BUG
				fp=f+tl;// in this way  *(fp) == '\0' (single translation pass) as soon as we continue
				if(!*_p)
					goto ret;
				--_p;//note that the outermost loop will increment this anyway
				continue;//we jump straight to while(NUL)
			}
			++fp;++tp;
		}
		++_p;
	} 
ret:
	return;
}

	fim_byte_t* slurp_binary_FD(FILE* fd, size_t  *rs)
	{
			/*
			 * ripped off quickly from slurp_binary_fd
			 * FIXME : it is not throughly tested
			 * */
			fim_byte_t	*buf=NULL;
			int	inc=FIM_FILE_BUF_SIZE,rb=0,nrb=0;
			buf=(fim_byte_t*)fim_calloc(inc,1);
			if(!buf) 
				goto ret;
			while((nrb=fim_fread(buf+rb,1,inc,fd))>0)
			{
				fim_byte_t *tb;
				// if(nrb==inc) a full read. let's try again
				// else we assume this is the last read (could not be true, of course)
				tb=(fim_byte_t*)realloc(buf,rb+=nrb);
				if(tb!=NULL)
					buf=tb;
				else
					{rb-=nrb;continue;}
			}
			if(rs)
			{
				*rs=rb;
			}
ret:
			return buf;
	}

	fim_char_t* slurp_binary_fd(int fd,int *rs)
	{
			/*
			 * If badly tuned, this code is a true allocator grinder :)
			 *
			 * slurps a binary file (possibly a stream) and returns the allocated memory.
			 *
			 * FIXME : use stat if possible.
			 * FIXME : it is not throughly tested
			 * */
			fim_char_t	*buf=NULL;
			int	inc=FIM_FILE_BUF_SIZE,rb=0,nrb=0;
			buf = fim_stralloc(inc);
			if(!buf)
			       	goto ret;
			while((nrb=read(fd,buf+rb,inc))>0)
			{
				fim_char_t *tb;
				// if(nrb==inc) a full read. let's try again
				// else we assume this is the last read (could not be true, of course)
				tb=(fim_char_t*)realloc(buf,rb+=nrb);
				if(tb!=NULL)
					buf=tb;
				else
					{rb-=nrb;continue;}
			}
			if(rs)
				*rs=rb;
ret:
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

#if 0
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
#endif

/*
 * Turns newline characters in NULs.
 * Does stop on the first NUL encountered.
 */
void chomp(fim_char_t *s)
{
	for(;*s;++s)
		if(*s=='\n')
			*s='\0';
}

/*
 * cleans the input string terminating it when some non printable character is encountered
 * (except newline)
 * */
void sanitize_string_from_nongraph_except_newline(fim_char_t *s, int c)
{	
	int n=c;
	if(s)
		while(*s && (c--||!n))
			if(!isgraph(*s)&&*s!='\n')
			{
				*s=' ';
				++s;
			}
			else
			       	++s;
	return;
}

/*
 * cleans the input string terminating it when some non printable character is encountered
 * */
void sanitize_string_from_nongraph(fim_char_t *s, int c)
{	
	int n=c;
	if(s)
		while(*s && (c--||!n))
			if(!isgraph(*s)||*s=='\n')
			{
				*s=' ';
				++s;
			}
			else
			       	++s;
	return;
}

/*
 *	Allocation of a small string for storing the 
 *	representation of a double.
 */
fim_char_t * dupnstr (float n, const fim_char_t c)
{
	//allocation of a single string
	fim_char_t *r = (fim_char_t*) fim_malloc (32);
	if(!r){/*assert(r);*/throw FIM_E_NO_MEM;}
	sprintf(r,"%f%c",n,c);
	return (r);
}

fim_char_t * dupnstr (const fim_char_t c1, double n, const fim_char_t c2)
{
	//allocation of a single string
	fim_char_t *r = (fim_char_t*) fim_malloc (32);
	if(!r){/*assert(r);*/throw FIM_E_NO_MEM;}
	sprintf(r,"%c%f%c",c1,n,c2);
	return (r);
}

/*
 *	Allocation of a small string for storing the *	representation of an integer.
 */
fim_char_t * dupnstr (fim_int n)
{
	//allocation of a single string
	fim_char_t *r = (fim_char_t*) fim_malloc (FIM_PRINTINUM_BUFSIZE);
	if(!r){/*assert(r);*/throw FIM_E_NO_MEM;}
	if(sizeof(fim_int)==sizeof(int))
		sprintf(r,"%d",(int)n);
	else
		sprintf(r,"%lld",(long long int)n);
	return (r);
}

/*
 *	Allocation and duplication of a single string
 */
fim_char_t * dupstr (const fim_char_t* s)
{
	fim_char_t *r = (fim_char_t*) fim_malloc (strlen (s) + 1);
	if(!r){/*assert(r);*/throw FIM_E_NO_MEM;}
	strcpy (r, s);
	return (r);
}

/*
 *	Allocation and duplication of a single string, slash-quoted
 */
fim_char_t * dupsqstr (const fim_char_t* s)
{
	int l=0;
	fim_char_t *r = (fim_char_t*) fim_malloc ((l=strlen (s)) + 3);
	if(!r){/*assert(r);*/throw FIM_E_NO_MEM;}
	else
	{
		r[0]='/';
		strcpy (r+1  , s);
		strcat (r+1+l,"/");
	}
	return (r);
}

/*
 *	Allocation and duplication of a single string (not necessarily terminating)
 */
#ifdef HAVE_FGETLN
static fim_char_t * dupstrn (const fim_char_t* s, size_t l)
{
	fim_char_t *r = (fim_char_t*) fim_malloc (l + 1);
	strncpy(r,s,l);
	r[l]='\0';
	return (r);
}
#endif /* HAVE_FGETLN */

static int pick_word(const fim_char_t *f, unsigned int *w)
{
	/*
		FIXME : what is this ? :)
	*/
	int fd = open(f,O_RDONLY);
	if(fd==-1)
	       	goto ret;
	if(read(fd,w,sizeof(int))==sizeof(int))
		fd=0;
ret:
	return fd;
}

/*
 * Will be improved, if needed.
 * */
fim_int fim_rand(void)
{
	/*
	 * Please don't use Fim random numbers for cryptographical purposes ;)
	 * Note that we use /dev/urandom because it will never block on reading.
	 * Reading from     /dev/random could instead block.
	 * */
	unsigned int w,r;
	if(pick_word(FIM_LINUX_RAND_FILE,&w)==0)
	       	r = (w%RAND_MAX);// TODO: are we sure that RAND_MAX corresponds to FIM_LINUX_RAND_FILE ?
	else
	{
		srand(clock());
		r = rand();
	}
	return (fim_int) r; /* FIXME: shall document this limitation  */
}

	bool regexp_match(const fim_char_t*s, const fim_char_t*r, int ignorecase, int ignorenewlines)
	{
		/*
		 *	given a string s, and a Posix regular expression r, this
		 *	method returns true if there is match. false otherwise.
		 */
		regex_t regex;		//should be static!!!
		const int nmatch=1;	// we are satisfied with the first match, aren't we ?
		regmatch_t pmatch[nmatch];
		bool match=true;

		/*
		 * we allow for the default match, in case of null regexp
		 */
		if(!r || !strlen(r))
			goto ret;

		/* fixup code for a mysterious bug
		 */
		if(*r=='*')
		{
			match = false;
			goto ret;
		}

		if(ignorenewlines)
		{
			fim::string aux;
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
			goto ret;
		}
		else
		{
			/*	no match	*/
		};
		regfree(&regex);
		match = false;
ret:
		return match;
	}

int strchr_count(const fim_char_t*s, int c)
{
	int n=0;
	if(!s)
		return 0;
	while((s=strchr(s,c))!=NULL && *s)
		++n,++s;
	return n;
}

int newlines_count(const fim_char_t*s)
{
	/*
	 * "" 0
	 * "aab" 0
	 * "aaaaba\nemk" 1
	 * "aaaaba\nemk\n" 2
	 * */
	int c=strchr_count(s,'\n');
	if(s[strlen(s)-1]=='\n')
		++c;
	return c;
}

const fim_char_t* next_row(const fim_char_t*s, int cols)
{
	/*
	 * returns a pointer to the first character *after*
	 * the newline or the last one of the string.
	 *
	 * for cols=3:
	 * next_row("123\n")  -> \0
	 * next_row("123\n4") ->  4
	 * next_row("12")     -> \0
	 * next_row("1234")   ->  4
	 * */
	const fim_char_t *b=s;int l=strlen(s);
	if(!s)
		return NULL;
	if((s=strchr(s,'\n'))!=NULL)
	{
		// we have a newline marking the end of line:
		// with newline-column merge (*s==\n and s+1 is after)
		if((s-b)<=cols)
		       	return s+1;
		// ... or without merge (b[cols]!=\n and belongs to the next line)
		else
		       	return b+=cols;
	}
	return b+(l>=cols?cols:l);// no newlines in this string; we return the cols'th character or the NUL
}

int lines_count(const fim_char_t*s, int cols)
{
	/* for cols=6
	 *
	 * "" 0
	 * "aab" 0
	 * "aaaaba\nemk" 1
	 * "aaaaba\nemk\n" 2
	 * "aaaabaa\nemk\n" 3
	 * */
	if(cols<=0)
		return -1;
	if(cols==0)
		return newlines_count(s);

	int n=0;
	const fim_char_t*b;
	if(!s)
		return 0;
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

int fim_common_test(void)
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
		((fim_byte_t*)&out)[i]=((fim_byte_t*)&in)[b-i-1];
		((fim_byte_t*)&out)[b-i-1]=((fim_byte_t*)&in)[i];
	}
	return out;
}

int int2lsbf(int in)
{
	int one=0x01;
	if( 0x01 & (*(fim_byte_t*)(&one)) )/*true on msbf (like ppc), false on lsbf (like x86)*/
		return swap_bytes_in_int(in);
	return in;
}

int int2msbf(int in)
{
	int one=0x01;
	if( 0x01 & (*(fim_byte_t*)(&one)) )/*true on msbf (like ppc), false on lsbf (like x86)*/
		return in;
	return swap_bytes_in_int(in);
}

double getmilliseconds(void)
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

#if 0
struct fim_bench_struct { void *data; };

typedef fim_err_t (*fim_bench_ft)(struct fim_bench_struct*);
static fim_err_t fim_bench_video(struct fim_bench_struct*)
{
	//cc.clear_rect(0, width()-1, 0,height()/2);
	return FIM_ERR_NO_ERROR;
}
#endif

const fim_char_t * fim_getenv(const fim_char_t * name)
{
	/*
	*  A getenv() wrapper function.
	*/
#ifdef HAVE_GETENV
	return getenv(name);
#else /* HAVE_GETENV */
	return NULL;
#endif /* HAVE_GETENV */
}

FILE * fim_fread_tmpfile(FILE * fp)
{
	/*
	*  We transfer a stream contents in a tmpfile(void)
	* NEW
	*/
	FILE *tfd=NULL;
	if( ( tfd=tmpfile() )!=NULL )
	{	
		/* todo : read errno in case of error and print some report.. */
		const size_t buf_size=FIM_STREAM_BUFSIZE;
		fim_char_t buf[buf_size];size_t rc=0,wc=0;/* on some systems fwrite has attribute warn_unused_result */
		while( (rc=fim_fread(buf,1,buf_size,fp))>0 )
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

double fim_atof(const fim_char_t *nptr)
{
	/* the original atof suffers from locale 'problems', like non dotted radix representations */
	/* although, atof can be used if one calls setlocale(LC_ALL,"C");  */
	double n=0.0;
	double d=1.0;
	bool sign=false;
	while( *nptr == '-' ){++nptr;sign=!sign;}
	if(!nptr)
		return n;
	while( isdigit(*nptr) )
	{
		n+=.1*((double)(*nptr-'0'));
		n*=10.0;
		++nptr;
	}
	if(*nptr!='.')
		return sign?-n:n;
	++nptr;
	while( isdigit(*nptr) )
	{
		d/=10.0;
		n+=d*((double)(*nptr-'0'));
		++nptr;
	}
	return sign?-n:n;
}

ssize_t fim_getline(fim_char_t **lineptr, size_t *n, FILE *stream)
{
	/*
	 * WARNING : untested!
	 */
#ifdef HAVE_GETLINE
	return getline(lineptr,n,stream);
#endif /* HAVE_GETLINE */
#ifdef HAVE_FGETLN
	{	
		/* for BSD (in stdlib.h) */
		fim_char_t *s,*ns;
		size_t len=0;
		s=fgetln(stream,&len);
		if(!s)
			return EINVAL;
		*lineptr=dupstrn(s,len);
		*n=len;
		return len;
	}
#endif /* HAVE_FGETLN */
	return EINVAL;
}

	bool is_dir(const fim::string nf)
	{
		struct stat stat_s;
		/*	if the directory doesn't exist, return */
		if(-1==stat(nf.c_str(),&stat_s))
			return false;
		if( ! S_ISDIR(stat_s.st_mode))
			return false;
		return true;
	}

	bool is_file(const fim::string nf)
	{
		/* FIXME */
#if 0
		return !is_dir(nf);
#else
		struct stat stat_s;
		/*	if the file (it can be a device, but not a directory) doesn't exist, return */
		if(-1==stat(nf.c_str(),&stat_s))
			return false;
		if( S_ISDIR(stat_s.st_mode))
			return false;
		/*if(!S_IFREG(stat_s.st_mode))return false;*/
		return true;
#endif
	}

	bool is_file_nonempty(const fim::string nf)
	{
		/* FIXME: merge the stat-using functions into one, with arguments! */
#if 0
		return !is_dir(nf);
#else
		struct stat stat_s;
		/*	if the file (it can be a device, but not a directory) doesn't exist, return */
		if(-1==stat(nf.c_str(),&stat_s))
			return false;
		if( S_ISDIR(stat_s.st_mode))
			return false;
		/*if(!S_IFREG(stat_s.st_mode))return false;*/
		if( stat_s.st_size == 0 )
			return false;
		return true;
#endif
	}

int fim_isspace(int c){return isspace(c);}
int fim_isquote(int c){return c=='\'' || c=='\"';}
#define FIM_WANT_ZLIB 0
FILE *fim_fopen(const char *path, const char *mode)
{
#if FIM_WANT_ZLIB
	/* cast necessary; in v.1.2.3.4 declared as void* */
	return (FILE*)gzopen(path,mode);
#else /* FIM_WANT_ZLIB */
	return fopen(path,mode);
#endif /* FIM_WANT_ZLIB */
}

int fim_fclose(FILE*fp)
{
#if FIM_WANT_ZLIB
	return gzclose(fp);
#else /* FIM_WANT_ZLIB */
	return fclose(fp);
#endif /* FIM_WANT_ZLIB */
}

size_t fim_fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
#if FIM_WANT_ZLIB
	return gzread(stream,ptr,size*nmemb);
#else /* FIM_WANT_ZLIB */
	return fread(ptr,size,nmemb,stream);
#endif /* FIM_WANT_ZLIB */
}

int fim_rewind(FILE *stream)
{
#if FIM_WANT_ZLIB
	gzrewind(stream);
	return 0;
#else /* FIM_WANT_ZLIB */
	rewind(stream);
	return 0;
#endif /* FIM_WANT_ZLIB */
}

int fim_fseek(FILE *stream, long offset, int whence)
{
#if FIM_WANT_ZLIB
	return gzseek(stream,offset,whence);
	0;
#else /* FIM_WANT_ZLIB */
	return fseek(stream,offset,whence);
#endif /* FIM_WANT_ZLIB */
}

int fim_fgetc(FILE *stream)
{
#if FIM_WANT_ZLIB
	return gzgetc(stream);
#else /* FIM_WANT_ZLIB */
	return fgetc(stream);
#endif /* FIM_WANT_ZLIB */
}

int fim_snprintf_XB(char *str, size_t size, size_t q)
{
	/* result fits in 5 bytes */
	char u='B',b=' ';
	size_t d=1;
	int src;
	if(q/d>1024)
		d*=FIM_CNS_K,u='K',b='B';
	if(q/d>1024)
		d*=FIM_CNS_K,u='M';
	if(q/d>1024)
		d*=FIM_CNS_K,u='G';
#if (SIZEOF_SIZE_T > 4)
	if(q/d>1024)
		d*=FIM_CNS_K,u='T';
	if(q/d>1024)
		d*=FIM_CNS_K,u='P';
#endif
	if(q/d<10)
		src = snprintf(str, size, "%1.1f%c%c",((float)q)/((float)d),u,b);
	else
		src = snprintf(str, size, "%zd%c%c",q/d,u,b);
	return src;
}

fim_byte_t * fim_pm_alloc(unsigned int width, unsigned int height, bool want_calloc)
{
	size_t nmemb=1, size=1;
	nmemb *= width;
	nmemb *= height;
	nmemb *= 3;
	/* FIXME: shall implement overflow checks here */
	if(want_calloc)
		return (fim_byte_t*)fim_calloc(nmemb, 1);
	else
		return (fim_byte_t*)fim_malloc(nmemb);
}

const fim_char_t * fim_basename_of(const fim_char_t * s)
{
	if(s && *s)
	{
#if 0
		size_t sl = strlen(s);

		while(sl > 0 )
		{
			sl--;
			if(s[sl]=='/')
				return s+sl+1;
		}
#else
		const fim_char_t *bn = strrchr(s,'/');
		if(bn)
			s=bn+1;
#endif
	}
	return s;
}

fim_int fim_atoi(const char*s)
{
	if(sizeof(fim_int)==sizeof(int))
		return atoi(s);
	else
		return atoll(s);
}
