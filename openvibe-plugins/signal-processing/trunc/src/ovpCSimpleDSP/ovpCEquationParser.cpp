#include "ovpCEquationParser.h"

#include <string>
#include <iostream>
#include <algorithm>
#include <functional>
#include <cctype>

using namespace boost;
using namespace boost::spirit;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;
using namespace OpenViBEToolkit;

//#define EQ_PARSER_DEBUG

namespace
{
	// because std::tolower has multiple signatures,
	// it can not be easily used in std::transform
	// this workaround is taken from http://www.gcek.net/ref/books/sw/cpp/ticppv2/
	template <class charT>
	charT to_lower(charT c)
	{
		return std::tolower(c);
	}
};

functionPointer CEquationParser::m_pFunctionTable[]=
{
	&op_neg, &op_add, &op_sub, &op_mul, &op_div,
	&op_abs, &op_acos, &op_asin, &op_atan,
	&op_ceil, &op_cos, &op_exp, &op_floor,
	&op_log, &op_log10, &op_power, &op_sin,
	&op_sqrt, &op_tan,
	&op_if_then_else,
	&op_cmp_lower,
	&op_cmp_greater,
	&op_cmp_lower_equal,
	&op_cmp_greater_equal,
	&op_cmp_equal,
	&op_cmp_not_equal,
	&op_bool_and,
	&op_bool_or,
	&op_bool_not,
	&op_bool_xor,
};

CEquationParser::CEquationParser(TBoxAlgorithm<IBoxAlgorithm>& oPlugin, float64** ppVariable, uint32 ui32VariableCount)
	:m_pTree(NULL)
	,m_ppVariable(ppVariable)
	,m_ui32VariableCount(ui32VariableCount)
	,m_ui32FunctionStackSize(1024)
	,m_pFunctionList(NULL)
	,m_pFunctionListBase(NULL)
	,m_ui64FunctionContextStackSize(1024)
	,m_pFunctionContextList(NULL)
	,m_pFunctionContextListBase(NULL)
	,m_ui64StackSize(1024)
	,m_pStack(NULL)
	,m_ui64TreeCategory(OP_USERDEF)
	,m_f64TreeParameter(0)
	,m_oParentPlugin(oPlugin)
{
}

CEquationParser::~CEquationParser()
{
	delete m_pFunctionListBase;
	delete m_pFunctionContextListBase;
	delete m_pStack;
	delete m_pTree;
}

boolean CEquationParser::compileEquation(const char * pEquation)
{
	//parses the equation
	tree_parse_info<> l_oInfo = ast_parse(pEquation, m_oGrammar, space_p);

	//If the parsing was successful
	if (l_oInfo.full)
	{
#ifdef EQ_PARSER_DEBUG
		std::cout << "parsing succeeded\n";
		createAbstractTree(l_oInfo);

		m_pTree->printTree();
		std::cout << "leveling tree ......." << std::endl;

		m_pTree->levelOperators();
		m_pTree->printTree();
		std::cout << "simplifying tree ......." << std::endl;

		m_pTree->simplifyTree();
		m_pTree->printTree();
		std::cout << "using Neg tree ......." << std::endl;

		m_pTree->useNegationOperator();
		m_pTree->printTree();
		std::cout << "generating code ......." << std::endl;
#else
		//creates the AST
		createAbstractTree(l_oInfo);

		//CONSTANT FOLDING
		//levels the associative/commutative operators (+ and *)
		m_pTree->levelOperators();
		//simplifies the AST
		m_pTree->simplifyTree();
		//tries to replace nodes to use the NEG operator and reduce complexity
		m_pTree->useNegationOperator();
#endif
		//Detects if it is a special tree (updates m_ui64TreeCategory and m_f64TreeParameter)
		m_pTree->recognizeSpecialTree(m_ui64TreeCategory, m_f64TreeParameter);

		//Unrecognize special tree
		m_ui64TreeCategory = OP_USERDEF;

		//If it is not a special tree, we need to generate some code to reach the result
		if(m_ui64TreeCategory == OP_USERDEF)
		{
			//allocates the function stack
			m_pFunctionList = new functionPointer[m_ui32FunctionStackSize];
			m_pFunctionListBase = m_pFunctionList;

			//Allocates the function context stack
			m_pFunctionContextList = new functionContext[(size_t)m_ui64FunctionContextStackSize];
			m_pFunctionContextListBase = m_pFunctionContextList;
			m_pStack = new float64[(size_t)m_ui64StackSize];

			//generates the code
			m_pTree->generateCode(*this);

			//computes the number of steps to get to the result
			m_ui64NumberOfOperations = m_pFunctionList - m_pFunctionListBase;
		}

		return true;
	}
	else
	{
		//if the parsing failed
		m_oParentPlugin.getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning << pEquation << " Failed parsing\n";
		std::string l_oErrorString;

		for (int i = 0; i < (l_oInfo.stop - pEquation); i++)
		{
			l_oErrorString+=" ";
		}
		l_oErrorString+="^--Here\n";
		m_oParentPlugin.getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning << l_oErrorString.c_str();

		return false;
	}
}

void CEquationParser::createAbstractTree(tree_parse_info<> oInfo)
{
	m_pTree = new CAbstractTree(createNode(oInfo.trees.begin()));
}

CAbstractTreeNode * CEquationParser::createNode(iter_t const& i)
{
	if (i->value.id() == CEquationGrammar::expressionID)
	{
		if (*i->value.begin() == '+')
		{
			return new CAbstractTreeParentNode(OP_ADD, createNode(i->children.begin()), createNode(i->children.begin()+1), true);
		}
		//replaces (- X Y) by (+ X (-Y)) (in fact (+ X (* -1 Y)) )
		if (*i->value.begin() == '-')
		{
			return new CAbstractTreeParentNode(OP_ADD,
				createNode(i->children.begin()),
				new CAbstractTreeParentNode(OP_MUL,
					new CAbstractTreeValueNode(-1),
					createNode(i->children.begin()+1),
					true),
				true);
		}
	}
	else if(i->value.id() == CEquationGrammar::termID)
	{
		if (*i->value.begin() == '*')
		{
			return new CAbstractTreeParentNode(OP_MUL, createNode(i->children.begin()), createNode(i->children.begin()+1), true);
		}
		if (*i->value.begin() == '/')
		{
			return new CAbstractTreeParentNode(OP_DIV, createNode(i->children.begin()), createNode(i->children.begin()+1));
		}
	}
	else if(i->value.id() == CEquationGrammar::factorID)
	{
		if (*i->value.begin() == '-')
		{
			// -X => (* -1 X), useful to simplify the tree later
			return new CAbstractTreeParentNode(
				OP_MUL,
				new CAbstractTreeValueNode(-1), createNode(i->children.begin()),
				true);
		}
		else if (*i->value.begin() == '+')
		{
			return createNode(i->children.begin());
		}
	}
	else if (i->value.id() == CEquationGrammar::realID)
	{
		std::string l_sValue(i->value.begin(), i->value.end());
		return new CAbstractTreeValueNode(strtod(l_sValue.c_str(), NULL));
	}
	else if (i->value.id() == CEquationGrammar::variableID)
	{
		uint32 l_ui32Index=0;
		std::string l_sValue(i->value.begin(), i->value.end());
		if(l_sValue!="x") l_ui32Index=l_sValue[0]-'a';
		if(l_ui32Index>=m_ui32VariableCount)
		{
			m_oParentPlugin.getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning << "No such input " << l_ui32Index+1 << " (referenced with variable [" << CString(l_sValue.c_str()) << "])\n";
			return new CAbstractTreeValueNode(0);
		}
		return new CAbstractTreeVariableNode(l_ui32Index);
	}
	else if (i->value.id() == CEquationGrammar::constantID)
	{
		std::string l_sValue(i->value.begin(), i->value.end());

		//converts the string to lowercase
		std::transform(l_sValue.begin(), l_sValue.end(), l_sValue.begin(), ::to_lower<std::string::value_type>);

		//creates a new value node from the value looked up in the constant's symbols table
		return new CAbstractTreeValueNode(*find(mathConstant_p, l_sValue.c_str()));
	}
	else if (i->value.id() == CEquationGrammar::functionID)
	{
		std::string l_sValue(i->value.begin(), i->value.end());
		uint64 * l_ui64FunctionIdentifier;

		//converts the string to lowercase
		std::transform(l_sValue.begin(), l_sValue.end(), l_sValue.begin(), ::to_lower<std::string::value_type>);

		//gets the function's Id from the unary function's symbols table
		if( (l_ui64FunctionIdentifier = find(unaryFunction_p, l_sValue.c_str())) != NULL)
		{
			std::string l_sValue(i->children.begin()->value.begin(), i->children.begin()->value.end());

			return new CAbstractTreeParentNode(
				*l_ui64FunctionIdentifier,
				createNode(i->children.begin()),
				false);
		}
		//gets the function's Id from the binary function's symbols table
		else if( (l_ui64FunctionIdentifier = find(binaryFunction_p, l_sValue.c_str())) != NULL)
		{
			return new CAbstractTreeParentNode(
				*l_ui64FunctionIdentifier,
				createNode(i->children.begin()),
				createNode(i->children.begin()+1),
				false);
		}
	}
	else if (i->value.id() == CEquationGrammar::ifthenID)
	{
		return new CAbstractTreeParentNode(
			OP_IF_THEN_ELSE,
			createNode(i->children.begin()),
			createNode(i->children.begin()+1),
			createNode(i->children.begin()+2),
			false);
	}
	else if (i->value.id() == CEquationGrammar::comparisonID)
	{
		std::string l_sValue(i->value.begin(), i->value.end());
		uint64 * l_ui64FunctionIdentifier;

		//converts the string to lowercase
		std::transform(l_sValue.begin(), l_sValue.end(), l_sValue.begin(), ::to_lower<std::string::value_type>);

		//gets the function's Id from the comparison function's symbols table
		if( (l_ui64FunctionIdentifier = find(comparison1Function_p, l_sValue.c_str())) != NULL)
		{
			std::string l_sValue(i->children.begin()->value.begin(), i->children.begin()->value.end());

			return new CAbstractTreeParentNode(
				*l_ui64FunctionIdentifier,
				createNode(i->children.begin()),
				createNode(i->children.begin()+1),
				false);
		}
		//gets the function's Id from the comparison function's symbols table
		else if( (l_ui64FunctionIdentifier = find(comparison2Function_p, l_sValue.c_str())) != NULL)
		{
			std::string l_sValue(i->children.begin()->value.begin(), i->children.begin()->value.end());

			return new CAbstractTreeParentNode(
				*l_ui64FunctionIdentifier,
				createNode(i->children.begin()),
				createNode(i->children.begin()+1),
				false);
		}
	}
	else if (i->value.id() == CEquationGrammar::booleanID)
	{
		std::string l_sValue(i->value.begin(), i->value.end());
		uint64 * l_ui64FunctionIdentifier;

		//converts the string to lowercase
		std::transform(l_sValue.begin(), l_sValue.end(), l_sValue.begin(), ::to_lower<std::string::value_type>);

		//gets the function's Id from the binary boolean function's symbols table
		if( (l_ui64FunctionIdentifier = find(binaryBoolean1Function_p, l_sValue.c_str()))!= NULL)
		{
			std::string l_sValue(i->children.begin()->value.begin(), i->children.begin()->value.end());

			return new CAbstractTreeParentNode(
				*l_ui64FunctionIdentifier,
				createNode(i->children.begin()),
				createNode(i->children.begin()+1),
				false);
		}
		//gets the function's Id from the binary boolean function's symbols table
		else if( (l_ui64FunctionIdentifier = find(binaryBoolean2Function_p, l_sValue.c_str()))!= NULL)
		{
			std::string l_sValue(i->children.begin()->value.begin(), i->children.begin()->value.end());

			return new CAbstractTreeParentNode(
				*l_ui64FunctionIdentifier,
				createNode(i->children.begin()),
				createNode(i->children.begin()+1),
				false);
		}
		//gets the function's Id from the binary boolean function's symbols table
		else if( (l_ui64FunctionIdentifier = find(binaryBoolean3Function_p, l_sValue.c_str()))!= NULL)
		{
			std::string l_sValue(i->children.begin()->value.begin(), i->children.begin()->value.end());

			return new CAbstractTreeParentNode(
				*l_ui64FunctionIdentifier,
				createNode(i->children.begin()),
				createNode(i->children.begin()+1),
				false);
		}
		//gets the function's Id from the binary boolean function's symbols table
		else if( (l_ui64FunctionIdentifier = find(unaryBooleanFunction_p, l_sValue.c_str()))!= NULL)
		{
			std::string l_sValue(i->children.begin()->value.begin(), i->children.begin()->value.end());

			return new CAbstractTreeParentNode(
				*l_ui64FunctionIdentifier,
				createNode(i->children.begin()),
				false);
		}
	}

	return NULL;
}

void CEquationParser::push_value(float64 f64Value)
{
	*(m_pFunctionList++) = op_loadVal;
	(*(m_pFunctionContextList++)).m_f64DirectValue = f64Value;
}

void CEquationParser::push_var(uint32 ui32Index)
{
	*(m_pFunctionList++) = op_loadVar;
	(*(m_pFunctionContextList++)).m_ppIndirectValue = &m_ppVariable[ui32Index];
}

void CEquationParser::push_op(uint64 ui64Operator)
{
	*(m_pFunctionList++) = m_pFunctionTable[ui64Operator];
	(*(m_pFunctionContextList++)).m_ppIndirectValue=NULL;
}

// Functions called by our "pseudo - VM"

void CEquationParser::op_neg(float64*& pStack, functionContext& pContext)
{
#ifdef EQ_PARSER_DEBUG
	std::cout << "neg : " << *(pStack);
#endif
	*pStack = - (*pStack);
#ifdef EQ_PARSER_DEBUG
	std::cout << " = " << *pStack << std::endl;
#endif
}

void CEquationParser::op_add(float64 *& pStack, functionContext& pContext)
{
#ifdef EQ_PARSER_DEBUG
	std::cout << "add : " << *(pStack) << " + " << *(pStack-1);
#endif
	pStack--;
	*(pStack) = *(pStack+1) + *(pStack);
#ifdef EQ_PARSER_DEBUG
	std::cout << " = " << *pStack << std::endl;
#endif
}

void CEquationParser::op_sub(float64 *& pStack, functionContext& pContext)
{
#ifdef EQ_PARSER_DEBUG
	std::cout << "sub : " << *(pStack) << " - " << *(pStack-1);
#endif
	pStack--;
	*(pStack) = *(pStack+1) - *(pStack);
#ifdef EQ_PARSER_DEBUG
	std::cout << " = " << *pStack << std::endl;
#endif
}

void CEquationParser::op_mul(float64 *& pStack, functionContext& pContext)
{
#ifdef EQ_PARSER_DEBUG
	std::cout << "mult : " << *(pStack) << " * " << *(pStack-1);
#endif
	pStack--;
	*(pStack) = *(pStack+1) * *(pStack);
#ifdef EQ_PARSER_DEBUG
	std::cout << " = " << *pStack << std::endl;
#endif
}

void CEquationParser::op_div(float64 *& pStack, functionContext& pContext)
{
#ifdef EQ_PARSER_DEBUG
	std::cout << "divi : " << *(pStack) << " / " << *(pStack-1);
#endif
	pStack--;
	*(pStack) = *(pStack+1) / *(pStack);

#ifdef EQ_PARSER_DEBUG
	std::cout << " = " << *pStack << std::endl;
#endif
}

void CEquationParser::op_power(float64 *& pStack, functionContext& pContext)
{
#ifdef EQ_PARSER_DEBUG
	std::cout << "pow: " << *(pStack) << " " << *(pStack-1) << std::endl;
#endif
	pStack--;
	*pStack = pow(*(pStack+1), *(pStack));
#ifdef EQ_PARSER_DEBUG
	std::cout << "Pow Result: " << *pStack << std::endl;
#endif
}

void CEquationParser::op_abs(float64*& pStack, functionContext& pContext)
{
	*pStack = fabs(*(pStack));
}

void CEquationParser::op_acos(float64*& pStack, functionContext& pContext)
{
	*pStack = acos(*(pStack));
}

void CEquationParser::op_asin(float64*& pStack, functionContext& pContext)
{
	*pStack = asin(*(pStack));
}

void CEquationParser::op_atan(float64*& pStack, functionContext& pContext)
{
	*pStack = atan(*(pStack));
}

void CEquationParser::op_ceil(float64*& pStack, functionContext& pContext)
{
	*pStack = ceil(*(pStack));
}

void CEquationParser::op_cos(float64*& pStack, functionContext& pContext)
{
	*pStack = cos(*(pStack));
}

void CEquationParser::op_exp(float64*& pStack, functionContext& pContext)
{
	*pStack = exp(*(pStack));
}

void CEquationParser::op_floor(float64*& pStack, functionContext& pContext)
{
	*pStack = floor(*(pStack));
}

void CEquationParser::op_log(float64 *& pStack, functionContext& pContext)
{
	*pStack = log(*(pStack));
}

void CEquationParser::op_log10(float64*& pStack, functionContext& pContext)
{
	*pStack = log10(*(pStack));
}

void CEquationParser::op_sin(float64*& pStack, functionContext& pContext)
{
	*pStack = sin(*(pStack));
}

void CEquationParser::op_sqrt(float64 *& pStack, functionContext& pContext)
{
	*pStack = sqrt(*(pStack));
}

void CEquationParser::op_tan(float64*& pStack, functionContext& pContext)
{
	*pStack = tan(*(pStack));
}

void CEquationParser::op_if_then_else(float64*& pStack, functionContext& pContext)
{
	pStack--;
	pStack--;
	if(*(pStack+2))
	{
		*pStack = *(pStack+1);
	}
	else
	{
		// *pStack = *pStack;
	}
}

void CEquationParser::op_cmp_lower(float64*& pStack, functionContext& pContext)
{
	pStack--;
	pStack[0] = (pStack[1]<pStack[0]?1:0);
}

void CEquationParser::op_cmp_greater(float64*& pStack, functionContext& pContext)
{
	pStack--;
	pStack[0] = (pStack[1]>pStack[0]?1:0);
}

void CEquationParser::op_cmp_lower_equal(float64*& pStack, functionContext& pContext)
{
	pStack--;
	pStack[0] = (pStack[1]<=pStack[0]?1:0);
}

void CEquationParser::op_cmp_greater_equal(float64*& pStack, functionContext& pContext)
{
	pStack--;
	pStack[0] = (pStack[1]>=pStack[0]?1:0);
}

void CEquationParser::op_cmp_equal(float64*& pStack, functionContext& pContext)
{
	pStack--;
	pStack[0] = (pStack[1]==pStack[0]?1:0);
}

void CEquationParser::op_cmp_not_equal(float64*& pStack, functionContext& pContext)
{
	pStack--;
	pStack[0] = (pStack[1]!=pStack[0]?1:0);
}

void CEquationParser::op_bool_and(float64*& pStack, functionContext& pContext)
{
	pStack--;
	pStack[0] = (pStack[1]!=0 && pStack[0]!=0?1:0);
}

void CEquationParser::op_bool_or(float64*& pStack, functionContext& pContext)
{
	pStack--;
	pStack[0] = (pStack[1]!=0 || pStack[0]!=0?1:0);
}

void CEquationParser::op_bool_not(float64*& pStack, functionContext& pContext)
{
	pStack[0] = pStack[0]!=0?0:1;
}

void CEquationParser::op_bool_xor(float64*& pStack, functionContext& pContext)
{
	pStack--;
	pStack[0] = (pStack[1]!=pStack[0]?1:0);
}

void CEquationParser::op_loadVal(float64 *& pStack, functionContext& pContext)
{
#ifdef EQ_PARSER_DEBUG
	std::cout << "loadVal : " << pContext.m_f64DirectValue << std::endl;
#endif
	*(++pStack) = pContext.m_f64DirectValue;
}

void CEquationParser::op_loadVar(float64 *& pStack, functionContext& pContext)
{
#ifdef EQ_PARSER_DEBUG
	std::cout << "loadVar : " << **(pContext.m_ppIndirectValue) << std::endl;
#endif
	*(++pStack) = **(pContext.m_ppIndirectValue);
}
