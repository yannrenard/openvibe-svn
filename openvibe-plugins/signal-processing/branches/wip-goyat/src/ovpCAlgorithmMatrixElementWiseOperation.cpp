#include "ovpCAlgorithmMatrixElementWiseOperation.h"

// #include <iostream>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::WipGionescu;

boolean CAlgorithmMatrixElementWiseOperation::initialize(void)
{
	ip_pInputs.initialize(this->getInputParameter(OVP_Algorithm_MatrixElementWiseOperation_InputParameterId_Inputs));
	ip_pGrammar.initialize(this->getInputParameter(OVP_Algorithm_MatrixElementWiseOperation_InputParameterId_Grammar));
	ip_pParser.initialize(this->getInputParameter(OVP_Algorithm_MatrixElementWiseOperation_InputParameterId_ParserOperator));
	
	op_pResult.initialize(this->getOutputParameter(OVP_Algorithm_MatrixElementWiseOperation_OutputParameterId_Result));

	return true;
}

boolean CAlgorithmMatrixElementWiseOperation::uninitialize(void)
{
	op_pResult.uninitialize();

	ip_pParser.uninitialize();
	ip_pGrammar.uninitialize();
	ip_pInputs.uninitialize();

	return true;
}

boolean CAlgorithmMatrixElementWiseOperation::process(void)
{
	void*                               l_pvInputs   = ip_pInputs;
	std::vector<OpenViBE::IMatrix*>*    l_pInputs    = static_cast< std::vector<OpenViBE::IMatrix*>* >(l_pvInputs);
	OpenViBE::CString*	                l_pGrammar   = ip_pGrammar;
	CMatrixElementWiseOperation*		m_parser	 = ip_pParser;
	OpenViBE::IMatrix*	                l_pResult    = static_cast< OpenViBE::IMatrix* >((void*) op_pResult);

    if(this->isInputTriggerActive(OVP_Algorithm_MatrixElementWiseOperation_InputTriggerId_Evaluate))
	{
		if(this->evaluate(*l_pResult, *l_pInputs, *l_pGrammar))
		  {
			this->activateOutputTrigger(OVP_Algorithm_MatrixElementWiseOperation_OutputTriggerId_Success, true);
		  }
		else
		  {
			this->activateOutputTrigger(OVP_Algorithm_MatrixElementWiseOperation_OutputTriggerId_Fail, true);
		  }
	}

	return true;
}

boolean CAlgorithmMatrixElementWiseOperation::evaluate(OpenViBE::IMatrix& rResult, const std::vector<OpenViBE::IMatrix*>& rImputs, const OpenViBE::CString& rFormula)
{
	return m_parser->evaluate(rResult, rImputs, rFormula,getLogManager());
}
