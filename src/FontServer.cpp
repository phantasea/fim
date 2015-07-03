/* $LastChangedDate: 2015-02-06 01:47:58 +0100 (Fri, 06 Feb 2015) $ */
/*
 FontServer.cpp : Font Server code from fbi, adapted for fim.

 (c) 2007-2014 Michele Martone
 (c) 1998-2006 Gerd Knorr <kraxel@bytesex.org>

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
 * This file comes from fbi, and will undergo severe reorganization.
 * */



#include <dirent.h>
#include "fim.h"

#define FIM_FONT_DEBUG 0
//#define ff_stderr stdout
#define ff_stderr stderr
#define FIM_PSF1_MAGIC0     0x36
#define FIM_PSF1_MAGIC1     0x04
#define FIM_PSF2_MAGIC0     0x72
#define FIM_PSF2_MAGIC1     0xb5
#define FIM_PSF2_MAGIC2     0x4a
#define FIM_PSF2_MAGIC3     0x86
#define FIM_MAX_FONT_HEIGHT 256
#define FIM_SAVE_CONSOLEFONTNAME(CFN) cc.setVariable(FIM_VID_FBFONT,CFN);
namespace fim
{

	FontServer::FontServer( )
	{
	}


#if 1
void FontServer::fb_text_init1(const fim_char_t *font_, struct fs_font **_f)
{ 
    const fim_char_t*font=(fim_char_t*)font_;
    const fim_char_t *fonts[2] = { font, NULL };
#if FIM_FONT_DEBUG
    std::cout << "before consolefont:" << "(0x"<<((void*)*_f) <<")\n";
#endif /* FIM_FONT_DEBUG */
    if (NULL == *_f)
	*_f = fs_consolefont(font ? fonts : NULL);
#if FIM_FONT_DEBUG
    std::cout << "after consolefont :" << "(0x"<<((void*)*_f) <<")\n";
#endif /* FIM_FONT_DEBUG */
#ifdef FIM_USE_X11_FONTS
    if (NULL == *_f && 0 == fs_connect(NULL))
	*_f = fs_open(font ? font : x11_font);
#endif /* FIM_USE_X11_FONTS */
#if FIM_FONT_DEBUG
    std::cout << "after fs_open     :" << "(0x"<<((void*)*_f) <<")\n";
#endif /* FIM_FONT_DEBUG */
    if (NULL == *_f) {
	FIM_FPRINTF(ff_stderr, "font \"%s\" is not available\n",font);
	exit(1);
    }
}

#if 1
static const fim_char_t *default_font[] = {
    /* why the heck every f*cking distribution picks another
       location for these fonts ??? (GK)
       +1 (MM) */
#ifdef FIM_DEFAULT_CONSOLEFONT
	FIM_DEFAULT_CONSOLEFONT,
#endif /* FIM_DEFAULT_CONSOLEFONT */
    "/usr/share/consolefonts/Uni3-TerminusBoldVGA14.psf.gz",
    "/usr/lib/kbd/consolefonts/lat9-16.psf.gz",/* added for a Mandriva backport */
    "/usr/share/consolefonts/lat1-16.psf",
    "/usr/share/consolefonts/lat1-16.psf.gz",
    "/usr/share/consolefonts/lat1-16.psfu.gz",
    "/usr/share/kbd/consolefonts/lat1-16.psf",
    "/usr/share/kbd/consolefonts/lat1-16.psf.gz",
    "/usr/share/kbd/consolefonts/lat1-16.psfu.gz",
    "/usr/lib/kbd/consolefonts/lat1-16.psf",
    "/usr/lib/kbd/consolefonts/lat1-16.psf.gz",
    "/usr/lib/kbd/consolefonts/lat1-16.psfu.gz",
    "/lib/kbd/consolefonts/lat1-16.psf",
    "/lib/kbd/consolefonts/lat1-16.psf.gz",
    "/lib/kbd/consolefonts/lat1-16.psfu.gz",
    /* added for Ubuntu 10, but a search mechanism or a fim user variable would be wiser */
    "/lib/kbd/consolefonts/Lat2-VGA14.psf.gz",
    "/lib/kbd/consolefonts/Lat2-VGA16.psf.gz",
    "/lib/kbd/consolefonts/Lat2-VGA8.psf.gz",
    "/lib/kbd/consolefonts/Uni2-VGA16.psf.gz",
    /* end ubuntu add */
    /* begin debian squeeze add */
    "/usr/share/consolefonts/default8x16.psf.gz",
    "/usr/share/consolefonts/default8x9.psf.gz",
    "/usr/share/consolefonts/Lat15-Fixed16.psf.gz",
    "/usr/share/consolefonts/default.psf.gz",
    /* end debian squeeze add */
#if FIM_WANT_HARDCODED_FONT
    FIM_DEFAULT_HARDCODEDFONT_STRING,
#endif /* FIM_WANT_HARDCODED_FONT */
    NULL
};

fim::string get_default_font_list(void)
{
	fim::string dfl;
	const fim_char_t ** filename=default_font;
	for(int i = 0; filename[i] != NULL; i++)
       	{
		dfl+=filename[i];
		dfl+="\n";
	}
	return dfl;
}
#endif

static int probe_font_file(const fim_char_t *fontfilename)
{
    	FILE *fp=NULL;
	if ( strlen(fontfilename)>3 && 0 == strcmp(fontfilename+strlen(fontfilename)-3,".gz"))
	{
		#ifdef FIM_USE_ZCAT
		/* FIXME */
		fp = FbiStuff::fim_execlp(FIM_EPR_ZCAT,FIM_EPR_ZCAT,fontfilename,NULL);
		#endif /* FIM_USE_ZCAT */
	}
	else
	{
		fp = fopen(fontfilename, "r");
	}

	if (NULL == fp)
		goto no;

	if (fgetc(fp) != 0x36 || fgetc(fp) != 0x04)
		goto no;
 
     	/* this is enough */
	if(fp)
		fclose(fp);
	return 0;
no:
	if(fp)fclose(fp);
	return -1;
}

void fim_free_fs_font(struct fs_font *f_)
{
	if(f_)
	{
		if(f_->eindex) fim_free(f_->eindex);
		if(f_->gindex) fim_free(f_->gindex);
		if(f_->glyphs) fim_free(f_->glyphs);
		if(f_->extents) fim_free(f_->extents);
		fim_free(f_);
	}
}

struct fs_font* FontServer::fs_consolefont(const fim_char_t **filename)
{
    /* this function is too much involved: it shall be split in pieces */
    int  i=0;
    int  fr;
    const fim_char_t *h=NULL;
    struct fs_font *f_ = NULL;
    const fim_char_t *fontfilename=NULL;
    FILE *fp=NULL;
    fim_char_t fontfilenameb[FIM_PATH_MAX];
    bool robmn=true;/* retry on bad magic numbers */
#if FIM_WANT_HARDCODED_FONT
    unsigned char dfontdata[] =
#include "default_font_byte_array.h"/* FIXME: this is horrible practice */
#endif /* FIM_WANT_HARDCODED_FONT */

#if FIM_WANT_HARDCODED_FONT
    /* shortcut: no access() call required */
    if (filename && *filename && 0 == strcmp(filename[0],FIM_DEFAULT_HARDCODEDFONT_STRING))
	    goto openhardcodedfont;
#endif /* FIM_WANT_HARDCODED_FONT */

    if (NULL == filename)
	filename = fim::default_font;

scanlistforafontfile:
    for(i = 0; filename[i] != NULL; i++) {
	if (-1 == access(filename[i],R_OK))
	{
#if FIM_WANT_HARDCODED_FONT
    		if (0 == strcmp(filename[i],FIM_DEFAULT_HARDCODEDFONT_STRING))
			goto openhardcodedfont;
#endif /* FIM_WANT_HARDCODED_FONT */
#if FIM_FONT_DEBUG
    std::cout << "no access to " << filename[i] << "\n";
#endif /* FIM_FONT_DEBUG */
	    fim_perror(NULL);
	    continue;
	}
	break;
    }
    fontfilename=filename[i];
    filename+=i;//new
#if FIM_FONT_DEBUG
    std::cout << "probing :" << fontfilename << "\n";
#endif /* FIM_FONT_DEBUG */

#if FIM_LINUX_CONSOLEFONTS_DIR_SCAN 
    if(NULL == fontfilename)
    {
	/* will scan FIM_LINUX_CONSOLEFONTS_DIR directory for console fonts */
	fim::string nf = FIM_LINUX_CONSOLEFONTS_DIR;
	DIR *dir=NULL;
	struct dirent *de=NULL;

	if( !is_dir( nf.c_str() ))
		goto oops;
	if ( ! ( dir = opendir(nf.c_str() ) ))
		goto oops;

	while( ( de = readdir(dir) ) != NULL )
	{
		if(is_file(de->d_name) && regexp_match(de->d_name,"8x.*\\.psf") && access(de->d_name,R_OK))
    		{
			nf = FIM_LINUX_CONSOLEFONTS_DIR;
			nf+="/";
			nf+=de->d_name;
			strncpy(fontfilenameb,nf.c_str(),FIM_PATH_MAX-1);
			fontfilename=fontfilenameb;
			if(probe_font_file(fontfilename)==0)
				break;
			/* FIXME */
		}
	}
	closedir(dir);
    }
#endif /* FIM_LINUX_CONSOLEFONTS_DIR_SCAN */

#if FIM_WANT_HARDCODED_FONT
openhardcodedfont:
    if (NULL == fontfilename)
    {
	FIM_SAVE_CONSOLEFONTNAME(FIM_DEFAULT_HARDCODEDFONT_STRING);
    	fp=fmemopen(dfontdata,sizeof(dfontdata),"r");
	if(fp)
		goto gotafp;
    }
#endif /* FIM_WANT_HARDCODED_FONT */
    if (NULL == fontfilename) {
	FIM_FPRINTF(ff_stderr, "can't find console font file\n");
	goto oops;
    }

    h = fontfilename+strlen(fontfilename)-3;
    if ( h>fontfilename && 0 == strcmp(h,".gz")) {
	#ifdef FIM_USE_ZCAT
	/* FIXME */
	fp = FbiStuff::fim_execlp(FIM_EPR_ZCAT,FIM_EPR_ZCAT,fontfilename,NULL);
	#else /* FIM_USE_ZCAT */
	FIM_FPRINTF(ff_stderr, "built with no gzip decoder!\n");
	#endif /* FIM_USE_ZCAT */
    } else {
	fp = fopen(fontfilename, "r");
    }
    if (NULL == fp) {
	FIM_FPRINTF(ff_stderr, "can't open %s: %s\n",fontfilename,strerror(errno));
	goto oops;
    }
    FIM_SAVE_CONSOLEFONTNAME(fontfilename);
gotafp:
{
    int m0=0,m1=0;
    m0=fgetc(fp);
    m1=fgetc(fp);
    if (m0 == EOF     || m1 == EOF     ) {
	FIM_FPRINTF(ff_stderr, "problems reading two first bytes from %s.\n",fontfilename);
	goto oops;
    }
    if (m0 == FIM_PSF2_MAGIC0     && m1 == FIM_PSF2_MAGIC1     ) {
	FIM_FPRINTF(ff_stderr, "can't use font %s: first two magic bytes (0x%x 0x%x) conform to PSF version 2, which is unsupported.\n",fontfilename,m0,m1);
	goto oops;
    }
    if (m0 != FIM_PSF1_MAGIC0     || m1 != FIM_PSF1_MAGIC1     ) {
	FIM_FPRINTF(ff_stderr, "can't use font %s: first two magic bytes (0x%x 0x%x) not conforming to PSF version 1\n",fontfilename,m0,m1);
	goto oops;
    }
}
//    FIM_FPRINTF(ff_stderr, "using linux console font \"%s\"\n",filename[i]);

    f_ =(struct fs_font*) fim_calloc(1,sizeof(*f_));
    if(!f_)goto aoops;
	
    fgetc(fp);
    f_->maxenc = 256;
    f_->width  = 8;	/* FIXME */
    f_->height = fgetc(fp);
    if(f_->height<0 || f_->height>FIM_MAX_FONT_HEIGHT) goto oops;
    f_->fontHeader.min_bounds.left    = 0;
    f_->fontHeader.max_bounds.right   = f_->width;
    f_->fontHeader.max_bounds.descent = 0;
    f_->fontHeader.max_bounds.ascent  = f_->height;

    f_->glyphs  =(fim_byte_t*) fim_malloc(f_->height * 256);
    if(!f_->glyphs) goto aoops;
    f_->extents = (FSXCharInfo*)fim_malloc(sizeof(FSXCharInfo)*256);
    if(!f_->extents) goto aoops;
    fr=fread(f_->glyphs, 256, f_->height, fp);
    if(!fr)goto aoops;/* new */
    fclose(fp);fp=NULL;

    f_->eindex  =(FSXCharInfo**) fim_malloc(sizeof(FSXCharInfo*)   * 256);
    if(!f_->eindex) goto aoops;
    f_->gindex  = (fim_byte_t**)fim_malloc(sizeof(fim_byte_t*) * 256);
    if(!f_->gindex) goto aoops;
    for (i = 0; i < 256; i++) {
	f_->eindex[i] = f_->extents +i;
	f_->gindex[i] = f_->glyphs  +i * f_->height;
	f_->eindex[i]->left    = 0;
	f_->eindex[i]->right   = 7;
	f_->eindex[i]->width   = 8;/* FIXME */
	f_->eindex[i]->descent = 0;
	f_->eindex[i]->ascent  = f_->height;
    }
    return f_;
aoops:
    robmn=false;/* no retry: this is a allocation-related oops */
    if(f_)
	fim_free_fs_font(f_);
oops:
    if(fp){fclose(fp);fp=NULL;}
    if(robmn && filename[0] && filename[1]){++filename;goto scanlistforafontfile;}else robmn=false;
    return NULL;
}
#endif




}


