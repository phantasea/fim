/* $Id: SDLDevice.cpp 268 2009-12-08 23:05:55Z dezperado $ */
/*
 SDLDevice.cpp : sdllib device Fim driver file

 (c) 2008-2009 Michele Martone
 based on code (c) 1998-2006 Gerd Knorr <kraxel@bytesex.org>

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
 * NOTES : The SDL support is INCOMPLETE:
 *
 *  - largely inefficient, please do not be surprised
 *  - input problems when coupled with readline
 */
#include "fim.h"

#ifdef FIM_WITH_LIBSDL

#include "SDLDevice.h"

#define min(x,y) ((x)<(y)?(x):(y))

#define FIM_SDL_DEBUG 1
#undef FIM_SDL_DEBUG

#ifdef FIM_SDL_DEBUG
#define FIM_SDL_INPUT_DEBUG(C,MSG)  \
/* i miss sooo much printf() :'( */ \
std::cout << (size_t)getmilliseconds() << " : "<<MSG<<" : "; \
std::cout.setf ( std::ios::hex, std::ios::basefield ); \
std::cout.setf ( std::ios::showbase ); \
std::cout << *(int*)(C) <<"\n"; \
std::cout.unsetf ( std::ios::showbase ); \
std::cout.unsetf ( std::ios::hex );
#else
#define FIM_SDL_INPUT_DEBUG(C,MSG) {}
#endif

	/* WARNING : TEMPORARY, FOR DEVELOPEMENT PURPOSES */

	SDLDevice::SDLDevice(MiniConsole & mc_):DisplayDevice(mc_),vi(NULL)
	{
		FontServer::fb_text_init1(fontname,&f);	// FIXME : move this outta here
		keypress = 0;
		h=0;
		current_w=current_h=0;
	}

	int SDLDevice::clear_rect_(
		void* dst,	// destination array 
		int oroff,int ocoff,	// row  and column  offset of the first output pixel
		int orows,int ocols,	// rows and columns drawable in the output buffer
		int ocskip		// output columns to skip for each line
	)
	{
		/* output screen variables */
//		int 
//			oi,// output image row index
//			oj;// output image columns index

		int lor,loc;
    		
		if( oroff <0 ) return -8;
		if( ocoff <0 ) return -9;
		if( orows <=0 ) return -10;
		if( ocols <=0 ) return -11;
		if( ocskip<0 ) return -12;

		if( oroff>orows ) return -8-10*100;
		if( ocoff>ocols ) return -9-11*100;

		if( ocskip<ocols ) return -12-11*100;

		/*
		 * orows and ocols is the total number of rows and columns in the output window.
		 * no more than orows-oroff rows and ocols-ocoff columns will be rendered, however
		 * */

		lor = orows-1;
		loc = ocols-1;
		
		return  -1;
	}

	int  SDLDevice::display(
		//struct ida_image *img, // source image structure
		void *ida_image_img, // source image structure
		//void* rgb,// source rgb array
		int iroff,int icoff, // row and column offset of the first input pixel
		int irows,int icols,// rows and columns in the input image
		int icskip,	// input columns to skip for each line
		int oroff,int ocoff,// row and column offset of the first output pixel
		int orows,int ocols,// rows and columns to draw in output buffer
		int ocskip,// output columns to skip for each line
		int flags// some flags
	)
	{
		
		/*
		 * TODO : generalize this routine and put in common.cpp
		 * */
		/*
		 * FIXME : centering mechanisms missing here; an intermediate function
		 * shareable with FramebufferDevice would be nice, if implemented in AADevice.
		 * */
		//was : void
		unsigned char* rgb = ida_image_img?((struct ida_image*)ida_image_img)->data:NULL;// source rgb array
		if ( !rgb ) return -1;
	
		if( iroff <0 ) return -2;
		if( icoff <0 ) return -3;
		if( irows <=0 ) return -4;
		if( icols <=0 ) return -5;
		if( icskip<0 ) return -6;
		if( oroff <0 ) return -7;
		if( ocoff <0 ) return -8;
		if( orows <=0 ) return -9;
		if( ocols <=0 ) return -10;
		if( ocskip<0 ) return -11;
		if( flags <0 ) return -12;

		if( iroff>irows ) return -2-3*100 ;
		if( icoff>icols ) return -3-5*100;
//		if( oroff>orows ) return -7-9*100;//EXP
//		if( ocoff>ocols ) return -8-10*100;//EXP
		if( oroff>height() ) return -7-9*100;//EXP
		if( ocoff>width()) return -8-10*100;//EXP

		if( icskip<icols ) return -6-5*100;
		if( ocskip<ocols ) return -11-10*100;
	
		orows  = min( orows, height());
		ocols  = min( ocols,  width()); 
		ocskip = width(); 	//FIXME maybe this is not enough and should be commented or rewritten!

		if( orows  > height() ) return -9 -99*100;
		if( ocols  >  width() ) return -10-99*100;
		if( ocskip <  width() ) return -11-99*100;
		if( icskip<icols ) return -6-5*100;

		ocskip = width();// output columns to skip for each line

		if(icols<ocols) { ocoff+=(ocols-icols-1)/2; ocols-=(ocols-icols-1)/2; }
		if(irows<orows) { oroff+=(orows-irows-1)/2; orows-=(orows-irows-1)/2; }


//		int h=1;
//		int x, y;
		int ytimesw;

		if(SDL_MUSTLOCK(screen))
		{
			if(SDL_LockSurface(screen) < 0) return -1;
		}

		int idr,idc,lor,loc;

		idr = iroff-oroff;
		idc = icoff-ocoff;

		lor = (min(orows-1,irows-1-iroff+oroff));
		loc = (min(ocols-1,icols-1-icoff+ocoff));

		int ii,ij;
		int oi,oj;
		int mirror=flags&FIM_FLAG_MIRROR, flip=flags&FIM_FLAG_FLIP;//STILL UNUSED : FIXME
		unsigned char * srcp;

		// FIXME : temporary
//		clear_rect(  ocoff, ocoff+ocols, oroff, oroff+orows); 
//		clear_rect(  0, ocols, 0, orows); 
		clear_rect(  0, width()-1, 0, height()-1); 

		if(!mirror && !flip)
		for(oi=oroff;FIM_LIKELY(oi<lor);++oi)
		for(oj=ocoff;FIM_LIKELY(oj<loc);++oj)
		{
			ytimesw = (oi)*screen->pitch/Bpp;

			ii    = oi + idr;
			ij    = oj + idc;
			srcp  = ((unsigned char*)rgb)+(3*(ii*icskip+ij));

			setpixel(screen, oj, ytimesw, (int)srcp[0], (int)srcp[1], (int)srcp[2]);
		}
		else
		for(oi=oroff;FIM_LIKELY(oi<lor);++oi)
		for(oj=ocoff;FIM_LIKELY(oj<loc);++oj)
		{

			ytimesw = (oi)*screen->pitch/Bpp;
			/* UNFINISHED : FIX ME */
			ii    = oi + idr;
			ij    = oj + idc;
			
			if(mirror)ij=((icols-1)-ij);
			if( flip )ii=((irows-1)-ii);
			srcp  = ((unsigned char*)rgb)+(3*(ii*icskip+ij));

			setpixel(screen, oj, ytimesw, (int)srcp[0], (int)srcp[1], (int)srcp[2]);
		}

		if(SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);

		SDL_Flip(screen);

		return 0;
	}

	int SDLDevice::initialize(key_bindings_t &key_bindings)
	{
		/*
		 *
		 * */
		if (SDL_Init(SDL_INIT_VIDEO) < 0 ) return 1;
		/* automatic selection of video mode (the current one) */
		if (!(screen = SDL_SetVideoMode(0,	/* width  */
						0,	/* height */
						0,	/* depth (bits per pixel) */
						SDL_FULLSCREEN|SDL_HWSURFACE)))
		{
			std::cout << "problems initializing sdl .. \n";
			SDL_Quit();
			return -1;
		}
		else
		{
			vi = SDL_GetVideoInfo();
			if(!vi)
				return -1;

			current_w=vi->current_w;
			current_h=vi->current_h;
			bpp      =vi->vfmt->BitsPerPixel;
			Bpp      =vi->vfmt->BytesPerPixel;
		}
		/* Enable Unicode translation ( for a more flexible input handling ) */
	        SDL_EnableUNICODE( 1 );

		key_bindings["PageUp" ]=SDLK_PAGEUP;
		key_bindings["PageDown" ]=SDLK_PAGEDOWN;
		key_bindings["Left" ]=SDLK_LEFT;
		key_bindings["Right"]=SDLK_RIGHT;
		key_bindings["Up"   ]=SDLK_UP;
		key_bindings["Down" ]=SDLK_DOWN;
		key_bindings["Space"]=SDLK_SPACE;
		key_bindings["End"  ]=SDLK_END;
		key_bindings["Home" ]=SDLK_HOME;
		key_bindings["F1" ]=SDLK_F1;
		key_bindings["F2" ]=SDLK_F2;
		key_bindings["F3" ]=SDLK_F3;
		key_bindings["F4" ]=SDLK_F4;
		key_bindings["F5" ]=SDLK_F5;
		key_bindings["F6" ]=SDLK_F6;
		key_bindings["F7" ]=SDLK_F7;
		key_bindings["F8" ]=SDLK_F8;
		key_bindings["F9" ]=SDLK_F9;
		key_bindings["F10"]=SDLK_F10;
		key_bindings["F11"]=SDLK_F11;
		key_bindings["F12"]=SDLK_F12;

		// textual console reformatting
		mc.setGlobalVariable(FIM_VID_CONSOLE_ROWS,height()/(2*f->height));
		mc.reformat(    width() /    f->width   );

		return 0;
	}

	void SDLDevice::finalize()
	{
		finalized=true;
		SDL_Quit();
	}

	int SDLDevice::get_chars_per_column()
	{
		return height() / f->height;
	}

	int SDLDevice::get_chars_per_line()
	{
		return width() / f->width;
	}

	int SDLDevice::width()
	{
		return current_w;
	}

	int SDLDevice::height()
	{
		return current_h;
	}

	inline void SDLDevice::setpixel(SDL_Surface *screen, int x, int y, Uint8 r, Uint8 g, Uint8 b)
	{
		/*
		 * this function is taken straight from the sdl documentation: there are smarter ways to do this.
		 * */

		switch (bpp)
		{
		case  8:
		{
			Uint8 *pixmem8;
			Uint8 colour;
			colour = SDL_MapRGB( screen->format, b, g, r );
			pixmem8 = (Uint8*)((char*)( screen->pixels)  + (y + x)*Bpp);
			*pixmem8 = colour;
		}
		break;
		case 15:
		case 16:
		{
			Uint16 *pixmem16;
			Uint16 colour;
			colour = SDL_MapRGB( screen->format, b, g, r );
			pixmem16 = (Uint16*)((char*)( screen->pixels)  + (y + x)*Bpp);
			*pixmem16 = colour;
		}
		break;
		case 24:
		{
			Uint32 *pixmem32;
			Uint32 colour;
			colour = SDL_MapRGB( screen->format, b, g, r );
			pixmem32 = (Uint32*)((char*)( screen->pixels)  + (y + x)*Bpp);
			*pixmem32 = colour;
		}
		break;
		case 32:
		{
			Uint32 *pixmem32;
			Uint32 colour;
			colour = SDL_MapRGBA( screen->format, b, g, r, 255 );
			pixmem32 = (Uint32*)((char*)( screen->pixels)  + (y + x)*Bpp);
			*pixmem32 = colour;
		}
		break;
		default:
		{
			/* 1,2,4 modes unsupported for NOW */
		}
		}

	}

	int SDLDevice::get_input(unsigned int * c )
	{
//		int keypress=0;
		bool ctrl_on=0;
		bool alt_on=0;
		bool shift_on=0;
		*c = 0x0;	/* blank */

//		while(SDL_PollEvent(&event))
		if(SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
				keypress = 1;
				
				break;
				case SDL_KEYDOWN:

				if(event.key.keysym.mod == KMOD_RCTRL || event.key.keysym.mod == KMOD_LCTRL ) ctrl_on=true;
				if(event.key.keysym.mod == KMOD_RALT  || event.key.keysym.mod == KMOD_LALT  )  alt_on=true;
				if(event.key.keysym.mod == KMOD_RSHIFT  || event.key.keysym.mod == KMOD_LSHIFT  )  shift_on=true;

			//	std::cout << "sym : " << (int)event.key.keysym.sym << "\n" ;
			//	std::cout << "uni : " << (int)event.key.keysym.unicode<< "\n" ;
			//	if(shift_on)std::cout << "shift_on\n";

				if( event.key.keysym.unicode == 0x0 )
				{
					/* arrows and stuff */
					if(event.key.keysym.sym<256)
					{
						FIM_SDL_INPUT_DEBUG(c,"uhm");
						*c=event.key.keysym.sym;
						return 1;
					}
					else
					if(
						event.key.keysym.sym!=SDLK_LSHIFT
					&&	event.key.keysym.sym!=SDLK_RSHIFT
					&&	event.key.keysym.sym!=SDLK_LALT
					&&	event.key.keysym.sym!=SDLK_RALT
					&&	event.key.keysym.sym!=SDLK_LCTRL
					&&	event.key.keysym.sym!=SDLK_RCTRL
					)
					{
						/* arrows.. .. */
						*c=event.key.keysym.sym;
						FIM_SDL_INPUT_DEBUG(c,"arrow");
						return 1;
					}
					else
					{
						FIM_SDL_INPUT_DEBUG(c,"shift");
						/* we ignore lone shift or alt .. */
						return 0;
					}
				}

				if(alt_on)
				{
					*c=(unsigned char)event.key.keysym.unicode;
					*c|=(1<<31);	/* FIXME : a dirty trick */
					return 1;
				}

				if( 	event.key.keysym.unicode < 0x80)
				{
					/* 
					 * SDL documentation 1.2.12 about unicode:
					 * It is useful to note that unicode values < 0x80 translate directly
					 * a characters ASCII value.
					 * */
			//		if(event.key.keysym.mod == KMOD_RCTRL || event.key.keysym.mod == KMOD_LCTRL ) std::cout << "ctrl ! \n";
					*c=(unsigned char)event.key.keysym.unicode;

					if(ctrl_on)
					{
						// if(*c-1+'a'=='c')std::exit(-1);//works 
						if(*c-1+'a'<='z')
						{
							//std::cout << "with control : " << *c+'a'-1 << "\n";
						}
						else
						{
							/* other chars */
							*c-='a';
							*c+= 1 ;
						}
					}
					if(*c)	/* !iscntrl(c) */
					{
						/* the usual chars */
						FIM_SDL_INPUT_DEBUG(c,"keysim");
						return 1;
					}
					else	/*  iscntrl(c) */
					{
						FIM_SDL_INPUT_DEBUG(c,"iscntrl");
						return 0;
					}
					/*
					 * p.s.: note that we get 0 in some cases (e.g.: KMOD_RSHIFT, ...).
					 * */
				}
				else
				{
					cout << "sorry, no support for wide chars in fim\n";
					/*  no support for wide chars in fim */
					return 0;
				}
				return 0;

				break;
			}
			return 0;
		}

		return 0;
	}

	int SDLDevice::fill_rect(int x1, int x2, int y1,int y2, int color)
	{
		int y;
		/*
		 * This could be optimized
		 * */
		for(y=y1;y<y2;++y)
		{
			memset(((char*)(screen->pixels)) + y*screen->pitch + x1*Bpp,color, (x2-x1)* Bpp);
		}
		return 0;
	}

	int SDLDevice::clear_rect(int x1, int x2, int y1,int y2)
	{
		int y;
		/*
		 * This could be optimized
		 * */
		for(y=y1;y<=y2;++y)
		{
			bzero(((char*)(screen->pixels)) + y*screen->pitch + x1*Bpp, (x2-x1+1)* Bpp);
		}
		return 0;
	}

void SDLDevice::fs_render_fb(int x_, int y, FSXCharInfo *charInfo, unsigned char *data)
{

/* 
 * These preprocessor macros should serve *only* for font handling purposes.
 * */
#define BIT_ORDER       BitmapFormatBitOrderMSB
#ifdef BYTE_ORDER
#undef BYTE_ORDER
#endif
#define BYTE_ORDER      BitmapFormatByteOrderMSB
#define SCANLINE_UNIT   BitmapFormatScanlineUnit8
#define SCANLINE_PAD    BitmapFormatScanlinePad8
#define EXTENTS         BitmapFormatImageRectMin

#define SCANLINE_PAD_BYTES 1
#define GLWIDTHBYTESPADDED(bits, nBytes)                                    \
        ((nBytes) == 1 ? (((bits)  +  7) >> 3)          /* pad to 1 byte  */\
        :(nBytes) == 2 ? ((((bits) + 15) >> 3) & ~1)    /* pad to 2 bytes */\
        :(nBytes) == 4 ? ((((bits) + 31) >> 3) & ~3)    /* pad to 4 bytes */\
        :(nBytes) == 8 ? ((((bits) + 63) >> 3) & ~7)    /* pad to 8 bytes */\
        : 0)

	int row,bit,bpr,x;

	bpr = GLWIDTHBYTESPADDED((charInfo->right - charInfo->left), SCANLINE_PAD_BYTES);
	for (row = 0; row < (charInfo->ascent + charInfo->descent); row++)
	{
		for (x = 0, bit = 0; bit < (charInfo->right - charInfo->left); bit++) 
		{
			if (data[bit>>3] & fs_masktab[bit&7])
			{	// WARNING !
				setpixel(screen,x_+x,(y+row)*screen->pitch/Bpp,(Uint8)0xff,(Uint8)0xff,(Uint8)0xff);
			}
			x += Bpp/Bpp;/* FIXME */
		}
		data += bpr;
	}

#undef BIT_ORDER
#undef BYTE_ORDER
#undef SCANLINE_UNIT
#undef SCANLINE_PAD
#undef EXTENTS
#undef SCANLINE_PAD_BYTES
#undef GLWIDTHBYTESPADDED
}

int SDLDevice::fs_puts(struct fs_font *f, unsigned int x, unsigned int y, const unsigned char *str)
{
    int i,c/*,j,w*/;

    for (i = 0; str[i] != '\0'; i++) {
	c = str[i];
	if (NULL == f->eindex[c])
	    continue;
	/* clear with bg color */
//	w = (f->eindex[c]->width+1)*Bpp;
#if 0
#ifdef FIM_IS_SLOWER_THAN_FBI
	for (j = 0; j < f->height; j++) {
/////	    memset_combine(start,0x20,w);
	    memset(start,0,w);
	    start += fb_fix.line_length;
	}
#else
	//sometimes we can gather multiple calls..
	if(fb_fix.line_length==(unsigned int)w)
	{
		//contiguous case
		memset(start,0,w*f->height);
	    	start += fb_fix.line_length*f->height;
	}
	else
	for (j = 0; j < f->height; j++) {
	    memset(start,0,w);
	    start += fb_fix.line_length;
	}
#endif
#endif
	/* draw char */
	//fs_render_fb(fb_fix.line_length,f->eindex[c],f->gindex[c]);
	fs_render_fb(x,y,f->eindex[c],f->gindex[c]);
	x += f->eindex[c]->width;
	/* FIXME : SLOW ! */
	if ((int)x > width() - f->width)
		goto err;
    }
	return x;
err:
	return -1;
}

	int SDLDevice::status_line(const unsigned char *msg)
	{
		if(SDL_MUSTLOCK(screen))
		{
			if(SDL_LockSurface(screen) < 0) return -1;
		}

		int y;
		int ys=3;// FIXME

		y = height() - f->height - ys;
		clear_rect(0, width()-1, y+1,y+f->height+ys-1);
		fs_puts(f, 0, y+ys, msg);
		fill_rect(0,width()-1, y, y+1, 0xFF);	// FIXME : NO 1!

		if(SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
		SDL_Flip(screen);
		return 0;
	}

	int SDLDevice::catchInteractiveCommand(int seconds)
	{
		/*
		 * Whether there is some input in the input queue.
		 * Note that in this way the event will be lost.
		 * */
		return (SDL_PollEvent(&event));
	}

	void SDLDevice::flush()
	{
	}

	void SDLDevice::lock()
	{
		if(SDL_MUSTLOCK(screen))
		{
			if(SDL_LockSurface(screen) < 0) return;
		}
	}

	void SDLDevice::unlock()
	{
		if(SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
		SDL_Flip(screen);
		
	}

#endif
