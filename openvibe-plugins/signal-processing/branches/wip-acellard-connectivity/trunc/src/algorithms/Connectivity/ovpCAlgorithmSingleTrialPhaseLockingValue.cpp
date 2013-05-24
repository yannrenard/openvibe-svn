//#if defined(TARGET_HAS_ThirdPartyEIGEN)

#include "ovpCAlgorithmPhaseLockingValue.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;

using namespace Eigen;


boolean CAlgorithmPhaseLockingValue::initialize(void)
{

	ip_pSignal1.initialize(this->getInputParameter(OVP_Algorithm_PhaseLockingValue_InputParameterId_Matrix1));
	ip_pSignal2.initialize(this->getInputParameter(OVP_Algorithm_PhaseLockingValue_InputParameterId_Matrix2));
	ip_pChannelPairs.initialize(this->getInputParameter(OVP_Algorithm_PhaseLockingValue_InputParameterId_Matrix));
	op_pMatrix.initialize(this->getOutputParameter(OVP_Algorithm_PhaseLockingValue_OutputParameterId_Matrix));
	return true;
}

boolean CAlgorithmPhaseLockingValue::uninitialize(void)
{
	op_pMatrix.uninitialize();
	ip_pMatrix1.uninitialize();
	ip_pMatrix2.uninitialize();
	ip_pChannelPairs.uninitialize();
	return true;
}

boolean CAlgorithmPhaseLockingValue::process(void)
{


		int l_ui32ChannelCount = ip_pMatrix->getDimensionSize(0);
	    int l_ui32SamplesPerChannel = ip_pMatrix->getDimensionSize(1);

	    IMatrix* l_pInputMatrix=ip_pMatrix;
	    IMatrix* l_pOutputMatrix=op_pMatrix;


	    if(this->isInputTriggerActive(OVP_Algorithm_ARBurgMethod_InputTriggerId_Initialize))
	    {

		if( l_pInputMatrix->getDimensionCount() != 2)
	        {
		    this->getLogManager() << LogLevel_Error << "The input matrix must have 2 dimensions";
	            return false;
	        }

		if(l_pInputMatrix->getDimensionSize(1) < 2*m_ui32Order)
	        {
	            this->getLogManager() << LogLevel_Error << "The input vector must be greater than twice the order";
	            return false;
	        }

		// Setting size of output

		l_pOutputMatrix->setDimensionCount(2); // the output matrix will have 2 dimensions
	    	l_pOutputMatrix->setDimensionSize(0,1); // only one row vector giving the coefficients
	    	l_pOutputMatrix->setDimensionSize(1,(m_ui32Order+1)*l_ui32ChannelCount);// The number of coefficients per channel is equal to the order

	    }



	    if(this->isInputTriggerActive(OVP_Algorithm_ARBurgMethod_InputTriggerId_Process))
	    {

	        this->activateOutputTrigger(OVP_Algorithm_ARBurgMethod_OutputTriggerId_ProcessDone, true);
	    }
}


//#endif //TARGET_HAS_ThirdPartyEIGEN

