/* $LastChangedDate: 2015-04-18 21:28:34 +0200 (Sat, 18 Apr 2015) $ */
/*
 Cache.h : Cache manager header file

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
#ifndef FIM_CACHE_H
#define FIM_CACHE_H
#include "fim.h"

#ifdef HAVE_SYS_TIME_H
#include <stdint.h>
#include <unistd.h>
typedef uint64_t fim_time_t;
#else /* HAVE_SYS_TIME_H */
typedef time_t fim_time_t;
#endif /* HAVE_SYS_TIME_H */
#define FIM_CR_BS 0 /* base */
#define FIM_CR_CN 1 /* not detailed */
#define FIM_CR_CD 2 /* detailed */
namespace fim
{
#ifdef FIM_NAMESPACES
class Cache:public Namespace
#else
class Cache
#endif /* FIM_NAMESPACES */
{
	typedef std::map<fim::Image*,fim_time_t > 	   lru_t;	//filename - last usage time
	typedef std::map<cache_key_t,fim::Image* >  cachels_t;	//filename - image
	typedef std::map<fim::Image*,cache_key_t >  rcachels_t;	//image - filename
	typedef std::map<cache_key_t,int >        ccachels_t;	//filename - counter
	typedef std::map<cache_key_t,ViewportState >        vcachels_t;	//filename to viewport state
	typedef std::map<cache_key_t,std::vector<fim::Image*> > cloned_cachels_t;	//filename - cloned images??
	typedef std::map<fim::Image*,int >  	   cuc_t;	//image - filename

	cachels_t 	imageCache_;
	rcachels_t	reverseCache_;
	lru_t		lru_;
	ccachels_t	usageCounter_;
	vcachels_t	viewportInfo_;
	cloned_cachels_t cloneCache_;
	cuc_t		cloneUsageCounter_;
	std::set< fim::Image* > clone_pool_;
	time_t time0_;
//	clone_counter_t cloneCounter;

	/*	whether we should free some cache ..	*/
	bool need_free(void)const;

	/**/
	int lru_touch(cache_key_t key);

	bool is_in_cache(fim::Image* oi)const;
	bool is_in_cache(cache_key_t key)const;
	bool is_in_clone_cache(fim::Image* oi)const;
	fim_time_t last_used(cache_key_t key)const;

	bool cacheNewImage( fim::Image* ni );
	Image * loadNewImage(cache_key_t key, fim_page_t page = 0);

	/*	returns an image from the cache or loads it from disk marking it as used in the LRU (internal) */
	Image * getCachedImage(cache_key_t key);

	/*	the caller declares this image as free	*/
//	int free(fim::Image* oi);

	/*	erases the image from the cache	*/
	int erase(fim::Image* oi);

	/*	erases the image clone from the cache	*/
	int erase_clone(fim::Image* oi);

	/* get the lru_ element. if unused is true, only an unused image will be returned, _if any_*/
	Image* get_lru( bool unused = false )const;

	int free_some_lru(void);

	bool free_all(void);
	
	int used_image(cache_key_t key)const;

	fim_time_t reltime(void)const;

	public:
	Cache(void);

#if FIM_WANT_BDI
	Image dummy_img_;	// experimental
#endif	/* FIM_WANT_BDI */

	/*	free() and counter update */
	bool freeCachedImage(Image *image, const ViewportState *vsp);

	/*	getCachedImage() and counter update */
	Image * useCachedImage(cache_key_t key, ViewportState *vsp, fim_page_t page = 0);
	
	/* FIXME */
	Image * setAndCacheStdinCachedImage(Image * image);

	/**/
	int prefetch(cache_key_t key);

	void touch(cache_key_t key);

	fim::string getReport(int type = FIM_CR_CD )const;
	~Cache(void);
	virtual size_t byte_size(void)const;
	size_t img_byte_size(void)const;

	/*	the count of cached images	*/
	int cached_elements(void)const;
	void desc_update(void);
};
}

#endif /* FIM_CACHE_H */

