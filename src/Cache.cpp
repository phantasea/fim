/* $LastChangedDate: 2015-04-18 21:28:34 +0200 (Sat, 18 Apr 2015) $ */
/*
 Cache.cpp : Cache manager source file

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
/*	#include <malloc.h>	*/
#ifdef HAVE_SYS_TIME_H
	#include <sys/time.h>
#else /* HAVE_SYS_TIME_H */
	#include <time.h>
#endif /* HAVE_SYS_TIME_H */

#define FIM_CACHE_INSPECT 0
#if FIM_CACHE_INSPECT
#define FIM_PR(X) printf("CACHE:%c:%20s:%s",X,__func__,getReport(FIM_CR_CD).c_str());
#else /* FIM_CACHE_INSPECT */
#define FIM_PR(X) 
#endif /* FIM_CACHE_INSPECT */

//#define FIM_CACHE_DEBUG 0

#if 0
#define FIM_LOUD_CACHE_STUFF FIM_PR(-10); FIM_LINE_COUT
#else
#define FIM_LOUD_CACHE_STUFF 
#endif
#define FIM_VCBS(VI) ( sizeof(VI) + VI.size() * ( sizeof(vcachels_t::mapped_type) + sizeof(vcachels_t::key_type) ) )
/* TODO: maybe fim_basename_of is excessive ?  */
	extern CommandConsole cc;
namespace fim
{
	static fim_time_t fim_time(void) /* stand-alone function */
	{
#ifdef HAVE_SYS_TIME_H
		struct timeval tv;
		const fim_time_t prec = 1000; /* fraction of second precision */
		gettimeofday(&tv, NULL);
		return tv.tv_sec * prec + tv.tv_usec / ( 1000000 / prec );
#else /* HAVE_SYS_TIME_H */
		return time(NULL);
#endif /* HAVE_SYS_TIME_H */
	}

	fim_time_t Cache::reltime(void)const
	{
		return fim_time()-time0_;
	}

	Cache::Cache(void)
		:Namespace(&cc)
#if FIM_WANT_BDI
		,dummy_img_()
#endif	/* FIM_WANT_BDI */
	{
		/*	FIXME : potential flaw ?	*/
		FIM_LOUD_CACHE_STUFF;
		lru_.erase(lru_.begin(),lru_.end());
		time0_ = fim_time();
	}

	int Cache::cached_elements(void)const
	{
		FIM_LOUD_CACHE_STUFF;
		return imageCache_.size();
	}

	Image* Cache::get_lru( bool unused )const
	{
		lru_t::const_iterator lrui;

		/* warning : syscall ! */
		fim_time_t m_time;
		m_time = reltime();
		Image*  l_img=NULL;
		cachels_t::const_iterator ci;
		FIM_LOUD_CACHE_STUFF;

		if ( cached_elements() < 1 )
			goto ret;

		for( ci=imageCache_.begin();ci!=imageCache_.end();++ci)
		if( ci->second /* <- so we can call this function in some intermediate states .. */
			 && last_used(ci->first) < m_time  &&  (  (! unused) || (used_image(ci->first)<=0)  ) )
		{
			l_img  = ci->second;
			m_time = last_used(ci->first);
		}
ret:
		return l_img;
	}

	bool Cache::free_all(void)
	{
		/*
		 * free all unused elements from the cache
		 */
		rcachels_t rcc = reverseCache_;

		FIM_LOUD_CACHE_STUFF;
		FIM_PR(' ');
                for(    rcachels_t::const_iterator rcci=rcc.begin(); rcci!=rcc.end();++rcci )
			if(usageCounter_[rcci->first->getKey()]==0)
				erase( rcci->first );
		return true;
	}

	int Cache::free_some_lru(void)
	{
		/*
		 * this triggering deletion (and memory freeing) of cached elements
		 * (yes, it is a sort of garbage collector, with its pros and cons)
		 */
		FIM_LOUD_CACHE_STUFF;
		FIM_PR(' ');
		if ( cached_elements() < 1 )
			return 0;
		return erase( get_lru(true)  );
	}

	int Cache::erase_clone(fim::Image* oi)
	{
		FIM_LOUD_CACHE_STUFF;
		FIM_PR(' ');
		if(!oi || !is_in_clone_cache(oi))
			return -1;
#ifdef FIM_CACHE_DEBUG
		cout << "deleting " << fim_basename_of(oi->getName()) << "\n";
#endif /* FIM_CACHE_DEBUG */
		cloneUsageCounter_.erase(oi);
		delete oi;
		clone_pool_.erase(oi);
		return 0;
	}

	bool Cache::need_free(void)const
	{
		/*	temporary : we free elements for more than some cached images	*/

		/*
		struct mallinfo mi = mallinfo();
		cout << "allocated : " <<  mi.uordblks << "\n";
		if( mi.uordblks > getIntGlobalVariable(FIM_VID_MAX_CACHED_MEMORY) )
			return true;

		these are not the values we want ..
		*/
		int mci = getGlobalIntVariable(FIM_VID_MAX_CACHED_IMAGES);
		int mcm = getGlobalIntVariable(FIM_VID_MAX_CACHED_MEMORY); /* getIntGlobalVariable */
		size_t smcm = mcm > 0 ? mcm : 0;

	       	if( smcm > 0 && img_byte_size()/FIM_CNS_CSU > smcm )
			goto rt;

		if(mci==-1)
			goto rf;

		/* return ( cached_elements() > ( ( mci>0)?mci:-1 ) ); */
		if(mci > 0 && cached_elements() > mci)
			goto rt;
rf:
		return false;
rt:
		return true;
	}

	int Cache::used_image(cache_key_t key)const
	{
		/*	acca' nun stimm'a'ppazzia'	*/
		//return usageCounter_[key] ;
		FIM_LOUD_CACHE_STUFF;
		return ( usageCounter_.find(key)!=usageCounter_.end() ) ?  (*(usageCounter_.find(key))).second : 0;
	}

	bool Cache::is_in_clone_cache(fim::Image* oi)const
	{
		/*	acca' nun stimm'a'ppazzia'	*/
		FIM_LOUD_CACHE_STUFF;
		if(!oi)
			return -1;
		//return *(clone_pool_.find(oi))==oi;
		return ( clone_pool_.find(oi)!=clone_pool_.end() )	
			&&
			((*clone_pool_.find(oi)) == oi );
	}

	bool Cache::is_in_cache(cache_key_t key)const
	{
		/*	acca' nun stimm'a'ppazzia'	*/
		//return imageCache_[key]!=NULL;
		FIM_LOUD_CACHE_STUFF;
		return ( imageCache_.find(key)!=imageCache_.end() )
			&&
			((*(imageCache_.find(key))).second!=NULL) ;
	}

	bool Cache::is_in_cache(fim::Image* oi)const
	{
		/*	acca' nun stimm'a'ppazzia'	*/
		FIM_LOUD_CACHE_STUFF;
		if(!oi)
			return -1;
		return ( reverseCache_.find(oi)!=reverseCache_.end() )	
			&&
			( (*(reverseCache_.find(oi))).second.first.c_str()== oi->getKey().first );
	}

#if 0
	int Cache::free(fim::Image* oi)
	{
		/*	acca' nun stimm'a'ppazzia'	*/
		if(!oi)
			return -1;

		if(!is_in_cache(oi))
		{
#if 0
			/* if the image is not already one of ours, it 
			 * is probably a cloned one, and the caller 
			 * didn't know this.
			 *
			 * in this case we keep it in the cache, 
			 * so it could be useful in the future.
			 * */
			if( oi->revertToLoaded() )//removes internal scale caches
				cacheImage( oi ); //FIXME : validity should be checked ..
#else
			delete oi;
#endif
			return 0;
		}

		/*
		 * fixme : we should explicitly mark for deletion someday.. 
		 * */

		//if(need_free())return erase(oi);
		/*	careful here !!	*/
		//if(need_free())free_some_lru();
		else return 0;	/* no free needed */
	}
#endif

	int Cache::prefetch(cache_key_t key)
	{
		int retval = 0;
		FIM_PR('*');

		FIM_LOUD_CACHE_STUFF;
		if(is_in_cache(key))
		{
			goto ret;
			FIM_PR('c');
		}
	  	if(need_free())
			free_some_lru();
		if(need_free())
		{
			FIM_PR('f');
			goto ret; /* skip prefetch if cache is full */
		}
		if(key.first == FIM_STDIN_IMAGE_NAME)
		{
			FIM_PR('s');
			goto ret;// just a fix in the case the browser is still lame
		}
#ifdef FIM_CACHE_DEBUG
		std::cout << "prefetch request for "<< key.first << " \n";
#endif /* FIM_CACHE_DEBUG */

    		if( regexp_match(key.first.c_str(),FIM_CNS_ARCHIVE_RE,1) )
		{
			/* FIXME: This is a hack. One shall determine unprefetchability othwerwise. */
			FIM_PR('j');
			goto ret;
		}

		if(!loadNewImage(key))
		{
			retval = -1;
			goto ret;
		}
		setGlobalVariable(FIM_VID_CACHED_IMAGES,(fim_int)cached_elements());
		setGlobalVariable(FIM_VID_CACHE_STATUS,getReport().c_str());
ret:
		FIM_PR('.');
		return retval;
	}

	Image * Cache::loadNewImage(cache_key_t key, fim_page_t page)
	{
		Image *ni = NULL;
		FIM_PR('*');

		FIM_LOUD_CACHE_STUFF;
		/*	load attempt as alternative approach	*/
		try
		{
		if( ( ni = new Image(key.first.c_str(), NULL, page) ) )
		{
#ifdef FIM_CACHE_DEBUG
			std::cout << "loadNewImage("<<key.first.c_str()<<")\n";
#endif /* FIM_CACHE_DEBUG */
			if( (!ni->cacheable()) || cacheNewImage( ni ) )
				goto ret;
		}
		}
		catch(FimException e)
		{
			FIM_PR('E');
			ni = NULL; /* not a big problem */
//			if( e != FIM_E_NO_IMAGE )throw FIM_E_TRAGIC;  /* hope this never occurs :P */
		}
ret:
		FIM_PR(' ');
		return ni;
	}
	
	Image * Cache::getCachedImage(cache_key_t key)
	{
		/*
		 * returns an image if already in cache ..
		 * */
		Image *ni = NULL;
		FIM_LOUD_CACHE_STUFF;
		FIM_PR(' ');
	
		/*	acca' nun stimm'a'ppazzia'	*/
		//if(!key.first)return ni;

		/*	cache lookup */
		//this->cached_elements();
		if( ( ni = this->imageCache_[key]) )
		{
			this->lru_touch(key);
		}
		return ni;
	}

	bool Cache::cacheNewImage( fim::Image* ni )
	{
		FIM_LOUD_CACHE_STUFF;
		FIM_PR(' ');
#ifdef FIM_CACHE_DEBUG
					std::cout << "going to cache: "<< ni << "\n";
#endif /* FIM_CACHE_DEBUG */

		/*	acca' nun stimm'a'ppazzia'	*/
		if(!ni)
			return false;

		this->imageCache_[ni->getKey()]=ni;
		this->reverseCache_[ni]= ni->getKey();
		lru_touch( ni->getKey() );
		usageCounter_[ ni->getKey()]=0; // we yet don't assume any usage
		setGlobalVariable(FIM_VID_CACHED_IMAGES,(fim_int)cached_elements());
		return true;
	}
	
	int Cache::erase(fim::Image* oi)
	{
		/*
		 * erases the image from the image cache
		 * */
		/*	acca' nun stimm'a'ppazzia'	*/
		int retval=-1;
		FIM_PR(' ');

		FIM_LOUD_CACHE_STUFF;
		if(!oi)
		{
			goto ret;
		}

		if(is_in_cache(oi) )
		{
			usageCounter_[oi->getKey()]=0;
			/* NOTE : the user should call usageCounter_.erase(key) after this ! */
#ifdef FIM_CACHE_DEBUG
			std::cout << "will erase  "<< oi << " " <<  fim_basename_of(oi->getName()) << " time:"<< lru_[oi] << "\n";
			cout << "deleting " << fim_basename_of(oi->getName()) << "\n";
#endif /* FIM_CACHE_DEBUG */
			lru_.erase(oi);
			imageCache_.erase(reverseCache_[oi]);
			reverseCache_.erase(oi);
//			delete imageCache_[reverseCache_[oi]];
			delete oi;
			setGlobalVariable(FIM_VID_CACHED_IMAGES,(fim_int)cached_elements());
			retval = 0;
		}
ret:
		return retval;
	}

	fim_time_t Cache::last_used(cache_key_t key)const
	{
		fim_time_t retval=0;

		FIM_LOUD_CACHE_STUFF;
		if(imageCache_.find(key)==imageCache_.end())
			goto ret;
		if(lru_.find(imageCache_.find(key)->second )==lru_.end())
			goto ret;
		retval = lru_.find(imageCache_.find(key)->second )->second;
ret:
		return retval;
	}

	int Cache::lru_touch(cache_key_t key)
	{
		/*
		 * if the specified file is cached, in this way it is marked as used, too
		 *
		 * NOTE : the usage count is not affected, 
		 * */
		FIM_LOUD_CACHE_STUFF;
		FIM_PR(' ');
//		std::cout << lru_[imageCache_[key]] << " -> ";
		lru_[imageCache_[key]]= reltime();
//		std::cout << lru_[imageCache_[key]] << "\n";
		return 0;
	}

	bool Cache::freeCachedImage(Image *image, const ViewportState *vsp)
	{
		/*
		 * TODO: rename to free().
		 *
		 * If the supplied image is cached as a master image of a clone, it is freed and deregistered.
		 * If not, no action is performed.
		 * */
		// WARNING : FIXME : DANGER !!
		FIM_LOUD_CACHE_STUFF;
		FIM_PR('*');

		if( !image )
			goto err;

//		if( is_in_cache(image) && usageCounter_[image->getKey()]==1 )
		if(vsp)
		{
			viewportInfo_[image->getKey()] = *vsp;
		}
		if( is_in_clone_cache(image) )
		{
			usageCounter_[image->getKey()]--;
			erase_clone(image);	// we _always_ immediately delete clones
			setGlobalVariable(FIM_VID_CACHE_STATUS,getReport().c_str());
			goto ret;
		}
		else
		if( is_in_cache(image) )
		{
			usageCounter_[image->getKey()]--;
#if FIM_WANT_EXPERIMENTAL_MIPMAPS
			image->mm_free();
#endif /* FIM_WANT_EXPERIMENTAL_MIPMAPS */
			if(
				(usageCounter_[image->getKey()])==0 && 
				image->getKey().second!=FIM_E_STDIN 
				)
			{
#if 0
				if( need_free() && image->getKey().second!=FIM_E_STDIN )
				{
					cache_key_t key = image->getKey();
					this->erase( image );
					usageCounter_.erase(key);
				}
#else
				/* doing it here is dangerous : */
				if( need_free() )
				{
					Image * lrui = get_lru(true);

					if( lrui ) 
					{
						cache_key_t key = lrui->getKey();
						
						if( FIM_VCBS(viewportInfo_) > FIM_CNS_VICSZ )
							viewportInfo_.erase(key);

						if(( key.second != FIM_E_STDIN ))
						{	
							this->erase( lrui );
							usageCounter_.erase(key);
						}
					}
					// missing usageCounter_.erase()..
				}
#endif
			}
			setGlobalVariable(FIM_VID_CACHE_STATUS,getReport().c_str());
			goto ret;
		}
err:
		FIM_PR('.');
		return false;
ret:
		FIM_PR('.');
		return true;
	}

	Image * Cache::useCachedImage(cache_key_t key, ViewportState *vsp, fim_page_t page)
	{
		/*
		 * TODO: rename to get().
		 *
		 * The caller invokes this method to obtain an Image object pointer.
		 * If the object is cached and it already used, a clone is built and returned.
		 *
		 * If we have an unused master copy, we return that.
		 *
		 * Then declare this image as used and increase a relative counter.
		 *
		 * A freeImage action will do the converse operation (and delete).
		 * If the image is not already cached, it is loaded, if possible.
		 *
		 * So, if there is no such image, NULL is returned
		 * */
		Image * image = NULL;

		FIM_LOUD_CACHE_STUFF;
		FIM_PR('*');
#ifdef FIM_CACHE_DEBUG
		std::cout << "  useCachedImage(\""<<fim_basename_of(key.first.c_str())<<" of type "<< ( key.second == FIM_E_FILE ? " file ": " stdin ")<<"\")\n";
#endif /* FIM_CACHE_DEBUG */
		if(!is_in_cache(key)) 
		{
			/*
			 * no Image cached at all for this filename
			 * */
			image = loadNewImage(key,page);
			if(!image)
				goto ret; // bad luck!
			if(!image->cacheable())
				goto ret;
			usageCounter_[key]=1;
			setGlobalVariable(FIM_VID_CACHE_STATUS,getReport().c_str());
			goto ret;
//			usageCounter_[key]=0;
		}
		else
		{
			/*
			 * at least one copy of this filename image is in cache
			 * */
			image = getCachedImage(key);// in this way we update the LRU cache :)
			if(!image)
			{
				// critical error
#ifdef FIM_CACHE_DEBUG
				cout << "critical internal cache error!\n";
#endif /* FIM_CACHE_DEBUG */
				setGlobalVariable(FIM_VID_CACHE_STATUS,getReport().c_str());
				goto ret;
			}
			if( used_image( key ) )
			{
				// if the image was already used, cloning occurs
//				image = image->getClone(); // EVIL !!
				try
				{
#ifdef FIM_CACHE_DEBUG
					Image * oi=image;
#endif /* FIM_CACHE_DEBUG */
					image = new Image(*image); // cloning
#ifdef FIM_CACHE_DEBUG
					std::cout << "  cloned image: \"" <<fim_basename_of(image->getName())<< "\" "<< image << " from \""<<fim_basename_of(oi->getName()) <<"\" " << oi << "\n";
#endif /* FIM_CACHE_DEBUG */
				}
				catch(FimException e)
				{
					FIM_PR('E');
					/* we will survive :P */
					image = NULL; /* we make sure no taint remains */
//					if( e != FIM_E_NO_IMAGE )throw FIM_E_TRAGIC;  /* hope this never occurs :P */
				}
				if(!image)
					goto ret; //means that cloning failed.

				clone_pool_.insert(image); // we have a clone

				cloneUsageCounter_[image]=1;
			}
#if FIM_WANT_EXPERIMENTAL_MIPMAPS
			if(getGlobalIntVariable(FIM_VID_WANT_MIPMAPS)>0)
				if(!image->has_mm())
					image->mm_make();
#endif /* FIM_WANT_EXPERIMENTAL_MIPMAPS */
			lru_touch( key );
			// if loading and eventual cloning succeeded, we count the image as used of course
			usageCounter_[key]++;
			setGlobalVariable(FIM_VID_CACHE_STATUS,getReport().c_str());
			goto ret;	//so, it could be a clone..
		}
ret:
		if(vsp && image)
		{
			*vsp = viewportInfo_[image->getKey()];
			/* *vsp = viewportInfo_[key]; */
		}
		FIM_PR('.');
		return image;
	}

	Image * Cache::setAndCacheStdinCachedImage(Image * image)
	{
		/* FIXME : document me
		 * */
		cache_key_t key(FIM_STDIN_IMAGE_NAME,FIM_E_STDIN);
		FIM_LOUD_CACHE_STUFF;
		FIM_PR('*');

		if(!image)
			goto ret;
		
		try
		{
#ifdef FIM_CACHE_DEBUG
			Image * oi=image;
#endif /* FIM_CACHE_DEBUG */
			image = new Image(*image); // cloning
			if(image)
			{
				cacheNewImage( image );
			}
		}
		catch(FimException e)
		{
			FIM_PR('E');
			/* we will survive :P */
			image = NULL; /* we make sure no taint remains */
//			if( e != FIM_E_NO_IMAGE )throw FIM_E_TRAGIC;  /* hope this never occurs :P */
		}
		if(!image)
			goto ret; //means that cloning failed.
		setGlobalVariable(FIM_VID_CACHE_STATUS,getReport().c_str());
ret:
		return image;	//so, it could be a clone..
	}

	fim::string Cache::getReport(int type)const
	{
		/* TODO: rename to info(). */
		fim::string cache_report;

		if(type == FIM_CR_CN || type == FIM_CR_CD)
		{
			fim_char_t buf[FIM_PRINTFNUM_BUFSIZE];
			int mci = getGlobalIntVariable(FIM_VID_MAX_CACHED_IMAGES);
			int mcm = getGlobalIntVariable(FIM_VID_MAX_CACHED_MEMORY);
			mcm = mcm >= 0 ? mcm*FIM_CNS_CSU:0;
			cache_report  = " ";
			cache_report += "count:";
			cache_report += fim::string(cached_elements());
			cache_report += "/";
			cache_report += fim::string(mci);
			cache_report += " ";
			cache_report += "occupation:";
			fim_snprintf_XB(buf, sizeof(buf), img_byte_size());cache_report += buf;
			cache_report += "/";
			fim_snprintf_XB(buf, sizeof(buf), mcm);cache_report += buf;
			cache_report += " ";
			for(ccachels_t::const_iterator ci=usageCounter_.begin();ci!=usageCounter_.end();++ci)
			if(
			  ( type == FIM_CR_CD && ( imageCache_.find(ci->first) != imageCache_.end() ) ) ||
			  ( type == FIM_CR_CN && ( imageCache_.find(ci->first) != imageCache_.end()  && ci->second) )
			  )
			{
				cache_report+= fim_basename_of((*ci).first.first.c_str());
				cache_report+=":";
				cache_report+=fim::string((*ci).second);
				cache_report+=":";
				fim_snprintf_XB(buf, sizeof(buf), imageCache_.find(ci->first)->second->byte_size());
				cache_report += buf;
				cache_report+="@";
				cache_report+=fim::string((size_t)last_used(ci->first));
				cache_report+=" ";
			}
			cache_report += "\n";
			goto ret;
		}
		cache_report = "cache contents : \n";
		FIM_LOUD_CACHE_STUFF;
#if 0
		cachels_t::const_iterator ci;
		for( ci=imageCache_.begin();ci!=imageCache_.end();++ci)
		{	
			cache_report+=((*ci).first);
			cache_report+=" ";
			cache_report+=fim::string(usageCounter_[((*ci).first)]);
			cache_report+="\n";
		}
#else
		for(ccachels_t::const_iterator ci=usageCounter_.begin();ci!=usageCounter_.end();++ci)
		{	
			cache_report+=((*ci).first.first);
			cache_report+=":";
			cache_report+=fim::string((*ci).first.second);
			cache_report+=" ,usage:";
			cache_report+=fim::string((*ci).second);
			cache_report+="\n";
		}
		cache_report += "clone pool contents : \n";
		for(std::set< fim::Image* >::const_iterator  cpi=clone_pool_.begin();cpi!=clone_pool_.end();++cpi)
		{	
			cache_report+=fim_basename_of((*cpi)->getName());
			cache_report+=" " ; 
			cache_report+= string((int*)(*cpi)) ; 
			cache_report+=",";
		}
		cache_report+="\n";
#endif
ret:
		return cache_report;
	}

	Cache::~Cache(void)
	{
		cachels_t::const_iterator ci;

		FIM_LOUD_CACHE_STUFF;
		for( ci=imageCache_.begin();ci!=imageCache_.end();++ci)
			if(ci->second)
				delete ci->second;
	}

	size_t Cache::img_byte_size(void)const
	{
		size_t bs = 0;
		cachels_t::const_iterator ci;

		FIM_LOUD_CACHE_STUFF;
		for( ci=imageCache_.begin();ci!=imageCache_.end();++ci)
			if(ci->second)
				bs += ci->second->byte_size();
		return bs;
	}

	size_t Cache::byte_size(void)const
	{
		size_t bs = 0;
		cachels_t::const_iterator ci;
		FIM_LOUD_CACHE_STUFF;
		bs += img_byte_size();
		bs += sizeof(*this);
		bs += FIM_VCBS(viewportInfo_);
		/* 
		bs += sizeof(usageCounter_);
		bs += sizeof(reverseCache_); */
		/* TODO: incomplete ... */
		return bs;
	}

	void Cache::touch(cache_key_t key)
	{
		FIM_PR('*');
		getCachedImage(key);
		FIM_PR('.');
       	}

	void Cache::desc_update(void)
	{
		/* TODO: report error code */
#if FIM_WANT_PIC_CMTS
		cachels_t::const_iterator ci;

		FIM_LOUD_CACHE_STUFF;
		for( ci=imageCache_.begin();ci!=imageCache_.end();++ci)
			if(ci->second)
				ci->second->desc_update();
#endif /* FIM_WANT_PIC_CMTS */
	}
}

