/* $Id: defaultConfiguration.cpp 216 2009-02-21 23:39:23Z dezperado $ */
/*
 defaultConfiguration.cpp :
  The following code contains key bindings and aliases definitions
  which make up a first usable configuration for FIM.
 
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
#if FIM_DEFAULT_CONFIG
		alias("q","quit");
		bind('f',"files");
		bind('q',"quit");
		bind('f',"file");
		//bind('n',"next;display;");
		bind('n',"next");
		bind('h',"help");
		bind('x',"echo");
		bind('e',"echo");
		//bind('p',"prev;display;");
		bind('p',"prev");
		bind('P',"pop");
		bind('s',"sort");
		bind('r',"reload");
		bind('R',"redisplay");
		bind('d',"display");
		bind('+',"magnify");
		bind('a',"auto_scale");
		bind('v',"toggleVerbosity");
		bind('V',"toggleKeyVerbosity");
		bind('A',"A");
		bind('m',"mark");
		bind('-',"reduce");
//		bind(4283163,"panup");
		bind('k',"panup");
//		bind(4414235,"panright");
		bind('l',"panright");
//		bind(4348699,"pandown");
		bind('j',"pandown");
//		bind(4479771,"panleft");
		bind('h',"panleft");
		bind(' ',"scrolldown");
//		bind(2117491483,"next");
//		bind(2117425947,"prev");
		bind(29,"scale_factor_increase");
		bind(31,"scale_factor_decrease");
		bind(29,"scale_factor_grow");
		bind(31,"scale_factor_shrink");
		bind('i',"info");
		bind('P',"f=`files`/2;prev (--$f);\n");
		bind('N',"f=`files`/2;next (--$f);\n");

/*		alias('A',"'autotop=1-autotop;'");
		alias("toggleVerbosity","verbose=1-verbose;display;");
		alias("toggleKeyVerbosity","_verbose_keys=1-_verbose_keys;display;");*/
	
		setVariable(FIM_VID_CONSOLE_KEY,':');
/*		setVariable(FIM_VID_AUTOWIDTH,1);
		setVariable(FIM_VID_VERBOSE_KEYS,0);*/
		setVariable(FIM_VID_MAGNIFY_FACTOR,1.322f);
		setVariable(FIM_VID_SCALE_FACTOR_MULTIPLIER,1.1f);
		setVariable(FIM_VID_SCALE_FACTOR_DELTA,0.01f);
		setVariable(FIM_VID_REDUCE_FACTOR,1.322f);
/*		autocmd_add("Next","*","echo 'rendering next image..'");
		autocmd_add("Prev","*","echo 'rendering prev image..'");
		setVariable(FIM_VID_AUTOWIDTH,1);
		autocmd_add("PostNext","*","reload;");		//BUGFUL!
		autocmd_add("PostPrev","*","reload;");		//BUGFUL!
*/

//		autocmd_add("PreRedisplay","*","echo 'there should go autoscale'");		//NOT BUGFUL
//		autocmd_add("PreRedisplay","*","2magnify;");		//NOT BUGFUL!! WHY??? (8 too is ok)
//		autocmd_add("PreRedisplay","*","auto_scale_;");		//BUGFUL! (contains 1 magnify call..)
//		autocmd_add("PostReload","*","fresh=1;");		//BUGFUL! (contains 1 magnify call..)
	//	autocmd_add("PostRedisplay","*","fresh=1;");		//BUGFUL! (contains 1 magnify call..)
//		autocmd_add("PostDisplay","*","fresh=0;");		//BUGFUL! (contains 1 magnify call..)
//		autocmd_add("PreRedisplay","*","auto_scale;");		//BUGFUL!
//		autocmd_add("PostReload","*","redisplay;");		//BUGFUL!
//		autocmd_add("PostLoad","*","redisplay;");		//BUGFUL!
//		autocmd_add("PostNext","*","fresh=1;");		//BUGFUL! (contains 1 magnify call..)
//		autocmd_add("PostPrev","*","fresh=1;");		//BUGFUL! (contains 1 magnify call..)

//		autocmd_add("PostReload","*","echo 'scaled..';sleep 1;auto_scale;display;");		//BUGFUL!?! SHOULD NOT!
//		autocmd_add("PostReload","*","redisplay;");		//BUGFUL!?
//		autocmd_add("PostLoad"  ,"*","echo 'scaled..';sleep 1;auto_scale;display;");		//BUGFUL!?! SHOULD NOT!
/*		autocmd_add("PostLoad"  ,"*","fresh=1");		//BUGFUL!?! SHOULD NOT!
		autocmd_add("PostReload","*","fresh=1");		//BUGFUL!?! SHOULD NOT!
		autocmd_add("PostLoad"  ,"*","if(autowidth*fresh){auto_width_scale;}");		//BUGFUL!?! SHOULD NOT!
		autocmd_add("PostReload","*","if(autowidth*fresh){auto_width_scale;}");		//BUGFUL!?! SHOULD NOT!
		autocmd_add("PostLoad"  ,"*","fresh=0");		//BUGFUL!?! SHOULD NOT!
		autocmd_add("PostReload","*","fresh=0");		//BUGFUL!?! SHOULD NOT!
//		autocmd_add("PostLoad"  ,"*","redisplay;");		//BUGFUL!?
*/
/*		autocmd_add("PreExecutionCycle","*","reload;");	
		autocmd_add("PreExecutionCycle","*","redisplay;");	*/

//		autocmd_add("PreExecutionCycle","*","reload;auto_scale;display;");	

//		autocmd_add("PreRedisplay","*","auto_width_scale;");//BUGFUL!
//
//		autocmd_add("PostDisplay","*","auto_width_scale;");//BUGFUL!
//		autocmd_add("PostScale","*","display;");
//		autocmd_add("PostInteractiveCommand","*","display;");
//		autocmd_add("PostInteractiveCommand","*","auto_scale;");
//		autocmd_add("PostInteractiveCommand","*","redisplay;");	//necessary? altrimenti si blocca..
//

//		autocmd_add("PreExecutionCycle","*","fresh=1;");
//		autocmd_add("PreExecutionCycle","*","if(autowidth*fresh==1){auto_scale;redisplay;}");
//		autocmd_add("PreExecutionCycle","*","if(autowidth*fresh==0){redisplay;}");

		
//		autocmd_add("PreExecutionCycle","*","auto_scale;display;");
/*		autocmd_add("PreExecutionCycle","*","redisplay;");
		autocmd_add("PreExecutionCycle","*","fresh=0;");*/

/*		autocmd_add("PostInteractiveCommand","*","if(autowidth*fresh==1){auto_width_scale;redisplay;};");//ok ma non e' cio che voglio
		autocmd_add("PostInteractiveCommand","*","if(autowidth*fresh==0){redisplay;};");//ok ma non e' cio che voglio
		autocmd_add("PostInteractiveCommand","*","if(autowidth*fresh==0){redisplay;};");//ok ma non e' cio che voglio
		autocmd_add("PostInteractiveCommand","*","fresh=0;");//ok ma non e' cio che voglio
		*/
//		yyparse("next ( 4 );\n");
//		((browser)->)(*Browser::next)(std::vector)
		//SORTING IS A PROBLEM!
//		for(int i=0;i<commands.size();++i) if(commands[i]) commands[i]->execute(std::vector<Arg>());
//		init();

#endif
/*
 * The following code binds key codes to key symbols.
 * In this way the user can write a configuration file
 * and associate actions to key configurations..
 *
 * Note : certain key bindings (especially Control key with arrows 
 * or other non letter keys ) are known to be non working.
 *
 * But by modifying this file one can experiment!
 */
#if FIM_DEFAULT_KEY_CONFIG
		/*
		 * Standard key configuration
		 */
		char k[4];k[1]='\0';

		/*	97=0x61 to 122=0x7A	*/
		for(char i='a';i<='z';++i,*k=i) key_bindings[k]=i;

		
		/*	65=0x41 to 90=0x5A (shifted keys)	*/
		for(char i='A';i<='Z';++i,*k=i) key_bindings[k]=i;
		
		/*	32=0x20 to 65=0x40	*/
		for(char i=' ';i<='@';++i,*k=i) key_bindings[k]=i;

		/*	91=0x5B to 96=0x60	*/
		for(char i='[';i<'a';++i,*k=i) key_bindings[k]=i;

		/*	123=0x7B to 127=0x7F (DEL)	*/
		for(int i='{';i<=127;++i,*k=(char)i) key_bindings[k]=(char)i;
		// A note : DEL is not captured by the above configuration.
		
		/*
		 * Control keys handling
		 */
		k[0]='C'; k[1]='-'; k[3]='\0';
		//problem : no uppercase with control.sorry.
		for(char i='b';i<='j';++i,k[2]=i) key_bindings[k]=((i+1)-'a');
		for(char i='k';i<='z';++i,k[2]=i) key_bindings[k]=((i+1)-'a');
		for(char i=' ';i<='?';++i,k[2]=i) key_bindings[k]=( i+1)-'a' ;
		key_bindings["C-a"]=1;//to circumvent a mysterious arithmetical bug..
		/*
		 * The following bindings are known to be useless or bugful.
		*/
		/*
		for(char i='[';i<'a';++i,k[2]=i) key_bindings[k]=(i+1);
		for(int i='{';i<=127;++i,k[2]=(char)i) key_bindings[k]=(i+1)(char)i;
		*/
		
		/*
		 * The following means that when a decimal 4479771 will be read from
		 * the keyboard, it will be interpreted as a Left pressure.
		 */
		key_bindings["Left"]=4479771;
		// and so on..
		key_bindings["Right"]=4414235;
		key_bindings["Up"]=4283163;
		key_bindings["Down"]=4348699;
		key_bindings["PageDown"]=2117491483;
		key_bindings["PageUp"]=2117425947;
		key_bindings["Enter"]=10;
		key_bindings["Space"]=32;
		key_bindings["Backspace"]=127;
		key_bindings["Del"]=2117294875;
		key_bindings["Ins"]=2117229339;
		key_bindings["Home"]=2117163803;
		key_bindings["End"] =2117360411;
		key_bindings["Tab"] =9;
		key_bindings["Any"] =0;	/*	EXPERIMENTAL	*/
		key_bindings["Esc"] =27;

		exitBinding = key_bindings["Enter"];	/*  Enter will terminate loops */
//		key_bindings[" "]=' ';
//		key_bindings["."]='.';
//		key_bindings[","]=',';
//
{
		std::map<fim::string,int>::const_iterator ki;
		for( ki=key_bindings.begin();ki!=key_bindings.end();++ki)
		{
			inverse_key_bindings[(((*ki).second))]=((*ki).first);
			//inverse_key_bindings[10]="ciao";//((*ki).first);
		}
}
#endif
