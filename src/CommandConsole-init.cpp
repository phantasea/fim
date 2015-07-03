/* $LastChangedDate: 2015-02-12 19:10:43 +0100 (Thu, 12 Feb 2015) $ */
/*
 CommandConsole-init.cpp : Fim console initialization

 (c) 2010-2015 Michele Martone

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
 * TODO:
 * 	framebufferdevice -> device
 * 	output methods should be moved in some other, new class
 * */
#include "fim.h"
#ifdef FIM_DEFAULT_CONFIGURATION
#include "conf.h"
#endif /* FIM_DEFAULT_CONFIGURATION */
#include <sys/time.h>
#include <errno.h>

#ifdef FIM_USE_READLINE
#include "readline.h"
#endif /* FIM_USE_READLINE */

namespace fim
{

#if FIM_WANT_BENCHMARKS
static fim_err_t fim_bench_subsystem(Benchmarkable * bo)
{
	fim_int qbn,qbi;
	fim_fms_t tbtime,btime;// ms
	size_t times=1;

	if(!bo)
		return FIM_ERR_GENERIC;
	qbn=bo->get_n_qbenchmarks();

	for(qbi=0;qbi<qbn;++qbi)
	{
		times=1;
		tbtime=1000.0,btime=0.0;// ms
		bo->quickbench_init(qbi);
		do
		{
			btime=-getmilliseconds();
			//fim_bench_video(bfp);
			bo->quickbench(qbi);
			btime+=getmilliseconds();
			++times;
			tbtime-=btime;
		}
		while(btime>=0.0 && tbtime>0.0 && times>0);
		--times;
		tbtime=1000.0-tbtime;
		std::cout << bo->get_bresults_string(qbi,times,tbtime);
		bo->quickbench_finalize(qbi);
	}
	return FIM_ERR_NO_ERROR;
}
#endif /* FIM_WANT_BENCHMARKS */



	fim_err_t CommandConsole::init(fim::string device)
	{
		/*
		 * TODO : move most of this stuff to the constructor, some day.
		 */
		fim_int xres=0,yres=0;
		bool device_failure=false;
		int dosso=device.find(FIM_SYM_DEVOPTS_SEP_STR);
		bool wcs = false;
		std::string dopts;
		/* new : prevents atof, sprintf and such conversion mismatches! */
		setlocale(LC_ALL,"C");	/* portable (among Linux hosts) : should use dots for numerical radix separator */
		//setlocale(LC_NUMERIC,"en_US"); /* lame  */
		//setlocale(LC_ALL,""); /* just lame */
		displaydevice_=NULL;	/* TODO : is this really necessary ? */

		if(dosso>0)
			dopts=device.substr(dosso+strlen(FIM_SYM_DEVOPTS_SEP_STR),device.length()).c_str();

#ifndef FIM_WITH_NO_FRAMEBUFFER
		if(device.find(FIM_DDN_INN_FB)==0)
		{
			extern fim_char_t *default_fbdev,*default_fbmode; /* FIXME: we don't want globals! */
			extern int default_vt;
			extern float default_fbgamma;
			FramebufferDevice * ffdp=NULL;

			displaydevice_=new FramebufferDevice(
#ifndef FIM_WANT_NO_OUTPUT_CONSOLE
					mc_
#endif /* FIM_WANT_NO_OUTPUT_CONSOLE */
					);
			if(!displaydevice_ || ((FramebufferDevice*)displaydevice_)->framebuffer_init()){cleanup();return FIM_ERR_GENERIC;}
			ffdp=((FramebufferDevice*)displaydevice_);
			setVariable(FIM_VID_DEVICE_DRIVER,FIM_DDN_VAR_FB);
			if(default_fbdev)ffdp->set_fbdev(default_fbdev);
			if(default_fbmode)ffdp->set_fbmode(default_fbmode);
			if(default_vt!=-1)ffdp->set_default_vt(default_vt);
			if(default_fbgamma!=-1.0)ffdp->set_default_fbgamma(default_fbgamma);
			mangle_tcattr_=true;
		}
#endif	//#ifndef FIM_WITH_NO_FRAMEBUFFER


		#ifdef FIM_WITH_LIBIMLIB2
		if(device.find(FIM_DDN_VAR_IL2)==0)
		{
			DisplayDevice *imld=NULL;
			imld=new Imlib2Device(
#ifndef FIM_WANT_NO_OUTPUT_CONSOLE
					mc_,
#endif /* FIM_WANT_NO_OUTPUT_CONSOLE */
					dopts
					);
			if(imld && imld->initialize(sym_keys_)!=FIM_ERR_NO_ERROR){delete imld ; imld=NULL;}
			if(imld && displaydevice_==NULL)
			{
				displaydevice_=imld;
				setVariable(FIM_VID_DEVICE_DRIVER,FIM_DDN_VAR_IL2);
				mangle_tcattr_=false;
			}
			else
			{
				device_failure=true;
			}
			wcs = true; /* FIXME: want cookie stream (for readline with no stdin) */
		}
		#endif /* FIM_WITH_LIBIMLIB2 */

		#ifdef FIM_WITH_LIBSDL
		if(device.find(FIM_DDN_INN_SDL)==0)
		{
			DisplayDevice *sdld=NULL;
			fim::string fopts;
#if FIM_WANT_SDL_OPTIONS_STRING 
			fopts=dopts;
#endif /* FIM_WANT_SDL_OPTIONS_STRING */
			sdld=new SDLDevice(
#ifndef FIM_WANT_NO_OUTPUT_CONSOLE
					mc_,
#endif /* FIM_WANT_NO_OUTPUT_CONSOLE */
					fopts
					);
		       	if(sdld && sdld->initialize(sym_keys_)!=FIM_ERR_NO_ERROR){delete sdld ; sdld=NULL;}
			if(sdld && displaydevice_==NULL)
			{
				displaydevice_=sdld;
				setVariable(FIM_VID_DEVICE_DRIVER,FIM_DDN_VAR_SDL);
				mangle_tcattr_=false;
			}
			else
			{
				device_failure=true;
			}
			wcs = true; /* FIXME: want cookie stream (for readline with no stdin) */
		}
		#endif /* FIM_WITH_LIBSDL */

		#ifdef FIM_WITH_CACALIB
		if(device.find(FIM_DDN_INN_CACA)==0)
		{
			DisplayDevice *cacad=NULL;
			cacad=new CACADevice(
#ifndef FIM_WANT_NO_OUTPUT_CONSOLE
					mc_
#endif /* FIM_WANT_NO_OUTPUT_CONSOLE */
					); if(cacad && cacad->initialize(sym_keys_)!=FIM_ERR_NO_ERROR){delete cacad ; cacad=NULL;}
			if(cacad && displaydevice_==NULL)
			{
				displaydevice_=cacad;
				setVariable(FIM_VID_DEVICE_DRIVER,FIM_DDN_VAR_CACA);
				mangle_tcattr_=false;
			}
			else
				device_failure=true;
		}
		#endif /* FIM_WITH_CACALIB */

		#ifdef FIM_WITH_AALIB
		if(device.find(FIM_DDN_INN_AA)==0)
		{
		aad_=new AADevice(
#ifndef FIM_WANT_NO_OUTPUT_CONSOLE
				mc_,
#endif /* FIM_WANT_NO_OUTPUT_CONSOLE */
				dopts
				);

		if(aad_ && aad_->initialize(sym_keys_)!=FIM_ERR_NO_ERROR){delete aad_ ; aad_=NULL;}
		{
		if(aad_ && displaydevice_==NULL)
		{
			displaydevice_=aad_;
			setVariable(FIM_VID_DEVICE_DRIVER,FIM_DDN_VAR_AA);
#if FIM_WANT_SCREEN_KEY_REMAPPING_PATCH
			/*
			 * FIXME
			 *
			 * seems like the keymaps get shifted when running under screen
			 * weird, isn't it ?
			 * Regard this as a weird patch.
			 * */
			const fim_char_t * term = fim_getenv(FIM_CNS_TERM_VAR);
			if(term && string(term).re_match("screen"))
			{
				sym_keys_[FIM_KBD_LEFT]-=3072;
				sym_keys_[FIM_KBD_RIGHT]-=3072;
				sym_keys_[FIM_KBD_UP]-=3072;
				sym_keys_[FIM_KBD_DOWN]-=3072;
			}
			mangle_tcattr_=false;
#endif /* FIM_WANT_SCREEN_KEY_REMAPPING_PATCH */
		}
		else device_failure=true;
		}
		}
		#endif /* FIM_WITH_AALIB */
		if(mangle_tcattr_)tty_raw();// this inhibits unwanted key printout (raw mode), and saves the current tty state
		// FIXME: an error here on, leaves the terminal in raw mode, and this is not cool

		if( displaydevice_==NULL)
		{
			displaydevice_=&dummydisplaydevice_;
			setVariable(FIM_VID_DEVICE_DRIVER,FIM_DDN_VAR_DUMB);
			if(device_failure)
			{
				std::cerr << "Failure using the \""<<device<<"\" display device driver string (wrong options?)!\n";
				cleanup();
				return FIM_ERR_UNSUPPORTED_DEVICE;

			}
			else
			if(device!=FIM_DDN_VAR_DUMB)
				std::cerr << "Unrecognized display device string \""<<device<<"\" (valid choices are " FIM_DDN_VARS ")!\n",
			std::cerr << "Using the default \""<<FIM_DDN_INN_DUMB<<"\" display device instead.\n";
		}

		xres=displaydevice_->width(),yres=displaydevice_->height();

		// textual console reformatting (should go to displaydevice some day)
		displaydevice_->init_console();
		// FIXME: shall check the error result

#ifdef FIM_WINDOWS
		/* true pixels if we are in framebuffer mode */
		/* fake pixels if we are in text (er.. less than!) mode */
		if( xres<=0 || yres<=0 )
		{
			std::cerr << "Unable to spawn a suitable display.\n";
		       	return FIM_ERR_BAD_PARAMS;
		}

		try
		{
			window_ = new FimWindow( *this, Rect(0,0,xres,yres) );

			if(window_)window_->setroot();
		}
		catch(FimException e)
		{
			if( e == FIM_E_NO_MEM || true ) quit(FIM_E_NO_MEM);
		}

		/*
		 * TODO: exceptions should be launched here in case ...
		 * */
		addCommand(new Command(fim::string(FIM_FLT_WINDOW),fim::string(FIM_CMD_HELP_WINDOW), window_,&FimWindow::fcmd_cmd));
#else /* FIM_WINDOWS */
		try
		{
			viewport_ = new Viewport(*this);
		}
		catch(FimException e)
		{
			if( e == FIM_E_NO_MEM || true ) quit(FIM_E_NO_MEM);
		}
#endif /* FIM_WINDOWS */
		setVariable(FIM_VID_SCREEN_WIDTH, xres);
		setVariable(FIM_VID_SCREEN_HEIGHT,yres);

		/* Here the program loads initialization scripts */

	    	FIM_AUTOCMD_EXEC(FIM_ACM_PRECONF,"");
	    	FIM_AUTOCMD_EXEC(FIM_ACM_PREHFIMRC,"");

  #ifndef FIM_WANT_NOSCRIPTING
		if(preConfigCommand_!=fim::string(""))
			execute_internal(preConfigCommand_.c_str(),FIM_X_HISTORY);

		if(getIntVariable(FIM_VID_NO_DEFAULT_CONFIGURATION)==0 )
		{
    #ifdef FIM_DEFAULT_CONFIGURATION
			/* so the user could inspect what goes in the default configuration */
			setVariable(FIM_VID_FIM_DEFAULT_CONFIG_FILE_CONTENTS,FIM_DEFAULT_CONFIG_FILE_CONTENTS);
			execute_internal(FIM_DEFAULT_CONFIG_FILE_CONTENTS,FIM_X_QUIET);
    #endif		/* FIM_DEFAULT_CONFIGURATION */
		}
  #endif /* FIM_WANT_NOSCRIPTING */

#ifndef FIM_NOFIMRC
  #ifndef FIM_WANT_NOSCRIPTING
		fim_char_t rcfile[FIM_PATH_MAX];
		const fim_char_t *e = fim_getenv(FIM_CNS_HOME_VAR);

	    	FIM_AUTOCMD_EXEC(FIM_ACM_POSTHFIMRC,""); 
	    	FIM_AUTOCMD_EXEC(FIM_ACM_PREGFIMRC,""); 

		if((getIntVariable(FIM_VID_LOAD_DEFAULT_ETC_FIMRC)==1 )
		&& (getStringVariable(FIM_VID_DEFAULT_ETC_FIMRC)!=FIM_CNS_EMPTY_STRING)
				)
		{
			string ef=getStringVariable(FIM_VID_DEFAULT_ETC_FIMRC);
			if(is_file(ef.c_str()))
				if(FIM_ERR_NO_ERROR!=executeFile(ef.c_str()));
		}
		
		/* execution of command line-set autocommands */
	    	FIM_AUTOCMD_EXEC(FIM_ACM_POSTGFIMRC,""); 
	    	FIM_AUTOCMD_EXEC(FIM_ACM_PREUFIMRC,""); 

		{
			#include "grammar.h"
			setVariable(FIM_VID_FIM_DEFAULT_GRAMMAR_FILE_CONTENTS,FIM_DEFAULT_GRAMMAR_FILE_CONTENTS);
		}

		if(e && strlen(e)<FIM_PATH_MAX-8)//strlen("/" FIM_CNS_USR_RC_FILEPATH)+2
		{
			strcpy(rcfile,e);
			strcat(rcfile,"/" FIM_CNS_USR_RC_FILEPATH);
			if(getIntVariable(FIM_VID_NO_RC_FILE)!=1 )
			{
				if(
					(!is_file(rcfile) || FIM_ERR_NO_ERROR!=executeFile(rcfile))
	//			&&	(!is_file(FIM_CNS_SYS_RC_FILEPATH) || FIM_ERR_NO_ERROR!=executeFile(FIM_CNS_SYS_RC_FILEPATH))
				  )
  #endif /* FIM_WANT_NOSCRIPTING */
#endif /* FIM_NOFIMRC */
				{
					/*
					 if no configuration file is present, or fails loading,
					 we use the default configuration (raccomended !)  !	*/
  #ifdef FIM_DEFAULT_CONFIGURATION
					// 20110529 commented the following, as it is a (harmful) duplicate execution 
					//execute_internal(FIM_DEFAULT_CONFIG_FILE_CONTENTS,FIM_X_QUIET);
  #endif		/* FIM_DEFAULT_CONFIGURATION */
				}
#ifndef FIM_NOFIMRC
  #ifndef FIM_WANT_NOSCRIPTING
			}
		}
  #endif		/* FIM_WANT_NOSCRIPTING */
#endif		/* FIM_NOFIMRC */
	    	FIM_AUTOCMD_EXEC(FIM_ACM_POSTUFIMRC,""); 
	    	FIM_AUTOCMD_EXEC(FIM_ACM_POSTCONF,"");
#ifndef FIM_WANT_NOSCRIPTING
		for(size_t i=0;i<scripts_.size();++i) executeFile(scripts_[i].c_str());
#endif		/* FIM_WANT_NOSCRIPTING */
#ifdef FIM_AUTOCMDS
		if(postInitCommand_!=fim::string(""))
			autocmd_add(FIM_ACM_PREEXECUTIONCYCLE,"",postInitCommand_.c_str());
		if(postExecutionCommand_!=fim::string(""))
			autocmd_add(FIM_ACM_POSTEXECUTIONCYCLE,"",postExecutionCommand_.c_str());
#endif /* FIM_AUTOCMDS */

#ifdef FIM_USE_READLINE
		rl::initialize_readline( displaydevice_==NULL, wcs );
		load_or_save_history(true);
#endif /* FIM_USE_READLINE */
		if(getIntVariable(FIM_VID_SANITY_CHECK)==1 )
		{
#if FIM_WANT_BENCHMARKS
			fim_bench_subsystem(displaydevice_);
			fim_bench_subsystem(this);
#endif /* FIM_WANT_BENCHMARKS */
			quit(return_code_);
			exit(return_code_);
		}
		return FIM_ERR_NO_ERROR;
	}

#if FIM_WANT_BENCHMARKS
	fim_int CommandConsole::get_n_qbenchmarks(void)const
	{
		return 1;
	}

	string CommandConsole::get_bresults_string(fim_int qbi, fim_int qbtimes, fim_fms_t qbttime)const
	{
		string msg=FIM_CNS_EMPTY_STRING;
		switch(qbi)
		{
			case 0:
			msg+="fim variables set/get test";
			msg+=" : ";
			msg+=string((float)(((fim_fms_t)qbtimes)/((qbttime)*1.e-3)));
			msg+=" set/get /s\n";
		}
		return msg;
	}

	void CommandConsole::quickbench_init(fim_int qbi)
	{
	}

	void CommandConsole::quickbench_finalize(fim_int qbi)
	{
	}

	void CommandConsole::quickbench(fim_int qbi)
	{
		const fim_int max_sq=1024*1024;
		/* FIXME: may be enhanced with sequence numbers ... */
		cc.setVariable(fim_rand()%(max_sq),fim_rand());
		cc.getIntVariable(fim_rand()%max_sq);
	}
#endif /* FIM_WANT_BENCHMARKS */

	void CommandConsole::dumpDefaultFimrc(void)const
	{
#ifdef FIM_DEFAULT_CONFIGURATION
		std::cout << FIM_DEFAULT_CONFIG_FILE_CONTENTS << "\n";
#endif /* FIM_DEFAULT_CONFIGURATION */
	}
}


