/* $LastChangedDate: 2015-02-12 19:10:43 +0100 (Thu, 12 Feb 2015) $ */
/*
 CommandConsole.h : Fim console dispatcher header file

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

#ifndef FIM_COMMANDCONSOLE_H
#define FIM_COMMANDCONSOLE_H
#include "fim.h"
#include "DummyDisplayDevice.h"

#define FIM_WANT_RAW_KEYS_BINDING 1

namespace fim
{
class CommandConsole
#if FIM_WANT_BENCHMARKS
	: public Benchmarkable,
#endif /* FIM_WANT_BENCHMARKS */
	public Namespace
{
	public:
	friend class FbiStuff;

	private:
#ifndef FIM_WANT_NO_OUTPUT_CONSOLE
#ifndef FIM_KEEP_BROKEN_CONSOLE
	public:
	MiniConsole mc_;
#endif /* FIM_KEEP_BROKEN_CONSOLE */
#endif /* FIM_WANT_NO_OUTPUT_CONSOLE */
	FontServer fontserver_;

	fim::string postInitCommand_;
	fim::string preConfigCommand_;
	fim::string postExecutionCommand_;

	fim_int show_must_go_on_;
	fim_int return_code_;	/* new, to support the 'return' command */
	bool mangle_tcattr_;
	public:

	struct termios  saved_attributes_;
	fim_sys_int             saved_fl_; /* file status flags for stdin */

	/*
	 * the image browser_ logic
	 */
	Browser browser_;
	private:

#ifdef FIM_WINDOWS
	public:// 20110617 this is terrible, I know
	fim::FimWindow * window_;
	private:
#endif /* FIM_WINDOWS */
	/*
	 * the registered command methods and objects
	 */
	std::vector<Command*> commands_;			//command->method

	/*
	 * the aliases to actions (compounds of commands)
	 */
	typedef std::map<fim::string,std::pair<fim::string,fim::string> > aliases_t;	//alias->[commands,description]
	//typedef std::map<fim::string,fim::string> aliases_t;	//alias->commands
	aliases_t aliases_;	//alias->commands
	
	/*
	 * bindings of key codes to actions (compounds of commands)
	 */
	typedef std::map<fim_key_t,fim::string> bindings_t;		//code->commands
	bindings_t bindings_;		//code->commands
	typedef std::map<fim_key_t,fim::string> bindings_help_t; // code->help
	bindings_help_t bindings_help_;		//code->commands

	/*
	 * mapping of key name to key code
	 */
	sym_keys_t	sym_keys_;	//symbol->code

	typedef std::map<fim_key_t, fim::string> key_syms_t;//code->symbol
	key_syms_t key_syms_;//code->symbol

	private:

	fim_err_t load_or_save_history(bool load_or_save);

	/*
	 * the identifier->variable binding
	 */
	//typedef std::map<const fim::string,Var> variables_t;	//id->var
	//variables_t variables_;	//id->var

#if FIM_WANT_FILENAME_MARK_AND_DUMP
	/*
	 * the buffer of marked files
	 */
	typedef std::set<fim::string> marked_files_t;	//
	marked_files_t marked_files_;		//filenames
	public:
	fim::string marked_files_list(void)const;
	private:
#endif /* FIM_WANT_FILENAME_MARK_AND_DUMP */

	/*
	 * flags
	 */
#ifdef FIM_USE_READLINE
	/* no readline ? no console ! */
	fim_status_t 	ic_;				//in console if 1. not if 0. willing to exit from console mode if -1
#endif /* FIM_USE_READLINE */
	fim_cycles_t cycles_;					//fim execution cycles_ counter (quite useless)
	fim_key_t exitBinding_;				//The key bound to exit. If 0, the special "Any" key.

#ifdef FIM_AUTOCMDS
	/*
	 * the mapping structure for autocommands (<event,pattern,action>)
	 */
	typedef std::map<fim::string,args_t >  autocmds_p_t;	//pattern - commands
	typedef std::map<fim::string,autocmds_p_t >  autocmds_t;		//autocommand - pattern - commands
	autocmds_t autocmds_;
#endif /* FIM_AUTOCMDS */
	
	/*
	 * the last executed action (being a command line or key bounded command issued)
	 */
	fim::string last_action_;
	
#ifdef FIM_RECORDING
	bool recordMode_;
	typedef std::pair<fim::string,fim_tms_t > recorded_action_t;
	typedef std::vector<recorded_action_t > recorded_actions_t;
	recorded_actions_t recorded_actions_;

	bool dont_record_last_action_;
	fim::string memorize_last(const fim::string &cmd);
	fim::string repeat_last(const args_t &args);
	fim::string dump_record_buffer(const args_t &args);
	fim::string do_dump_record_buffer(const args_t &args)const;
	fim::string execute_record_buffer(const args_t &args);
	fim::string start_recording(void);
	fim::string fcmd_recording(const args_t &args);
	fim::string stop_recording(void);
	fim::string sanitize_action(const fim::string &cmd)const;

	void record_action(const fim::string &cmd);
#endif /* FIM_RECORDING */

	public:
	fim_str_t fim_stdin_;	// the standard input file descriptor
	private:
	fim_char_t prompt_[2];

#ifndef FIM_WANT_NOSCRIPTING
	args_t scripts_;		//scripts to execute : FIX ME PRIVATE
#endif /* FIM_WANT_NOSCRIPTING */

#if FIM_WANT_FILENAME_MARK_AND_DUMP
	public:
	void markCurrentFile(void);
	void unmarkCurrentFile(void);
	private:
#endif /* FIM_WANT_FILENAME_MARK_AND_DUMP */
#ifdef FIM_WITH_AALIB
	AADevice * aad_;
#endif /* FIM_WITH_AALIB */
	public:
	DummyDisplayDevice dummydisplaydevice_;
	DisplayDevice *displaydevice_;

	fim::string execute(fim::string cmd, args_t args);

	//const fim_char_t*get_prompt(void)const{return prompt_;}

	CommandConsole(void);
	private:
	CommandConsole& operator= (const CommandConsole&cc);
	public:
	bool display(void);
	bool redisplay(void);
	fim_char_t * command_generator (const fim_char_t *text,int state,int mask)const;
	fim_perr_t executionCycle(void);
	fim_err_t init(fim::string device);
	fim_bool_t inConsole(void)const;
	~CommandConsole(void);

	/* the following group is defined in Namespace.cpp */
	fim_float_t getFloatVariable(const fim::string &varname)const;
	fim::string getStringVariable(const fim::string &varname)const;
	fim_int  getIntVariable(const fim::string & varname)const;
	Var  getVariable(const fim::string & varname)const;
	fim_int  setVariable(const fim::string& varname,fim_int value);
	fim_float_t setVariable(const fim::string& varname, fim_float_t value);
	fim_int setVariable(const fim::string& varname,const fim_char_t*value);
	Var setVariable(const fim::string varname,const Var&value);
	Namespace * rns(const fim::string varname);
	const Namespace * c_rns(const fim::string varname)const;
	fim::string rnid(const fim::string & varname)const;

	fim_var_t getVariableType(const fim::string &varname)const;
	fim_err_t printVariable(const fim::string & varname)const;
	bool push(const fim::string nf, fim_flags_t pf=FIM_FLAG_DEFAULT);
	fim_err_t executeStdFileDescriptor(FILE *fd);
	fim::string readStdFileDescriptor(FILE* fd, int*rp=NULL);
#ifndef FIM_WANT_NOSCRIPTING
	bool push_scriptfile(const fim::string ns);
	bool with_scriptfile(void)const;
	fim::string fcmd_executeFile(const args_t &args);
#endif /* FIM_WANT_NOSCRIPTING */
	private:
	fim::string fcmd_echo(const args_t &args);
	fim::string do_echo(const args_t &args)const;
	//	fim::string get_expr_type(const args_t &args);
	fim::string fcmd_help(const args_t &args);
	fim::string fcmd_quit(const args_t &args);
	fim::string fcmd__stdout(const args_t &args);
	/* naming this stdout raises problems on some systems 
	   e.g.: 
# uname -a
Darwin hostname 7.9.0 Darwin Kernel Version 7.9.0: Wed Mar 30 20:11:17 PST 2005; root:xnu/xnu-517.12.7.obj~1/RELEASE
# gcc -v
Reading specs from /usr/libexec/gcc/darwin/ppc/3.3/specs
Thread model: posix
gcc version 3.3 20030304 (Apple Computer, Inc. build 1495)
*/
	fim::string fcmd_foo (const args_t &args);
	fim::string fcmd_status(const args_t &args);
	fim_err_t executeFile(const fim_char_t *s);
	fim_err_t execute_internal(const fim_char_t *ss, fim_xflags_t xflags);

	fim_err_t addCommand(Command *c);
	Command* findCommand(fim::string cmd)const;
	int findCommandIdx(fim::string cmd)const;
	fim::string fcmd_alias(std::vector<Arg> args);
	fim::string alias(const fim::string& a,const fim::string& c, const fim::string& d="");
	fim::string aliasRecall(fim::string cmd)const;
	fim::string fcmd_system(const args_t& args);
	fim::string fcmd_cd(const args_t& args);
	fim::string fcmd_pwd(const args_t& args);
	fim::string fcmd_sys_popen(const args_t& args);
#ifdef FIM_PIPE_IMAGE_READ
	fim::string fcmd_pread(const args_t& args);
#endif /* FIM_PIPE_IMAGE_READ */
	public:// 20110601
	fim_err_t fpush(FILE *tfd);
	private:
	fim::string fcmd_set_interactive_mode(const args_t& args);
	fim::string fcmd_set_in_console(const args_t& args);
#ifdef FIM_AUTOCMDS
	fim::string fcmd_autocmd(const args_t& args);
	fim::string autocmd_del(const fim::string event, const fim::string pattern, const fim::string action);
	fim::string fcmd_autocmd_del(const args_t& args);
	public:// 20110601
	fim::string autocmd_add(const fim::string &event,const fim::string &pat,const fim::string &cmd);
	private:
	fim::string autocmds_list(const fim::string event, const fim::string pattern)const;
#endif /* FIM_AUTOCMDS */
	typedef std::pair<fim::string,fim::string> autocmds_loop_frame_t;
	typedef std::pair<autocmds_loop_frame_t,fim::string> autocmds_frame_t;
	typedef std::vector<autocmds_loop_frame_t > autocmds_stack__t;
	typedef std::vector<autocmds_frame_t > autocmds_stack_t;
	//typedef std::set<autocmds_frame_t> autocmds_stack_t;
	autocmds_stack__t autocmds_loop_stack;
	autocmds_stack_t autocmds_stack;
	fim::string fcmd_bind(const args_t& args);
	fim::string getAliasesList(void)const;
	fim::string dummy(std::vector<Arg> args);
	fim::string fcmd_variables_list(const args_t& args);
	fim::string fcmd_commands_list(const args_t& args);
	fim::string fcmd_set(const args_t &args);
	fim::string fcmd_unalias(const args_t& args);
	//fim_char_t ** tokenize_(const fim_char_t *s);
	void executeBinding(const fim_key_t c);
	fim::string getBoundAction(const fim_key_t c)const;
	//	void execute(fim::string cmd);
	fim::string fcmd_eval(const args_t &args);
	void exit(fim_perr_t i)const;// FIXME: exit vs quit
	fim::string unbind(fim_key_t c);
	fim::string bind(fim_key_t c,fim::string binding);
	public:
	fim::string find_key_for_bound_cmd(fim::string binding);
	fim_err_t execDefaultConfiguration(void);
	private:
	fim::string unbind(const fim::string& key);
	fim::string fcmd_unbind(const args_t& args);
	fim::string getBindingsList(void)const;
	fim::string fcmd_dump_key_codes(const args_t& args);
	fim::string do_dump_key_codes(const args_t& args)const;
#ifndef FIM_WANT_NO_OUTPUT_CONSOLE
	fim::string fcmd_clear(const args_t& args);
	fim::string scroll_up(const args_t& args);
	fim::string scroll_down(const args_t& args);
#endif /* FIM_WANT_NO_OUTPUT_CONSOLE */
	fim_perr_t quit(fim_perr_t i=FIM_CNS_ERR_QUIT);
	public:
	fim_key_t find_keycode_for_bound_cmd(fim::string binding);

	fim_bool_t drawOutput(const fim_char_t*s=NULL)const;
	bool regexp_match(const fim_char_t*s, const fim_char_t*r, int rsic)const;
#ifdef FIM_AUTOCMDS
	fim::string autocmd_exec(const fim::string &event,const fim::string &fname);
	fim::string pre_autocmd_add(const fim::string &cmd);
	fim::string pre_autocmd_exec(void);
#endif /* FIM_AUTOCMDS */
	fim_int catchLoopBreakingCommand(fim_ts_t seconds=0);

	private:
	/* fim_key_t catchInteractiveCommand(fim_ts_t seconds=0)const; */
#ifdef FIM_AUTOCMDS
	fim::string autocmd_exec(const fim::string &event,const fim::string &pat,const fim::string &fname);
	void autocmd_push_stack(const autocmds_loop_frame_t& frame);
	void autocmd_pop_stack(const autocmds_loop_frame_t& frame);
	public:
	void autocmd_trace_stack(void);
	private:
	fim_bool_t autocmd_in_stack(const autocmds_loop_frame_t& frame)const;
#endif /* FIM_AUTOCMDS */
	fim::string current(void)const;

	fim::string get_alias_info(const fim::string aname)const;
#ifdef FIM_WINDOWS
	const FimWindow & current_window(void)const;
#endif /* FIM_WINDOWS */
	fim::string get_variables_list(void)const;
	fim::string get_aliases_list(void)const;
	fim::string get_commands_list(void)const;
	public:

	void printHelpMessage(const fim_char_t *pn="fim")const;
	void appendPostInitCommand(const fim_char_t* c);
	void appendPreConfigCommand(const fim_char_t* c);
	void appendPostExecutionCommand(const fim::string &c);
	bool appendedPostInitCommand(void)const;
	bool appendedPreConfigCommand(void)const;

	Viewport* current_viewport(void)const;
#ifdef FIM_WINDOWS
#else /* FIM_WINDOWS */
	Viewport* viewport_;
#endif /* FIM_WINDOWS */
	void dumpDefaultFimrc(void)const;

	void tty_raw(void);
	void tty_restore(void);
	void cleanup(void);
	
	fim::string print_commands(void)const;

	void status_screen(const fim_char_t *desc);
	void set_status_bar(fim::string desc, const fim_char_t *info);
	void set_status_bar(const fim_char_t *desc, const fim_char_t *info);
        bool is_file(fim::string nf)const;
	fim::string fcmd_do_getenv(const args_t& args);
	bool isVariable(const fim::string &varname)const;
	fim::string dump_reference_manual(const args_t& args);
	fim::string get_reference_manual(const args_t& args);
	private:
	fim::string get_commands_reference(FimDocRefMode refmode=DefRefMode)const;
	fim::string get_variables_reference(FimDocRefMode refmode=DefRefMode)const;
	public:
	bool set_wm_caption(const fim_char_t *str);
	fim_err_t resize(fim_coo_t w, fim_coo_t h);
	fim_err_t display_reinit(const fim_char_t *rs);
	fim::string fcmd_basename(const args_t& args);
	fim::string fcmd_desc(const args_t& args);
	fim_bool_t key_syms_update(void);
#if FIM_WANT_BENCHMARKS
	virtual fim_int get_n_qbenchmarks(void)const;
	virtual string get_bresults_string(fim_int qbi, fim_int qbtimes, fim_fms_t qbttime)const;
	virtual void quickbench_init(fim_int qbi);
	virtual void quickbench_finalize(fim_int qbi);
	virtual void quickbench(fim_int qbi);
#endif /* FIM_WANT_BENCHMARKS */
	virtual size_t byte_size(void)const;
	public:
#if FIM_WANT_PIC_CMTS
	ImgDscs id_;
#endif /* FIM_WANT_PIC_CMTS */
};
}

#endif /* FIM_COMMANDCONSOLE_H */
