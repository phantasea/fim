/* $Id: FontServer.cpp 273 2009-12-21 17:26:56Z dezperado $ */
/*
 FontServer.cpp : Font Server code from fbi, adapted for fim.

 (c) 2007-2009 Michele Martone
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

namespace fim
{

	FontServer::FontServer( )
	{
	}


#if 1
/* 20080507 unused ? */
void FontServer::fb_text_init1(const char *font, struct fs_font **_f)
{
    const char   *fonts[2] = { font, NULL };

    if (NULL == *_f)
	*_f = fs_consolefont(font ? fonts : NULL);
#ifndef X_DISPLAY_MISSING
    if (NULL == *_f && 0 == fs_connect(NULL))
	*_f = fs_open(font ? font : x11_font);
#endif
    if (NULL == *_f) {
	FIM_FPRINTF(stderr, "font \"%s\" is not available\n",font);
	exit(1);
    }
}

#if 1
static const char *default_font[] = {
    /* why the heck every f*cking distribution picks another
       location for these fonts ??? (GK)
       +1 (MM) */
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
    NULL
};
#endif

static int probe_font_file(const char *fontfilename)
{
    	FILE *fp=NULL;
	if ( strlen(fontfilename)>3 && 0 == strcmp(fontfilename+strlen(fontfilename)-3,".gz"))
	{
		#ifdef FIM_USE_ZCAT
		/* FIXME */
		fp = FbiStuff::fim_execlp("zcat","zcat",fontfilename,NULL);
		#endif
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
	if(fp)fclose(fp);
ok:
	return 0;
no:
	if(fp)fclose(fp);
	return -1;
}

struct fs_font* FontServer::fs_consolefont(const char **filename)
{
    int  i=0;
    int  fr;
    const char *h=NULL;
    struct fs_font *f = NULL;
    const char *fontfilename=NULL;
    FILE *fp=NULL;
    char fontfilenameb[FIM_PATH_MAX];

    if (NULL == filename)
	filename = fim::default_font;

    for(i = 0; filename[i] != NULL; i++) {
	if (-1 == access(filename[i],R_OK))
	    continue;
	break;
    }
    fontfilename=filename[i];

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
#endif

    if (NULL == fontfilename) {
	FIM_FPRINTF(stderr, "can't find console font file\n");
	return NULL;
    }

    h = fontfilename+strlen(fontfilename)-3;
    if ( h>fontfilename && 0 == strcmp(h,".gz")) {
	#ifdef FIM_USE_ZCAT
	/* FIXME */
	fp = FbiStuff::fim_execlp("zcat","zcat",fontfilename,NULL);
	#endif
    } else {
	fp = fopen(fontfilename, "r");
    }
    if (NULL == fp) {
	FIM_FPRINTF(stderr, "can't open %s: %s\n",fontfilename,strerror(errno));
	return NULL;
    }

    if (fgetc(fp) != 0x36 ||
	fgetc(fp) != 0x04) {
	FIM_FPRINTF(stderr, "can't use font %s\n",fontfilename);
	return NULL;
    }
//    FIM_FPRINTF(stderr, "using linux console font \"%s\"\n",filename[i]);

    f =(struct fs_font*) fim_calloc(sizeof(*f),1);
    if(!f)goto oops;
    memset(f,0,sizeof(*f));
	
    fgetc(fp);
    f->maxenc = 256;
    f->width  = 8;	/* FIXME */
    f->height = fgetc(fp);
    f->fontHeader.min_bounds.left    = 0;
    f->fontHeader.max_bounds.right   = f->width;
    f->fontHeader.max_bounds.descent = 0;
    f->fontHeader.max_bounds.ascent  = f->height;

    f->glyphs  =(unsigned char*) fim_malloc(f->height * 256);
    if(!f->glyphs) goto oops;
    f->extents = (FSXCharInfo*)fim_malloc(sizeof(FSXCharInfo)*256);
    if(!f->extents) goto oops;
    fr=fread(f->glyphs, 256, f->height, fp);
    if(!fr)return NULL;/* new */
    fclose(fp);

    f->eindex  =(FSXCharInfo**) fim_malloc(sizeof(FSXCharInfo*)   * 256);
    if(!f->eindex) goto oops;
    f->gindex  = (unsigned char**)fim_malloc(sizeof(unsigned char*) * 256);
    if(!f->gindex) goto oops;
    for (i = 0; i < 256; i++) {
	f->eindex[i] = f->extents +i;
	f->gindex[i] = f->glyphs  +i * f->height;
	f->eindex[i]->left    = 0;
	f->eindex[i]->right   = 7;
	f->eindex[i]->width   = 8;/* FIXME */
	f->eindex[i]->descent = 0;
	f->eindex[i]->ascent  = f->height;
    }
    return f;
oops:
    if(f)
    {
    	if(f->eindex) fim_free(f->eindex);
    	if(f->gindex) fim_free(f->gindex);
    	if(f->glyphs) fim_free(f->glyphs);
    	if(f->extents) fim_free(f->extents);
	fim_free(f);
    }
    return NULL;
}
#endif




}


