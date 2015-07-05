/* $Id: Cache.h 232 2009-03-29 17:32:02Z dezperado $ */
/*
 Cache.h : Cache manager header file

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
#ifndef FIM_CACHE_H
#define FIM_CACHE_H
#include "fim.h"

namespace fim
{
#ifdef FIM_NAMESPACES
class Cache:public Namespace
#else
class Cache
#endif
{	
	typedef std::map<fim::Image*,time_t > 	   lru_t;	//filename - last usage time
	typedef std::map<cache_key_t,fim::Image* >  cachels_t;	//filename - image
	typedef std::map<fim::Image*,cache_key_t >  rcachels_t;	//image - filename
	typedef std::map<cache_key_t,int >        ccachels_t;	//filename - counter
	typedef std::map<cache_key_t,std::vector<fim::Image*> > cloned_cachels_t;	//filename - cloned images??
	typedef std::map<fim::Image*,int >  	   cuc_t;	//image - filename

	cachels_t 	imageCache;
	rcachels_t	reverseCache;
	lru_t		lru;
	ccachels_t	usageCounter;
	cloned_cachels_t cloneCache;
	cuc_t		cloneUsageCounter;
	std::set< fim::Image* > clone_pool;
//	clone_counter_t cloneCounter;

	/*	the count of cached images	*/
	int cached_elements()const;

	/*	whether we should free some cache ..	*/
	bool need_free()const;

	/**/
	int lru_touch(cache_key_t key);

	bool is_in_cache(fim::Image* oi)const;
	bool is_in_cache(cache_key_t key)const;
	bool is_in_clone_cache(fim::Image* oi)const;
	time_t last_used(cache_key_t key)const;

	bool cacheNewImage( fim::Image* ni );
	Image * loadNewImage(cache_key_t key);

	/*	returns an image from the cache or loads it from disk marking it as used in the LRU (internal) */
	Image * getCachedImage(cache_key_t key);

	/*	the caller declares this image as free	*/
//	int free(fim::Image* oi);

	/*	erases the image from the cache	*/
	int erase(fim::Image* oi);

	/*	erases the image clone from the cache	*/
	int erase_clone(fim::Image* oi);

	/* get the lru element. if unused is true, only an unused image will be returned, _if any_*/
	Image* get_lru( bool unused = false )const;

	int free_some_lru();

	bool free_all();
	
	int used_image(cache_key_t key)const;
	public:
	Cache();

	/*	free() and counter update */
	bool freeCachedImage(Image *image);

	/*	getCachedImage() and counter update */
	Image * useCachedImage(cache_key_t key);
	
	/* FIXME */
	Image * setAndCacheStdinCachedImage(Image * image);

	/**/
	int prefetch(cache_key_t key);

	fim::string getReport();
	~Cache();
};
}

#endif

