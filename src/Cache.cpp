/* $Id: Cache.cpp 232 2009-03-29 17:32:02Z dezperado $ */
/*
 Cache.cpp : Cache manager source file

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
/*	#include <malloc.h>	*/
	#include <time.h>
namespace fim
{
	Cache::Cache()
	{
		/*	FIXME : potential flaw ?	*/
		lru.erase(lru.begin(),lru.end());
	}

	int Cache::cached_elements()const
	{
		//int count=0;
		//cachels_t::const_iterator ci;

		// FIXME : :)
		//for( ci=imageCache.begin();ci!=imageCache.end();++ci)++count;
		//return count;
		return imageCache.size();
	}

	Image* Cache::get_lru( bool unused )const
	{
		lru_t::const_iterator lrui;

		/* warning : syscall ! */
		time_t m_time;
		m_time = time(NULL);
		Image*  l_img=NULL;

		if ( cached_elements() < 1 ) return NULL;
		cachels_t::const_iterator ci;
		for( ci=imageCache.begin();ci!=imageCache.end();++ci)
		if( ci->second /* <- so we can call this function in some intermediate states .. */
			 && last_used(ci->first) < m_time  &&  (  (! unused) || (used_image(ci->first)<=0)  ) )
		{
			l_img  = ci->second;
			m_time = last_used(ci->first);
		}
		return l_img;
	}

	bool Cache::free_all()
	{
		/*
		 * free all unused elements from the cache
		 */
		
		rcachels_t rcc = reverseCache;
                for(    rcachels_t::const_iterator rcci=rcc.begin(); rcci!=rcc.end();++rcci )
			if(usageCounter[rcci->first->getKey()]==0)erase( rcci->first );
		return true;
	}

	int Cache::free_some_lru()
	{
		/*
		 * this triggering deletion (and memory freeying) of cached elements
		 * (yes, it is a sort of garbage collector, with its pros and cons)
		 */
		if ( cached_elements() < 1 ) return 0;
		return erase( get_lru()  );
	}

	int Cache::erase_clone(fim::Image* oi)
	{
		if(!oi || !is_in_clone_cache(oi))return -1;
#ifdef FIM_CACHE_DEBUG
		cout << "deleting " << oi->getName() << "\n";
#endif
		cloneUsageCounter.erase(oi);
		delete oi;
		clone_pool.erase(oi);
		return 0;
	}

	bool Cache::need_free()const
	{
		/*	temporary : we free elements for more than some cached images	*/

		/*
		struct mallinfo mi = mallinfo();
		cout << "allocated : " <<  mi.uordblks << "\n";
		if( mi.uordblks > getIntGlobalVariable(FIM_VID_MAX_CACHED_MEMORY) )return true;

		these are not the values we want ..
		*/

		int mci = getGlobalIntVariable(FIM_VID_MAX_CACHED_IMAGES);
		if(mci==-1)return false;
		return ( cached_elements() > ( ( mci>0)?mci:-1 ) );
	}

	int Cache::used_image(cache_key_t key)const
	{
		/*	acca' nun stimm'a'ppazzia'	*/
		//return usageCounter[key] ;
		return ( usageCounter.find(key)!=usageCounter.end() ) ?  (*(usageCounter.find(key))).second : 0;
	}

	bool Cache::is_in_clone_cache(fim::Image* oi)const
	{
		/*	acca' nun stimm'a'ppazzia'	*/
		if(!oi)return -1;
		//return *(clone_pool.find(oi))==oi;
		return ( clone_pool.find(oi)!=clone_pool.end() )	
			&&
			((*clone_pool.find(oi)) == oi );
	}

	bool Cache::is_in_cache(cache_key_t key)const
	{
		/*	acca' nun stimm'a'ppazzia'	*/
		//return imageCache[key]!=NULL;
		return ( imageCache.find(key)!=imageCache.end() )
			&&
			((*(imageCache.find(key))).second!=NULL) ;
	}

	bool Cache::is_in_cache(fim::Image* oi)const
	{
		/*	acca' nun stimm'a'ppazzia'	*/
		if(!oi)return -1;
		//return reverseCache[oi]!=cache_key_t("",FIM_E_FILE);// FIXME
		return ( reverseCache.find(oi)!=reverseCache.end() )	
			&&
			( (*(reverseCache.find(oi))).second.first.c_str()== oi->getKey().first );
			
	}

#if 0
	int Cache::free(fim::Image* oi)
	{
		/*	acca' nun stimm'a'ppazzia'	*/
		if(!oi)return -1;

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
//		if(need_free())
//			free_some_lru();
		if(key.first == FIM_STDIN_IMAGE_NAME)
			return 0;// just a fix in the case the browser is still lame
		if(is_in_cache(key))
			return 0;
		if(!loadNewImage(key))
			return -1;
		setGlobalVariable(FIM_VID_CACHED_IMAGES,cached_elements());
		setGlobalVariable(FIM_VID_CACHE_STATUS,getReport().c_str());
		return 0;
//		return getCachedImage(key)?0:-1;
	}

	Image * Cache::loadNewImage(cache_key_t key)
	{
		Image *ni = NULL;
		/*	load attempt as alternative approach	*/
		try
		{
		if( ( ni = new Image(key.first.c_str()) ) )// FIXME
		{	
#ifdef FIM_CACHE_DEBUG
			std::cout << "loadNewImage("<<key.first.c_str()<<")\n";
#endif
			if( cacheNewImage( ni ) ) return ni;
		}
		}
		catch(FimException e)
		{
			ni = NULL; /* not a big problem */
//			if( e != FIM_E_NO_IMAGE )throw FIM_E_TRAGIC;  /* hope this never occurs :P */
		}
		return NULL;
	}
	
	Image * Cache::getCachedImage(cache_key_t key)
	{
		/*
		 * returns an image if already in cache ..
		 * */
		Image *ni = NULL;
	
		/*	acca' nun stimm'a'ppazzia'	*/
		//if(!key.first)return ni;

		/*	cache lookup */
		//this->cached_elements();
		if( ( ni = this->imageCache[key]) )
		{
			this->lru_touch(key);
			return ni;
		}
		return ni;//could be NULL
	}

	bool Cache::cacheNewImage( fim::Image* ni )
	{

#ifdef FIM_CACHE_DEBUG
					std::cout << "going to cache: "<< ni << "\n";
#endif

		/*	acca' nun stimm'a'ppazzia'	*/
		if(!ni)return false;

		this->imageCache[ni->getKey()]=ni;
		this->reverseCache[ni]= ni->getKey();
		lru_touch( ni->getKey() );
		usageCounter[ ni->getKey()]=0; // we yet don't assume any usage
		setGlobalVariable(FIM_VID_CACHED_IMAGES,cached_elements());
		return true;
	}
	
	int Cache::erase(fim::Image* oi)
	{
		/*
		 * erases the image from the image cache
		 * */
		/*	acca' nun stimm'a'ppazzia'	*/
		if(!oi)
		{
			return -1;
		}

		if(is_in_cache(oi) )
		{
			usageCounter[oi->getKey()]=0;
			/* NOTE : the user should call usageCounter.erase(key) after this ! */
			lru.erase(oi);
			imageCache.erase(reverseCache[oi]);
			reverseCache.erase(oi);
//			delete imageCache[reverseCache[oi]];
#ifdef FIM_CACHE_DEBUG
			std::cout << "will erase  "<< oi << "\n";
			cout << "deleting " << oi->getName() << "\n";
#endif
			delete oi; // NEW !!
			setGlobalVariable(FIM_VID_CACHED_IMAGES,cached_elements());
			return 0;
		}
		return -1;
	}

	time_t Cache::last_used(cache_key_t key)const
	{
		if(imageCache.find(key)==imageCache.end())return 0;
		if(lru.find(imageCache.find(key)->second )==lru.end())return 0;
		return lru.find(imageCache.find(key)->second )->second;
		//return lru[imageCache[key]]=time(NULL);
	}

	int Cache::lru_touch(cache_key_t key)
	{
		/*
		 * if the specified file is cached, in this way it is marked as used, too
		 *
		 * NOTE : the usage count is not affected, 
		 * */
		//if(!fname) return -1;
		//if(!imageCache[key])return -1;
		//if(fim::string(fname)=="")return -1;
		lru[imageCache[key]]=time(NULL);
		return 0;
	}

	bool Cache::freeCachedImage(Image *image)
	{
		/*
		 * if the supplied image is cached as a master image of a clone, it is freed and deregistered.
		 * if not, no action is performed.
		 * */
		// WARNING : FIXME : DANGER !!
		if( !image )return false;
//		if( is_in_cache(image) && usageCounter[image->getKey()]==1 )
		if( is_in_clone_cache(image) )
		{
			usageCounter[image->getKey()]--;
			erase_clone(image);	// we _always_ immediately delete clones
			setGlobalVariable(FIM_VID_CACHE_STATUS,getReport().c_str());
			return true;
		}
		else
		if( is_in_cache(image) )
		{
			usageCounter[image->getKey()]--;
			if(
				(usageCounter[image->getKey()])==0 && 
				image->getKey().second!=FIM_E_STDIN 
				)
			{
#if 0
				if( need_free() && image->getKey().second!=FIM_E_STDIN )
				{
					cache_key_t key = image->getKey();
					this->erase( image );
					usageCounter.erase(key);
				}
#else
				/* doing it here is dangerous : */
				if( need_free() )
				{
					Image * lrui = get_lru(true);
					if(lrui && ( lrui->getKey().second!=FIM_E_STDIN ))
					{	
						cache_key_t key = lrui->getKey();
						this->erase( lrui );
						usageCounter.erase(key);
					}
						// missing usageCounter.erase()..
				}
#endif
			}
			setGlobalVariable(FIM_VID_CACHE_STATUS,getReport().c_str());
			return true;
		}
		return false;
	}

	Image * Cache::useCachedImage(cache_key_t key)
	{
		/*
		 * the calling function needs an image, so calls this method.
		 * if we already have the desired image and it is already used,
		 * a clone is built and returned.
		 *
		 * if we have an unused master, we return it.
		 *
		 * then declare this image as used and increase a relative counter.
		 *
		 * a freeImage action will do the converse operation (and delete).
		 * if the image is not already cached, it is loaded, if possible.
		 *
		 * so, if there is no such image, NULL is returned
		 * */
#ifdef FIM_CACHE_DEBUG
		std::cout << "  useCachedImage(\""<<key.first<<","<<key.second<<"\")\n";
#endif
		Image * image=NULL;
		if(!is_in_cache(key)) 
		{
			/*
			 * no Image cached at all for this filename
			 * */
			image = loadNewImage(key);
			if(!image)return NULL; // bad luck!
			usageCounter[key]=1;
			setGlobalVariable(FIM_VID_CACHE_STATUS,getReport().c_str());
			return image;
//			usageCounter[key]=0;
		}
		else
		{
			/*
			 * at least one copy of this filename image is in cache
			 * */
			image=getCachedImage(key);// in this way we update the LRU cache :)
			if(!image)
			{
				// critical error
#ifdef FIM_CACHE_DEBUG
				cout << "critical internal cache error!\n";
#endif
				setGlobalVariable(FIM_VID_CACHE_STATUS,getReport().c_str());
				return NULL;
			}
			if( used_image( key ) )
			{
				// if the image was already used, cloning occurs
//				image = image->getClone(); // EVIL !!
				try
				{
#ifdef FIM_CACHE_DEBUG
					Image * oi=image;
#endif
					image = new Image(*image); // cloning
#ifdef FIM_CACHE_DEBUG
					std::cout << "  cloned image: \"" <<image->getName()<< "\" "<< image << " from \""<<oi->getName() <<"\" " << oi << "\n";
#endif

				}
				catch(FimException e)
				{
					/* we will survive :P */
					image = NULL; /* we make sure no taint remains */
//					if( e != FIM_E_NO_IMAGE )throw FIM_E_TRAGIC;  /* hope this never occurs :P */
				}
				if(!image)return NULL; //means that cloning failed.

				clone_pool.insert(image); // we have a clone
				cloneUsageCounter[image]=1;
			}
			lru_touch( key );
			// if loading and eventual cloning succeeded, we count the image as used of course
			usageCounter[key]++;
			setGlobalVariable(FIM_VID_CACHE_STATUS,getReport().c_str());
			return image;	//so, it could be a clone..
		}
	}

	Image * Cache::setAndCacheStdinCachedImage(Image * image)
	{
		/* FIXME : document me
		 * */
		if(!image) return NULL;
		cache_key_t key(FIM_STDIN_IMAGE_NAME,FIM_E_STDIN);
		
		try
		{
#ifdef FIM_CACHE_DEBUG
			Image * oi=image;
#endif
			image = new Image(*image); // cloning
			if(image)
			{
				cacheNewImage( image );
			}
		}
		catch(FimException e)
		{
			/* we will survive :P */
			image = NULL; /* we make sure no taint remains */
//			if( e != FIM_E_NO_IMAGE )throw FIM_E_TRAGIC;  /* hope this never occurs :P */
		}
		if(!image)return NULL; //means that cloning failed.
		setGlobalVariable(FIM_VID_CACHE_STATUS,getReport().c_str());
		return image;	//so, it could be a clone..
	}

	fim::string Cache::getReport()
	{
		fim::string cache_report = "cache contents : \n";
#if 0
		cachels_t::const_iterator ci;
		for( ci=imageCache.begin();ci!=imageCache.end();++ci)
		{	
			cache_report+=((*ci).first);
			cache_report+=" ";
			cache_report+=fim::string(usageCounter[((*ci).first)]);
			cache_report+="\n";
		}
#else
		ccachels_t::const_iterator ci;
		for( ci=usageCounter.begin();ci!=usageCounter.end();++ci)
		{	
			cache_report+=((*ci).first.first);
			cache_report+=":";
			cache_report+=fim::string((*ci).first.second);
			cache_report+=" ";
			cache_report+=fim::string((*ci).second);
			cache_report+="\n";
		}
		std::set< fim::Image* >::const_iterator cpi;
		cache_report += "clone pool contents : \n";
		for( cpi=clone_pool.begin();cpi!=clone_pool.end();++cpi)
		{	
			cache_report+=(*cpi)->getName();
			cache_report+=" " ; 
			cache_report+= string((int*)(*cpi)) ; 
			cache_report+=",";
		}
		cache_report+="\n";
#endif
		return cache_report;
	}

	Cache::~Cache()
	{
		cachels_t::const_iterator ci;
		for( ci=imageCache.begin();ci!=imageCache.end();++ci)
			if(ci->second)delete ci->second;
	}
}

