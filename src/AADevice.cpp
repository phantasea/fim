/* $Id: AADevice.cpp 253 2009-06-13 18:51:26Z dezperado $ */
/*
 AADevice.cpp : aalib device Fim driver file

 (c) 2008-2009 Michele Martone

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

#ifdef FIM_WITH_AALIB

#include "AADevice.h"
#include <aalib.h>

/*
  FIXME : aalib has two resolutions : an input one, and a screen one.
  	  this is not well handled by our code, as we expect a 1:1 mapping.
 */
#define min(x,y) ((x)<(y)?(x):(y))

	int AADevice::clear_rect_(
		void* dst,	// destination gray array and source rgb array
		int oroff,int ocoff,	// row  and column  offset of the first output pixel
		int orows,int ocols,	// rows and columns drawable in the output buffer
		int ocskip		// output columns to skip for each line
	)
	{
		/* output screen variables */
		int 
			oi,// output image row index
			oj;// output image columns index

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

/*		cout << iroff << " " << icoff << " " << irows << " " << icols << " " << icskip << "\n";
		cout << oroff << " " << ocoff << " " << orows << " " << ocols << " " << ocskip << "\n";
		cout << idr << " " << idc << " " << "\n";
		cout << loc << " " << lor << " " << "\n";*/

		/* TODO : unroll me :) */
		for(oi=oroff;oi<lor;++oi)
		for(oj=ocoff;oj<loc;++oj)
		{
			((char*)(dst))[oi*ocskip+oj]=0;
		}
		return  0;
		
	}


	int matrix_copy_rgb_to_gray(
		void* dst, void* src,	// destination gray array and source rgb array
		int iroff,int icoff,	// row  and column  offset of the first input pixel
		int irows,int icols,	// rows and columns in the input image
		int icskip,		// input columns to skip for each line
		int oroff,int ocoff,	// row  and column  offset of the first output pixel
		int orows,int ocols,	// rows and columns drawable in the output buffer
		int ocskip,		// output columns to skip for each line
		int flags		// some flags :flag values in fim.h : FIXME
	)
	{
		/*
		 * This routine will be optimized, some day.
		 * Note that it is not a method.
		 * TODO : optimize and generalize, in all possible ways:
		 * 	RGBTOGRAY
		 * 	RGBTOGRAYGRAYGRAY
		 * 	...
		 *
		 * This routine copies the pixelmap starting at (iroff,icoff) in the input image
		 * to the output buffer, starting at (oroff,ocoff).
		 *
		 * The last source pixel copied will be the one :
		 * 	(min(irows-1,orows-1-oroff+iroff),min(icols-1,ocols-1-ocoff+icoff))
		 * in the input buffer and will be written to:
		 * 	(min(orows-1,irows-1-iroff+oroff),min(ocols-1,icols-1-icoff+ocoff))
		 * in the output buffer.
		 *
		 * It assumes that both pixelmaps are stored in row major order, with row strides
		 * of icskip (columns in the  input matrix between rows) for the  input array src,
		 * of ocskip (columns in the output matrix between rows) for the output array dst.
		 *
		 * The routine needs to know the  input image rows (irows), columns (icols).
		 * The routine needs to know the output image rows (orows), columns (ocols).
		 *
		 * It assumes the input pixelmap having three bytes for pixel, and the puts in the 
		 * output buffer the corresponding single byte gray values.
		 * */

		int
			ii,// output image row index
			ij;// output image columns index

		/* output screen variables */
		int 
			oi,// output image row index
			oj;// output image columns index

		int gray;
		char *srcp;
		int idr,idc,lor,loc;
    		
		int mirror=flags&FIM_FLAG_MIRROR, flip=flags&FIM_FLAG_FLIP;//STILL UNUSED : FIXME

		if ( !src ) return -1;
	
		if( iroff <0 ) return -3;
		if( icoff <0 ) return -4;
		if( irows <=0 ) return -5;
		if( icols <=0 ) return -6;
		if( icskip<0 ) return -7;
		if( oroff <0 ) return -8;
		if( ocoff <0 ) return -9;
		if( orows <=0 ) return -10;
		if( ocols <=0 ) return -11;
		if( ocskip<0 ) return -12;
		if( flags <0 ) return -13;

		if( iroff>irows ) return -3-4*100 ;
		if( icoff>icols ) return -4-6*100;
		if( oroff>orows ) return -8-10*100;
		if( ocoff>ocols ) return -9-11*100;

		if( icskip<icols ) return -7-6*100;
		if( ocskip<ocols ) return -12-11*100;

		/*
		 * orows and ocols is the total number of rows and columns in the output window.
		 * no more than orows-oroff rows and ocols-ocoff columns will be rendered, however
		 * */

//		dr=(min(irows-1,orows-1-oroff+iroff))-(min(orows-1,irows-1-iroff+oroff));
//		dc=(min(icols-1,ocols-1-ocoff+icoff))-(min(ocols-1,icols-1-icoff+ocoff));

		idr = iroff-oroff;
		idc = icoff-ocoff;

		lor = (min(orows-1,irows-1-iroff+oroff));
		loc = (min(ocols-1,icols-1-icoff+ocoff));

/*		cout << iroff << " " << icoff << " " << irows << " " << icols << " " << icskip << "\n";
		cout << oroff << " " << ocoff << " " << orows << " " << ocols << " " << ocskip << "\n";
		cout << idr << " " << idc << " " << "\n";
		cout << loc << " " << lor << " " << "\n";*/

		/* TODO : unroll me and optimize me :) */
		if(!mirror && !flip)
		for(oi=oroff;FIM_LIKELY(oi<lor);++oi)
		for(oj=ocoff;FIM_LIKELY(oj<loc);++oj)
		{
			ii    = oi + idr;
			ij    = oj + idc;
			srcp  = ((char*)src)+(3*(ii*icskip+ij));
			gray  = (int)srcp[0];
			gray += (int)srcp[1];
			gray += (int)srcp[2];
			((char*)(dst))[oi*ocskip+oj]=(char)(gray/3);
		}
		else
		for(oi=oroff;FIM_LIKELY(oi<lor);++oi)
		for(oj=ocoff;FIM_LIKELY(oj<loc);++oj)
		{
			/*
			 * FIXME : these expressions () are correct, but some garbage is printed in flip mode!
			 * therefore instead of lor-oi+1 we use lor-oi !!
			 * DANGER
			 * */
			/*if(flip)  ii    = (lor-oi) + idr;
			else      ii    = oi + idr;

			if(mirror)ij    = (loc-oj) + idc;
			else      ij    = oj + idc;
			if(ij<0)return -1;*/

			ii    = oi + idr;
			ij    = oj + idc;
			
			if(mirror)ij=((icols-1)-ij);
			if( flip )ii=((irows-1)-ii);
			srcp  = ((char*)src)+(3*(ii*icskip+ij));
			if(mirror)ij=((icols-1)-ij);
			if( flip )ii=((irows-1)-ii);

			gray  = (int)srcp[0];
			gray += (int)srcp[1];
			gray += (int)srcp[2];
			((char*)(dst))[oi*ocskip+oj]=(char)(gray/3);
		}

		return  0;
	}

//#define width() aa_imgwidth(ascii_context)
//#define height() aa_imgheight(ascii_context)

//#define width() aa_scrwidth(ascii_context)
//#define height() aa_scrheight(ascii_context)

	int  AADevice::display(
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
		void* rgb = ida_image_img?((struct ida_image*)ida_image_img)->data:NULL;// source rgb array
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

//		I must still decide on the destiny of the following
//		orows  = min( irows-iroff, height());
//		ocols  = min( icols-icoff,  width());// rows and columns to draw in output buffer
//		ocskip = width();// output columns to skip for each line
//
		ocskip = aa_scrwidth(ascii_context);// output columns to skip for each line
		ocskip = aa_imgwidth(ascii_context);// output columns to skip for each line

		/*
		 * FIXME : since aa_flush() poses requirements to the way single viewports are drawn, 
		 * AADevice will behave single-windowed until I get a better understanding of aalib.
		 * Therefore these sanitizing line.
		 */
		//oroff  = ocoff = 0;
		//if(oroff)oroff=min(oroff,40);

		/* we zero the pixel field */
		//img or scr ?!
		//bzero(aa_image(ascii_context),aa_imgheight(ascii_context)*ocskip);
		//bzero(aa_image(ascii_context),width()*height());
		AADevice::clear_rect_( aa_image(ascii_context), oroff,ocoff, oroff+orows,ocoff+ocols, ocskip); 

	//	cout << iroff << " " << icoff << " " << irows << " " << icols << " " << icskip << "\n";
/*		cout << oroff << " " << ocoff << " " << orows << " " << ocols << " " << ocskip << "\n";
		cout << " aa_scrwidth(ascii_context):" << aa_scrwidth(ascii_context) << "  ";
		cout << "aa_scrheight(ascii_context):" <<aa_scrheight(ascii_context) << "\n";
		cout << " aa_imgwidth(ascii_context):" << aa_imgwidth(ascii_context) << "  ";
		cout << "aa_imgheight(ascii_context):" <<aa_imgheight(ascii_context) << "\n";
		cout << "ocskip " << ocskip << "\n";
		cout << "ocols "  << ocols  << "\n";*/
		
		if(icols<ocols) { ocoff+=(ocols-icols-1)/2; ocols-=(ocols-icols-1)/2; }
		if(irows<orows) { oroff+=(orows-irows-1)/2; orows-=(orows-irows-1)/2; }

		int r;
#if (!FIM_AALIB_DRIVER_DEBUG)
		if((r=matrix_copy_rgb_to_gray(
				aa_image(ascii_context),rgb,
				iroff,icoff, // row and column offset of the first input pixel
				irows,icols,// rows and columns in the input image
				icskip,	// input columns to skip for each line
				oroff,ocoff,// row and column offset of the first output pixel
				oroff+orows,ocoff+ocols,// rows and columns to draw in output buffer
				ocskip,// output columns to skip for each line
				flags
			)))
			return r;
#endif
			//return -50;

/*		aa_putpixel(ascii_context,ocols-1,orows-1,0xAA);
		aa_putpixel(ascii_context,0,orows-1,0xAA);
		aa_putpixel(ascii_context,ocols-1,0,0xAA);*/
/*		aa_putpixel(ascii_context, aa_scrwidth(ascii_context)+0, aa_scrheight(ascii_context)+0, 0xAA);
		aa_putpixel(ascii_context, aa_scrwidth(ascii_context)+1, aa_scrheight(ascii_context)+1, 0xAA);
		aa_putpixel(ascii_context, aa_scrwidth(ascii_context)+2, aa_scrheight(ascii_context)+2, 0xAA);
		aa_putpixel(ascii_context, aa_scrwidth(ascii_context)+3, aa_scrheight(ascii_context)+3, 0xAA);
		aa_putpixel(ascii_context, aa_scrwidth(ascii_context)+4, aa_scrheight(ascii_context)+4, 0xAA);
		aa_putpixel(ascii_context, aa_scrwidth(ascii_context)+5, aa_scrheight(ascii_context)+5, 0xAA);
		aa_putpixel(ascii_context, aa_scrwidth(ascii_context)+6, aa_scrheight(ascii_context)+6, 0xAA);

		aa_putpixel(ascii_context, aa_imgwidth(ascii_context)-1, aa_imgheight(ascii_context)-1, 0xAA);
		aa_putpixel(ascii_context, aa_imgwidth(ascii_context)-2, aa_imgheight(ascii_context)-2, 0xAA);*/

	//	((char*)aa_image(ascii_context))[]=;
		
//		std::cout << "width() : " << width << "\n"; //		std::cout << "height() : " << height << "\n";
		aa_render (ascii_context, &aa_defrenderparams,0, 0, width() , height() );
		aa_flush(ascii_context);
		return 0;
	}

	int AADevice::initialize(key_bindings_t &key_bindings)
	{
		aa_parseoptions (NULL, NULL, NULL, NULL);

		ascii_context = NULL;

		memcpy (&ascii_hwparms, &aa_defparams, sizeof (struct aa_hardware_params));
		//ascii_rndparms = aa_getrenderparams();
		//aa_parseoptions (&ascii_hwparms, ascii_rndparms, &argc, argv);

//		NOTE: if uncommenting this, remember to #ifdef HAVE_GETENV
//		char *e;int v;
//		if((e=getenv("COLUMNS"))!=NULL && (v=atoi(e))>0) ascii_hwparms.width  = v-1;
//		if((e=getenv("LINES"  ))!=NULL && (v=atoi(e))>0) ascii_hwparms.height = v-1;
//		if((e=getenv("COLUMNS"))!=NULL && (v=atoi(e))>0) ascii_hwparms.recwidth  = v;
//		if((e=getenv("LINES"  ))!=NULL && (v=atoi(e))>0) ascii_hwparms.recheight = v;

//		ascii_hwparms.width  = 80;
//		ascii_hwparms.height = 56;
//
//		ascii_hwparms.width  = 128-1;
//		ascii_hwparms.height = 48 -1;

		/*ascii_hwparms.width()  = 4;
		ascii_hwparms.height() = 4;*/
		
		name[0]='\0';
		name[1]='\0';
		ascii_save.name = name;
		ascii_save.format = &aa_text_format;
		ascii_save.file = NULL;
//		ascii_context = aa_init (&save_d, &ascii_hwparms, &ascii_save);
		ascii_context = aa_autoinit (&ascii_hwparms);
		if(!ascii_context)
		{
			std::cout << "problem initializing aalib!\n";
			return -1;
		}
		if(!aa_autoinitkbd(ascii_context, 0))
		{
			std::cout << "problem initializing aalib keyboard!\n";
			return -1;
		}
		/*if(!aa_autoinitmouse(ascii_context, 0))
		{
			std::cout << "problem initializing aalib mouse!\n";
			return -1;
		}*/
		aa_hidecursor (ascii_context);

#if 0
		/* won't help in the ill X11-windowed aalib shipped with many distros case */
		key_bindings["Left" ]=AA_LEFT;
		key_bindings["Right"]=AA_RIGHT;
		key_bindings["Up"   ]=AA_UP;
		key_bindings["Down" ]=AA_DOWN;
		key_bindings["Esc"  ]=AA_ESC;
#endif

		return 0;
	}

	void AADevice::finalize() 
	{
		finalized=true;
		aa_close(ascii_context);
	}
	int AADevice::get_chars_per_line(){return aa_scrwidth(ascii_context);}
	int AADevice::get_chars_per_column(){return aa_scrheight(ascii_context);}
	int AADevice::txt_width() { return aa_scrwidth(ascii_context ) ;}
	int AADevice::txt_height(){ return aa_scrheight(ascii_context) ;}
	int AADevice::width() { return aa_imgwidth(ascii_context ) ;}
	int AADevice::height(){ return aa_imgheight(ascii_context) ;}

	int AADevice::init_console()
	{
		//mc.setRows ( -height()/2);
		mc.setRows ( get_chars_per_column()/2 );
		mc.reformat(  txt_width()   );
		return 0;
	}

	int AADevice::fs_puts(struct fs_font *f, unsigned int x, unsigned int y, const unsigned char *str)
	{
#if (!FIM_AALIB_DRIVER_DEBUG)
		aa_puts(ascii_context,x,y,
			//AA_REVERSE,
			AA_NORMAL,
			//AA_SPECIAL,
			(const char*)str);
#endif
		return 0;
	}

	void AADevice::flush()
	{
		aa_flush(ascii_context);
	}

	int AADevice::clear_rect(int x1, int x2, int y1, int y2)
	{
		/* FIXME : only if initialized !
		 * TODO : define the exact conditions to use this method
		 * */
		
		return clear_rect_(aa_image(ascii_context),y1, x1, y2-y1+1, x2-x1+1,aa_imgwidth(ascii_context));
	}

	int AADevice::status_line(const unsigned char *msg)
	{
#if (!FIM_AALIB_DRIVER_DEBUG)
		aa_printf(ascii_context,0,txt_height()-1,AA_NORMAL,"%s",msg);
#endif
		aa_flush(ascii_context);
		return 0;
	}

	AADevice::~AADevice()
	{
		/* FIXME : seems like some aa stuff doesn't get freed. is it possible ? */
		if(!finalized)finalize();// finalize should be called explicitly !
	}

	int AADevice::get_input(unsigned int * c)
	{
		*c = 0x0;	/* blank */
		if(!c)return 0;
		*c = aa_getevent(ascii_context,0);/* 1 if want to receive AA_RELEASE events, too */
		if(*c==AA_UNKNOWN)*c=0;
		if(*c)std::cout << "";/* FIXME : removing this breaks things. console-related problem, I guess */
		if(!*c)return 0;
#if 0
		if(*c==AA_UP   ){*c=272;return 1;}
		if(*c==AA_DOWN ){*c=274;return 1;}
		if(*c==AA_LEFT ){*c=276;return 1;}
		if(*c==AA_RIGHT){*c=275;return 1;}
#else
		if(*c==AA_UP   ){*c=AA_UP;return 1;}
		if(*c==AA_DOWN ){*c=AA_DOWN;return 1;}
		if(*c==AA_LEFT ){*c=AA_LEFT;return 1;}
		if(*c==AA_RIGHT){*c=AA_RIGHT;return 1;}
#endif
		/* FIXME : see defaultConfiguration.cpp */
		if(*c==AA_UP   ){*c=4283163;return 1;}
		if(*c==AA_DOWN ){*c=4348699;return 1;}
		if(*c==AA_LEFT ){*c=4479771;return 1;}
		if(*c==AA_RIGHT){*c=4414235;return 1;}
		if(*c==AA_BACKSPACE){*c=127;return 1;}

		/* FIXME : these five bindings work only under X .. */
		if(*c==65765){*c=2117491483;return 1;}/* pageup   (arbitrary) */
		if(*c==65766){*c=2117425947;return 1;}/* pagedown (arbitrary) */
		if(*c==65779){*c=2117229339;return 1;}/* ins  (arbitrary) */
		if(*c==65760){*c=2117163803;return 1;}/* home (arbitrary) */
		if(*c==65767){*c=2117360411;return 1;}/* end  (arbitrary) */

		if(*c==65907)
		{
			status_line((const unsigned char*)"control key not yet supported in aa. sorry!");
			return 1;
			/* left ctrl (arbitrary) */
		}
		if(*c==65908)
		{
			status_line((const unsigned char*)"control key not yet supported in aa. sorry!");
			return 1;
			/* right ctrl (arbitrary) */
		}
		if(*c==65909)
		{
			status_line((const unsigned char*)"lock key not yet supported in aa. sorry!");
			return 1;
			/* right ctrl (arbitrary) */
		}
		if(*c==65906)
		{
			status_line((const unsigned char*)"shift key not yet supported in aa. sorry!");
			return 1;
			/* shift (arbitrary) */
		}
		if(*c==AA_ESC){*c=27;return 1;}/* esc  */
		if(*c==AA_MOUSE)
		{
			status_line((const unsigned char*)"mouse events not yet supported in aa. sorry!");
			return 1;
		}/* esc  */
		if(*c==AA_RESIZE )
		{
			status_line((const unsigned char*)"window resizing not yet supported. sorry!");
			/*aa_resize(ascii_context);*//*we are not yet ready : the Window and Viewport stuff .. */
			return 0;
		}/* esc  */

		//std::cout << "event : " << *c << "\n";
		//if(*c<0x80) return 1;
		return 1;
		//return 0;
	}
#endif
