/* $LastChangedDate: 2015-04-18 21:37:25 +0200 (Sat, 18 Apr 2015) $ */
/*
 fim.cpp : Fim main program and accessory functions

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
#include <signal.h>
#include <getopt.h>
#ifdef FIM_READLINE_H
#include "readline.h"	/* readline stuff */
#endif /* FIM_READLINE_H */
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
	fim_char_t *default_fbdev=NULL,*default_fbmode=NULL;
	int default_vt=-1;
	fim_float_t default_fbgamma=-1.0;
	fim_stream cout/*(1)*/;
	fim_stream cerr(2);
}

struct fim_options_t{
  const fim_char_t *name;
  int has_arg;
  int *flag;
  int val;
  const fim_char_t *desc;/* this is fim specific */
  const fim_char_t *optdesc;/* this is fim specific */
  const fim_char_t *mandesc;/* this is fim specific */
};

/*
 * Yet unfinished. 
 * This structure keeps hold of Fim's options flags.
 */
struct fim_options_t fim_options[] = {
    {"autozoom",   no_argument,       NULL, 'a',"scale according to a best fit.",NULL,
"Enable autozoom.  fim will automagically pick a reasonable zoom factor when loading a new image (as in fbi)."
    },
#if FIM_WANT_RAW_BITS_RENDERING
    {FIM_OSW_BINARY,     optional_argument,       NULL, 'b',"view any file as either a 1 or 24 bpp bitmap.","[=24|1]",
"Display (any filetype) binary files contents as they were raw 24 or 1 bits per pixel pixelmaps.\n" 
"Will pad with zeros.\n"
"Regard this as an easter bunny option.\n"
    },
#endif /* FIM_WANT_RAW_BITS_RENDERING */
#if FIM_WANT_TEXT_RENDERING
    {FIM_OSW_TEXT,     no_argument,       NULL, 0x74657874, "view any file as rendered text.",NULL,
"Display (any filetype) files contents as they were text.\n" 
"Will only show printable characters.\n"
"Regard this as an easter bunny option.\n"
    },
#endif /* FIM_WANT_RAW_BITS_RENDERING */
    {"cd-and-readdir", no_argument,       NULL, 0x4352,"step into the first loaded file directory and push other files.",NULL,"step into the first loaded file directory and push other files."},
    {FIM_OSW_EXECUTE_COMMANDS, required_argument,       NULL, 'c',"execute {commands} after initialization.","{commands}",
"The \\fBcommands\\fP string will be executed before entering the interactive loop.\n"
"Please note that if your commands are more complicated than a simple 'next' or 'pornview'\n"
"command, they must be quoted and escaped in a manner suitable for your shell!\n"
"\n"
"For example,\n"
"-c '*2;2pan_up;display;while(1){bottom_align;sleep \"1\" ; top_align}'\n"
"\n"
"(with the single quotes) will tell fim to first double the displayed image \n"
"size, then pan two times up, then display the image ; and then \n"
"do an endless loop consisting of bottom and top aligning, alternated.\n"
    },
    {FIM_OSW_EXECUTE_COMMANDS_EARLY, required_argument,       NULL, 'C',"execute {commands} after initialization, before any config loading.","{commands}",
"Just as the --" FIM_OSW_EXECUTE_COMMANDS " option, but commands will be executed before the loading of any config file.\n"
"\n"
"For example,\n"
"-C '" FIM_VID_SCALE_STYLE "=\" \"' will make fim start with no auto-scaling.\n"
"\n"
    },
    {"device",     required_argument, NULL, 'd',"specify a {framebuffer device}.","{framebuffer device}",
"Framebuffer device to use. Default is the one your vc is mapped to (as in fbi)."
    },
    {"dump-reference-help",      optional_argument /*no_argument*/,       NULL, 0x6472690a,"dump reference info","[=man].",
"Will dump to stdout the language reference help."
    },
    {"dump-default-fimrc",      no_argument,       NULL, 'D',"dump on standard output the default configuration.",NULL,
"The default configuration (the one hardcoded in the fim executable) is dumped on standard output and fim exits."
    },
    {FIM_OSW_EXECUTE_SCRIPT,   required_argument,       NULL, 'E',"execute {scriptfile} after initialization.","{scriptfile}",
"The \\fBscriptfile\\fP will be executed right after the default initialization file is executed."
    },
    {"etc-fimrc",       required_argument, NULL, 'f',"etc-fimrc read.","{fimrc}",
"Specify an alternative system wide initialization file (default: " FIM_CNS_SYS_RC_FILEPATH "), which will be executed prior to any other configuration file.\n"
    },
    {FIM_OSW_FINAL_COMMANDS,   required_argument,       NULL, 'F',"execute {commands} just before exit.","{commands}",
"The \\fBcommands\\fP string will be executed after exiting the interactive loop of the program (right before terminating the program)."
    },
    {"help",       optional_argument,       NULL, 'h',"Print (short, descriptive, long, or complete man) program invocation help, and terminate.","[=s|d|l|m]",
NULL
    },
#if FIM_WANT_PIC_CMTS
    {"load-image-descriptions-file",       required_argument,       NULL, 0x6c696466, "load image descriptions file", "{filename}", "Load image descriptions from {filename}. In {filename} each line is the name of an image file (its basename will be taken), then a Tab character (unless --load-image-descriptions-file is specified), then the description text. Each description will be put in the " FIM_VID_COMMENT " variable of the image at load time. Will override the comment eventually loaded from the file."
    },
    {"image-descriptions-file-separator",       required_argument,       NULL, 0x69646673, "image descriptions file separator character.", "{sepchar}", "A character to be used as a separator between the filename and the description part of lines specified just before a --load-image-descriptions-file."
    },
#endif /* FIM_WANT_PIC_CMTS */
#ifdef FIM_READ_STDIN_IMAGE
    {FIM_OSW_IMAGE_FROM_STDIN,      no_argument,       NULL, 'i',"read an image file from standard input.",NULL,
"Will read one single image from the standard input (the image data, not the filename).  May not work with all supported file formats."
"\nIn the image list, this image will be displayed as \"" FIM_STDIN_IMAGE_NAME "\".\n"
    },
#endif /* FIM_READ_STDIN_IMAGE */
    {"mode",       required_argument, NULL, 'm',"specify a video mode.","{vmode}",
"Name of the video mode to use video mode (must be listed in /etc/fb.modes).  Default is not to change the video mode.  In the past, the XF86 config file (/etc/X11/XF86Config) used to contain Modeline information, which could be fed to the modeline2fb perl script (distributed with fbset).  On many modern xorg based systems, there is no direct way to obtain a fb.modes file from the xorg.conf file.  So instead one could obtain useful fb.modes info by using the (fbmodes (no man page AFAIK)) tool, written by bisqwit.  An unsupported mode should make fim exit with failure.  But it is possible the kernel could trick fim and set a supported mode automatically, thus ignoring the user set mode."
    },
    {"no-rc-file",      no_argument,       NULL, 'N',"do not read the personal initialization file at startup.",NULL,
"No personal initialization file will be read (default is " FIM_CNS_USR_RC_COMPLETE_FILEPATH ") at startup."
    },
    {"no-etc-rc-file",      no_argument,       NULL, 0x4E4E,"do not read the system wide initialization file at startup.",NULL,
"No system wide initialization file will be read (default is " FIM_CNS_SYS_RC_FILEPATH ") at startup."
    },
    {"no-internal-config",      no_argument,       NULL, 0x4E4E4E,"do not execute the internal default configuration at startup.",NULL,
"No internal default configuration at startup (uses internal variable " FIM_VID_NO_DEFAULT_CONFIGURATION "). Will only provide a minimal working configuration. "
    },
    {"no-commandline",      no_argument,       NULL, 0x4E434C,"with internal command line mode disabled.",NULL, "with internal command line mode disabled."},
#if FIM_WANT_HISTORY
    {"no-history-save",      no_argument,       NULL, 0x4E4853,"do not save execution history.",NULL,
"Do not save execution history at finalization (uses internal variable " FIM_VID_SAVE_FIM_HISTORY "). "
    },
    {"no-history-load",      no_argument,       NULL, 0x4E484C,"do not load execution history.",NULL,
"Do not load execution history at startup. "
    },
    {"no-history",      no_argument,       NULL, 0x4E48,"do not load/save execution history.",NULL,
"Do not load or save execution history at startup. "
    },
#endif /* FIM_WANT_HISTORY */
    {FIM_OSW_SCRIPT_FROM_STDIN,      no_argument,       NULL, 'p',"read commands from standard input.",NULL,
"Will read commands from stdin prior to entering in interactive mode."
    },
    {FIM_OSW_OUTPUT_DEVICE,      required_argument,       NULL, 'o',"specify the desired output driver (aka graphic mode).",FIM_DDN_VARS,
"Will use the specified \\fBdevice\\fP as fim video output device, overriding automatic checks."
"The available devices depend on the original configuration/compilation options, so you should\n"
"get the list of available output devices issuing \\fBfim --version\\fP.\n"
"The \\fBaa\\fP option may be specified as  \\fBaa" FIM_SYM_DEVOPTS_SEP_STR "{['w']}\\fP ; the " FIM_MAN_fB("'w'") " character allows windowed mode in case of aalib running under X (otherwise, the DISPLAY environment variable will be unset for the current instance of fim).\n"
#if FIM_WANT_SDL_OPTIONS_STRING 
"The \\fBsdl\\fP option may be specified as  \\fBsdl" FIM_SYM_DEVOPTS_SEP_STR "{['w']['m']['r']['W']['M']['R']width:height}\\fP , where \\fBwidth\\fP is and \\fBheight\\fP are integer numbers specifying the desired resolution; the " FIM_MAN_fB("'w'") " character requests windowed mode; the " FIM_MAN_fB("'m'") " character requests mouse pointer display; the " FIM_MAN_fB("'r'") " character requests support for window resize; the same letters uppercase request explicit negation of the mentioned features.\n"
#endif /* FIM_WANT_SDL_OPTIONS_STRING */
#ifdef FIM_WITH_LIBIMLIB2
/* FIXME: shall document this */
#endif /* FIM_WITH_LIBIMLIB2 */
    },
    {"offset",      required_argument,       NULL,  0x6f66660a, "will open the first image file at the specified offset.","{bytes-offset[[:upper-offset]|+offset-range]}",
"Will use the specified \\fBoffset\\fP (in bytes) for opening the specified files. If \\fBupper-offset\\fP is specified, further bytes will be probed, until \\fBupper-offset\\fP. If \\fB+offset-range\\fP is specified, so many further bytes will be probed. This is useful for viewing images on damaged file systems; however, since the internal variables representation is sizeof(int) bytes based, you have a limited offset range."
    },
    {"text-reading",      no_argument,       NULL, 'P',"proceed scrolling as reading through a text document.",NULL,
"Enable textreading mode.  This has the effect that fim will display images scaled to the width of the screen, and aligned to the top.  Useful if the images you are watching text pages, all you have to do to get the next piece of text is to press space (in the default key configuration, of course)."
    },
    {"scroll",     required_argument, NULL, 's',"set scroll variable value.","{value}",
"Set scroll steps for internal variable " FIM_VID_STEPS " (default is " FIM_CNS_STEPS_DEFAULT ")."
    },
    {"slideshow",     required_argument, NULL, 0x7373,"interruptible slideshow mode.",FIM_CNS_EX_NUM_STRING,
"Interruptible slideshow mode; will wait for " FIM_CNS_EX_NUM_STRING " of seconds (assigned to the " FIM_VID_WANT_SLEEPS " variable after each loading; implemented by executing " FIM_CNS_SLIDESHOW_CMD " as a first command."
    },
    {"sanity-check",      no_argument,       NULL, 'S',"perform a sanity check.",NULL,
"A quick sanity check before starting the interactive fim execution, but after the initialization."
    },	/* NEW */
    {"no-framebuffer",      no_argument,       NULL, 't',"display images in text mode (as -o " FIM_DDN_INN_AA ").",NULL,
"Fim will not use the framebuffer but the aalib (ASCII art) driver instead (if you are curious, see (info aalib)).\n"
"If aalib was not enabled at tompile time, fim will work without displaying images at all."
    },
    {"vt",         required_argument, NULL, 'T',"specify a virtual terminal for the framebufer.","{terminal}",
"The \\fBterminal\\fP will be used as virtual terminal device file (as in fbi).\n"
"See (chvt (1)), (openvt (1)) for more info about this.\n"
"Use (con2fb (1)) to map a terminal to a framebuffer device.\n"
    },
    {"sort",     no_argument,       NULL, 0x736f7274 ,"sort images by pathname.",NULL,
"Sort files list before browsing according to full filename."
    },
    {"sort-basename",     no_argument,       NULL, 0x736f626e ,"sort images by basename.",NULL,
"Sort files list before browsing according to file basename's."
    },
    {"random",     no_argument,       NULL, 'u',"randomize images order.",NULL,
"Randomly shuffle the files list before browsing (seed depending on time() function)."
    },
    {"random-no-seed",     no_argument,       NULL, 0x7073,"randomize images order (always same sequence).",NULL,
"Randomly shuffle the files list before browsing (no seeding)."
    },
    {"verbose",    no_argument,       NULL, 'v',"verbose mode.",NULL,
"Be verbose: show status bar."
    },
    {"version",    no_argument,       NULL, 'V',"print program version.",NULL,
"Display program version, compile flags, enabled features, linked libraries information, supported filetypes/file loaders, and then terminate."
    },
    {"autowidth",   no_argument,       NULL, 'w',"scale according to width.",NULL,
"Scale the image according to the screen width."
    },
    {"no-auto-scale",   no_argument,   NULL,0x4E4053,"do not use any auto-scaling.",NULL,
"Do not scale the images after loading (will set '" FIM_VID_SCALE_STYLE "=\" \"';)."
    },
    {"autowindow",   no_argument,   NULL,0x61757769,"adapt window to image size.",NULL,
"Will resize the window size (if supported) to the image size. Don't use this with other image scaling options."
    },
    {"no-stat-push",   no_argument,   NULL,0x6e7363,"do not check file/dir existence with stat(2) at push time",NULL,
"Sets " FIM_VID_PRELOAD_CHECKS "=0 before initialization, thus disabling file/dir existence checks with stat(2) at push push time (and speeding up startup)."
    },
    {"autoheight",   no_argument,       NULL, 'H',"scale according to height.",NULL,
"Scale the image according to the screen height."
    },
    {FIM_OSW_DUMP_SCRIPTOUT,      required_argument,       NULL, 'W',"will record any executed command to the a {scriptfile}.","{scriptfile}",
"All the characters that you type are recorded in the file {scriptout}, until you exit Fim.  This is  useful  if  you want to create a script file to be used with \"fim -c\" or \":exec\" (analogous to Vim's -s and \":source!\").  If the {scriptout} file exists, it will be not touched (as in Vim's -w). "
    },
#ifdef FIM_READ_STDIN
    {"read-from-stdin",      no_argument,       NULL, '-',"read an image list from standard input.",NULL,
"Read file list from stdin: each line one file.\n"

"\n"
"Note that these the three standard input reading functionalities (-i,-p and -) conflict : if two or more of them occur in fim invocation, fim will exit with an error and warn about the ambiguity.\n"
"\n"
"See the section\n"
".B EXAMPLES\n"
"below to read some useful (and unique) ways of employing fim.\n"
    },
    {"read-from-stdin-elds",      required_argument,       NULL, 0x72667373,"--read-from-stdin filenames endline delimiter string.",NULL,
"Specify an endline delimiter string for breaking lines read via -/--read-from-stdin. Line text before the delimiter will be treated as names of files to load; the text after will be ignored until a newline. This is useful e.g. to description files as filename list files.\n"
    },
#endif /* FIM_READ_STDIN */
    {"autotop",   no_argument,       NULL, 'A',"align images to the top (UNFINISHED).",NULL,
	    NULL
    },
//    {"gamma",      required_argument, NULL, 'g',"set gamma (UNFINISHED)","{gamma}",
//" gamma correction.  Can also be put into the FBGAMMA environment variable.  Default is 1.0.  Requires Pseudocolor or Directcolor visual, doesn't work for Truecolor."
//    },
    {"quiet",      no_argument,       NULL, 'q',"quiet mode (UNFINISHED).",NULL,
	    NULL
    },
    {"resolution", required_argument, NULL, 'r',"set resolution (UNFINISHED).","{resolution}",
	    NULL
    },
    {"recursive", no_argument, NULL, 'R',"Push files/directories to the files list recursively.", NULL,
	    NULL
    },
/*    {"timeout",    required_argument, NULL, 't',"",NULL},*/  /* timeout value */	/* fbi's */
/*    {"once",       no_argument,       NULL, '1',"",NULL},*/  /* loop only once */
/*    {"font",       required_argument, NULL, 'f',"",NULL},*/  /* font */
/*    {"edit",       no_argument,       NULL, 'e',"",NULL},*/  /* enable editing */	/* fbi's */
/*    {"list",       required_argument, NULL, 'l',"",NULL},*/
//    {"backup",     no_argument,       NULL, 'b',"",NULL},	/* fbi's */
//    {"debug",      no_argument,       NULL, 'D',"",NULL},
//    {"preserve",   no_argument,       NULL, 'p',"",NULL},	/* fbi's */

    /* long-only options */
//    {"autoup",     no_argument,       &autoup,   1 },
//    {"autodown",   no_argument,       &autodown, 1 },
//    {"comments",   no_argument,       &comments, 1 },
    {0,0,0,0,0,0}
};

#if 0
// leftovers from the old man file; shall adapt these using .\"
 .TP
 .B -f font
 Set font.  This can be either a pcf console font file or a X11 font
 spec.  Using X11 fonts requires a font server (The one specified in
 the environment variable FONTSERVER or on localhost).  The FBFONT
 environment variable is used as default.  If unset, fim will
 fallback to 10x20 (X11) / lat1u-16.psf (console).
 .TP
 .B --autoup
 Like autozoom, but scale up only.
 .TP
 .B --autodown
 Like autozoom, but scale down only.
 .TP
 .B -u
 Randomize the order of the filenames.
 .TP
 .B -e
 Enable editing commands.
 .TP
 .B -b
 create backup files (when editing images).
 .TP
 .B -p
 preserve timestamps (when editing images).
 .TP
 .B --comments
 Display comment tags (if present) instead of the filename.  Probaby
 only useful if you added reasonable comments yourself (using wrjpgcom
 for example), otherwise you likely just find texts pointing to the
 software which created the image.
 P               pause the slideshow (if started with -t, toggle)
 {number}g    jump to image {number}
 .SH EDIT IMAGE
 fim also provides some very basic image editing facilities.  You have
 to start fim with the -e switch to use them.
 .P
 .nf
 Shift+D         delete image
 R               rotate 90° clockwise
 L               rotate 90° counter-clock wise
 .fi
 .P
 The delete function actually wants a capital letter 'D', thus you have
 to type Shift+D.  This is done to avoid deleting images by mistake
 because there are no safety bells:  If you ask fim to delete the image,
 it will be deleted without questions asked.
 .P
 The rotate function actually works for JPEG images only because it
 calls the jpegtran command to perform a lossless rotation if the image.
 It is especially useful if you review the images of your digital
 camera.
#endif

const int fim_options_count=sizeof(fim_options)/sizeof(fim_options_t);
struct option options[fim_options_count];

fim::string fim_help_opt(const char*qs)
{
	string result;

	if( qs && qs[0] == '-' && !qs[1] )
	{
		result += "The short command options of fim are: ";
		for(size_t i=0;i<fim_options_count-1;++i)
		if( isascii( fim_options[i].val ) )
		{
			if( fim_options[i].val != '-' )
				result += "-";
			result += fim_options[i].val;
			result += " ";
		}
		goto ret;
	}

	if( qs && qs[0] == '-' && qs[1] == '-' && !qs[2] )
	{
		result += "The long command options of fim are: ";
		for(size_t i=0;i<fim_options_count-1;++i)
		if( fim_options[i].name ) 
			result += "--",
			result += fim_options[i].name,
			result += " ";
		goto ret;
	}

	if( !qs || qs[0] != '-' || !qs[1] || (qs[1]!='-' && qs[2]) || (qs[1]=='-' && !qs[2])  )
	{
		goto ret;
	}

	for(size_t i=0;i<fim_options_count-1;++i)
		if(
				( ( (int) qs[1] ) == fim_options[i].val ) ||
				( qs[1] == '-' && 0 == strcmp(qs+2,fim_options[i].name) )
		  )
		{
			result += "A fim command option: ";
			if( isascii( fim_options[i].val ) )
			{
				result += "-", result += fim_options[i].val;
				if( fim_options[i].optdesc )
			       		result += " =", result += fim_options[i].optdesc;
				result += ", ";
			}
			if( ( fim_options[i].name ) )
			{
				result += "--", result += fim_options[i].name;
				if( fim_options[i].optdesc )
			       		result += " =", result += fim_options[i].optdesc;
				result += " ";
			}
			result += ": ";
			result += fim_options[i].desc;
			// result += fim_options[i].mandesc; // man/groff markup should be cleaned up before printing
			goto ret;
		}
		//result += fim_options[i].val << "\n";
ret:
	return result;
}

class FimInstance
{
	static void show_version();

string fim_dump_man_page_snippets(void)
{
	string ms;
	const fim_char_t *helparg="m";
	const fim_char_t *slob;
	const fim_char_t *sloe;
	const fim_char_t *slol;
	const fim_char_t *slom;
	ms+=
".TP\n"
".B --\n"
"The arguments before\n"
".B --\n"
"beginning with \n"
".B -\n"
"will be treated as command line options.\n"
"All arguments after\n"
".B --\n"
"will be treated as filenames regardlessly.\n"
".\n"
;
	if(*helparg=='m')
	{
		slol=".TP\n.B ";
		slob=" --";
		sloe="\n";
		slom="\n";
	}
	else
	{
		slol="\t-";
		slob="\t\t--";
		slom=FIM_CNS_EMPTY_STRING;
		sloe="\n";
	}
	for(size_t i=0;i<fim_options_count-1;++i)
	{	
		if(isascii(fim_options[i].val))
		{
	   		if((fim_options[i].val)!='-')
			{
				ms+=slol,ms+="-",ms+=string((fim_char_t)(fim_options[i].val));
				if(fim_options[i].has_arg==required_argument)
				{
					if(fim_options[i].optdesc)
					       	ms+=" ",ms+=fim_options[i].optdesc;
				}
				ms+=",";
			}
	 	  	else
			       	ms+=slol,ms+=" -, ";
		}
		else
		       	//ms+=".TP\n.B \t";
		       	ms+=".TP\n.B ";
		ms+=slob;
		ms+=fim_options[i].name ;
		switch(fim_options[i].has_arg)
		{
			case no_argument:
			break;
			case required_argument:
			//std::cout << " <arg>";
			if(fim_options[i].optdesc)
			       	ms+=" ",ms+=fim_options[i].optdesc;
			else
			       	ms+=" <arg>";
			break;
			case optional_argument:
			if(fim_options[i].optdesc)
			       	ms+=fim_options[i].optdesc;
			else
			       	ms+="[=arg]";
			break;
			default:
			;
		};
		ms+=slom;
		if(helparg&&*helparg=='d')
			ms+="\t\t ",ms+=fim_options[i].desc;
		if(helparg&&*helparg=='m')
		{
			if(fim_options[i].mandesc)
				ms+=fim_options[i].mandesc;
			else
			{
				ms+="\t\t ";
				if(fim_options[i].desc)
					ms+=fim_options[i].desc;
			}
		}
		//if(helparg||*helparg!='m') ms+=FIM_SYM_ENDL;
		ms+=sloe;
		//if(helparg&&*helparg=='l') std::cout << "TODO: print extended help here\n";
	}
	ms+="\n";
	return ms;
}

int fim_dump_man_page(void)
{
	string mp=
			string(".\\\"\n"
			".\\\" $Id""$\n"
			".\\\"\n"
			".TH fim 1 \"(c) 2007-" FIM_CNS_LCY " " FIM_AUTHOR_NAME "\"\n"
			".SH NAME\n"
			"fim - \\fBf\\fPbi (linux \\fBf\\fPrame\\fBb\\fPuffer \\fBi\\fPmageviewer) \\fBim\\fPproved\n"
			".SH SYNOPSIS\n"
			".B fim [{options}] [--] {imagefile} [{imagefiles}]\n.fi\n"
			".B ... | fim [{options}] [--] [{imagefiles}] -\n.fi\n")+
#ifdef FIM_READ_STDIN
			string(".B fim [{options}] [--] [{files}] - < {file_name_list_text_file}\n.fi\n")+
#endif /* FIM_READ_STDIN */
#ifdef FIM_READ_STDIN_IMAGE
			string(".B fim --" FIM_OSW_IMAGE_FROM_STDIN " [{options}] < {imagefile}\n.fi\n")+
#endif /* FIM_READ_STDIN_IMAGE */
#ifdef FIM_READ_STDIN
			string(".B fim --" FIM_OSW_SCRIPT_FROM_STDIN " [{options}] < {scriptfile}\n.fi\n")+
#endif /* FIM_READ_STDIN */
			string("\n"
			".SH DESCRIPTION\n"
			".B\nfim\nis a `swiss army knife' for displaying image files.\n"
			"It is capable of displaying image files using different graphical devices while offering a uniform look and feel; it features an internal command language specialized to the image viewing purposes; it is capable of interacting with standard input and output; the internal command language is accessible via a command line capable of autocompletion and history; it features command recording, supports initialization files, customizable key bindings, internal variables and command aliases, vim-like autocommands, JPEG comments and EXIF tags display, and much more.\n\n"
			"As a default,\n.B\nfim\ndisplays the specified file(s) on the detected graphical device (e.g. with SDL if X is detected, or the linux framebuffer device if not).  " FIM_CNS_DSFF " formats are supported. \nFor 'XCF' (Gimp's) images, fim will try to use '" FIM_EPR_XCFTOPNM "'.\nFor '.FIG' vectorial images, fim will try to use '" FIM_EPR_FIG2DEV "'.\nFor '.DIA' vectorial images, fim will try to use '" FIM_EPR_DIA "'.\nFor '.SVG' vectorial images, fim will try to use '" FIM_EPR_INKSCAPE "'.\nFor other formats fim will try to use ImageMagick's '" FIM_EPR_CONVERT "' executable.\n"
			"\n")+
#ifdef FIM_READ_DIRS
			string("\n""If " FIM_MAN_fB("{imagefile}") " is a directory, therein contained files of supported formats will be loaded. If " FIM_MAN_fB("{imagefile}") " contains a trailing slash (" FIM_CNS_SLASH_STRING "), it will be treated as a directory; otherwise a check will be made using " FIM_MAN_fB("stat(2)") ". To change this default, see description of the " FIM_VID_PUSHDIR_RE " variable and the --no-stat-push and --recursive  options.\n\n")+
#endif /* FIM_READ_DIRS */

			string("\n""If configured at build time, fim will be capable of using SDL or aalib output.\n\n")+
	//		string("Please note that a user guide of \n.B fim\nis in the " FIM_CNS_FIM_TXT " file distributed in the source package.\n\n")+
			string("This man page only describes the\n.B fim\ncommand line options.\n"
			"See man " FIM_MAN_fR("fimrc") "(5) for a full specification of the \n.B\nfim\nlanguage, commands, variables, and an example configuration file.\n"
			"\n"
			".SH USAGE\n"
			"You may invoke\n.B\nfim\nfrom an interactive shell and control it with the keyboard, as you would do with any image viewer with reasonable key bindings.\n"
			"\n.B\nfim\nis keyboard oriented: there are no user menus or buttons available.\n"
			"If you need some feature or setting which is not accessible from the default keyboard configuration, you probably need a custom configuration or simply need to type a custom command. For these, you can use the internal command and configuration language.\n"
			"The full specification for these is accessible at runtime using the internal help system (typing :help).\n"
			"\n"
			"\n.SH OPTIONS\n"
			"Accepted command line \n.B\n{options}\n:\n");
			mp+=fim_dump_man_page_snippets();
			mp+=string(".SH PROGRAM RETURN STATUS\n"
		     	"The program return status is ")+string(FIM_ERR_NO_ERROR)+string(" on correct operation; ");
			mp+=string(FIM_PERR_UNSUPPORTED_DEVICE)+string(" on unsupported device specification; ");
			mp+=string(FIM_PERR_BAD_PARAMS)+string(" on bad input; ");
			mp+=string(FIM_PERR_GENERIC)+string(" on a generic error; ");
			mp+=string(FIM_PERR_OOPS)+string(" on a signal-triggered program exit; ");
			mp+=string(" or a different value in case of an another error.\n"
			" The return status may be controlled by the use of the " FIM_FLT_QUIT " command.\n"
			".SH COMMON KEYS AND COMMANDS\n"
".nf\n"
"The following keys and commands are hardcoded in the minimal configuration. These are working by default before any config loading, and before the hardcoded config loading (see variable " FIM_VID_FIM_DEFAULT_CONFIG_FILE_CONTENTS ").\n\n"
//"cursor keys     scroll large images\n"
//"h,j,k,l		scroll large images left,down,up,right\n"
//"+, -            zoom in/out\n"
//"ESC, q          quit\n"
//"Tab             toggle output console visualization\n"
//"PgUp,p            previous image\n"
//"PgDn,n            next image\n"
//"Space  	        next image if on bottom, scroll down instead\n"
//"Return          next image, write the filename of the current image to stdout on exit from the program.\n"
);

#define FIM_ADD_DOCLINE_FOR_CMD(REP,CMD) if(cc.find_key_for_bound_cmd(CMD)!=""){if(REP!=1)mp+=FIM_XSTRINGIFY(REP);else mp+=" ";mp+=cc.find_key_for_bound_cmd(CMD);mp+="    ";if(REP!=1)mp+=FIM_XSTRINGIFY(REP);mp+=CMD;mp+="\n";}
			FIM_ADD_DOCLINE_FOR_CMD(1,FIM_FLC_NEXT);
			FIM_ADD_DOCLINE_FOR_CMD(1,FIM_FLC_PREV);
			FIM_ADD_DOCLINE_FOR_CMD(1,FIM_FLC_NEXT_FILE);
			FIM_ADD_DOCLINE_FOR_CMD(1,FIM_FLC_PREV_FILE);
			FIM_ADD_DOCLINE_FOR_CMD(1,FIM_FLC_NEXT_PAGE);
			FIM_ADD_DOCLINE_FOR_CMD(1,FIM_FLC_PREV_PAGE);
			/* TODO: may use a search-based method for locating an keys to other commands... */
			FIM_ADD_DOCLINE_FOR_CMD(1,FIM_FLA_MAGNIFY);
			FIM_ADD_DOCLINE_FOR_CMD(1,FIM_FLA_REDUCE);
			//FIM_ADD_DOCLINE_FOR_CMD(1,FIM_FLC_MIRROR);
			//FIM_ADD_DOCLINE_FOR_CMD(1,FIM_FLC_FLIP);
			FIM_ADD_DOCLINE_FOR_CMD(1,FIM_FLC_PAN_LEFT);
			FIM_ADD_DOCLINE_FOR_CMD(1,FIM_FLC_PAN_RIGHT);
			FIM_ADD_DOCLINE_FOR_CMD(1,FIM_FLC_PAN_UP);
			FIM_ADD_DOCLINE_FOR_CMD(1,FIM_FLC_PAN_DOWN);
			FIM_ADD_DOCLINE_FOR_CMD(1,FIM_FLT_ROTATE);
			FIM_ADD_DOCLINE_FOR_CMD(1,FIM_FLT_LIST);
			FIM_ADD_DOCLINE_FOR_CMD(1,FIM_FLT_SCROLLDOWN);
			FIM_ADD_DOCLINE_FOR_CMD(1,FIM_FLT_QUIT);
			mp+="You can type a number before a command binding to iterate the assigned command:\n";
			FIM_ADD_DOCLINE_FOR_CMD(3,FIM_FLC_PAN_UP);
			mp+=string(
//"d,x,D,X		diagonal scroll\n"
//"C-w			scale to the screen width\n"
//"H			scale to the screen heigth\n"
"\n"
FIM_INTERNAL_LANGUAGE_SHORTCUT_SHORT_HELP
"\n"
//"C-n		 after entering in search mode (/) and submitting a pattern, C-n (pressing the Control and the n key together) will jump to the next matching filename\n"
//"C-c		 terminate instantaneously fim\n"
//"T		 split horizontally the current window\n"
//"V		 split vertically the current window\n"
//"C		 close  the currently focused window\n"
//"H		 change the currently focused window with the one on the left\n"
//"J		 change the currently focused window with the lower\n"
//"K		 change the currently focused window with the upper\n"
//"L		 change the currently focused window with the one on the right\n"
//"U		 swap the currently focused window with the split sibling one (it is not my intention to be obscure, but precise  : try V, m,  U and see by yourself :) )\n"
//"d		move the image diagonally north-west\n"
//"D		move the image diagonally south-east\n"
//"x		move the image diagonally north-east\n"
//"X		move the image diagonally south-west\n"
//"m		mirror\n"
//"f		flip\n"
//"r		rotate\n"
"\n"
"You can visualize all of the default bindings invoking fim --dump-default-fimrc | grep bind .\n"
"You can visualize all of the default aliases invoking fim  --dump-default-fimrc | grep alias .\n"
"\n"
".fi\n"
".P\n"
"The Return vs. Space key thing can be used to create a file list while\n"
"reviewing the images and use the list for batch processing later on.\n"
"\n"
"All of the key bindings are reconfigurable; see the default \n"
".B fimrc\n"
"file for examples on this, or read the complete manual: the FIM.TXT file\n"
"distributed with fim.\n"
					)+
			string(
".SH AFFECTING ENVIRONMENT VARIABLES\n"
".nf\n"
//"" FIM_ENV_FBFONT "		(just like in fbi) a consolefont or a X11 (X Font Server - xfs) font file.\n"
"" FIM_ENV_FBFONT "		(just like in fbi) a Linux consolefont font file.\n"
"If using a gzipped font, the " FIM_EPR_ZCAT " program will be used to uncompress it (via " FIM_MAN_fB("execvp(3)") ").\n"
"If not specified, the following files will be probed and the first existing will be selected:\n\n");
mp+=get_default_font_list();
#if FIM_WANT_HARDCODED_FONT
mp+="\nIf the special " FIM_DEFAULT_HARDCODEDFONT_STRING " string is specified, a hardcoded font will be used.";
#endif /* FIM_WANT_HARDCODED_FONT */
mp+="\n";
mp+=string(
//"			For instance,  /usr/share/consolefonts/LatArCyrHeb-08.psf.gz is a Linux console file.\n"
//"			Consult 'man setfont' for your current font paths.\n"
//"			NOTE : Currently xfs is disabled.\n"
"" FIM_ENV_FBGAMMA "		(just like in fbi) gamma correction (applies to dithered 8 bit mode only). Default is " FIM_CNS_GAMMA_DEFAULT_STR ".\n"
"" FIM_ENV_FRAMEBUFFER "	(just like in fbi) user set framebuffer device file (applies only to the " FIM_DDN_INN_FB " mode).\n"
"If unset, fim will probe for " FIM_DEFAULT_FB_FILE ".\n"
"" FIM_CNS_TERM_VAR "		(only in fim) will influence the output device selection algorithm, especially if $" FIM_CNS_TERM_VAR "==\"screen\".\n"
#if defined(FIM_WITH_LIBSDL)
"" FIM_ENV_DISPLAY "	If this variable is set, then the " FIM_DDN_INN_SDL " driver will be probed by default.\n"
#elif defined(FIM_WITH_LIBIMLIB2)
"" FIM_ENV_DISPLAY "	If this variable is set, then the " FIM_DDN_INN_IL2 " driver will be probed by default.\n"
#endif /* FIM_WITH_LIBSDL */
".SH COMMON PROBLEMS\n"
".B fim\n"
"needs read-write access to the framebuffer devices (/dev/fbN or /dev/fb/N), i.e you (our\n"
"your admin) have to make sure fim can open the devices in rw mode.\n"
"The IMHO most elegant way is to use pam_console (see\n"
"/etc/security/console.perms) to chown the devices to the user logged\n"
"in on the console.  Another way is to create some group, chown the\n"
"special files to that group and put the users which are allowed to use\n"
"the framebuffer device into the group.  You can also make the special\n"
"files world writable, but be aware of the security implications this\n"
"has.  On a private box it might be fine to handle it this way\n"
"through.\n"
"\n"
"If using udev, you can edit :\n"
"/etc/udev/permissions.d/50-udev.permissions\n"
"and set these lines like here :\n"
" # fb devices\n"
" fb:root:root:0600\n"
" fb[0-9]*:root:root:0600\n"
" fb/*:root:root:0600\n"
".P\n"
"\n"
".B fim\n"
"also needs access to the linux console (i.e. /dev/ttyN) for sane\n"
"console switch handling.  That is obviously no problem for console\n"
"logins, but any kind of a pseudo tty (xterm, ssh, screen, ...) will\n"
".B not\n"
"work.\n"
".SH EXAMPLES\n"
".B fim media/ \n"
".fi \n"
"# Will load files from the directory media.\n"
".P\n"
".P\n"
"\n"
".B fim -R media/ --sort \n"
".fi \n"
"# Will open files found by recursive traversal of directory media, then sorting the list.\n"
".P\n"
".P\n"
"\n"
".B\n"
".B find /mnt/media/ -name *.jpg | fim - \n"
".fi \n"
"# Will make fim read the file list from standard input.\n"
".P\n"
".P\n"
"\n"
".B\n"
"find /mnt/media/ -name *.jpg | shuf | fim -\n"
".fi\n"
"# will make fim read the file list from standard input, randomly shuffled.\n"
".P\n"
".P\n"
"\n"
".B\n"
"cat script.fim | fim -p images/*\n"
".fi\n"
"# Will make fim read the script file\n"
".B script.fim\n"
"from standard input prior to displaying files in the directory\n"
".B images\n"
".P\n"
".P\n"
#ifdef FIM_READ_STDIN_IMAGE
"\n"
".B \n"
"scanimage ... | tee scan.ppm | fim -i\n"
".fi\n"
"# Will make fim read the image scanned from a flatbed scanner as soon as it is read \n"
".P\n"
".P\n"
#endif /* FIM_READ_STDIN_IMAGE */
"\n"
".B fim * > selection.txt\n"
".fi\n"
"# Will output the file names marked interactively with the '" FIM_FLT_LIST " \"mark\"' command in fim to a file.\n"
".P\n"
".P\n"
"\n"
".B fim * | fim -\n"
".fi\n"
"# will output the file names marked with 'm' in fim to a second instance of fim, in which these could be marked again.\n"
".P\n"
".P\n"
"\n"
".B fim\n-c 'pread \"vgrabbj -d /dev/video0 -o png\";reload'\n"
".fi\n"
"# will display an image grabbed from a webcam.\n"
".P\n"
".P\n"
"\n"
".B fim\n-o " FIM_DDN_INN_AA " -c 'pread \"vgrabbj -d /dev/video0 -o png\";reload;system \"fbgrab\" \"asciime.png\"'\n"
".fi\n"
"# if running in framebuffer mode, will save a png screenshot with an ASCII rendering of an image grabbed from a webcam.\n"
".P\n"
".P\n"
"\n"
".B fim\n"
"-c 'while(1){pread \"vgrabbj -d /dev/video0 -o png\";reload;sleep 1;};'\n"
".fi\n"
"# will display a sequence of images grabbed from a webcam; circa 1 per second.\n"
".P\n"
".P\n"
"\n"
".SH NOTES\n"
"This manual page is neither accurate nor complete. In particular, issues related to driver selection shall be described more accurately. Also the accurate sequence of autocommands execution, variables application is critical to understanding fim, and should be documented.\n"
#ifdef FIM_READ_STDIN_IMAGE
"The filename \"" FIM_STDIN_IMAGE_NAME "\" is reserved for images read from standard input (view this as a limitation), and thus handling files with such name may incur in limitations.\n"
#endif /* FIM_READ_STDIN_IMAGE */
#ifdef FIM_WITH_LIBSDL
"The SDL driver is quite inefficient, for a variety of reasons. In particular, its interaction with the readline library can be problematic (e.g.: when running in sdl mode without a terminal). This shall be fixed.\n"
#endif /* FIM_WITH_LIBSDL */
".SH BUGS\n"
".B fim\n"
"has bugs. Please read the \n"
".B BUGS\n"
"file shipped in the documentation directory to discover the known ones.\n"
".SH  FILES\n"
"\n"
".TP 15\n"
".B " FIM_CNS_DOC_PATH "\n"
"The directory with \n"
".B Fim\n"
"documentation files.\n"
".TP 15\n"
".B " FIM_CNS_SYS_RC_FILEPATH "\n"
"The system wide\n"
".B Fim\n"
"initialization file (executed at startup, after executing the hardcoded configuration).\n"

".TP 15\n"
".B " FIM_CNS_USR_RC_COMPLETE_FILEPATH "\n"
"The personal\n"
".B Fim\n"
"initialization file (executed at startup, after the system wide initialization file).\n"

".TP 15\n"
".B ~/.inputrc\n"
"If\n.B Fim\n"
"is built with GNU readline support, it will be susceptible to chages in the user set ~/.inputrc configuration file contents.  For details, see"
" (man " FIM_MAN_fR("readline") "(3))."
"\n"
			      )+
string(
".SH SEE ALSO\n"
"Other \n"
".B Fim \n"
"man pages: " FIM_MAN_fR("fimgs") "(1), " FIM_MAN_fR("fimrc") "(1).\n"
".fi\n"
"Or related programs: " FIM_MAN_fR("fbset") "(1), " FIM_MAN_fR("con2fb") "(1), " FIM_MAN_fR("convert") "(1), " FIM_MAN_fR("vim") "(1), " FIM_MAN_fR("fb.modes") "(8), " FIM_MAN_fR("fbset") "(8), " FIM_MAN_fR("fbgrab") "(1), " FIM_MAN_fR("fbdev") "(4), " FIM_MAN_fR("setfont") "(8), " FIM_MAN_fR("xfs") "(1).\n"
".SH AUTHOR\n"
".nf\n"
FIM_AUTHOR" is the author of fim, \"fbi improved\". \n"
".fi\n"
FBI_AUTHOR" is the author of \"fbi\", upon which\n.B fim\nwas originally based. \n"
".SH COPYRIGHT\n"
".nf\n"
"Copyright (C) 2007-" FIM_CNS_LCY " " FIM_AUTHOR "\n"
".fi\n"
"Copyright (C) 1999-2004 " FBI_AUTHOR "\n"
".P\n"
"This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.\n"
".P\n"
"This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.\n"
".P\n"
"You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.\n"
)+
			string("\n");
	std::cout << mp;
	return 0;
}

fim_perr_t help_and_exit(const fim_char_t *argv0, fim_perr_t code=FIM_PERR_NO_ERROR, const fim_char_t*helparg=NULL)
{
	if(helparg&&*helparg=='b')
	{
	    	std::cout << "fim - No loadable images specified.\nUse `fim --help' for detailed usage information.\n";
		goto done;
	}
	if(helparg&&*helparg=='m')
	{
		fim_dump_man_page(); 
		goto done;
	}
	    cc.printHelpMessage(argv0);
	    std::cout << " where OPTIONS are taken from :\n";
	    if(helparg&&*helparg=='l')
		    std::cout << "(EXPERIMENTAL: long help ('l') printout still unsupported)\n";
	    for(size_t i=0;i<fim_options_count-1;++i)
	    {	
		if(isascii(fim_options[i].val)){
	   	if((fim_options[i].val)!='-')
			std::cout << "\t-"<<(fim_char_t)(fim_options[i].val) ;
	   	else std::cout << "\t-";}else std::cout<<"\t";
		std::cout << "\t\t";
	    	std::cout << "--"<<fim_options[i].name ;
		switch(fim_options[i].has_arg){
		case no_argument:
		break;
		case required_argument:
		//std::cout << " <arg>";
		if(fim_options[i].optdesc)
		       	std::cout << " =" << fim_options[i].optdesc; else std::cout << " =<arg>";
		break;
		case optional_argument:
		if(fim_options[i].optdesc)
		       	std::cout << " " << fim_options[i].optdesc; else std::cout << "[=arg]";
		break;
		default:
		;
		};
		if(helparg&&*helparg=='d')
			std::cout << "\t\t " << fim_options[i].desc;
		std::cout << FIM_SYM_ENDL;
		//if(helparg&&*helparg=='l') std::cout << "TODO: print extended help here\n";
		}
		std::cout << "\n Please read the documentation distributed with the program.\n"
			  << " For further help, consult the online help in fim (:" FIM_FLT_HELP "), and man fim (1), fimrc (5).\n"
			  << " For bug reporting read the " FIM_CNS_BUGS_FILE " file.\n";
done:
	    std::exit(code);
	    return code;
}


	public:
	fim_perr_t main(int argc,char *argv[])
	{
		fim_perr_t retcode=FIM_PERR_NO_ERROR;
		/*
		 * an adapted version of the main function
		 * of the original version of the fbi program
		 */
		int              opt_index = 0;
		int              i;
		int		 want_random_shuffle=0;
	#ifdef FIM_READ_STDIN
		enum rsc { Nothing = 0, ImageFile = 1, FilesList = 2, Script = 3/*, Desc = 4*/ };
		int              read_stdin_choice = Nothing;
		int perform_sanity_check=0;
	#endif /* FIM_READ_STDIN */
		int c;
		int ndd=0;/*  on some systems, we get 'int dup(int)', declared with attribute warn_unused_result */
		bool appendedPostInitCommand=false;
		bool appendedPreConfigCommand=false;
		const char * sa = NULL;
		fim_flags_t pf = FIM_FLAG_DEFAULT; /* push flags */
#if FIM_WANT_PIC_CMTS
		fim_char_t sc = '\t'; /* separation character for --load-image-descriptions-file */
#endif /* FIM_WANT_PIC_CMTS */

	    	g_fim_output_device=FIM_CNS_EMPTY_STRING;
	
		setlocale(LC_ALL,"");	//uhm..

		{
			int foi;

			for(foi=0;foi<fim_options_count;++foi)
			{
				options[foi].name=fim_options[foi].name;
				options[foi].has_arg=fim_options[foi].has_arg;
				options[foi].flag=fim_options[foi].flag;
				options[foi].val=fim_options[foi].val;
			}
		}

	    	for (;;) {
		    /*c = getopt_long(argc, argv, "wc:u1evahPqVbpr:t:m:d:g:s:f:l:T:E:DNhF:",*/
		    c = getopt_long(argc, argv, "C:HAb?wc:uvahPqVr:m:d:g:s:T:E:f:DNhF:tfipW:o:SR",
				options, &opt_index);
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
		    //cc.setVariable(FIM_VID_AUTOTOP,(fim_int)1);
		    //TODO: still needs some tricking .. 
	#ifdef FIM_AUTOCMDS
		    cc.pre_autocmd_add(FIM_VID_SCALE_STYLE"='a';");
	#else /* FIM_AUTOCMDS */
		    cout << FIM_EMSG_NO_SCRIPTING;
	#endif /* FIM_AUTOCMDS */
		    break;
#if FIM_WANT_TEXT_RENDERING
		case 0x74657874:
		    	cc.setVariable(FIM_VID_TEXT_DISPLAY,(fim_int)1);
#else /* FIM_WANT_TEXT_RENDERING */
			std::cerr<<"Warning: the --" FIM_OSW_TEXT " option was disabled at compile time.\n";
#endif /* FIM_WANT_TEXT_RENDERING */

#if FIM_WANT_RAW_BITS_RENDERING
		case 'b':
		    //fim's
		    if(optarg && strstr(optarg,"1")==optarg && !optarg[1])
			{
		    	cc.setVariable(FIM_VID_BINARY_DISPLAY,(fim_int)1);
			}
		    else
		    if(optarg && strstr(optarg,"24")==optarg && !optarg[2])
			{
		    	cc.setVariable(FIM_VID_BINARY_DISPLAY,(fim_int)24);
			}
                    else
		    {
			if(optarg)
				std::cerr<<"Warning : the --" FIM_OSW_BINARY " option supports 1 or 24 bpp depths. Using "<<FIM_DEFAULT_AS_BINARY_BPP<<".\n";
		    	cc.setVariable(FIM_VID_BINARY_DISPLAY,(fim_int)FIM_DEFAULT_AS_BINARY_BPP);
                    }
		    break;
#else /* FIM_WANT_RAW_BITS_RENDERING */
			std::cerr<<"Warning: the --" FIM_OSW_BINARY " option was disabled at compile time.\n";
#endif /* FIM_WANT_RAW_BITS_RENDERING */
		case 'A':
		    //fbi's
		    //cc.setVariable(FIM_VID_AUTOTOP,(fim_int)1);
		    //FIXME: still needs some tricking .. 
	#ifdef FIM_AUTOCMDS
		    cc.pre_autocmd_add(FIM_VID_AUTOTOP "=1;");
	#endif /* FIM_AUTOCMDS */
		    break;
		case 'q':
		    //fbi's
		    //FIM_FPRINTF(stderr, "sorry, this feature will be implemented soon\n");
		    //cc.setVariable(FIM_VID_DISPLAY_STATUS,(fim_int)0);
	#ifdef FIM_AUTOCMDS
		    cc.pre_autocmd_add(FIM_VID_DISPLAY_STATUS"=0;");
	#endif /* FIM_AUTOCMDS */
		    break;
		case 'f':
	#ifndef FIM_WANT_NOSCRIPTING
		    cc.setVariable(FIM_VID_DEFAULT_ETC_FIMRC,optarg);
	#else /* FIM_WANT_NOSCRIPTING */
		    cout << FIM_EMSG_NO_SCRIPTING;
	#endif /* FIM_WANT_NOSCRIPTING */
		    break;
		case 0x7373:
	#ifndef FIM_WANT_NOSCRIPTING
		    	cc.setVariable(FIM_VID_WANT_SLEEPS,optarg);
	    		cc.autocmd_add(FIM_ACM_PREEXECUTIONCYCLE,"",FIM_CNS_SLIDESHOW_CMD);
	#else /* FIM_WANT_NOSCRIPTING */
		    cout << FIM_EMSG_NO_SCRIPTING;
	#endif /* FIM_WANT_NOSCRIPTING */
		    break;
		case 'S':
		    //fim's
	#ifdef FIM_AUTOCMDS
		    cc.setVariable(FIM_VID_SANITY_CHECK,(fim_int)1);
		    perform_sanity_check=1;
	#endif /* FIM_AUTOCMDS */
		    break;
		case 'v':
		    //fbi's
		    //cc.setVariable(FIM_VID_DISPLAY_STATUS,(fim_int)1);
	#ifdef FIM_AUTOCMDS
		    cc.pre_autocmd_add(FIM_VID_DISPLAY_STATUS"=1;");
	#endif /* FIM_AUTOCMDS */
		    break;
		case 'w':
		    //fbi's
	#ifdef FIM_AUTOCMDS
		    cc.pre_autocmd_add(FIM_VID_SCALE_STYLE"='w';");
	#endif /* FIM_AUTOCMDS */
		    break;
		case 0x6e7363:
		    //fim's
		    cc.setVariable(FIM_VID_PRELOAD_CHECKS,(fim_int)0);
		    break;
		case 0x4E4053:
		    //fbi's
	#ifdef FIM_AUTOCMDS
		    cc.pre_autocmd_add(FIM_VID_SCALE_STYLE"=' ';");// FIXME: shall document the allowed scaling character/options
	#endif /* FIM_AUTOCMDS */
		    break;
		case 'H':
		    //fbi's
	#ifdef FIM_AUTOCMDS
		    cc.pre_autocmd_add(FIM_VID_SCALE_STYLE"='h';");
	#endif /* FIM_AUTOCMDS */
		    break;
		case 'P':
		    //fbi's
	#ifdef FIM_AUTOCMDS
		    cc.pre_autocmd_add(FIM_VID_SCALE_STYLE"='w';" FIM_VID_AUTOTOP "=1;");
	#endif /* FIM_AUTOCMDS */
		    break;
		case 0x61757769:
		    //fim's
	#ifdef FIM_AUTOCMDS
		    cc.pre_autocmd_add(FIM_VID_SCALE_STYLE"='1';" "autocmd \"" FIM_ACM_POSTSCALE "\" \"\" \"" FIM_FLT_DISPLAY "'resize';\";");
	#endif /* FIM_AUTOCMDS */
		    break;
		case 0x6f66660a:
		    //fim's
	#ifdef FIM_AUTOCMDS
		{
			fim_int ipeppe_offset;
			ipeppe_offset = fim_atoi(optarg);
			if(ipeppe_offset<0)
				std::cerr<< "warning: ignoring user set negative offset value.\n";
			else
			if(ipeppe_offset>=0 && isdigit(*optarg))
			{
				string tmp;
				size_t peppe_offset=0,ro=0;
				ro=peppe_offset =(size_t)ipeppe_offset;
				if(strchr(optarg,':'))
					ro=(size_t)atoi(strchr(optarg,':')+1)-peppe_offset;
				if(strchr(optarg,'+'))
					ro=(size_t)atoi(strchr(optarg,'+')+1);
				tmp=FIM_VID_OPEN_OFFSET;       tmp+="="; tmp+=string((fim_int)peppe_offset);/* FIXME */ tmp+=";";
				tmp=FIM_VID_OPEN_OFFSET_RETRY; tmp+="="; tmp+=string((fim_int)ro);/* FIXME */ tmp+=";";
				cc.pre_autocmd_add(tmp);
				//std::cout << "adding autocmd " << tmp<< "\n";
				//std::cout << "peppe_offset" << peppe_offset<< "\n";
			}
		}
	#endif /* FIM_AUTOCMDS */
		    break;
		case 'g':
		    //fbi's
		    default_fbgamma = fim_atof(optarg);
		    break;
		case 'r':
		    cout << FIM_EMSG_UNFINISHED;
		    //fbi's
	// TODO
	//	    pcd_res = atoi(optarg);
		    break;
		case 'R':
		    //fim's
		    pf = FIM_FLAG_PUSH_REC ;
		    break;
		case 's':
	//	    if(atoi(optarg)>0) cc.setVariable(FIM_VID_STEPS,fim_atoi(optarg));
		    if(fim_atoi(optarg)>0)
		    {
		    	// fixme : still buggy
		    	fim::string s=FIM_VID_STEPS;
			s+=fim::string(fim_atoi(optarg));
			s+=";";
	#ifdef FIM_AUTOCMDS
			cc.pre_autocmd_add(s);
	#endif /* FIM_AUTOCMDS */
		    }
		    break;
	//	case 't':
		    //fbi's
	//	    timeout = atoi(optarg);
	//	    FIM_FPRINTF(stderr, "sorry, this feature will be implemented soon\n");
	//	    break;
		case 'u':
		    want_random_shuffle=1;
		    break;
		case 0x7073:
		    //FIM_FPRINTF(stderr, "sorry, this feature will be implemented soon\n");
		    //fim's
		    want_random_shuffle=-1;
		    break;
		case 0x736f626e:
		    //fim's
		    want_random_shuffle=c;
		    break;
		case 0x736f7274:
		    //fim's
		    want_random_shuffle=c;
		    break;
		case 'd':
		    //fbi's
		    default_fbdev = optarg;
		    break;
		case 'i':
		    //fim's
#ifdef FIM_READ_STDIN_IMAGE
		    read_stdin_choice = ImageFile;
#else /* FIM_READ_STDIN_IMAGE */
		    FIM_FPRINTF(stderr, FIM_EMSG_NO_READ_STDIN_IMAGE);
#endif /* FIM_READ_STDIN_IMAGE */
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
		    show_version();
		    return 0;
		    break;
		case 0x4352:
		    //fim's
		    cc.appendPostInitCommand( "if(" FIM_VID_FILELISTLEN "==1){_ffn=i:" FIM_VID_FILENAME ";" FIM_FLT_CD " i:" FIM_VID_FILENAME ";" FIM_FLT_LIST " 'remove' i:" FIM_VID_FILENAME ";" FIM_FLT_BASENAME " _ffn;_bfn='./'." FIM_VID_LAST_SYSTEM_OUTPUT ";" FIM_FLT_LIST " 'pushdir' '.';" FIM_FLT_LIST " 'sort';" FIM_FLT_GOTO " '?'._bfn;" FIM_FLT_RELOAD ";" FIM_FLT_REDISPLAY ";}if(" FIM_VID_FILELISTLEN "==0){" FIM_FLT_STDOUT " 'No files loaded: exiting.';" FIM_FLT_QUIT " 0;}");
		    appendedPostInitCommand=true;
		    break;
		case 'c':
		    //fim's
		    cc.appendPostInitCommand(optarg);
		    appendedPostInitCommand=true;
		    break;
		case 'C':
		    //fim's
		    cc.appendPreConfigCommand(optarg);
		    appendedPreConfigCommand=true;
		    break;
		case 'W':
		    //fim's
		    cc.setVariable(FIM_VID_SCRIPTOUT_FILE,optarg);
	#ifdef FIM_AUTOCMDS
		    cc.pre_autocmd_add(FIM_FLT_RECORDING" 'start';");
		    cc.appendPostExecutionCommand(FIM_FLT_RECORDING" 'stop';");
	#endif /* FIM_AUTOCMDS */
		    break;
		case 'F':
		    //fim's
		    cc.appendPostExecutionCommand(optarg);
		    break;
		case 'E':
		    //fim's
	#ifndef FIM_WANT_NOSCRIPTING
		    cc.push_scriptfile(optarg);
	#else /* FIM_WANT_NOSCRIPTING */
		    cout << FIM_EMSG_NO_SCRIPTING;
	#endif /* FIM_WANT_NOSCRIPTING */
		    break;
		case 'p':
		    //fim's (differing semantics from fbi's)
	#ifndef FIM_WANT_NOSCRIPTING
		    read_stdin_choice = Script;
        #else /* FIM_WANT_NOSCRIPTING */
		    cout << FIM_EMSG_NO_SCRIPTING;
        #endif /* FIM_WANT_NOSCRIPTING */
		    break;
		case 'D':
		    //fim's
	//	    cc.setNoFrameBuffer();	// no framebuffer (debug) mode
		    cc.dumpDefaultFimrc();
		    std::exit(0);
		    break;
		case 'N':
		    //fim's
			cc.setVariable(FIM_VID_NO_RC_FILE,(fim_int)1);
		    break;
		case 0x4E4E4E:// NNN
		    //fim's
		    	cc.setVariable(FIM_VID_NO_DEFAULT_CONFIGURATION,(fim_int)1);
		    break;
		case 0x4E4E:// NN
		    //fim's
		    	cc.setVariable(FIM_VID_LOAD_DEFAULT_ETC_FIMRC,(fim_int)0);
		    break;
		case 0x4E434C:// NCL
		    //fim's
		    	cc.setVariable(FIM_VID_CONSOLE_KEY,(fim_int)-1);
		    break;
#if FIM_WANT_HISTORY
		case 0x4E484C:// NHL
		    //fim's
		    	cc.setVariable(FIM_VID_LOAD_FIM_HISTORY,(fim_int)-1);
		    break;
		case 0x4E4853:// NHS
		    //fim's
		    	cc.setVariable(FIM_VID_SAVE_FIM_HISTORY,(fim_int)-1);
		    break;
		case 0x4E48:// NH
		    //fim's
		    	cc.setVariable(FIM_VID_SAVE_FIM_HISTORY,(fim_int)-1);
		    	cc.setVariable(FIM_VID_LOAD_FIM_HISTORY,(fim_int)-1);
		    break;
#endif /* FIM_WANT_HISTORY */
		case 't':
		    //fim's
			#ifdef FIM_WITH_AALIB
		    	g_fim_output_device=FIM_DDN_INN_AA;
			#else /* FIM_WITH_AALIB */
			std::cerr << "you should recompile fim with aalib support!\n";
			g_fim_output_device=FIM_DDN_INN_DUMB;
			#endif /* FIM_WITH_AALIB */
		    break;
		case 'o':
		    //fim's
		    	g_fim_output_device=optarg;
#if FIM_WANT_OUTPUT_DEVICE_STRING_CASE_INSENSITIVE
			{
				int si=g_fim_output_device.find(FIM_SYM_DEVOPTS_SEP_STR);
				if(si>0);else si=g_fim_output_device.end()-g_fim_output_device.begin();
				transform(g_fim_output_device.begin(), si+g_fim_output_device.begin(), g_fim_output_device.begin(),(int (*)(int))tolower);
			}
#endif /* FIM_WANT_OUTPUT_DEVICE_STRING_CASE_INSENSITIVE */
		    break;
		case 0x6472690a:
		    //fim's
		{
			args_t args;
			if(optarg)
				args.push_back(optarg);
			cc.dump_reference_manual(args);
			std::exit(0);
		}
		    break;
#if FIM_WANT_PIC_CMTS
		case 0x69646673:
		    if(optarg)
			    sc = *optarg;
		    break;
		case 0x6c696466:
		    cc.id_.fetch(optarg,sc);
		    break;
#endif /* FIM_WANT_PIC_CMTS */
	#ifdef FIM_READ_STDIN
		case '-':
		    //fim's
		    read_stdin_choice = FilesList;
		    break;
		case 0x72667373:
		    sa = optarg;
		    break;
		case 0:
		    //fim's
		    read_stdin_choice = FilesList;
		    break;
	#endif /* FIM_READ_STDIN */
		default:
		case 'h':
		    help_and_exit(argv[0],FIM_PERR_NO_ERROR,optarg);
		}
	    }
		for (i = optind; i < argc; i++)
		{
	#ifdef FIM_READ_STDIN
			if(*argv[i]=='-'&&!argv[i][1])
				read_stdin_choice = FilesList;
			else
	#endif /* FIM_READ_STDIN */
			{
				cc.push(argv[i],pf);
			}
		}
	
		lexer=new yyFlexLexer;	//used by YYLEX
		if(!lexer)
		{
			FIM_FPRINTF(stderr, "error while allocating the lexer!\n\n");
			retcode=-1;
			goto ret;
		}

	#ifdef FIM_READ_STDIN	
		if( ( read_stdin_choice == FilesList ) +
		#ifdef FIM_READ_STDIN_IMAGE
		( read_stdin_choice == ImageFile ) + 
		#endif /* FIM_READ_STDIN_IMAGE */
		( read_stdin_choice == Script ) > 1 )
		{
			/* FIXME: this case is now useless. Shall rather implement a warning for when -p and -i overlap. */
			FIM_FPRINTF(stderr, "error : you shouldn't specify more than one standard input reading options among (-, -p"
#ifdef FIM_READ_STDIN_IMAGE
					", -i"
#endif /* FIM_READ_STDIN_IMAGE */
					")!\n\n");
			retcode=help_and_exit(argv[0],FIM_PERR_NO_ERROR,"b");/* should return 0 or -1 ? */
			goto ret;
		}
		/*
		 * this is Vim's solution for stdin reading
		 * */
		if( read_stdin_choice == FilesList )
		{
		    	bool wv = false; /*cc.pre_autocmd_add(FIM_VID_DISPLAY_STATUS"=...;");*/ /* or verbose ... */
			fim_char_t *lineptr=NULL;
			size_t bs=0;
			int fc=0;

			while(fim_getline(&lineptr,&bs,stdin)>0)
			{
				chomp(lineptr);

				if(sa && lineptr && strstr(lineptr,sa))
					*strstr(lineptr,sa) = FIM_SYM_CHAR_NUL;
				cc.push(lineptr,pf);
				// printf("%s\n",lineptr);
				lineptr=NULL;
				if(wv)
					++fc, printf("%s %d\n",FIM_CNS_CLEARTERM,fc);
			}
			if(lineptr)
				fim_free(lineptr);
			close(0);
			ndd=dup(2);
		}
		#ifdef FIM_READ_STDIN_IMAGE
		else
		if( read_stdin_choice == ImageFile )
		{
#if !FIM_WANT_STDIN_FILELOAD_AFTER_CONFIG
			cc.fpush(fim_fread_tmpfile(stdin));
			close(0);
			ndd=dup(2);
#endif /* FIM_WANT_STDIN_FILELOAD_AFTER_CONFIG */
		}
		#endif /* FIM_READ_STDIN_IMAGE */
		else
		if( read_stdin_choice == Script )
		{
		    	fim_char_t* buf;
			buf=slurp_binary_fd(0,NULL);
			if(buf)
			       	cc.appendPostInitCommand(buf);
			if(buf)
			       	appendedPostInitCommand=true;
	//		if(buf) appendedPreConfigCommand=true;
			if(buf)
			       	fim_free(buf);
			close(0);
			ndd=dup(2);
		}
	#endif
		if(want_random_shuffle== 1)
			cc.browser_._random_shuffle(true);
		if(want_random_shuffle==-1)
			cc.browser_._random_shuffle(false);
		if(want_random_shuffle==0x736f7274)
			cc.browser_._sort();
		if(want_random_shuffle==0x736f626e)
			cc.browser_._sort('b');

		if(ndd==-1)
			fim_perror(NULL);
	
		if(cc.browser_.empty_file_list()
#ifndef FIM_WANT_NOSCRIPTING
			       	&& !cc.with_scriptfile()
#endif /* FIM_WANT_NOSCRIPTING */
			       	&& !appendedPostInitCommand 
			       	&& !appendedPreConfigCommand 
		#ifdef FIM_READ_STDIN_IMAGE
		&& ( read_stdin_choice != ImageFile )
		#endif /* FIM_READ_STDIN_IMAGE */
		&& !perform_sanity_check
		)
		{
			retcode=help_and_exit(argv[0],FIM_PERR_BAD_PARAMS,"b");goto ret;
		}
	
		/* output device guess */
		if( g_fim_output_device==FIM_CNS_EMPTY_STRING )
		{
			#if defined(FIM_WITH_LIBSDL) || defined(FIM_WITH_LIBIMLIB2)
			/* check to see if we are under X */
			if( fim_getenv(FIM_ENV_DISPLAY) )
			{
	#ifdef FIM_WITH_LIBIMLIB2
				g_fim_output_device=FIM_DDN_INN_IL2;
	#endif /* FIM_WITH_LIBIMLIB2 */
	#ifdef FIM_WITH_LIBSDL
				g_fim_output_device=FIM_DDN_INN_SDL;
	#endif /* FIM_WITH_LIBSDL */
			}
			else
			#endif
#ifndef FIM_WITH_NO_FRAMEBUFFER
			g_fim_output_device=FIM_DDN_INN_FB;
#else /* FIM_WITH_NO_FRAMEBUFFER */
	#ifdef FIM_WITH_AALIB
			g_fim_output_device=FIM_DDN_INN_AA;
	#else /* FIM_WITH_AALIB */
			g_fim_output_device=FIM_DDN_INN_DUMB ;
	#endif /* FIM_WITH_AALIB */
#endif	//#ifndef FIM_WITH_NO_FRAMEBUFFER
		}

		// TODO : we still need a good output device probing mechanism

		if((retcode=FIM_ERR_TO_PERR(cc.init(g_fim_output_device)))!=FIM_PERR_NO_ERROR) {goto ret;}
#ifdef FIM_READ_STDIN_IMAGE
#if FIM_WANT_STDIN_FILELOAD_AFTER_CONFIG
		if( read_stdin_choice == ImageFile )
		{
			cc.fpush(fim_fread_tmpfile(stdin));
			close(0);
			ndd=dup(2);
		}
#endif /* FIM_WANT_STDIN_FILELOAD_AFTER_CONFIG */
#endif /* FIM_READ_STDIN_IMAGE */
		retcode=cc.executionCycle();/* note that this could not return */
ret:
		return retcode;
	}

};

fim_perr_t main(int argc,char *argv[])
{
	/*
	 * FimInstance will encapsulate all of the fim's code someday.
	 * ...someday.
	 * */
	FimInstance fiminstance;

	fim::fim_var_help_db_init();
	return fiminstance.main(argc,argv);
}

		/* FIXME: we are including files here.
		 * this is a horrible programming practice and shall be fixed. */
#ifdef FIM_WITH_LIBPNG
#include <png.h>
#endif /* FIM_WITH_LIBPNG */
#ifdef HAVE_LIBJPEG
#include <jpeglib.h>
#endif /* HAVE_LIBJPEG */
#ifdef FIM_HANDLE_TIFF
#include <tiffio.h>
#endif /* FIM_HANDLE_TIFF */
#ifdef FIM_HANDLE_GIF
#include <gif_lib.h>
#endif /* FIM_HANDLE_GIF */
#ifdef FIM_USE_READLINE
#include "readline.h"
#endif /* FIM_USE_READLINE */
#if 0 /* namespace clashes */
#ifdef HAVE_LIBGRAPHICSMAGICK
#include <magick/api.h>
#include <magick/version.h>
#endif /* HAVE_LIBGRAPHICSMAGICK */
#endif
///#ifdef HAVE_LIBPOPPLER
//#include <poppler/PDFDoc.h> // getPDFMajorVersion getPDFMinorVersion
//#endif /* HAVE_LIBPOPPLER */

	void FimInstance::show_version(void)
	{
	    FIM_FPRINTF(stderr, 
			    FIM_CNS_FIM" "
	#ifdef FIM_VERSION
			    FIM_VERSION
	#endif /* FIM_VERSION */
	#ifdef SVN_REVISION
			    " ( repository version "
		SVN_REVISION
			    " )"
	#else /* SVN_REVISION */
	/* obsolete */
	# define FIM_REPOSITORY_VERSION  "$LastChangedDate: 2015-04-18 21:37:25 +0200 (Sat, 18 Apr 2015) $"
	# ifdef FIM_REPOSITORY_VERSION 
			    " ( repository version "
		FIM_REPOSITORY_VERSION 	    
			    " )"
	# endif /* FIM_REPOSITORY_VERSION  */
	#endif /* SVN_REVISION */
	#ifdef FIM_AUTHOR 
			    ", by "
			    FIM_AUTHOR
	#endif /* FIM_AUTHOR  */
			    ", built on %s\n",
			    __DATE__
	    		    " ( based on fbi version 1.31 (c) by 1999-2004 " FBI_AUTHOR_NAME " )\n"
	#ifdef FIM_WITH_LIBPNG
	#ifdef PNG_HEADER_VERSION_STRING 
	"Compiled with " PNG_HEADER_VERSION_STRING ""
	#endif /* PNG_HEADER_VERSION_STRING */
	#endif /* FIM_WITH_LIBPNG */
	#ifdef FIM_HANDLE_GIF
	#if defined(GIFLIB_MAJOR) && defined(GIFLIB_MINOR) && defined(GIFLIB_RELEASE)
	"Compiled with libgif, " FIM_XSTRINGIFY(GIFLIB_MAJOR) "." FIM_XSTRINGIFY(GIFLIB_MINOR) "." FIM_XSTRINGIFY(GIFLIB_RELEASE) ".\n"
	#else /* GIFLIB_MAJOR GIFLIB_MINOR GIFLIB_RELEASE */
	#ifdef GIF_LIB_VERSION
	"Compiled with libgif, " GIF_LIB_VERSION ".\n"
	#endif /* GIF_LIB_VERSION */
	#endif /* GIFLIB_MAJOR GIFLIB_MINOR GIFLIB_RELEASE */
	#endif /* FIM_HANDLE_GIF */
	#ifdef HAVE_LIBJPEG
	#ifdef JPEG_LIB_VERSION
	"Compiled with libjpeg, v." FIM_XSTRINGIFY(JPEG_LIB_VERSION) ".\n"
	#endif /* JPEG_LIB_VERSION */
	#endif /* HAVE_LIBJPEG */
#if 0 /* namespace clashes */
	#ifdef HAVE_LIBGRAPHICSMAGICK
	#ifdef MagickLibVersionText
	"Compiled with GraphicsMagick, v."MagickLibVersionText".\n"
	#endif /* MagickLibVersionText */
	#endif /* HAVE_LIBGRAPHICSMAGICK */
#endif
	#ifdef FIM_USE_READLINE
	// TODO: shall use RL_READLINE_VERSION instead
	#if defined(RL_VERSION_MINOR) && defined(RL_VERSION_MAJOR) && ((RL_VERSION_MAJOR)>=6)
	"Compiled with readline, v." FIM_XSTRINGIFY(RL_VERSION_MAJOR) "." FIM_XSTRINGIFY(RL_VERSION_MINOR) ".\n"
	#else
	"Compiled with readline, version unknown.\n"
	#endif /* defined(RL_VERSION_MINOR) && defined(RL_VERSION_MAJOR) && ((RL_VERSION_MAJOR)>=6) */
	#endif /* FIM_USE_READLINE */
	// for TIFF need TIFFGetVersion
	#ifdef FIM_CONFIGURATION
			"Configuration invocation: " FIM_CONFIGURATION "\n" 
	#endif /* FIM_CONFIGURATION */
	#ifdef CXXFLAGS
			"Compile flags: CXXFLAGS=" CXXFLAGS
	#ifdef CFLAGS
			"  CFLAGS=" CFLAGS
	#endif /* CFLAGS */
			"\n"
	#endif /* CXXFLAGS */
			"Fim options (features included (+) or not (-)):\n"
	#include "version.h"
	/* i think some flags are missing .. */
		"\nSupported output devices (for --" FIM_OSW_OUTPUT_DEVICE "): "
	#ifdef FIM_WITH_AALIB
		" " FIM_DDN_INN_AA
	#endif /* FIM_WITH_AALIB */
	#ifdef FIM_WITH_CACALIB
		" " FIM_DDN_INN_CACA
	#endif /* FIM_WITH_CACALIB */
	#ifdef FIM_WITH_LIBIMLIB2
		" " FIM_DDN_INN_IL2
	#endif /* FIM_WITH_LIBIMLIB2 */
	#ifdef FIM_WITH_LIBSDL
		" " FIM_DDN_INN_SDL
	#endif /* FIM_WITH_LIBSDL */
#ifndef FIM_WITH_NO_FRAMEBUFFER
		" " FIM_DDN_INN_FB
#endif //#ifndef FIM_WITH_NO_FRAMEBUFFER
	#if 1
		" " FIM_DDN_INN_DUMB
	#endif
		"\n"
		"\nSupported file formats: "
#ifdef ENABLE_PDF
		" pdf"
#endif /* ENABLE_PDF */
#ifdef HAVE_LIBSPECTRE
		" ps"
#endif /* HAVE_LIBSPECTRE */
#ifdef HAVE_LIBDJVU
		" djvu"
#endif /* HAVE_LIBDJVU */
#ifdef HAVE_LIBJPEG
		" jpeg"
#endif /* HAVE_LIBJPEG */
#ifdef FIM_HANDLE_TIFF
		" tiff"
#endif /* FIM_HANDLE_TIFF */
#ifdef FIM_HANDLE_GIF
		" gif"
#endif /* FIM_HANDLE_GIF */
#ifdef FIM_WITH_LIBPNG
		" png"
#endif /* FIM_WITH_LIBPNG */
		" ppm"	/* no library is needed for these */
		" bmp"
#ifdef HAVE_MATRIX_MARKET_DECODER
		" mtx (Matrix Market)"
#endif /* HAVE_MATRIX_MARKET_DECODER */
		"\n"
			    );
		
		fim_loaders_to_stderr();
	}
	/* WARNING: PLEASE DO NOT WRITE ANY MORE CODE AFTER THIS DECLARATION (RIGHT ABOVE, AN UNCLEAN CODING PRACTICE WAS USED) */

