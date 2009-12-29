#include <iostream>

#include "ovpCAlgorithmMatrixOperation.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::WipGionescu;

boolean CAlgorithmMatrixOperation::initialize(void)
{
	ip_pInputs.initialize(this->getInputParameter(OVP_Algorithm_MatrixOperation_InputParameterId_Inputs));
	ip_pGrammar.initialize(this->getInputParameter(OVP_Algorithm_MatrixOperation_InputParameterId_Grammar));
	
	op_pResult.initialize(this->getOutputParameter(OVP_Algorithm_MatrixOperation_OutputParameterId_Result));

	return true;
}

boolean CAlgorithmMatrixOperation::uninitialize(void)
{
	op_pResult.uninitialize();

	ip_pGrammar.uninitialize();
	ip_pInputs.uninitialize();

	return true;
}

boolean CAlgorithmMatrixOperation::process(void)
{

	void*                               l_pvInputs   = ip_pInputs;
	std::vector<OpenViBE::IMatrix*>*    l_pInputs    = static_cast< std::vector<OpenViBE::IMatrix*>* >(l_pvInputs);
	OpenViBE::CString*	                l_pGrammar   = ip_pGrammar;

	OpenViBE::IMatrix*	                l_pResult    = static_cast< OpenViBE::IMatrix* >((void*) op_pResult);

    std::cout << "GELU INFO >>> CAlgorithmMatrixOperation::process" << std::endl;
    std::cout << l_pResult->getDimensionCount() << " " << l_pResult->getDimensionSize(0) << " " << l_pResult->getBufferElementCount() << std::endl;
	
    if(this->isInputTriggerActive(OVP_Algorithm_MatrixOperation_InputTriggerId_Evaluate))
	{	if(this->evaluate(*l_pResult, *l_pInputs, *l_pGrammar))
		{
			this->activateOutputTrigger(OVP_Algorithm_MatrixOperation_OutputTriggerId_Success, true);
		}
		else
		{
			this->activateOutputTrigger(OVP_Algorithm_MatrixOperation_OutputTriggerId_Fail, true);
	}	}

	return true;
}

boolean CAlgorithmMatrixOperation::evaluate(OpenViBE::IMatrix& rResult, const std::vector<OpenViBE::IMatrix*>& rImputs, const OpenViBE::CString& rFormula)
{
    std::cout << "GELU INFO >>> CAlgorithmMatrixOperation::evaluate" << std::endl;
	
    boolean b = CMatrixOperation::evaluate(rResult, rImputs, rFormula);
    
    if(!b)
    {   std::cout << "GELU ERROR >>> " << __FILE__ << "::" << __LINE__ << " CAlgorithmMatrixOperation::evaluate" << std::endl;
		return false;
    }
    
    return true;
}
