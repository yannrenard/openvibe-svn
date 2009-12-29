////////////////////////////////////////////////////////////////////////////////
//
// File:            ovpCMatrixElementWiseOperation.cpp
// Purpose:         Parsing and Evaluation of compex expression containing
//                  matrix references
// 
// Original Author: Gelu Ionescu
// email:           ionescu@lis.inpg.fr
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
// the AST as well as a textual representation of the tree. 
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
#include <boost/lexical_cast.hpp>                     // lexical_cast<>
#include <boost/function.hpp>                         // function<>
#include <boost/spirit/core.hpp>                      // rule, grammar
#include <boost/spirit/tree/parse_tree.hpp>           // pt_parse
#include <boost/spirit/tree/ast.hpp>                  // ast_parse
#include <boost/spirit/tree/tree_to_xml.hpp>          // tree_to_xml
#include <boost/spirit/error_handling/exceptions.hpp> // assertion, 
                                                      // parser_error,
                                                      // throw_

using namespace boost::spirit;
using namespace boost;
using namespace std;

#include "ovpCMatrixElementWiseOperation.h"

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::WipGionescu;

typedef tree_match<const char*>    treematch_t;
typedef treematch_t::tree_iterator tree_iter_t;

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

std::vector<Pointer>				pointers;
std::map<std::string, size_t>		indexes;
size_t								pointersSize;

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

	// Meta function from scanner type to a proper rule type
	template<typename ScannerT> struct definition
	{	rule<ScannerT, parser_context<>, parser_tag<factorID > >		factor_p;
		rule<ScannerT, parser_context<>, parser_tag<termID > >			term_p;
		rule<ScannerT, parser_context<>, parser_tag<expressionID > >	expression_p;
		rule<ScannerT, parser_context<>, parser_tag<unaryID > >			unary_p;
		rule<ScannerT, parser_context<>, parser_tag<variableID > >		variable_p;
		rule<ScannerT, parser_context<>, parser_tag<numberID > >		number_p;
		rule<ScannerT> start_p;

		// Arithmetic expression grammar:
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
				= leaf_node_d[ lexeme_d[as_lower_d[chlit<>('m')] >> max_limit_d(pointersSize - 1)[uint_p] ] ];
		
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

// Map binary operators to operations
static std::map<char, function<OpenViBE::float64 (OpenViBE::float64, OpenViBE::float64)> > op;

// Map unary operators to operations
static std::map<char, function<OpenViBE::float64 (OpenViBE::float64)> >        uop;

// Convert AST to fully parenthesized infix notation:
string infix(const tree_iter_t& i)
{
	string	ret = string(i->value.begin(), i->value.end());
	
	if(i->value.id() == MyGrammar::factorID) // Simple numeric literal
		;
	else if(i->value.id() == MyGrammar::numberID)
		;
	else if(i->value.id() == MyGrammar::variableID)
		;
	else if(i->value.id() == MyGrammar::unaryID) // Unary expression
	{	tree_iter_t j = i->children.begin();
    	ret =	"(" +
					string(i->value.begin(),i->value.end()) +
					infix(j) +
				")";
	}
	else
    {	// Binary expression
    	tree_iter_t j = i->children.begin();
    	tree_iter_t k = i->children.begin()+1;
       	
		ret =	"(" + 
					infix(j) + " " +
					string(i->value.begin(),i->value.end()) + " " +
					infix(k) +
                ")";
	}
	
	return ret;
}

string infix(tree_parse_info<> t)
{
	return infix(t.trees.begin());
}

// Convert AST to parenthesized prefix notation:
string prefix(const tree_iter_t& i)
{
	string	ret = string(i->value.begin(), i->value.end());
	
	if (i->value.id() == MyGrammar::factorID)				// Simple numeric literal
		;
	else if(i->value.id() == MyGrammar::numberID)
		;
	else if(i->value.id() == MyGrammar::variableID)
		;
	else if (i->value.id() == MyGrammar::unaryID)			// Unary expression
    	ret = 	"(" +
					string(i->value.begin(),i->value.end()) + " " +
					prefix(i->children.begin()) +
                ")";
	else													// Binary expression
		ret = 	"(" +
					string(i->value.begin(),i->value.end()) + " " +
					prefix(i->children.begin()) + " " +
					prefix(i->children.begin()+1) + 
                ")";
				
	return ret;
}

string prefix(tree_parse_info<> t)
{
	return prefix(t.trees.begin());
}

// Convert AST to postfix notation:
string postfix(const tree_iter_t& i)
{
	string	ret = string(i->value.begin(), i->value.end());
	
	if (i->value.id() == MyGrammar::factorID)				// Simple numeric literal
		;
	else if(i->value.id() == MyGrammar::numberID)
		;
	else if(i->value.id() == MyGrammar::variableID)
		;
	else if (i->value.id() == MyGrammar::unaryID)			// Unary expression
    	ret = 	postfix(i->children.begin()) + " " +
           		string(i->value.begin(),i->value.end())
           		;
	else													// Binary expression
		ret = 	postfix(i->children.begin()) + " " +
           		postfix(i->children.begin()+1) + " " +
           		string(i->value.begin(),i->value.end())
          		;
  	
	return ret;
}

string postfix(tree_parse_info<> t)
{
	return postfix(t.trees.begin());
}

// Helpers for tree printing:
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

// The print functions return a string containing a multiline tree
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
	return_t	ret;

	string 		l	 = string(i->value.begin(), i->value.end());

	if (i->value.id() == MyGrammar::factorID)				// Numeric literal
	{	unsigned int	s	 = l.size();
    	unsigned int 	half = static_cast<unsigned int>(std::floor(((OpenViBE::float64)l.size())/2.0));
    	
		ret = return_t(l + "\n", half, s - half);
	} 
  	else if (i->value.id() == MyGrammar::numberID)
	{	unsigned int	s	 = l.size();
    	unsigned int 	half = static_cast<unsigned int>(std::floor(((OpenViBE::float64)l.size())/2.0));
    	
		ret = return_t(l + "\n", half, s - half);
	} 
  	else if (i->value.id() == MyGrammar::variableID)
	{	unsigned int	s	 = l.size();
    	unsigned int 	half = static_cast<unsigned int>(std::floor(((OpenViBE::float64)l.size())/2.0));
    	
		ret = return_t(l + "\n", half, s - half);
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

		
		ret	= return_t(oss.str(), operand.l, operand.r);
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
    	unsigned int gap = (lsize < 3 or rsize < 2) ? 3 : 1;

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
    	} while (right or left);
    
		ret	= return_t(o.str(), lsize + (gap==1 ? 0 : 1), rsize + (gap==1 ? 1 : 2));
	}
 	
	return ret;
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
OpenViBE::float64 evaluate(const tree_iter_t& i)
{
	OpenViBE::float64 ret = 0;
	
	string	value = string(i->value.begin(), i->value.end());

	if (i->value.id() == MyGrammar::factorID)		// Simple numeric literal    
    	;
 	else if (i->value.id() == MyGrammar::numberID)
    	ret = atof(value.c_str());
 	else if (i->value.id() == MyGrammar::variableID)
		ret = pointers[indexes.find(value)->second]();
	else if (i->value.id() == MyGrammar::unaryID)	// Unary expression
   		ret = uop[*i->value.begin()](evaluate(i->children.begin()));
	else											// Binary expression
	{	try
		{	
			ret = op[*i->value.begin()](evaluate(i->children.begin()), evaluate(i->children.begin() + 1));
    	}
		catch (divide_by_zero& dvz)
		{	
			divide_by_zero ex(std::string(dvz.what()) + std::string(" in ") + infix(i));
			std::cout << ex.what() << std::endl;
	}	}
	
	return ret;
}

OpenViBE::float64 evaluate(tree_parse_info<> t)
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

bool CMatrixElementWiseOperation::parse(const int nbEntries, const char* formula, const bool debug /*= false*/)
{
	bool	ret		= true;
	pointersSize	= nbEntries;

    MyGrammar grammar;
    
	// Most errors can be caught in the parse with parser exceptions
    try
	{
		tree_parse_info<const char*> tree = ast_parse(formula, grammar, space_p);
		if(debug)
			std::cout << "Tree:\n"   << print(tree) << std::endl;
    }
	catch (parser_error<Errors, char const*> x)
	{	switch (x.descriptor)
		{	case close_expected:
				cout << "Expected close parenthesis" << endl;
				break;
			case expression_expected:
				cout << "Expected operand" << endl;
				break;
		}
	
		std::cout << "-->| " << formula << endl;
		std::cout << "at | " << setw(std::distance(formula, x.where) + 1) 
							 << "^" << endl;

		ret	= false;
    }

    if(!ret)
    {   std::cout << "GELU ERROR >>> " << __FILE__ << "::" << __LINE__ << " CMatrixElementWiseOperation::parse" << std::endl;
		return false;
    }
    
	return ret;
}

bool  CMatrixElementWiseOperation::evaluate(OpenViBE::IMatrix& rResult, const std::vector<OpenViBE::IMatrix*>& rImputs, const OpenViBE::CString& rFormula)
{
	const char* formula = rFormula;

	if(op.size() == 0)
	{	// Initialize the binary operators:
		op['*'] = multiplies<OpenViBE::float64>();
		op['/'] = ::divides<OpenViBE::float64>();
		op['+'] = plus<OpenViBE::float64>();
		op['-'] = minus<OpenViBE::float64>();

		// Initialize the unary operators:
		uop['-'] = negate<OpenViBE::float64>();
	}

	pointers.clear();
	indexes.clear();

	int ii = 0;
	for(std::vector<OpenViBE::IMatrix*>::const_iterator it=rImputs.begin(); it != rImputs.end(); it++)
	{	pointers.push_back((*it)->getBuffer());

		std::ostringstream os;
		os << 'm' << ii;
		indexes.insert(pair<std::string, size_t>(os.str(), ii++));
        
        if((ii == 0) && (rResult.getDimensionCount() == 0))
        {   rResult.setDimensionCount((*it)->getDimensionCount());
	        for(int i=0; i < (*it)->getDimensionCount(); i++)
                rResult.setDimensionSize(i, (*it)->getDimensionSize(i));
	}   }

	bool	ret		= true;
	pointersSize	= rImputs.size();

    MyGrammar						grammar;
	tree_parse_info<const char*>	tree;    
	// Most errors can be caught in the parse with parser exceptions
    try
	{
		tree = ast_parse(formula, grammar, space_p);
    }
	catch (parser_error<Errors, char const*> x)
	{	switch (x.descriptor)
		{	case close_expected:
				cout << "Expected close parenthesis" << endl;
				break;
			case expression_expected:
				cout << "Expected operand" << endl;
				break;
		}
	
		std::cout << "-->| " << rFormula << endl;
		std::cout << "at | " << setw(std::distance(formula, x.where) + 1) 
							 << "^" << endl;

		ret	= false;
    }

    if(ret && tree.full)
	{	try
		{	int					size = rResult.getBufferElementCount();
			OpenViBE::float64*	data = rResult.getBuffer();
			
			for(int ii=0; ii < size; ii++, data++)
			{	*data	= ::evaluate(tree);
				
				for(size_t jj=0; jj < pointers.size(); jj++)
					++pointers[jj];
			}
		}
		catch (std::runtime_error& e)
		{
			std::cout << e.what() << std::endl;
		}
	}
	else	// Or was the expression rejected?
	{	// But open parenthesis and missing operators are very hard to
		// check for during the parse, so it is easier to check 
		// for an incomplete parse afterwards:
		switch (*tree.stop)
		{	case ')':
				cout << "Expected open parenthesis\n";
				break;
			case '(':
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				cout << "Expected binary operator\n";
				break;
			default:
				cout << "Not a number\n";
	}	}

    if(!ret)
    {   std::cout << "GELU ERROR >>> " << __FILE__ << "::" << __LINE__ << " CMatrixElementWiseOperation::evaluate" << std::endl;
		return false;
    }
    
    // debug(rImputs, rResult);
    
	return ret;
}

void CMatrixElementWiseOperation::debug(const std::vector<OpenViBE::IMatrix*>& rImputs, const OpenViBE::IMatrix& rResult)
{
	for(std::vector<OpenViBE::IMatrix*>::const_iterator it=rImputs.begin(); it != rImputs.end(); it++)
    {   const OpenViBE::float64* ptr = (*it)->getBuffer();
        
        std::cout << "in 0x" << ptr << " (dim = " << (*it)->getBufferElementCount() << ") = ";
        for(int ii=0; ii < 5; ii++)
            std::cout << *ptr++ << " ";
            
        std::cout << std::endl;
    }
    
    std::cout << "out 0x" << rResult.getBuffer() << " (dim = " << rResult.getBufferElementCount() << ") = ";
    const OpenViBE::float64* ptr = rResult.getBuffer();
    for(int ii=0; ii < 5; ii++)
        std::cout << *ptr++ << " ";
    std::cout << std::endl;
}
