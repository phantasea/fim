/* $LastChangedDate: 2015-02-13 00:24:59 +0100 (Fri, 13 Feb 2015) $ */
/*
 Var.h : Var class header file

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

/*
 * This class is horrible. 
 */
#ifndef FIM_VAR_H
#define FIM_VAR_H

//#include "fim.h"
#include "string.h"

#define FIM_FFL_PRT printf("In %s located in %20s:%d :\n",__func__,__FILE__,__LINE__)
#if 0
#define DBG(X) FIM_FFL_PRT;std::cout<<X;
#else
#define DBG(X) 
#endif

namespace fim
{
class Var
{
	int type;
	union
	{
		float f;
		fim_int i;
	};
	fim::string s;
	public:
	Var(const Var &v)
	:type(0),i(0),s(v.s)
	{
		this->set(v);
	}

	int   set(const Var &v)
	{
		DBG("(v:?)\n");
		this->type=v.type;
		if(type=='i')
			this->i=v.i;
		if(type=='f')
			this->f=v.f;
		if(type=='s')
			this->s=v.s;
		return 0;
	}

	Var(float v)
	:type(0),i(0),s(fim::string())
	{
		DBG("(f:"<<v<<")\n");
		type='f';
		f=v;
	}

	Var(bool v)
	:type(0),i(0),s(fim::string())
	{
		DBG("(i:"<<v<<")\n");
		type='i';
		i=v?1:0;
	}

	Var(fim_int v)
	:type(0),i(0),s(fim::string())
	{
		DBG("(i:"<<v<<")\n");
		type='i';
		i=v;
	}

	Var(void)
	:type(0),i(0),s(fim::string())
	{
		const fim_char_t *s="0";
		DBG("(v())\n");
		type='i';
		if(type=='i')i=fim_atoi(s);
		else
		       	if(type=='f')
				f=fim_atof(s);
		else
		       	if(type=='s')
			this->s=s;
		else
		       	i=0;
	}

	Var(const fim::string s)
	:type(0),i(0),s(fim::string())
	{
		type='s';
		this->s=s.c_str();
	}

	Var(const fim_char_t*s)
	:type(0),i(0),s(fim::string())
	{
		type='s';
		this->s=s;
	}
/*
	Var(const fim_char_t*s="0",int type_='i')
	{
		type=type_;
		if(type=='i')i=fim_atoi(s);
		else if(type=='f')f=fim_atof(s);
		else if(type=='s')this->s=s;
		else i=0;
	}
*/
/*	void operator= (int   i){if(type=='i')this->i=i;}
	void operator= (float f){if(type=='f')this->f=f;}*/
#if 0
	void operator= (int   i){type='i';this->i=i;}
	void operator= (float f){setFloat(f);}
	void operator= (fim::string &s){setString(s);}
#else
	const Var& operator= (int   i){DBG("2i:"<<i<<"\n";type='i');this->i=i;return *this;}
	const Var& operator= (float f){setFloat(f);return *this;}
	const Var& operator= (fim::string &s){setString(s);return *this;}
	/* const Var& operator= (const Var &v){type=v.type;;return *this;} */
	const Var& operator= (const Var &v){set(v);return *this;}
	Var concat(const Var &v)const{return this->getString()+v.getString();}
#endif
	float setFloat(float f){type='f';return this->f=f;}
	fim_int   setInt(fim_int i){type='i';return this->i=i;}
	fim::string setString(fim::string &s){type='s';this->s=s;return this->s;}
	int getType(void)const{return type;}
	fim_int getInt(void)const{return(type=='i')?i:
		(type=='f'?((fim_int)(f)):
		 (type=='s'?(atoi(s.c_str())):0)
		 )
		;}

	float getFloat(void)const{
	
	return(type=='f')?f:
		(type=='i'?
		 	((float)i):
			((type=='s')?fim_atof(s.c_str()):0.0f)
			)
			;}

	fim::string getString(void)const
	{
		fim_char_t buf[16];
		DBG("t:"<<(char)type <<"\n");
		if(type=='s')return this->s;
		else
		{
			if(type=='i')
			{
				if(sizeof(fim_int)==sizeof(int))
					sprintf(buf,"%d",(int)i);
				else
					sprintf(buf,"%lld",(long long int)i);
			}
			else
			       	if(type=='f')
					sprintf(buf,"%f",f);
			DBG(" v:"<<buf <<"\n");
			return buf;
		}
		
	}

//	operator int(void)const{return getInt();}
///	operator float(void)const{return getFloat();}
//	operator string(void)const{return getString();}

	/*
	 * These should be refined some day :)
	 * */
	Var  operator<=(const Var &v)const { return getFloat()<=v.getFloat(); }
	Var  operator>=(const Var &v)const { return getFloat()>=v.getFloat(); }
	Var  operator< (const Var &v)const { return getFloat()< v.getFloat(); }
	Var  operator> (const Var &v)const { return getFloat()> v.getFloat(); }
	#define _both(t) (((getType()==t) && (v.getType()==t)))
	#define _types() DBG("t1:"<<(getType())<<",t2:"<<(v.getType())<<"\n");
	#define _some_string() (getType()=='s' || v.getType()=='s')
	#define _numeric() (!_some_string())
	#define _p_t(op) DBG(op<<"("<<(fim_char_t)getType()<<","<<(fim_char_t)v.getType()<<")\n");
	Var operator!=(const Var &v)const {
		//_p_t("!=")
		_types() 
		if(_both('i'))return getInt  () !=v.getInt  (); 
		if(_both('f'))return getFloat() !=v.getFloat();
		if(_both('s'))
		{
			return getString()!=v.getString(); 
		}
		return getFloat()!=v.getFloat();
	}
	Var operator==(const Var &v)const {DBG("EQV\n"); return 1-(*this != v).getInt(); }
	Var operator/ (const Var &v)const
	{
		if(_both('i')) return getInt()/(v.getInt()!=0?v.getInt():1); 
		return getFloat()/(v.getFloat()!=0.0?v.getFloat():1); 
	}
	Var operator* (const Var &v)const
	{
		if(_both('i'))return getInt  ()*v.getInt  (); 
		if(_both('f'))return getFloat()*v.getFloat();
		if(_both('s'))return getFloat()*v.getFloat();
		return getFloat()*v.getFloat(); 
	}
	Var operator| (const Var &v)const
	{
		return getInt  ()|v.getInt  (); 
	}
	Var operator& (const Var &v)const
	{
		return getInt  ()&v.getInt  (); 
	}
	Var operator+ (const Var &v)const
	{
		if(_both('i'))return getInt  ()+v.getInt  (); 
		if(_both('f'))return getFloat()+v.getFloat();
		if(_both('s'))return getString()+v.getString();//yes...
		return getFloat()+v.getFloat(); 
	}
	Var operator- (const Var &v)const
	{
		DBG("SUB"<<getType()<<"-"<<v.getType()<<"\n"); 
		DBG("SUB"<<getFloat()<<"-"<<v.getFloat()<<"\n"); 
		//std::cout<<"t1:"<<(getType())<<",t2:"<<(v.getType())<<"\n";
		if(_both('i'))return getInt  ()-v.getInt  (); 
		if(_both('f'))return getFloat()-v.getFloat();
		if(_both('s'))return getFloat()-v.getFloat();//yes...
		return getFloat()-v.getFloat(); 
	}
	int eq (const char*s)const
	{
		int aeq = 1;
		if(s)
			aeq = strcmp(s,this->getString().c_str());
		return aeq == 0;
	}
	int re (const Var &v)const
	{
		return re_match(v).getInt();
	}
/*
	Var operator< (const Var &v)const
	{
		if(_both('i'))return getInt  ()<v.getInt  (); 
		if(_both('f'))return getFloat()<v.getFloat();
		if(_both('s'))return getString()<v.getString();//yes...
		return getFloat()+v.getFloat(); 
	}
	*/
	Var operator- (void)const {
	if(getType()=='i')return - getInt  (); 
	if(getType()=='f')return - getFloat(); 
	if(getType()=='s')return - getFloat(); 
	}
	Var operator% (const Var &v)const { return getInt()%v.getInt(); }
//	Var operator, (const Var &v)const { return (getString()+v.getString()); }
	Var operator&&(const Var &v)const { return getInt()&&v.getInt(); }
	Var operator||(const Var &v)const { return getInt()||v.getInt(); }
	Var re_match(const Var &v)const { return getString().re_match(v.getString().c_str()); }
	#undef _p_t
	#undef _numeric
	#undef _some_string
	#undef _both
	#undef _types
/*	Var operator==(const Var &v)const
	{
		return (type==v.getType()) && (i==v.getInt());
	}*/
	std::ostream& print(std::ostream &os)const;
};
	fim::string fim_var_help_db_query(const fim::string &id);
	void fim_var_help_db_init(void);
	fim::string fim_get_variables_reference(FimDocRefMode refmode);
	std::ostream& operator<<(std::ostream &os, const Var & var);
}


#endif /* FIM_VAR_H */
