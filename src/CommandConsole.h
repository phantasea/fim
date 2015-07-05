/* $Id: CommandConsole.h 243 2009-04-19 11:22:41Z dezperado $ */
/*
 CommandConsole.h : Fim console dispatcher header file

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

#ifndef FIM_COMMANDCONSOLE_H
#define FIM_COMMANDCONSOLE_H
#include "fim.h"
#include "DummyDisplayDevice.h"


namespace fim
{
class CommandConsole
{
	friend class FbiStuff;

	private:
#ifndef FIM_KEEP_BROKEN_CONSOLE
	public:
	MiniConsole mc;
#endif
	FontServer fontserver;

	fim::string postInitCommand;
	fim::string postExecutionCommand;

	int show_must_go_on;
	int return_code;	/* new, to support the 'return' command */
	public:

	struct termios  saved_attributes;
	int             saved_fl;

	/*
	 * the image browser logic
	 */
	Browser browser;
	private:

#ifdef FIM_WINDOWS
	fim::Window * window;
#endif
	/*
	 * the registered command methods and objects
	 */
	std::vector<Command*> commands;			//command->method

	/*
	 * the aliases to actions (compounds of commands)
	 */
	typedef std::map<fim::string,std::pair<fim::string,fim::string> > aliases_t;	//alias->[commands,description]
	//typedef std::map<fim::string,fim::string> aliases_t;	//alias->commands
	aliases_t aliases;	//alias->commands
	
	/*
	 * bindings of key codes to actions (compounds of commands)
	 */
	typedef std::map<int,fim::string> bindings_t;		//code->commands
	bindings_t bindings;		//code->commands

	/*
	 * mapping of key name to key code
	 */
	key_bindings_t	key_bindings;	//symbol->code

	typedef std::map<int, fim::string> inverse_key_bindings_t;//code->symbol
	inverse_key_bindings_t inverse_key_bindings;//code->symbol

	private:

	int save_history();
	int load_history();

	/*
	 * the identifier->variable binding
	 */
	typedef std::map<const fim::string,Var> variables_t;	//id->var
	variables_t variables;	//id->var

	/*
	 * the buffer of marked files
	 */
	std::set<fim::string> marked_files;		//filenames

	/*
	 * flags
	 */
#ifdef FIM_USE_READLINE
	/* no readline ? no console ! */
	int 	ic;					//in console if 1. not if 0. willing to exit from console mode if -1
#endif
	int	cycles;					//fim execution cycles counter (quite useless)
	int	exitBinding;				//The key bound to exit. If 0, the special "Any" key.

#ifdef FIM_AUTOCMDS
	/*
	 * the mapping structure for autocommands (<event,pattern,action>)
	 */
	typedef std::map<fim::string,args_t >  autocmds_p_t;	//pattern - commands
	typedef std::map<fim::string,autocmds_p_t >  autocmds_t;		//autocommand - pattern - commands
	autocmds_t autocmds;
#endif
	
	/*
	 * the last executed action (being a command line or key bounded command issued)
	 */
	fim::string last_action;
	
#ifdef FIM_RECORDING
	bool recordMode;
	typedef std::pair<fim::string,int> recorded_action_t;
	typedef std::vector<recorded_action_t > recorded_actions_t;
	recorded_actions_t recorded_actions;

	bool dont_record_last_action;
	fim::string memorize_last(const fim::string &cmd);
	fim::string repeat_last(const args_t &args);
	fim::string dump_record_buffer(const args_t &args);
	fim::string do_dump_record_buffer(const args_t &args)const;
	fim::string execute_record_buffer(const args_t &args);
	fim::string start_recording(const args_t &args);
	fim::string stop_recording(const args_t &args);
	fim::string sanitize_action(const fim::string &cmd)const;

	void record_action(const fim::string &cmd);
#endif

	public:
	int fim_stdin;	// the standard input file descriptor
	private:
	char prompt[2];

#ifndef FIM_NOSCRIPTING
	args_t scripts;		//scripts to execute : FIX ME PRIVATE
#endif

	void markCurrentFile();
	#ifdef FIM_WITH_AALIB
	AADevice * aad;
	#endif
	public:
	DummyDisplayDevice dummydisplaydevice;
	DisplayDevice *displaydevice;

	fim::string execute(fim::string cmd, args_t args);

	const char*get_prompt()const{return prompt;}

	CommandConsole();
	private:
	CommandConsole& operator= (const CommandConsole&cc){return *this;/* a nilpotent assignation */}
	public:

	fim::string markCurrentFile(const args_t& args);
	bool display();
	bool redisplay();
	char * command_generator (const char *text,int state)const;
	int executionCycle();
	int init(string device);
	int  inConsole()const;
	~CommandConsole();
	float getFloatVariable(const fim::string &varname)const;
	fim::string getStringVariable(const fim::string &varname)const;
	int  getVariableType(const fim::string &varname)const;
	int  getIntVariable(const fim::string & varname)const;
	Var  getVariable(const fim::string & varname)const;
	int  printVariable(const fim::string & varname)const;
	int  setVariable(const fim::string& varname,int value);
	float setVariable(const fim::string& varname,float value);
	int setVariable(const fim::string& varname,const char*value);
	Var setVariable(const fim::string varname,const Var&value);//NEW
	bool push(const fim::string nf);
	int executeStdFileDescriptor(FILE *fd);
	fim::string readStdFileDescriptor(FILE* fd);
#ifndef FIM_NOSCRIPTING
	bool push_scriptfile(const fim::string ns);
	bool with_scriptfile()const;
	fim::string executeFile(const args_t &args);
#endif
	private:
	fim::string echo(const args_t &args);
	fim::string do_echo(const args_t &args)const;
//	fim::string get_expr_type(const args_t &args);
	fim::string help(const args_t &args);
	fim::string quit(const args_t &args);
	fim::string _stdout(const args_t &args);
	/* naming this stdout raises problems on some systems 
	e.g.: 
	# uname -a
	Darwin hostname 7.9.0 Darwin Kernel Version 7.9.0: Wed Mar 30 20:11:17 PST 2005; root:xnu/xnu-517.12.7.obj~1/RELEASE
	# gcc -v
	Reading specs from /usr/libexec/gcc/darwin/ppc/3.3/specs
	Thread model: posix
	gcc version 3.3 20030304 (Apple Computer, Inc. build 1495)
	*/
	fim::string do_stdout(const args_t &args)const;
	fim::string foo (const args_t &args);
	fim::string do_return(const args_t &args);
	fim::string status(const args_t &args);
	int  executeFile(const char *s);
	int execute(const char *ss, int add_history_, int suppress_output_);

	int  toggleStatusLine();
	int  addCommand(Command *c);
	Command* findCommand(fim::string cmd)const;
	fim::string alias(std::vector<Arg> args);
	fim::string alias(const fim::string& a,const fim::string& c);
	fim::string aliasRecall(fim::string cmd)const;
	fim::string system(const args_t& args);
	fim::string cd(const args_t& args);
	fim::string pwd(const args_t& args);
	fim::string sys_popen(const args_t& args);
#ifdef FIM_PIPE_IMAGE_READ
	fim::string pread(const args_t& args);
#endif
	fim::string set_interactive_mode(const args_t& args);
	fim::string set_in_console(const args_t& args);
	fim::string autocmd(const args_t& args);
	fim::string autocmd_del(const fim::string event, const fim::string pattern, const fim::string action);
	fim::string autocmd_del(const args_t& args);
	fim::string autocmd_add(const fim::string &event,const fim::string &pat,const fim::string &cmd);
	fim::string autocmds_list(const fim::string event, const fim::string pattern)const;
	typedef std::pair<fim::string,fim::string> autocmds_loop_frame_t;
	typedef std::pair<autocmds_loop_frame_t,fim::string> autocmds_frame_t;
	typedef std::vector<autocmds_loop_frame_t > autocmds_stack__t;
	typedef std::vector<autocmds_frame_t > autocmds_stack_t;
	//typedef std::set<autocmds_frame_t> autocmds_stack_t;
	autocmds_stack__t autocmds_loop_stack;
	autocmds_stack_t autocmds_stack;
	fim::string bind(const args_t& args);
	fim::string getAliasesList()const;
	fim::string dummy(std::vector<Arg> args);
	fim::string variables_list(const args_t& args){return get_variables_list();}
	fim::string commands_list(const args_t& args){return get_commands_list();}
	fim::string set(const args_t &args);
	fim::string unalias(const args_t& args);
	char ** tokenize_(const char *s);
	void executeBinding(const int c);
	fim::string getBoundAction(const int c)const;
//	void execute(fim::string cmd);
	fim::string eval(const args_t &args);
	void exit(int i)const;
	fim::string unbind(int c);
	fim::string bind(int c,fim::string binding);
	fim::string unbind(const fim::string& key);
	fim::string unbind(const args_t& args);
	fim::string getBindingsList()const;
	fim::string dump_key_codes(const args_t& args);
	fim::string do_dump_key_codes(const args_t& args)const;
	fim::string clear(const args_t& args);
	fim::string scroll_up(const args_t& args);
	fim::string scroll_down(const args_t& args);
	int quit(int i=0);
	public:

	int  drawOutput(const char*s=NULL)const;
	bool regexp_match(const char*s, const char*r)const;
#ifdef FIM_AUTOCMDS
	fim::string autocmd_exec(const fim::string &event,const fim::string &fname);
	fim::string pre_autocmd_add(const fim::string &cmd);
#endif
	int catchLoopBreakingCommand(int seconds=0);

	private:
	int catchInteractiveCommand(int seconds=0)const;
#ifdef FIM_AUTOCMDS
	fim::string autocmd_exec(const fim::string &event,const fim::string &pat,const fim::string &fname);
	void autocmd_push_stack(const autocmds_loop_frame_t& frame);
	void autocmd_pop_stack(const autocmds_loop_frame_t& frame);
	public:
	void autocmd_trace_stack();
	private:
	int  autocmd_in_stack(const autocmds_loop_frame_t& frame)const;
#endif
	fim::string current()const{ return browser.current();}

	fim::string get_alias_info(const fim::string aname)const;
#ifdef FIM_WINDOWS
	const Window & current_window()const;
#endif
	fim::string get_variables_list()const;
	fim::string get_aliases_list()const;
	fim::string get_commands_list()const;
	public:

	void printHelpMessage(char *pn="fim")const;
	void appendPostInitCommand(const char* c);
	void appendPostExecutionCommand(const fim::string &c);
	bool appendedPostInitCommand()const;

	Viewport* current_viewport()const;
#ifdef FIM_WINDOWS
#else
	Viewport* viewport;
#endif
	void dumpDefaultFimrc()const;

	void tty_raw();
	void tty_restore();
	void cleanup();
	
	fim::string print_commands()const;

	void status_screen(const char *desc);
	void set_status_bar(fim::string desc, const char *info);
	void set_status_bar(const char *desc, const char *info);
        bool is_file(fim::string nf)const;
	fim::string do_getenv(const args_t& args);
	bool isVariable(const fim::string &varname)const;
	fim::string dump_reference_manual(const args_t& args);
	fim::string get_reference_manual(const args_t& args);
	private:
	fim::string get_commands_reference()const;
	fim::string get_variables_reference()const;
};
}

#endif
