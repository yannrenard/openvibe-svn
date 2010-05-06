////////////////////////////////////////////////////////////////////////////////
//
// File:            ovpCMatrixElementWiseOperation.cpp
// Purpose:         Parsing and Evaluation of complex expression containing
//                  matrix references
// 
// Original Author: Gelu Ionescu
// email:           ionescu@lis.inpg.fr
// secund Author : Matthieu Goyat
//
// License:         
// Copyright:       
//
////////////////////////////////////////////////////////////////////////////////
// 
// This class implements the expression parsing and evaluation using Boost.Spirit.
// 
// It does so by creating an AST and then implementing an evaluating visitor 
// for such nodes.  It also outputs the expression in infix, postfix,
// and prefix notation.  Furthermore it prints an xml coded version of
// the AST as well as a textual representation of the l_tree. 
//
//
////////////////////////////////////////////////////////////////////////////////
//
// Uncomment to enable commandline mode for use in scripts:
// You can then put lines like:
// c () { ~/bin/spirit_evaluator "$*" 1000; }
// t () { ~/bin/spirit_evaluator "$*" 10 1; }
// in your .bashrc to have a handy command line calculator after you
// copy your executable into your bin directory. 
// c 6*6^5 
// outputs
// 46656
// and 
// t 6x6^5
// outputs
// 46656
//   *      
//  / \__   
// 6     ^  
//      / \ 
//     6   5
// 
////////
//
// Uncomment to enable debugging output from spirit's parse system:
//#define BOOST_SPIRIT_DEBUG
//
////////////////////////////////////////////////////////////////////////////////

// Standard Includes:
#include <string>									// string, getline
#include <iostream>									// cout, cerr
#include <sstream>									// istringstream, ostringstream
#include <iomanip>									// setw, setfill
#include <map>										// map
#include <valarray>									// operations
#include <cmath>									// abs, pow, floor
#include <functional>								// multiplies, plus, minus 
#include <algorithm>								// max

// Boost Includes:
#include <boost/lexical_cast.hpp>                      // lexical_cast<>
#include <boost/function.hpp>                          // function<>
#include <boost/spirit/core.hpp>                     // rule, l_ogrammar
//#include <boost/spirit/include/classic_core.hpp>       // rule, l_ogrammar
#include <boost/spirit/tree/parse_tree.hpp>          // pt_parse
//#include <boost/spirit/include/classic_parse_tree.hpp> // pt_parse
#include <boost/spirit/tree/ast.hpp>                 // ast_parse
//#include <boost/spirit/include/classic_ast.hpp>        // ast_parse
#include <boost/spirit/tree/tree_to_xml.hpp>         // tree_to_xml
//#include <boost/spirit/include/classic_tree_to_xml.hpp>// tree_to_xml
#include <boost/spirit/error_handling/exceptions.hpp>  // assertion, 
                                                       // parser_error,
                                                       // throw_

using namespace boost::spirit;
using namespace boost;
using namespace std;

#include "ovpCMatrixElementWiseOperation.h"

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::WipGionescu;



// Errors to check for during the parse:
enum Errors
{	close_expected,
	expression_expected,
};

// Assertions to use during the parse:
assertion<Errors> expect_close(close_expected);
assertion<Errors> expect_expression(expression_expected);

struct Pointer
{
    Pointer(const OpenViBE::float64* _ptr = 0)
		: ptr(_ptr) {}

    OpenViBE::float64 operator()() const
    {
        return *ptr;
    }

	void operator ++()				{	ptr++;		}
	void set(const OpenViBE::float64* _ptr)	{	ptr = _ptr;	}

    mutable const OpenViBE::float64* ptr;
};


struct MyGrammar : public grammar<MyGrammar>
{	// Explicit identifiers to switch properly when evaluating the AST
	typedef enum
	{	noID,
		factorID,
		termID,
		expressionID,
		unaryID,
		variableID,
		numberID,
	} id_type;

	static size_t g_pointersSize;
	static std::vector<Pointer> g_pointers;
	static std::map<std::string, size_t> g_indexes;
	
		// Map binary operators to operations
	static	std::map<char, boost::function<OpenViBE::float64 (OpenViBE::float64, OpenViBE::float64)> > op;
		// Map unary operators to operations
	static	std::map<char, boost::function<OpenViBE::float64 (OpenViBE::float64)> >        uop;
	
	MyGrammar(size_t parametre=0)
	{
	g_pointersSize=parametre;
	}
	
	// Meta function from scanner type to a proper rule type
	template<typename ScannerT> struct definition
	{	rule<ScannerT, parser_context<>, parser_tag<factorID > >		factor_p;
		rule<ScannerT, parser_context<>, parser_tag<termID > >			term_p;
		rule<ScannerT, parser_context<>, parser_tag<expressionID > >	expression_p;
		rule<ScannerT, parser_context<>, parser_tag<unaryID > >			unary_p;
		rule<ScannerT, parser_context<>, parser_tag<variableID > >		variable_p;
		rule<ScannerT, parser_context<>, parser_tag<numberID > >		number_p;
		rule<ScannerT> start_p;

		// Arithmetic expression l_ogrammar:
		definition(const MyGrammar& self)
		{	factor_p		// numbers, variables or parentheticals
				= number_p 
				| variable_p 
				| ( inner_node_d['(' >> expect_expression(start_p) >> expect_close(ch_p(')'))] );

			unary_p			// unary operators (right-to-left)
				= (root_node_d[ch_p('-')]) >> expect_expression(unary_p)
				| factor_p;

			term_p			// multiplicatives (left-to-right) 
				= unary_p >> *(root_node_d[ch_p('*')|'/'] >> expect_expression(unary_p));

			expression_p	// additives       (left-to-right)
				= term_p  >> *(root_node_d[ch_p('+')|'-'] >> expect_expression(term_p));

			number_p
				= leaf_node_d[real_p];

			variable_p
				= leaf_node_d[ lexeme_d[as_lower_d[chlit<>('m')] >> max_limit_d(g_pointersSize - 1)[uint_p] ] ];
		
			start_p
				= expect_expression(expression_p);

			#ifdef BOOST_SPIRIT_DEBUG
			BOOST_SPIRIT_DEBUG_RULE(factor_p);
			BOOST_SPIRIT_DEBUG_RULE(term_p);
			BOOST_SPIRIT_DEBUG_RULE(unary_p);
			BOOST_SPIRIT_DEBUG_RULE(number_p);
			BOOST_SPIRIT_DEBUG_RULE(variable_p);
			BOOST_SPIRIT_DEBUG_RULE(expression_p);
			BOOST_SPIRIT_DEBUG_RULE(start_p);
			#endif
		}

		// Specify the starting rule for the parse
		const rule<ScannerT> & start() const { return start_p; }
	};
};

size_t								MyGrammar::g_pointersSize=0;
std::vector<Pointer>				MyGrammar::g_pointers;
std::map<std::string, size_t>		MyGrammar::g_indexes;

std::map<char, boost::function<OpenViBE::float64 (OpenViBE::float64, OpenViBE::float64)> > MyGrammar::op;
std::map<char, boost::function<OpenViBE::float64 (OpenViBE::float64)> >        MyGrammar::uop;
		
// Convert AST to fully parenthesized infix notation:
string infix(const tree_iter_t& i)
{
	string	l_bret = string(i->value.begin(), i->value.end());
	
	if(i->value.id() == MyGrammar::factorID) // Simple numeric literal
		{;}
	else if(i->value.id() == MyGrammar::numberID)
		{;}
	else if(i->value.id() == MyGrammar::variableID)
		{;}
	else if(i->value.id() == MyGrammar::unaryID) // Unary expression
	{	tree_iter_t j = i->children.begin();
    	l_bret =	"(" +
					string(i->value.begin(),i->value.end()) +
					infix(j) +
				")";
	}
	else
    {	// Binary expression
    	tree_iter_t j = i->children.begin();
    	tree_iter_t k = i->children.begin()+1;
       	
		l_bret =	"(" + 
					infix(j) + " " +
					string(i->value.begin(),i->value.end()) + " " +
					infix(k) +
                ")";
	}
	
	return l_bret;
}

string infix(tree_parse_info<> t)
{
	return infix(t.trees.begin());
}

// Convert AST to parenthesized prefix notation:
string prefix(const tree_iter_t& i)
{
	string	l_bret = string(i->value.begin(), i->value.end());
	
	if (i->value.id() == MyGrammar::factorID)				// Simple numeric literal
		{;}
	else if(i->value.id() == MyGrammar::numberID)
		{;}
	else if(i->value.id() == MyGrammar::variableID)
		{;}
	else if (i->value.id() == MyGrammar::unaryID)			// Unary expression
    	{
			l_bret = 	"(" +
					string(i->value.begin(),i->value.end()) + " " +
					prefix(i->children.begin()) +
                ")";
		}
	else													// Binary expression
		{
			l_bret = 	"(" +
					string(i->value.begin(),i->value.end()) + " " +
					prefix(i->children.begin()) + " " +
					prefix(i->children.begin()+1) + 
                ")";
		}
				
	return l_bret;
}

string prefix(tree_parse_info<> t)
{
	return prefix(t.trees.begin());
}

// Convert AST to postfix notation:
string postfix(const tree_iter_t& i)
{
	string	l_bret = string(i->value.begin(), i->value.end());
	
	if (i->value.id() == MyGrammar::factorID)				// Simple numeric literal
		{;}
	else if(i->value.id() == MyGrammar::numberID)
		{;}
	else if(i->value.id() == MyGrammar::variableID)
		{;}
	else if (i->value.id() == MyGrammar::unaryID)			// Unary expression
    	{
			l_bret = 	postfix(i->children.begin()) + " " +
           		string(i->value.begin(),i->value.end())
           		;
		}
	else													// Binary expression
		{
			l_bret = 	postfix(i->children.begin()) + " " +
           		postfix(i->children.begin()+1) + " " +
           		string(i->value.begin(),i->value.end())
          		;
		}
  	
	return l_bret;
}

string postfix(tree_parse_info<> t)
{
	return postfix(t.trees.begin());
}

// Helpers for l_tree printing:
//typedef pair<string,std::pair<unsigned int,unsigned int> > return_t;
struct return_t
{	string			s;
	unsigned int	l;
	unsigned int	r;
 	
	return_t(const string& _s, unsigned int _l, unsigned int _r)
	 	: s(_s)
		, l(_l)
		, r(_r)
		{ }
  	return_t()
		{ }
	void operator =(const return_t& src) {	s = src.s; l = src.l; r = src.r;	}
};

static return_t print(const tree_iter_t&);

// The print functions return a string containing a multiline l_tree
// representation of the AST.  It works by recursively building up
// blocks of text containing sub-expressions and then merging the
// subexpressions.  The alignment is based on the assumption that all
// operators are one character wide.
string print(tree_parse_info<> t)
{
	return print(t.trees.begin()).s;
}

return_t print(const tree_iter_t& i)
{
	return_t	l_bret;

	string 		l	 = string(i->value.begin(), i->value.end());

	if (i->value.id() == MyGrammar::factorID)				// Numeric literal
	{	unsigned int	s	 = l.size();
    	unsigned int 	half = static_cast<unsigned int>(std::floor(((OpenViBE::float64)l.size())/2.0));
    	
		l_bret = return_t(l + "\n", half, s - half);
	} 
  	else if (i->value.id() == MyGrammar::numberID)
	{	unsigned int	s	 = l.size();
    	unsigned int 	half = static_cast<unsigned int>(std::floor(((OpenViBE::float64)l.size())/2.0));
    	
		l_bret = return_t(l + "\n", half, s - half);
	} 
  	else if (i->value.id() == MyGrammar::variableID)
	{	unsigned int	s	 = l.size();
    	unsigned int 	half = static_cast<unsigned int>(std::floor(((OpenViBE::float64)l.size())/2.0));
    	
		l_bret = return_t(l + "\n", half, s - half);
	} 
	else if (i->value.id() == MyGrammar::unaryID)			// Unary expression
	{	return_t operand = print(i->children.begin());

		ostringstream oss;
    	oss << setw(operand.l) << "" 
        	<< string(i->value.begin(),i->value.end()) 
        	<< setw(operand.r-1) << "" << "\n"; 
    	oss << setw(operand.l) << "" 
        	<< "|" 
       		<< setw(operand.r-1) << "" << "\n"; 
    	oss << operand.s ; 

		
		l_bret	= return_t(oss.str(), operand.l, operand.r);
	}
	else													// Binary expression
	{	return_t lrand = print(i->children.begin());
    	return_t rrand = print(i->children.begin()+1);
    	istringstream	left(lrand.s);
    	istringstream	right(rrand.s);
    	string 			l, r;
    	getline(left, l);
    	getline(right,r);
    	unsigned int lsize = l.size();
    	unsigned int rsize = r.size();
    	unsigned int gap = (lsize < 3 || rsize < 2) ? 3 : 1;

	   	ostringstream o;
    	// output operator row
    	o	<< setw(lsize+(gap == 1 ? 0 : 1)) << "" 
      		<< string(i->value.begin(),i->value.end()) 
      		<< setw(rsize+(gap == 1 ? 0 : 1)) << "" << "\n";
    	// output arrow row
    	o	<< setw(lrand.l+(gap == 1 ?  1 :  1)) << "" 
      		<< setw(lrand.r+(gap == 1 ? -2 : -1)) << setfill('_') << "" 
      		<< "/ \\" 
      		<< setw(rrand.l+(gap == 1 ? -1 :  0)) << setfill('_') << "" 
      		<< setw(rrand.r+(gap == 1 ?  0 :  0)) << setfill(' ') << "" << "\n";
    	// output subexpression rows
		do
		{	if (l.empty())
				o << setw(lsize+0) << "";
			else
				o << "" << l;
     		
			o << setw(gap) << "";
      
	  		if (r.empty())
				o << setw(rsize+0) << " ";
			else
				o << r;
      		
			o << '\n';
      
      		if (left)
				getline(left,l);
			if (left.eof())
				l.erase();
      		
			if (right)
				getline(right,r);
			if (right.eof())
				r.erase();
    	} while (right || left);
    
		l_bret	= return_t(o.str(), lsize + (gap==1 ? 0 : 1), rsize + (gap==1 ? 1 : 2));
	}
 	
	return l_bret;
}

// Convert AST to xml
string xmlout(tree_parse_info<> t)
{ 
	// tell tree_to_xml how to print each of the rules
	std::map<parser_id, std::string> rule_names;
	rule_names[MyGrammar::factorID]			= "factor";
	rule_names[MyGrammar::termID]			= "term";
	rule_names[MyGrammar::expressionID]		= "expression";
	rule_names[MyGrammar::unaryID]			= "unary";
	rule_names[MyGrammar::numberID]			= "constant";
	rule_names[MyGrammar::variableID]		= "variable";

	ostringstream out;
	tree_to_xml(out, t.trees, infix(t), rule_names);
	
	return out.str();
}            

// Exception class for division by zero
struct divide_by_zero : public std::runtime_error
{
	divide_by_zero()
  		: runtime_error("division by zero")
		{}
  	divide_by_zero(const std::string& _what)
		:	runtime_error(_what)
		{}
};

// Exception class for root_of_negative
struct root_of_negative : public std::runtime_error
{
	root_of_negative()
		: runtime_error("fractional root of negative")
		{}
  	root_of_negative(const std::string& _what)
		: runtime_error(_what)
		{}
};

// The evaluation function for the AST
OpenViBE::float64 CMatrixElementWiseOperation::evaluate(const tree_iter_t& i)
{
	OpenViBE::float64 l_bret = 0;
	
	string	value = string(i->value.begin(), i->value.end());

	if (i->value.id() == MyGrammar::factorID)		// Simple numeric literal    
    	{;}
 	else if (i->value.id() == MyGrammar::numberID)
    	{l_bret = atof(value.c_str());}
 	else if (i->value.id() == MyGrammar::variableID)
		{l_bret = MyGrammar::g_pointers[MyGrammar::g_indexes.find(value)->second]();}
	else if (i->value.id() == MyGrammar::unaryID)	// Unary expression
   		{l_bret = MyGrammar::uop[*i->value.begin()](evaluate(i->children.begin()));}
	else											// Binary expression
	{	try
		{	
			l_bret = MyGrammar::op[*i->value.begin()](evaluate(i->children.begin()), evaluate(i->children.begin() + 1));
    	}
		catch (divide_by_zero& dvz)
		{	
			divide_by_zero ex(std::string(dvz.what()) + std::string(" in ") + infix(i));
			//std::cout << ex.what() << std::endl;
	}	}
	
	return l_bret;
}

OpenViBE::float64 CMatrixElementWiseOperation::evaluate(tree_parse_info<> t)
{
	return evaluate(t.trees.begin());
}

// Division function object
template <typename T>
struct divides
{
	divides() : verifError(1.0e-12) {}

  	T operator()(const T& l, const T& r)
	{	
		if (fabs(r) < verifError)
		{
      		throw divide_by_zero();
    	}
    	
		return l/r;
  	}

	OpenViBE::float64 verifError;
};

CMatrixElementWiseOperation::CMatrixElementWiseOperation(void)
{
}

bool CMatrixElementWiseOperation::parse(const int nbEntries, const char* l_cformula, const bool debug, OpenViBE::Kernel::ILogManager & log)
{
	bool	l_bret		= true;
	MyGrammar::g_pointersSize	= nbEntries;

    MyGrammar l_ogrammar(nbEntries);
    
	// Most errors can be caught in the parse with parser exceptions
    try
	{
		tree_parse_info<const char*> l_tree = ast_parse(l_cformula, l_ogrammar, space_p);
		if(debug)
			{
			 string str=print(l_tree);
			 log << OpenViBE::Kernel::LogLevel_Debug << "l_tree:\n"  << str.c_str() << "\n";
			}
    }
	catch (parser_error<Errors, char const*> x)
	{	switch (x.descriptor)
		{	case close_expected:
				log << OpenViBE::Kernel::LogLevel_Warning << "Expected close parenthesis" << "\n";
				break;
			case expression_expected:
				log << OpenViBE::Kernel::LogLevel_Warning << "Expected operand" << "\n";
				break;
		}
	
		//std::cout << "-->| " << l_cformula << endl;
		//std::cout << "at | " << setw(std::distance(l_cformula, x.where) + 1) 
			//				 << "^" << endl;
		log << OpenViBE::Kernel::LogLevel_Debug << "-->| " << l_cformula << "\n";
		string str; for(int i=0; i<std::distance(l_cformula, x.where) + 1; i++) {str.push_back(' ');}
		log<< "at | " << str.c_str() << "^" << "\n";
		
		l_bret	= false;
    }

	return l_bret;
}
	
bool  CMatrixElementWiseOperation::evaluate(OpenViBE::IMatrix& rResult, const std::vector<OpenViBE::IMatrix*>& rImputs, const OpenViBE::CString& rFormula,OpenViBE::Kernel::ILogManager & log)
{
	const char* l_cformula = rFormula;

	if(MyGrammar::op.size() == 0)
	{	// Initialize the binary operators:
		MyGrammar::op['*'] = multiplies<OpenViBE::float64>();
		MyGrammar::op['/'] = ::divides<OpenViBE::float64>();
		MyGrammar::op['+'] = plus<OpenViBE::float64>();
		MyGrammar::op['-'] = minus<OpenViBE::float64>();

		// Initialize the unary operators:
		MyGrammar::uop['-'] = negate<OpenViBE::float64>();
	}

	MyGrammar::g_pointers.clear();
	MyGrammar::g_indexes.clear();

	int ii = 0;
	for(std::vector<OpenViBE::IMatrix*>::const_iterator it=rImputs.begin(); it != rImputs.end(); it++)
	{	MyGrammar::g_pointers.push_back((*it)->getBuffer());

		std::ostringstream os;
		os << 'm' << ii;
		MyGrammar::g_indexes.insert(pair<std::string, size_t>(os.str(), ii++));
        
        if((ii == 0) && (rResult.getDimensionCount() == 0))
        {   rResult.setDimensionCount((*it)->getDimensionCount());
	        for(int i=0; i < (*it)->getDimensionCount(); i++)
                {rResult.setDimensionSize(i, (*it)->getDimensionSize(i));}
	}   }

	bool	l_bret		= true;
	MyGrammar::g_pointersSize	= rImputs.size();

    MyGrammar						l_ogrammar(rImputs.size());
	tree_parse_info<const char*>	l_tree;    
	// Most errors can be caught in the parse with parser exceptions
    try
	{
		l_tree = ast_parse(l_cformula, l_ogrammar, space_p);
    }
	catch (parser_error<Errors, char const*> x)
	{	switch (x.descriptor)
		{	case close_expected:
				log << OpenViBE::Kernel::LogLevel_Warning << "Expected close parenthesis" << "\n";
				break;
			case expression_expected:
				log << OpenViBE::Kernel::LogLevel_Warning << "Expected operand" << "\n";
				break;
		}
	
		log << OpenViBE::Kernel::LogLevel_Debug << "-->| " << rFormula << "\n";
		string str; for(int i=0; i<std::distance(l_cformula, x.where) + 1; i++) {str.push_back(' ');}
		log<< "at | " << str.c_str() << "^" << "\n";
			
		l_bret	= false;
    }

    if(l_bret && l_tree.full)
	{	try
		{	int					size = rResult.getBufferElementCount();
			OpenViBE::float64*	data = rResult.getBuffer();
			
			for(int ii=0; ii < size; ii++, data++)
			{	*data	= evaluate(l_tree);
				
				for(size_t jj=0; jj < MyGrammar::g_pointers.size(); jj++)
					++MyGrammar::g_pointers[jj];
			}
		}
		catch (std::runtime_error& e)
		{
			log << OpenViBE::Kernel::LogLevel_Debug <<e.what() << "\n";
		}
	}
	else	// Or was the expression rejected?
	{	// But open parenthesis and missing operators are very hard to
		// check for during the parse, so it is easier to check 
		// for an incomplete parse afterwards:
		switch (*l_tree.stop)
		{	case ')':
				//cout << "Expected open parenthesis\n";
				log << OpenViBE::Kernel::LogLevel_Warning << "Expected open parenthesis\n";
				break;
			case '(':
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				//cout << "Expected binary operator\n";
				log << OpenViBE::Kernel::LogLevel_Warning << "Expected binary operator\n";
				break;
			default:
				//cout << "Not a number\n";
				log << OpenViBE::Kernel::LogLevel_Warning << "Not a number\n";
	}	}

	return l_bret;
}
	
void CMatrixElementWiseOperation::debug(const std::vector<OpenViBE::IMatrix*>& rImputs, const OpenViBE::IMatrix& rResult, OpenViBE::Kernel::ILogManager & log)
{
	for(std::vector<OpenViBE::IMatrix*>::const_iterator it=rImputs.begin(); it != rImputs.end(); it++)
    {   const OpenViBE::float64* ptr = (*it)->getBuffer();
        
		log << OpenViBE::Kernel::LogLevel_Debug << "in 0x" << ptr << " (dim = " << (*it)->getBufferElementCount() << ") = ";
        for(int ii=0; ii < 5; ii++)
            {
			 log <<*ptr++ << " ";
			}
		log << "\n";
    }
    
	log << OpenViBE::Kernel::LogLevel_Debug << "out 0x" << rResult.getBuffer() << " (dim = " << rResult.getBufferElementCount() << ") = ";
    const OpenViBE::float64* ptr = rResult.getBuffer();
    for(int ii=0; ii < 5; ii++)
        {
		 log << *ptr++ << " ";
		}
	log << "\n";
	
}


