/* $Id: Image.cpp 259 2009-10-07 15:08:58Z dezperado $ */
/*
 Image.cpp : Image manipulation and display

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

#include "Image.h"

/*
 * TODO :
 *	Windowing related problems:
 *
 * 	Implement a mechanism such that each Image instance owns
 *	one only copy of the original image, and zero or more rescaled versions,
 *	for display use only.
 * 	Once freed, an image could free all of its buffers, depending on the caching policy.
 *
 * 	When windowing will be implemented, note that redisplay will be also affected
 * 	after window geometry change. Update mechanisms are needed..
 */
namespace fim
{
/*
 *	There is a general rule here:
 *	 Public functions should be safe when called in 
 *	 any internal state from the outside.
 *	 Private ones are stricter.
 * 
 */
	int Image::original_width()
	{
		//WARNING : assumes the image is valid
		if(orientation%2) return fimg->i.height;
		return fimg->i.width;
	}

	int Image::original_height()
	{
		//WARNING : assumes the image is valid
		if(orientation%2) return fimg->i.width;
		return fimg->i.height;
	}

	int Image::width()
	{
		//WARNING : assumes the image is valid
		return img->i.width;
	}

	int Image::height()
	{
		//WARNING : assumes the image is valid
		return img->i.height;
	}

	Image::Image(const char *fname_, FILE*fd):
		scale(0.0),
		ascale(0.0),
		newscale(0.0),
		angle(0.0),
		newangle(0.0),
		page(0),
                img     (NULL),
                fimg    (NULL),
		orientation(0),
                invalid(0),
		no_file(true),
		fis(fim::string(fname_)==fim::string(FIM_STDIN_IMAGE_NAME)?FIM_E_STDIN:FIM_E_FILE),
                fname     ("")

	{
		/*
		 *	an image object is created from an image filename
		 */
		reset();	// pointers blank
		if( !load(fname_,fd,0) || check_invalid() || (!fimg) ) 
		{
			cout << "warning : invalid loading ! \n";
			if( getGlobalIntVariable(FIM_VID_DISPLAY_STATUS_BAR)||getGlobalIntVariable(FIM_VID_DISPLAY_BUSY))
				cc.set_status_bar( fim::string("error while loading \"")+ fim::string(fname_)+ fim::string("\"") , "*");
			invalid = 1;
			throw FimException();
		}
		else
		{
		}
	}

	void Image::reset()
	{
		/*
		 * pointers are blanked and values set to default 
		 * */
                scale    = 1.0;
                newscale = 1.0;
                ascale   = 1.0;
                angle   = 0.0;
		setVariable(FIM_VID_SCALE  ,scale*100);
		setVariable(FIM_VID_ASCALE ,ascale);
		setVariable(FIM_VID_ANGLE ,angle);
		no_file=true;	//reloading allowed

                invalid=0;
                fimg    = NULL;
                img     = NULL;
                orientation=0;
		setVariable(FIM_VID_ORIENTATION ,0);
	}
	
	bool Image::reload()
	{
		/*
			reloads the file (no hope for streams, therefore)
			FIXME : still unused
		*/
		bool b=false;
		FILE *fd=fopen(fname.c_str(),"r");
		if(!fd)
			return b;
		b=load(fname.c_str(),fd,page);
		fclose(fd);// FIXME : the fd could already be closed !
		return b;
	}
	
	bool Image::load(const char *fname_, FILE* fd, int want_page)
	{
		/*
		 *	an image is loaded and initializes this image.
		 *	returns false if the image does not load
		 */
		if(fname_==NULL && fname==""){return false;}//no loading = no state change
		this->free();
		fname=fname_;
		if( getGlobalIntVariable(FIM_VID_DISPLAY_STATUS_BAR)||getGlobalIntVariable(FIM_VID_DISPLAY_BUSY))
		{
			if( getGlobalIntVariable(FIM_VID_WANT_PREFETCH) == 1)
				cc.set_status_bar("please wait while prefetching...", "*");
			else
				cc.set_status_bar("please wait while reloading...", "*");
		}

		fimg = FbiStuff::read_image((char*)fname_,fd,want_page);

    		if(strcmp(FIM_STDIN_IMAGE_NAME,fname_)==0)
		{
			no_file=true;	//no file is associated to this image (to prevent reloading)
			fis = FIM_E_STDIN; // yes, it seems redundant but it is necessary
		}
		else 
			no_file=false;	//reloading allowed

		img=fimg;	/* no scaling : one copy only */
		should_redraw();

		if(! img)
		{
			cout<<"warning : image loading error!\n"   ;invalid=1;return false;
		}
		else page=want_page;


#ifdef FIM_NAMESPACES
		setVariable(FIM_VID_HEIGHT ,(int)fimg->i.height);
		setVariable(FIM_VID_WIDTH ,(int)fimg->i.width );
		setVariable(FIM_VID_SHEIGHT,(int) img->i.height);
		setVariable(FIM_VID_SWIDTH,(int) img->i.width );
		if(cc.displaydevice)
		setVariable(FIM_VID_FIM_BPP ,(int) cc.displaydevice->get_bpp());
		setVariable(FIM_VID_SCALE  ,newscale*100);
		setVariable(FIM_VID_ASCALE,ascale);
		setVariable(FIM_VID_ANGLE , angle);
		setVariable(FIM_VID_NEGATED , 0);
#endif

		setGlobalVariable(FIM_VID_HEIGHT ,(int)fimg->i.height);
		setGlobalVariable(FIM_VID_WIDTH  ,(int)fimg->i.width );
		setGlobalVariable(FIM_VID_SHEIGHT,(int) img->i.height);
		setGlobalVariable(FIM_VID_SWIDTH ,(int) img->i.width );
		if(cc.displaydevice)
		setGlobalVariable(FIM_VID_FIM_BPP ,(int) cc.displaydevice->get_bpp());
		//setGlobalVariable(FIM_VID_SCALE  ,newscale*100);
		//setGlobalVariable(FIM_VID_ASCALE ,ascale);
		return true;
	}

	Image::~Image()
	{
		/*
		 * buffers are freed
		 * */
#ifdef FIM_CACHE_DEBUG
		std::cout << "freeing Image " << this << "\n";
#endif
		this->free();
	}

        int Image::tiny()const
	{
		/*
		 * image width or height is <= 1
		 * */
		if(!img)return 1; return ( img->i.width<=1 || img->i.height<=1 );
	}

	int Image::scale_multiply(double  sm)
	{
		/*
		 * current scale is multiplied by a factor
		 * */
		if(scale*sm>0.0)newscale=scale*sm;rescale();return 0;
	}

	int Image::scale_increment(double ds)
	{
		/*
		 * current scale is multiplied by a factor
		 * */
		if(scale+ds>0.0)newscale=scale+ds;rescale();return 0;
	}

	int Image::setscale(double ns)
	{
		/*
		 * a new scale is set
		 * */
		newscale=ns;rescale();
		return 0;
	}

        bool Image::check_valid()
	{
		/*
		 * well,why not ?
		 * */
		return ! check_invalid();
	}

        bool Image::check_invalid()
        {
                /*
		 * the image is declared invalid if the image structures are not loaded.
                 */

		//ACHTUNG! 
		if(!img ){img=fimg;}
                if(!img)
                {
                        invalid=1;
                        return true;
                }
		invalid=0;
                return false;
        }

        void Image::free()
        {
		/*
		 * the image descriptors are freed if necessary and pointers blanked
		 * */
                if(fimg!=img && img ) FbiStuff::free_image(img );
                if(fimg     ) FbiStuff::free_image(fimg);
                reset();
        }

// if the image rescaling mechanism is suspected of bugs, this will inhibit its use.
#define FIM_BUGGED_RESCALE 0

	int Image::rescale( fim_scale_t ns )
	{
		/*
		 * effective image rescaling
		 * */
#if FIM_BUGGED_RESCALE
		return 0;
#endif
		if(ns>0.0)newscale=ns;//patch

		if( check_invalid() ) return - 1;
		if(tiny() && newscale<scale){newscale=scale;return 0;}

		int neworientation=getOrientation();
		fim_angle_t	gascale=getGlobalFloatVariable(FIM_VID_ASCALE);
		fim_scale_t	newascale=getFloatVariable(FIM_VID_ASCALE);
		newascale=(newascale>0.0 && newascale!=1.0)?newascale:((gascale>0.0 && gascale!=1.0)?gascale:1.0);
		
		//float newascale=getFloatVariable(FIM_VID_ASCALE); if(newascale<=0.0) newascale=1.0;
		/*
		 * The global angle variable value will override the local if not 0 and the local unset
		 * */
		fim_angle_t	gangle  =getGlobalFloatVariable(FIM_VID_ANGLE),
			newangle=getFloatVariable(FIM_VID_ANGLE);
		newangle=angle?newangle:((gangle!=0.0)?gangle:newangle);

		if(	newscale == scale
			&& newascale == ascale
			&& neworientation == orientation
			//&& newangle == angle
			&& ( !newangle  && !angle )
		)
		{
			return 0;/*no need to rescale*/
		}
		orientation=((neworientation%4)+4)%4; // fix this

		setGlobalVariable(FIM_VID_SCALE,newscale*100);
		if(fimg)
		{
			/*
			 * In case of memory allocation failure, we would
			 * like to recover the current image  :) . 
			 *
			 * Here it would be nice to add some sort of memory manager 
			 * keeping score of copies and ... too complicated ...
			 */
			struct ida_image *backup_img=img;
			if(getGlobalIntVariable(FIM_VID_DISPLAY_STATUS_BAR)||getGlobalIntVariable(FIM_VID_DISPLAY_BUSY))
			{
				if( getGlobalIntVariable(FIM_VID_WANT_PREFETCH) == 1)
					cc.set_status_bar("please wait while prefetching...", "*");
				else
					cc.set_status_bar("please wait while rescaling...", "*");
			}

#define FIM_PROGRESSIVE_RESCALING 0
#if FIM_PROGRESSIVE_RESCALING
			/*
			 * progressive rescaling is computationally convenient in when newscale<scale
			 * at the cost of a progressively worsening image quality (especially when newscale~scale)
			 * and a sequence ----+ will suddenly 'clear' out the image quality, so it is not a desirable
			 * option ...
			 * */
			if( 
				//( newscale>scale && scale > 1.0) ||
				( newscale<scale && scale < 1.0) )
				img = scale_image( img,newscale/scale,newascale);
			else
				img = scale_image(fimg,newscale,newascale);
#else
			img = FbiStuff::scale_image(fimg,newscale,newascale);
#endif
			/* orientation can be 0,1,2,3 */
			if( img && orientation!=0 && orientation != 2)
			{
				// we make a backup.. who knows!
				// FIXME: should use a faster and memory-smarter method : in-place
				struct ida_image *rb=img;
				rb  = FbiStuff::rotate_image90(rb,orientation==1?0:1);
				if(rb)
				{
					FbiStuff::free_image(img);
					img=rb;
				}
			}
			if( img && orientation!=0 && orientation == 2)
			{	
				// we make a backup.. who knows!
				struct ida_image *rbb=NULL,*rb=NULL;
				// FIXME: should use a faster and memory-smarter method : in-place
				rb  = FbiStuff::rotate_image90(img,0);
				if(rb)rbb  = FbiStuff::rotate_image90(rb,0);
				if(rbb)
				{
					FbiStuff::free_image(img);
					FbiStuff::free_image(rb);
					img=rbb;
				}
				else
				{
					if(rbb)FbiStuff::free_image(rbb);
					if(rb )FbiStuff::free_image(rb);
				}
			}

			/* we rotate only in case there is the need to do so */
			if( img && ( angle != newangle || newangle) )
			{	
				// we make a backup.. who knows!
				struct ida_image *rbb=NULL,*rb=NULL;
				rb  = FbiStuff::rotate_image(img,newangle);
				if(rb)rbb  = FbiStuff::rotate_image(rb,0);
				if(rbb)
				{
					FbiStuff::free_image(img);
					FbiStuff::free_image(rb);
					img=rbb;
				}
				else
				{
					if(rbb)FbiStuff::free_image(rbb);
					if(rb )FbiStuff::free_image(rb);
				}
			}

			if(!img)
			{
				img=backup_img;
				if(getGlobalIntVariable(FIM_VID_DISPLAY_BUSY))
					cc.set_status_bar( "rescaling failed (insufficient memory?!)", getInfo().c_str());
				sleep(1);	//just to give a glimpse..
			}
			else 
			{
				/* reallocation succeeded */
				if( backup_img && backup_img!=fimg ) FbiStuff::free_image(backup_img);
				scale=newscale;
				ascale=newascale;
				angle =newangle;
	        		should_redraw();
			}

			/*
			 * it is important to set these values after rotation, too!
			 * */
			setVariable(FIM_VID_HEIGHT ,(int)fimg->i.height);
			setVariable(FIM_VID_WIDTH  ,(int)fimg->i.width );
			setVariable(FIM_VID_SHEIGHT,(int) img->i.height);
			setVariable(FIM_VID_SWIDTH ,(int) img->i.width );
			setVariable(FIM_VID_ASCALE , ascale );
			//setGlobalVariable(FIM_VID_ANGLE  ,  angle );
		}
		else should_redraw(0);
		orientation=neworientation;
		return 0;
	}

	void Image::reduce(float factor)
	{
		/*
		 * scale is adjusted by a dividing factor
		 * */
		newscale = scale / factor;
		rescale();
	}

	void Image::magnify(float factor)
	{
		/*
		 * scale is adjusted by a multiplying factor
		 * */
		newscale = scale * factor;
		rescale();
	}

	/*
	void Image::resize(int nw, int nh)
	{
		//fixme
		if(check_invalid())return;
	}*/

	Image::Image(const Image& image):
		scale(image.scale),
		ascale(image.ascale),
		newscale(image.newscale),
		angle(image.angle),
		newangle(image.newangle),
		page(0),
                img     (NULL),
                fimg    (NULL),
		orientation(image.orientation),
                //invalid(0),
                invalid(image.invalid),
		no_file(true),
		fis(image.fis),
                fname     (image.fname)
	{
		/*
		 * builds a clone of this image.
		 * it should be completely independent from this object.
		 * */
		reset();
		img  = fbi_image_clone(image.img );
		fimg = fbi_image_clone(image.fimg);

		/* an exception is launched immediately */
		if(!img || !fimg)
			///* temporarily, for security reasons :  throw FIM_E_NO_IMAGE*/;
		{
			std::cerr << "fatal error : " << __FILE__ << ":" << __LINE__ << "( are you sure you gave an image file in standard input, uh ?)\n";
			throw FimException();
			std::exit(*(int*)NULL);// FIXME
		}
	}

	Image * Image::getClone()
	{
		/*
		 * returns a clone of this image.
		 * it should be an object completely independent from this.
		 * */
		return new Image(*this);
	}

/*
 *	Creates a little description of some image,
 *	and places it in a NUL terminated static buffer.
 */
fim::string Image::getInfo()
{
	/*
	 * a short information about the current image is returned
	 *
	 * WARNING:
	 * the returned info, if not NULL, belongs to a statical buffer which LIVES with the image!
	 */
	//FIX ME !
	if(!fimg)return "";

	static char linebuffer[128];
	char pagesinfobuffer[128];
	char imagemode[3],*imp;
	int n=getGlobalIntVariable(FIM_VID_FILEINDEX);
	imp=imagemode;

	//if(getGlobalIntVariable(FIM_VID_AUTOFLIP))*(imp++)='F';
	//if(getGlobalIntVariable(FIM_VID_AUTOMIRROR))*(imp++)='M';

	// should flip ? should mirror ?
	int flip   =
	(((getGlobalIntVariable(FIM_VID_AUTOFLIP)== 1)|(getGlobalIntVariable("v:"FIM_VID_FLIPPED)== 1)|(getIntVariable(FIM_VID_FLIPPED)== 1))&&
	!((getGlobalIntVariable(FIM_VID_AUTOFLIP)==-1)|(getGlobalIntVariable("v:"FIM_VID_FLIPPED)==-1)|(getIntVariable(FIM_VID_FLIPPED)==-1)));
	int mirror   =
	(((getGlobalIntVariable(FIM_VID_AUTOMIRROR)== 1)|(getGlobalIntVariable("v:"FIM_VID_MIRRORED)== 1)|(getIntVariable(FIM_VID_MIRRORED)== 1))&&
	!((getGlobalIntVariable(FIM_VID_AUTOMIRROR)==-1)|(getGlobalIntVariable("v:"FIM_VID_MIRRORED)==-1)|(getIntVariable(FIM_VID_MIRRORED)==-1)));

	if(flip  )*(imp++)='F';
	if(mirror)*(imp++)='M';



	*imp='\0';
	if(fimg->i.npages>1)
		snprintf(pagesinfobuffer,sizeof(pagesinfobuffer)," [%d/%d]",page+1,fimg->i.npages);
	else
		*pagesinfobuffer='\0';
		
	snprintf(linebuffer, sizeof(linebuffer),
	     "%s%.0f%% %dx%d%s%s %d/%d",
	     /*fcurrent->tag*/ 0 ? "* " : "",
	     scale*100,
	     this->width(), this->height(),
	     imagemode,
	     pagesinfobuffer,
	     n?n:1, /* ... */
	     (getGlobalIntVariable(FIM_VID_FILELISTLEN))
	     );
	return fim::string(linebuffer);
}

	bool Image::update()
	{
		/*
		 * updates the image according to its variables
		 *
		 * FIXME: a temporary method
		 * */
		setVariable(FIM_VID_FRESH,0);

		/*
		 * rotation dispatch
		 * */
                int neworientation=getOrientation();
		if( neworientation!=orientation)
		{
			rescale();
			orientation=neworientation;
			return true;
		}
		return false;
	}

	int Image::getOrientation()
	{
		/*
		 * warning : this should work more intuitively
		 * */
		return ((
		(  getIntVariable(FIM_VID_ORIENTATION)
		+getGlobalIntVariable("v:"FIM_VID_ORIENTATION)
		+getGlobalIntVariable(FIM_VID_ORIENTATION)
		)
		%4)+4)%4;
	}

	int Image::rotate( float angle )
	{
		/*
		 * rotates the image the specified amount of degrees
		 * */
		float newangle=this->angle+angle;
		if( check_invalid() ) return -1;
		setVariable(FIM_VID_ANGLE,newangle);
		return rescale();	// FIXME : necessary *only* for image update and display
	}

	void Image::should_redraw(int should)const
	{
		/* FIXME : this is BAD style ! */
	        if(cc.displaydevice)
		        cc.displaydevice->redraw=1;
	}

	bool Image::prev_page(int j)
	{
		string s=fname;
		if(have_prevpage(j))
			return load(s.c_str(),NULL,page-j);
		else
			return false;
	} 

	bool Image::goto_page(int j)
	{
		string s=fname;
		if( j>0 )--j;
		if( !fimg )
			return false;
		if( j<0 )j=fimg->i.npages-1;
		if( j>page ? have_nextpage(j-page) : have_prevpage(page-j) )
			return load(s.c_str(),NULL,j);
		else
			return false;
	} 

	bool Image::next_page(int j)
	{
		string s=fname;
		if(have_nextpage(j))
			return load(s.c_str(),NULL,page+j);
		else
			return false;
	} 

	cache_key_t Image::getKey()const
	{
		return cache_key_t(fname.c_str(),fis);
	}

	bool Image::is_multipage()const
	{
		if( fimg && ( fimg->i.npages>1 ) )
			return fimg->i.npages>1 ;
		return 0;
	}

	bool Image::have_nextpage(int j)const
	{
		/* FIXME : missing overflow check */
		return (is_multipage() && page+j < fimg->i.npages);
	} 

	bool Image::have_prevpage(int j)const
	{
		/* FIXME : missing overflow check */
		return (is_multipage() && page-j >= 0);
	}
 
	bool Image::gray_negate()
	{
		/* FIXME : NEW, but unused */
		int n;
		int th=1;/* 0 ... 256 * 3 * 3 */

		if(!img || !img->data)
			return false;

		if(!fimg || !fimg->data)
			return false;
	
		for( n=0; n< 3*fimg->i.width*fimg->i.height ; n+=3 )
		{
			int r,g,b,s,d;
			r=fimg->data[n+0];
			g=fimg->data[n+1];
			b=fimg->data[n+2];
			s=r+g+b;
			d=( s - 3 * r ) * ( s - 3 * g ) * ( s - 3 * b );
			d=d<0?-d:d;
			if( d < th )
			{
				fimg->data[n+0]=~fimg->data[n+0];
				fimg->data[n+1]=~fimg->data[n+1];
				fimg->data[n+2]=~fimg->data[n+2];
			}
		}

		for( n=0; n< 3*img->i.width*img->i.height ; n+=3 )
		{
			int r,g,b,s,d;
			r=img->data[n+0];
			g=img->data[n+1];
			b=img->data[n+2];
			s=r+g+b;
			d=d<0?-d:d;
			if( d < th )
			{
				img->data[n+0]=~img->data[n+0];
				img->data[n+1]=~img->data[n+1];
				img->data[n+2]=~img->data[n+2];
			}
		}

		setGlobalVariable("i:"FIM_VID_NEGATED,1-getGlobalIntVariable("i:"FIM_VID_NEGATED));

       		should_redraw();

		return true;
	} 

	bool Image::negate()
	{
		/* NEW */

		/* FIXME */
		/*return gray_negate();*/

		if(! img || ! img->data)
			return false;
		if(!fimg || !fimg->data)
			return false;

		for( unsigned char * p = fimg->data; p < fimg->data + 3*fimg->i.width*fimg->i.height ;++p)
			*p = ~ *p;

		for( unsigned char * p = img->data; p < img->data + 3*img->i.width*img->i.height ;++p)
			*p = ~ *p;

		setGlobalVariable("i:"FIM_VID_NEGATED,1-getGlobalIntVariable("i:"FIM_VID_NEGATED));

       		should_redraw();

		return true;
	} 
}

