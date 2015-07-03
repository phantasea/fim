/* $LastChangedDate: 2015-02-13 00:24:59 +0100 (Fri, 13 Feb 2015) $ */
/*
 interpreter.cpp : Fim language interpreter

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

#include <cstdio>
#include <map>
#include "lex.h"
#include "yacc.tab.hpp"
#include "fim.h"
#include "common.h"

#if 0
#define DBG(X) std::cout<<X;
#else
#define DBG(X) 
#endif

namespace fim
{
	extern CommandConsole cc;
}


/*
 *	This code implements the interpreter of the script language
 *	It is triggered by the flex and bison files.
 *
 *	This code will be fully cleaned when the Fim language will settle.
 */
std::ostream & operator<<(std::ostream &os,const nodeType &p)
{
	os<< "type " << p.type << FIM_SYM_ENDL;
	return os;
}

Var ex(nodeType *p);

/*
 *	this function evaluates a single 'arg' entry
 */
Var cvar(nodeType *p)
{
	nodeType *np=p;
  	fim::string arg;
	int i;
	if(p->type == typeOpr && p->opr.oper==FIM_SYM_STRING_CONCAT)
	{
		DBG(".:"<<FIM_SYM_ENDL);
	for(i=0;i<p->opr.nops;++i)
	{
		np=(p->opr.op[i]);
		arg+=(string)(cvar(np).getString());
	}
		return arg;//NEW 20080221
	}
	else
	if(p->type == typeOpr && p->opr.oper=='a' )
	{
		DBG("a:"<<FIM_SYM_ENDL);
	for(i=0;i<p->opr.nops;++i)
	{
		//warning : only 1 should be allowed here..
		np=(p->opr.op[i]);
		arg=cvar(np).getString();
	}
		//return arg;//NEW 20080221
	}
	else
	if(p->type == stringCon )
	{
		DBG("stringCon:"<<FIM_SYM_ENDL);
		arg=(p->scon.s);
		return arg;//NEW 20080221
	}
	else
	if(p->type == vId )
	{	
		DBG("cvId:"<<FIM_SYM_ENDL);
#if 0
		if(0 && p->scon.s && 0==strcmp(p->scon.s,"random"))
		{
			arg=fim_rand();//FIXME
		}
		else
#endif
		//arg=fim::cc.getStringVariable(p->scon.s);
		return fim::cc.getVariable(p->scon.s);
	}
	else if(p->type == intCon )
	{
		/* FIXME : int cast is due to some sick inner conversion */
		DBG("cvar:intCon:"<<p->con.value<<FIM_SYM_ENDL);
		return Var((fim_int)(p->con.value));
	}
	else if(p->type == floatCon)
	{
		DBG("cvar:floatCon:"<<p->fid.f<<FIM_SYM_ENDL);
		return p->fid.f;
	}
	else
	{
		DBG("nest:\n");
		return ex(p);
	}
	return arg;
}

/*
 *	this function evaluates a whole chain of arg entries
 */
std::vector<fim::string> var(nodeType *p)
{
	nodeType *np=p;
  	std::vector<fim::string> args;
	int i;
	if(p->type == typeOpr && np->opr.oper=='a' )
	for(i=0;i<p->opr.nops;++i)
	{
		DBG("'a'args:"<<i<<"/"<<p->opr.nops<<":\n");
		np=(p->opr.op[i]);
		if(np->type == stringCon )
		{
			args.push_back(np->scon.s);
		}
		else
		if(np->type == typeOpr && np->opr.oper=='a' )
		{
		  	std::vector<fim::string> vargs=var(np);
			for(size_t j=0;j<vargs.size();++j) args.push_back(vargs[j]);
		}
		else
		{
			DBG("CVARB\n");
			args.push_back(cvar(np).getString());
			DBG("CVARA\n");
		}
		//return args;
	}
	else
	if(p->type == typeOpr && np->opr.oper==FIM_SYM_STRING_CONCAT)
	for(i=0;i<p->opr.nops;++i)
	{
		DBG("'i'args:"<<i<<"/"<<p->opr.nops<<":\n");
		np=(p->opr.op[i]);
		if(np->type == typeOpr && np->opr.oper==FIM_SYM_STRING_CONCAT)
		{
		  	std::vector<fim::string> vargs=var(np);
			for(size_t j=0;j<vargs.size();++j) args.push_back(vargs[j]);
		}
	}
	else
	{
		DBG("~:\n");
		args.push_back(cvar(np).getString());
	}
	DBG("?:\n");
	return args;
}

using namespace fim;
Var ex(nodeType *p)
{
	fim_int iValue;
	float fValue;
	fim_char_t *s=NULL;

  	std::vector<fim::string> args;
	int typeHint;
	if (!p)
		return (fim_int)0;
	switch(p->type)
	{
		case intCon:
			/* FIXME : are we sure this case executes ? */
			DBG("intCon:\n");
			return (fim_int)p->con.value;
	        case floatCon:
			DBG("ex:floatCon:"<<p->fid.f<<FIM_SYM_ENDL);
			/* FIXME : are we sure this case executes ? */
			return p->fid.f;
		case vId:
		{
			DBG("vId:\n");
			/*
			 * variable identifier encountered
			 * */
#if 0
			if(0 && p->scon.s && 0==strcmp(p->scon.s,"random"))
			       	return fim_rand();//FIXME
			else
#endif
			if(fim::cc.getVariableType(p->scon.s)==FIM_SYM_TYPE_INT)
			{
				DBG("vId:"<<p->scon.s<<":"<<(int)fim::cc.getIntVariable(p->scon.s)<<FIM_SYM_ENDL);
				//return (int)fim::cc.getIntVariable(p->scon.s);
				return fim::cc.getVariable(p->scon.s);
			}

			if(fim::cc.getVariableType(p->scon.s)==FIM_SYM_TYPE_FLOAT)
			{
				DBG("'f':\n");
				//return (float)fim::cc.getFloatVariable(p->scon.s);
				return fim::cc.getVariable(p->scon.s);
			}
			else
			{
				DBG("'s':\n");
			/* if(fim::cc.getVariableType(p->scon.s)=='s') */
				//return (fim::string)fim::cc.getStringVariable(p->scon.s);
				return fim::cc.getVariable(p->scon.s);
			}
		}
		case stringCon:
			DBG("'stringCon':\n");
			// a single string token was encountered
			return Var(p->scon.s);
		case typeOpr:	/*	some operator	*/
			DBG("'typeOpr':\n");
		switch(p->opr.oper)
		{
			case FIM_SYM_STRING_CONCAT:
				// we use the ',' operator
				//return (ex(p->opr.op[0]) , ex(p->opr.op[1]));
				//string collation (we assume these are strings !)
				DBG(".:\n");
				return (ex(p->opr.op[0]) + ex(p->opr.op[1]));
			case WHILE:
				while(ex(p->opr.op[0]).getInt() && (fim::cc.catchLoopBreakingCommand(0)==0))
				{
					ex(p->opr.op[1]);
				}
				return (fim_int)0;
			case IF:
				DBG("IF:"<<(ex(p->opr.op[0]).getInt())<<FIM_SYM_ENDL);
				if (ex(p->opr.op[0]).getInt())
					ex(p->opr.op[1]);
				else if (p->opr.nops > 2)
					ex(p->opr.op[2]);
				return (fim_int)0;
			case FIM_SYM_SEMICOLON:
				/*
				 *		;
				 *             / \
				 *          cmd   cmd
				 * */
				ex(p->opr.op[0]);
				return ex(p->opr.op[1]);
			case 'r': 
			DBG("r\n");
			//if( p->opr.nops ==2 && (p->opr.op[0])->type=='x')
			if( p->opr.nops == 2 )
			{
				int times=ex(p->opr.op[1]).getInt();
				if(times<0)
					return (fim_int)-1;
				for (int i=0;i<times && fim::cc.catchLoopBreakingCommand(0)==0;++i)
				{
					ex(p->opr.op[0]);
				}
			  	return (fim_int)0;	
			}
			else
				return (fim_int)-1;
			case 'x': 
				DBG("X\n");
			  /*
			   * when encountering an 'x' node, the first (left) subtree should 
			   * contain the string with the identifier of the command to 
			   * execute.
			   */
			  {
			  	if( p->opr.nops<1 )
			  {
				  DBG("NO-OP\n");
				  return (fim_int)-1;
			  }
			  if(p->opr.nops==2)	//int yacc.ypp we specified only 2 ops per x node
		          {
				  nodeType *np=p;	
				  //nodeType *dp;
	                          np=(np->opr.op[1]); //the right subtree first node
				  while( np &&    np->opr.nops >=1 )
				  if( np->opr.oper=='a' )
			  	  {
					  std::vector<fim::string> na;
					  na=var(np);
				          for(fim_size_t i=0;i<na.size();++i)
                                          { //std::cout << "?"<<na[i]<<FIM_SYM_ENDL;
					    args.push_back(na[i]);}// FIXME : non sono sicuro che questo serva
					  DBG("A:"<<np->opr.nops<<FIM_SYM_ENDL);
					  break;
#if 0
					  return 0;
					  /*
					   * we descend the right subtree  (the subtree of arguments)
					   * (thus we waste the benefit of the multi argument operator!)
					   */
					  dp=np->opr.op[0];	//we descend 1 step in the left subtree (under arg)
					  dp=dp->opr.op[0];
	                          	  if(np->opr.nops < 2) 
					  {
						np=NULL;
				          }
					  else
					  {
						np=(np->opr.op[1]);
				          }
	                   		  if( ((dp->opr.op[0])) && (dp->type)==stringCon)//|| (dp->type)==intCon) 
					  {	
						  //probably dead code
					  }
	                   		  if( ((dp->opr.op[0])) && (dp->type)==typeOpr)//|| (dp->type)==intCon) 
					  {	
						  //probably dead code
					  }
					  else if( ((dp->opr.oper==FIM_SYM_STRING_CONCAT)))
					  {
				  	//cout <<  "DEAD CODE\n";
						  //probably dead code
				          }
					  else;
					  assert(dp);
#endif
				  }
			  	  else if( np->opr.oper==FIM_SYM_STRING_CONCAT )
				  {
				  	//cout <<  "DEAD CODE\n";
					  //probably dead code
				  }
			  }
			  {
				  DBG("A.\n");
				/*
				 * single command execution
				 */
				fim::string result;
				//std::cout  <<"GULP:"<< p->opr.op[0]->scon.s<< args[0] <<FIM_SYM_ENDL;
//				if(args.size()>0)
//					std::cout  <<"GULP:"<< (int*)p->opr.op[0]->scon.s<<" "<<p->opr.op[0]->scon.s<<" "<<args[0] <<FIM_SYM_ENDL;
//				else
//					std::cout  <<"GULP:"<< args.size() <<FIM_SYM_ENDL;
				if(p)
				if(p->opr.op[0])
				if(p->opr.op[0]->scon.s) result =
				       	fim::cc.execute(p->opr.op[0]->scon.s,args);
				/* sometimes there are NULLs  : BAD !!  */
				return fim_atoi(result.c_str());
			  }
		}
		case 'a':
			// we shouldn't be here, because 'a' (argument) nodes are evaluated elsewhere
			assert(0);
			return (fim_int)-1;
		case '=':
			//assignment of a variable
			s=p->opr.op[0]->scon.s;
			DBG("SV:"<<s<<FIM_SYM_ENDL)
			typeHint=p->opr.op[0]->typeHint;
			if(typeHint==FIM_SYM_TYPE_FLOAT)
			{
				DBG("SVf"<<s<<FIM_SYM_ENDL);
				fValue=p->opr.op[1]->fid.f;
				fim::cc.setVariable(s,fValue);
				return (fim_int)fValue;
			}//FIM_SYM_TYPE_INT
			else if(typeHint=='s')
			{
				DBG("SVs\n");
				if(p->opr.op[1]->type!=stringCon)
				{
					//this shouldn't happen
				}
				else 
				{
					DBG("SVs:"<<s<<":"<<p->opr.op[0]->scon.s<<FIM_SYM_ENDL);
					// got a string!
		       		        fim::cc.setVariable(s,p->opr.op[0]->scon.s);
#if 0
					if(0 && 0==strcmp(s,"random"))
			                	return fim_rand();//FIXME
					else
#endif
			                	//return fim::cc.getIntVariable(s);
			                	return fim::cc.getStringVariable(s);
				}
				return (fim_int)-1;
			}//FIM_SYM_TYPE_INT
			else if(typeHint==FIM_SYM_TYPE_INT)
			{
				iValue=ex(p->opr.op[1]).getInt();
				DBG("SVi:"<<s<<":"<<iValue<<""<<FIM_SYM_ENDL);
				fim::cc.setVariable(s,iValue);
				return iValue;
			}
			else if(typeHint=='v')
			{
				DBG("SVv\n");
				//this shouldn't happen
			}
			else if(typeHint=='a')
			{
				DBG("SVa\n");
				//fim::string r=cvar(p->opr.op[1]);
				Var v=cvar(p->opr.op[1]);
				//iValue=r;
				fim::cc.setVariable(s,v);
			        DBG("SET:"<<s<<":"<<v.getString()<<" '"<<(fim_char_t)v.getType()<<"'\n");
			        DBG("GET:"<<s<<":"<<fim::cc.getVariable(s).getString()<<FIM_SYM_ENDL);
			        //DBG("GET:"<<s<<":"<<fim::cc.getStringVariable(s)<<FIM_SYM_ENDL);

				if(/*want_bugs*/0){
				//fim::cc.setVariable(s,r.c_str());
			        string rs = fim::cc.getStringVariable(s);
				DBG("RS:"<<rs <<FIM_SYM_ENDL);
				return rs;
				}
				else return v;
				// 20080220
				//return iValue;
			}
			else
			{
				DBG("SV?\n");
			}
#if FIM_WANT_AVOID_FP_EXCEPTIONS
			case '%': {Var v1=ex(p->opr.op[0]),v2=ex(p->opr.op[1]); if(v2.getInt())return v1%v2; else return v2;};
			case '/': {Var v1=ex(p->opr.op[0]),v2=ex(p->opr.op[1]); if(v2.getInt())return v1/v2; else return v2;};
#else /* FIM_WANT_AVOID_FP_EXCEPTIONS */
			case '%': return ex(p->opr.op[0]) % ex(p->opr.op[1]); // FIXME: may generate an exception
			case '/': return ex(p->opr.op[0]) / ex(p->opr.op[1]); // FIXME: may generate an exception
#endif /* FIM_WANT_AVOID_FP_EXCEPTIONS */
			case '+': return ex(p->opr.op[0]) + ex(p->opr.op[1]);
			case '!': return (fim_int)(((ex(p->opr.op[0])).getInt())==0?1:0);
			/* unary minus is still under definition */
			case UMINUS:
				return Var((fim_int)0) - ex(p->opr.op[0]);
			case '-': 
				DBG("SUB\n");
				if ( 2==p->opr.nops) {Var d= ex(p->opr.op[0]) - ex(p->opr.op[1]);return d;}
				else return Var((fim_int)0) - ex(p->opr.op[0]);
			case '*': return ex(p->opr.op[0]) * ex(p->opr.op[1]);
			case '<': return ex(p->opr.op[0]) < ex(p->opr.op[1]);
			case '>': return ex(p->opr.op[0]) > ex(p->opr.op[1]);
			//comparison operators : evaluation to integer..
			case GE: return ex(p->opr.op[0]) >= ex(p->opr.op[1]);
			case LE: return ex(p->opr.op[0]) <= ex(p->opr.op[1]);
			case NE: return ex(p->opr.op[0]) != ex(p->opr.op[1]);
			case EQ: {DBG("EQ\n");return ex(p->opr.op[0]) == ex(p->opr.op[1]);}
			case REGEXP_MATCH: return ex(p->opr.op[0]).re_match(ex(p->opr.op[1]));
			case AND:return ex(p->opr.op[0]) && ex(p->opr.op[1]);
			case OR :return ex(p->opr.op[0]) || ex(p->opr.op[1]);
			case BOR :return ex(p->opr.op[0]) | ex(p->opr.op[1]);
			case BAND:return ex(p->opr.op[0]) & ex(p->opr.op[1]);
			/* FIXME: NEED A DEFAULT CASE */
		}	
		case cmdId:/* FIXME : cmdId is dead */
			DBG("cmdId ?!\n");
			return Var((fim_int)0);

	}
	return Var((fim_int)0);
}
