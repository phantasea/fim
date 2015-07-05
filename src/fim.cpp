/* $Id: fim.cpp 269 2009-12-08 23:45:10Z dezperado $ */
/*
 fim.cpp : Fim main program and accessory functions

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

/*
 * This file contains mainly the code that couldn't fit in any of the existing classes.
 * When the fbi->fim transition will be complete, it will be probably very very small, 
 * as then all lone functions will be encapsulated.
 *
 * p.s.: it will be also _much_ cleaner ...
 * */

#include "fim.h"
#include <signal.h>
#include <getopt.h>
#ifdef FIM_READLINE_H
#include "readline.h"	/* readline stuff */
#endif
/*
 * We use the STL (Standard Template Library)
 */
using std :: endl;
using std :: ifstream;
using std :: ofstream;
using std :: map;
using std :: multimap;
using std :: pair;
using std :: vector;


/*
 * Global variables.
 * */
	fim::string g_fim_output_device;
	FlexLexer *lexer;

/*
 * (nearly) all Fim stuff is in the fim namespace.
 * */
namespace fim
{
	/*
	 * Globals : should be encapsulated.
	 * */
	fim::CommandConsole cc;
	char *default_fbdev=NULL,*default_fbmode=NULL;
	int default_vt=-1;
	float default_fbgamma=-1.0;
}

/*
 * Yet unfinished. 
 * This structure keeps hold of Fim's options flags.
 */
struct option fim_options[] = {
    {"version",    no_argument,       NULL, 'V'},  /* version */
    {"help",       no_argument,       NULL, 'h'},  /* help */
    {"device",     required_argument, NULL, 'd'},  /* device */
    {"mode",       required_argument, NULL, 'm'},  /* video mode */
    {"binary",     optional_argument,       NULL, 'b'},  /* binary mode */
    {"gamma",      required_argument, NULL, 'g'},  /* set gamma */
    {"quiet",      no_argument,       NULL, 'q'},  /* quiet */
    {"verbose",    no_argument,       NULL, 'v'},  /* verbose */
    {"scroll",     required_argument, NULL, 's'},  /* set scrool */
/*    {"timeout",    required_argument, NULL, 't'},*/  /* timeout value */	/* fbi's */
/*    {"once",       no_argument,       NULL, '1'},*/  /* loop only once */
    {"resolution", required_argument, NULL, 'r'},  /* select resolution */
    {"random",     no_argument,       NULL, 'u'},  /* randomize images */
/*    {"font",       required_argument, NULL, 'f'},*/  /* font */
    {"etc-fimrc",       required_argument, NULL, 'f'},  /* etc-fimrc read (experimental) */
    {"autozoom",   no_argument,       NULL, 'a'},
    {"autotop",   no_argument,       NULL, 'A'},
    {"autowidth",   no_argument,       NULL, 'w'},
/*    {"edit",       no_argument,       NULL, 'e'},*/  /* enable editing */	/* fbi's */
/*    {"list",       required_argument, NULL, 'l'},*/
    {"vt",         required_argument, NULL, 'T'},
//    {"backup",     no_argument,       NULL, 'b'},	/* fbi's */
    {"execute-script",   required_argument,       NULL, 'E'},
    {"execute-commands", required_argument,       NULL, 'c'},
    {"final-commands",   required_argument,       NULL, 'F'},
//    {"debug",      no_argument,       NULL, 'D'},
    {"no-rc-file",      no_argument,       NULL, 'N'},
    {"dump-default-fimrc",      no_argument,       NULL, 'D'},
#ifdef FIM_READ_STDIN
    {"read-from-stdin",      no_argument,       NULL, '-'},
#endif
    {"no-framebuffer",      no_argument,       NULL, 't'},
    {"text-reading",      no_argument,       NULL, 'P'},
#ifdef FIM_READ_STDIN_IMAGE
    {"image-from-stdin",      no_argument,       NULL, 'i'},
#endif
//    {"preserve",   no_argument,       NULL, 'p'},	/* fbi's */
    {"script-from-stdin",      no_argument,       NULL, 'p'},
    {"sanity-check",      no_argument,       NULL, 'S'},	/* NEW */
    {"write-scriptout",      required_argument,       NULL, 'W'},
    {"offset",      required_argument,       NULL,  0xFFD8FFE0},/* NEW */
    {"output-device",      required_argument,       NULL, 'o'},
    {"dump-reference-help",      /*optional_argument : TODO */no_argument,       NULL, 0xd15cbab3},/* note : still undocumented switch */

    /* long-only options */
//    {"autoup",     no_argument,       &autoup,   1 },
//    {"autodown",   no_argument,       &autodown, 1 },
//    {"comments",   no_argument,       &comments, 1 },
    {0,0,0,0}
};



class FimInstance
{


	static void version()
	{
	    FIM_FPRINTF(stderr, 
			    "FIM - Fbi IMproved "
	#ifdef FIM_VERSION
			    FIM_VERSION
	#endif
	#ifdef SVN_REVISION
			    " ( repository version "
		SVN_REVISION
			    " )"
	#else
	/* obsolete */
	# define FIM_REPOSITORY_VERSION  "$Id: fim.cpp 269 2009-12-08 23:45:10Z dezperado $"
	# ifdef FIM_REPOSITORY_VERSION 
			    " ( repository version "
		FIM_REPOSITORY_VERSION 	    
			    " )"
	# endif
	#endif
	#ifdef FIM_AUTHOR 
			    ", by "
			    FIM_AUTHOR
	#endif
			    ", built on %s\n",
			    __DATE__
	    		    " ( based on fbi version 1.31 (c) by 1999-2003 Gerd Hoffmann )\n"
	#ifdef CXXFLAGS
			"Compile flags: CXXFLAGS="CXXFLAGS
	#ifdef CFLAGS
			"  CFLAGS="CFLAGS
	#endif
			"\n"
	#endif
			"Fim options (features included (+) or not (-)):\n"
	#include "version.h"
	/* i think some flags are missing .. */
		"\nSupported output devices (for --output-device) : "
	#ifdef FIM_WITH_AALIB
		" aa"
	#endif
	#ifdef FIM_WITH_CACALIB
		" caca"
	#endif
	#ifdef FIM_WITH_LIBSDL
		" sdl"
	#endif
#ifndef FIM_WITH_NO_FRAMEBUFFER
		" fb"
#endif //#ifndef FIM_WITH_NO_FRAMEBUFFER
	#if 1
		" dumb"
	#endif
		"\n"
		"\nSupported file formats : "
#ifdef ENABLE_PDF
		" pdf"
#endif
#ifdef HAVE_LIBSPECTRE
		" ps"
#endif
#ifdef HAVE_LIBDJVU
		" djvu"
#endif
#ifdef HAVE_LIBJPEG
		" jpeg"
#endif
#ifdef FIM_HANDLE_TIFF
		" tiff"
#endif
#ifdef FIM_HANDLE_GIF
		" gif"
#endif
#ifdef FIM_WITH_LIBPNG
		" png"
#endif
		" ppm"	/* no library is needed for these */
		" bmp"
#ifdef HAVE_MATRIX_MARKET_DECODER
		" mtx (Matrix Market)"
#endif
		"\n"
			    );
	}

int help_and_exit(char *argv0, int code=0)
{
	    cc.printHelpMessage(argv0);
	    std::cout << " where OPTIONS are taken from :\n";
	    for(size_t i=0;i<(sizeof(fim_options)/sizeof(struct option))-1;++i)
	    {	
		if(isascii(fim_options[i].val)){
	   	if((fim_options[i].val)!='-')std::cout << "\t-"<<(char)(fim_options[i].val) ;
	   	else std::cout << "\t-";}else std::cout<<"\t";
		std::cout << "\t\t";
	    	std::cout << "--"<<fim_options[i].name ;
		switch(fim_options[i].has_arg){
		case no_argument:
		break;
		case required_argument:
		std::cout << " <arg>";
		break;
		case optional_argument:
		std::cout << " [=arg]";
		break;
		default:
		;
		};
		std::cout << "\n";
		}
		std::cout << "\n Please read the documentation distributed with the program, in FIM.TXT.\n"
			  << " For further help, consult the online help in fim (:help), and man fim (1).\n"
			  << " For bug reporting please read the BUGS file.\n";
	    std::exit(code);
	    return code;
}


	public:
	int main(int argc,char *argv[])
	{
		char *default_fbdev=NULL,*default_fbmode=NULL;
		int default_vt=-1;
		int retcode=0;
		float default_fbgamma=-1.0;
		/*
		 * an adapted version of the main function
		 * of the original version of the fbi program
		 */
	// 	int              timeout = -1;	// fbi's
		int              opt_index = 0;
		int              i;
	#ifdef FIM_READ_STDIN
		int              read_file_list_from_stdin;
		read_file_list_from_stdin=0;
		#ifdef FIM_READ_STDIN_IMAGE
		int		 read_one_file_from_stdin;
		read_one_file_from_stdin=0;
		#endif
		int		 read_one_script_file_from_stdin;
		read_one_script_file_from_stdin=0;
		int perform_sanity_check=0;
	#endif
	//	char             *desc,*info;
		int c;
		int ndd=0;/*  on some systems, we get 'int dup(int)', declared with attribute warn_unused_result */
		bool appendedPostInitCommand=false;
	    	g_fim_output_device="";
	
		setlocale(LC_ALL,"");	//uhm..
	    	for (;;) {
		    /*c = getopt_long(argc, argv, "wc:u1evahPqVbpr:t:m:d:g:s:f:l:T:E:DNhF:",*/
		    c = getopt_long(argc, argv, "Ab?wc:uvahPqVr:m:d:g:s:T:E:DNhF:tfipW:o:S",
				fim_options, &opt_index);
		if (c == -1)
		    break;
		switch (c) {
	/*	case 0:
		    // long option, nothing to do
		    break;*/
	//	case '1':
		    //fbi's
	//	    FIM_FPRINTF(stderr, "sorry, this feature will be implemented soon\n");
	//	    once = 1;
	//	    break;
		case 'a':
		    //fbi's
		    //cc.setVariable(FIM_VID_AUTOTOP,1);
		    //TODO: still needs some tricking .. 
	#ifdef FIM_AUTOCMDS
		    cc.pre_autocmd_add("v:"FIM_VID_AUTO_SCALE_V"=1;");
		    cc.pre_autocmd_add(FIM_VID_AUTOWIDTH"=0;");/*  these mutual interactions are annoying */
	#endif
		    break;
		case 'b':
		    //fim's
		    if(optarg && strstr(optarg,"1")==optarg && !optarg[1])
			{
		    	cc.setVariable(FIM_VID_BINARY_DISPLAY,1);
			}
		    else
		    if(optarg && strstr(optarg,"24")==optarg && !optarg[2])
			{
		    	cc.setVariable(FIM_VID_BINARY_DISPLAY,24);
			}
                    else
		    {
			if(optarg)std::cerr<<"Warning : the --binary option supports 1 or 24 bpp depths. Using 24.\n";
		    	cc.setVariable(FIM_VID_BINARY_DISPLAY,24);
                    }
		    break;
		case 'A':
		    //fbi's
		    //cc.setVariable(FIM_VID_AUTOTOP,1);
		    //FIXME: still needs some tricking .. 
	#ifdef FIM_AUTOCMDS
		    cc.pre_autocmd_add(FIM_VID_AUTOTOP"=1;");
	#endif
		    break;
		case 'q':
		    //fbi's
		    //FIM_FPRINTF(stderr, "sorry, this feature will be implemented soon\n");
		    //cc.setVariable(FIM_VID_DISPLAY_STATUS,0);
	#ifdef FIM_AUTOCMDS
		    cc.pre_autocmd_add(FIM_VID_DISPLAY_STATUS"=0;");
	#endif
		    break;
		case 'f':
		    cc.setVariable(FIM_VID_LOAD_DEFAULT_ETC_FIMRC,0);
		    /*
		     * note that this solution is temporary, because it clashes with -E (should have precedence, instead)
		     * */
		    cc.push_scriptfile(optarg);
		    break;
		case 'S':
		    //fim's
	#ifdef FIM_AUTOCMDS
		    cc.setVariable(FIM_VID_SANITY_CHECK,1);
		    perform_sanity_check=1;
	#endif
		    break;
		case 'v':
		    //fbi's
		    //cc.setVariable(FIM_VID_DISPLAY_STATUS,1);
	#ifdef FIM_AUTOCMDS
		    cc.pre_autocmd_add(FIM_VID_DISPLAY_STATUS"=1;");
	#endif
		    break;
		case 'w':
		    //fbi's
		    //cc.setVariable(FIM_VID_AUTOWIDTH,1);
	#ifdef FIM_AUTOCMDS
		    cc.pre_autocmd_add(FIM_VID_AUTOWIDTH"=1;");
	#endif
		    break;
		case 'P':
		    //fbi's
	#ifdef FIM_AUTOCMDS
		    cc.pre_autocmd_add(FIM_VID_AUTOWIDTH"=1;"FIM_VID_AUTOTOP"=1;");
	#endif
		    break;
		case 0xFFD8FFE0:
		    //fbi's
	 	    // NEW
	#ifdef FIM_AUTOCMDS
		{
			int ipeppe_offset;

			ipeppe_offset=(int)atoi(optarg);
			if(ipeppe_offset<0)
				std::cerr<< "warning: ignoring user set negative offset value.\n";
			else
			if(ipeppe_offset>0)
			{
				string tmp;
				size_t peppe_offset=0;
				peppe_offset =(size_t)ipeppe_offset;
				tmp=FIM_VID_OPEN_OFFSET;
				tmp+="=";
				tmp+=string((int)peppe_offset);/* FIXME */
				tmp+=";";
				cc.pre_autocmd_add(tmp);
				//std::cout << "peppe_offset" << peppe_offset<< "\n";
			}
		}
	#endif
		    break;
		case 'g':
		    //fbi's
		    default_fbgamma = fim_atof(optarg);
		    break;
		case 'r':
		    //fbi's
	// TODO
	//	    pcd_res = atoi(optarg);
		    break;
		case 's':
	//	    if(atoi(optarg)>0) cc.setVariable(FIM_VID_STEPS,atoi(optarg));
		    if(atoi(optarg)>0)
		    {
		    	// fixme : still buggy
		    	fim::string s="steps=";
			s+=fim::string((int)atoi(optarg));
			s+=";";
	#ifdef FIM_AUTOCMDS
			cc.pre_autocmd_add(s);
	#endif
		    }
		    break;
	//	case 't':
		    //fbi's
	//	    timeout = atoi(optarg);
	//	    FIM_FPRINTF(stderr, "sorry, this feature will be implemented soon\n");
	//	    break;
		case 'u':
		    //fbi's
		    FIM_FPRINTF(stderr, "sorry, this feature will be implemented soon\n");
	//	    randomize = 1;
		    break;
		case 'd':
		    //fbi's
		    default_fbdev = optarg;
		    break;
		case 'i':
		    //fim's
#ifdef FIM_READ_STDIN_IMAGE
		    read_one_file_from_stdin=1;
#else
		    FIM_FPRINTF(stderr, "sorry, the reading of images from stdin was disabled at compile time\n");
#endif
		    break;
		case 'm':
		    //fbi's
		    default_fbmode = optarg;
		    break;
	//removed, editing features :
	/*	case 'f':
	//	    fontname = optarg;
		    break;
		case 'e':
	//	    editable = 1;
		    break;
		case 'b':
	//	    backup = 1;
		    break;
		case 'p':
	//	    preserve = 1;
		    break;*/
	//	case 'l':
		    //fbi's
	//	    flist_add_list(optarg);
	//	    FIM_FPRINTF(stderr, "sorry, this feature will be implemented soon\n");
	//	    break;
		case 'T':
		    //fbi's virtual terminal
		    default_vt = atoi(optarg);
		    break;
		case 'V':
		    version();
		    return 0;
		    break;
		case 'c':
		    //fim's
		    cc.appendPostInitCommand(optarg);
		    appendedPostInitCommand=true;
		    break;
		case 'W':
		    //fim's
		    cc.setVariable(FIM_VID_SCRIPTOUT_FILE,optarg);
	#ifdef FIM_AUTOCMDS
		    cc.pre_autocmd_add("start_recording;");
		    cc.appendPostExecutionCommand("stop_recording");
	#endif

		    break;
		case 'F':
		    //fim's
		    cc.appendPostExecutionCommand(optarg);
		    break;
		case 'E':
		    //fim's
	#ifndef FIM_NOSCRIPTING
		    cc.push_scriptfile(optarg);
	#else
		    cout << "sorry, no scripting available!\n";
	#endif
		    break;
		case 'p':
		    //fim's (differing semantics from fbi's)
	#ifndef FIM_NOSCRIPTING
		    read_one_script_file_from_stdin=1;
	#else
		    cout << "sorry, no scripting available!\n";
	#endif
		    break;
		case 'D':
		    //fim's
	//	    cc.setNoFrameBuffer();	// no framebuffer (debug) mode
		    cc.dumpDefaultFimrc();
		    std::exit(0);
		    break;
		case 'N':
		    //fim's
			cc.setVariable(FIM_VID_NO_RC_FILE,1);
		    break;
		case 't':
		    //fim's
			#ifdef FIM_WITH_AALIB
		    	g_fim_output_device="aa";
			#else
			std::cerr << "you should recompile fim with aalib support!\n";
			g_fim_output_device="dumb";
			#endif
		    break;
		case 'o':
		    //fim's
		    	g_fim_output_device=optarg;
		    break;
		case 0xd15cbab3:
		    //fim's
		    cc.dump_reference_manual(args_t());
	            std::exit(0);
		    break;
	#ifdef FIM_READ_STDIN
		case '-':
		    //fim's
		    read_file_list_from_stdin=1;
		    break;
		case 0:
		    //fim's
		    read_file_list_from_stdin=1;
		    break;
	#endif
		default:
		case 'h':
		    help_and_exit(argv[0]);
		}
	    }
		for (i = optind; i < argc; i++)
		{
	#ifdef FIM_READ_STDIN
			if(*argv[i]=='-'&&!argv[i][1])read_file_list_from_stdin=1;
			else
	#endif
			{
				cc.push(argv[i]);
			}
		}
	
		lexer=new yyFlexLexer;	//used by YYLEX
	

	#ifdef FIM_READ_STDIN	
		if( read_file_list_from_stdin +
		#ifdef FIM_READ_STDIN_IMAGE
		read_one_file_from_stdin+
		#endif
		read_one_script_file_from_stdin > 1)
		{
			FIM_FPRINTF(stderr, "error : you shouldn't specify more than one standard input reading options among (-, -p, ad -i)!\n\n");
			retcode=help_and_exit(argv[0],0);/* should return 0 or -1 ? */
			goto ret;
		}
		/*
		 * this is Vim's solution for stdin reading
		 * */
		if(read_file_list_from_stdin)
		{
			char *lineptr=NULL;
			size_t bs=0;
			while(fim_getline(&lineptr,&bs,stdin)>0)
			{
				chomp(lineptr);
				cc.push(lineptr);
				//printf("%s\n",lineptr);
				lineptr=NULL;
			}
			if(lineptr)fim_free(lineptr);
			close(0);
			ndd=dup(2);
		}
		#ifdef FIM_READ_STDIN_IMAGE
		else
		if(read_one_file_from_stdin)
		{
			/*
			 * we read a whole image file from stdin
			 * */
			FILE *tfd=NULL;
			if( ( tfd=fim_fread_tmpfile(stdin) )!=NULL )
			{	
				Image* stream_image=NULL;
				/*
				 * Note that it would be much nicer to do this in another way,
				 * but it would require to rewrite much of the file loading stuff
				 * (which is quite fbi's untouched stuff right now)
				 * */
				try{
					stream_image=new Image(FIM_STDIN_IMAGE_NAME,tfd);
				}catch (FimException e){/* write me */}

				// DANGEROUS TRICK!
				cc.browser.set_default_image(stream_image);
				if(!cc.browser.cache.setAndCacheStdinCachedImage(stream_image))
					std::cerr << "problems caching standard input image!\n";// FIXME

				cc.browser.push(FIM_STDIN_IMAGE_NAME);
				//fclose(tfd);	// uncommenting this will cause a segfault (why ? FIXME)
			}
			close(0);
			ndd=dup(2);
		}
		#endif
		else
		if(read_one_script_file_from_stdin)
		{
		    	char* buf;
			buf=slurp_binary_fd(0,NULL);
			if(buf) cc.appendPostInitCommand(buf);
			if(buf) appendedPostInitCommand=true;
			if(buf) fim_free(buf);
			close(0);
			ndd=dup(2);
		}
	#endif

		if(ndd==-1)
			perror(NULL);
	
		if(cc.browser.empty_file_list() && !cc.with_scriptfile() && !appendedPostInitCommand 
		#ifdef FIM_READ_STDIN_IMAGE
		&& !read_one_file_from_stdin
		#endif
		&& !perform_sanity_check
		)
		{retcode=help_and_exit(argv[0],-1);goto ret;}
	
		/* output device guess */
		if( g_fim_output_device=="" )
		{
			#ifdef FIM_WITH_LIBSDL
			/* check to see if we are under X */
			if( fim_getenv("DISPLAY") )
			{
				g_fim_output_device="sdl";
			}
			else
			#endif
#ifndef FIM_WITH_NO_FRAMEBUFFER
			g_fim_output_device="fb";
#else
	#ifdef FIM_WITH_AALIB
			g_fim_output_device="aa";
	#else
			g_fim_output_device="dummy";
	#endif
#endif	//#ifndef FIM_WITH_NO_FRAMEBUFFER
		}

		// TODO : we still need a good output device probing mechanism

		if(cc.init(g_fim_output_device)!=0) return -1;
		retcode=cc.executionCycle();/* note that this could not return */
ret:
		return retcode;
	}

};

int main(int argc,char *argv[])
{
	/*
	 * FimInstance will encapsulate all of the fim's code someday.
	 * ...someday.
	 * */
	FimInstance fiminstance;
	Var::var_help_db_init();
	return fiminstance.main(argc,argv);
}


