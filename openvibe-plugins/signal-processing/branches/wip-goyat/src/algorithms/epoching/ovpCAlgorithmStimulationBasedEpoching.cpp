#include "ovpCAlgorithmStimulationBasedEpoching.h"

#include <system/Memory.h>
#include <iostream>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;

boolean CAlgorithmStimulationBasedEpoching::initialize(void)
{
	m_pInputSignal.initialize(getInputParameter(OVP_Algorithm_StimulationBasedEpoching_InputParameterId_InputSignal));
	m_ui64OffsetSampleCount.initialize(getInputParameter(OVP_Algorithm_StimulationBasedEpoching_InputParameterId_OffsetSampleCount));
	m_pOutputSignal.initialize(getOutputParameter(OVP_Algorithm_StimulationBasedEpoching_OutputParameterId_OutputSignal));

	return true;
}

boolean CAlgorithmStimulationBasedEpoching::uninitialize(void)
{
	m_pOutputSignal.uninitialize();
	m_ui64OffsetSampleCount.uninitialize();
	m_pInputSignal.uninitialize();

	return true;
}

boolean CAlgorithmStimulationBasedEpoching::process(void)
{
	if(isInputTriggerActive(OVP_Algorithm_StimulationBasedEpoching_InputTriggerId_Reset))
	{
		//std::cout<<std::endl<<"make a reset"<<std::endl;
		m_ui64ReceivedSamples=0;
		m_ui64SamplesToSkip=m_ui64OffsetSampleCount;
	}

	if(isInputTriggerActive(OVP_Algorithm_StimulationBasedEpoching_InputTriggerId_PerformEpoching))
	{
		//std::cout<<"make a perform";
		uint32 l_ui32InputSampleCount=m_pInputSignal->getDimensionSize(1);
		uint32 l_ui32OutputSampleCount=m_pOutputSignal->getDimensionSize(1);

		//std::cout<<" /s skip : "<<m_ui64SamplesToSkip;
		if(m_ui64SamplesToSkip!=0)
		{
			if(m_ui64SamplesToSkip>=l_ui32InputSampleCount)
			{
				m_ui64SamplesToSkip-=l_ui32InputSampleCount;
				//std::cout<<std::endl;
				return true;
			}
		}

		uint32 l_ui32SamplesToCopy=(uint32)(l_ui32InputSampleCount-m_ui64SamplesToSkip);
		//std::cout<<" /s copy : "<<l_ui32SamplesToCopy<<" | "<<l_ui32OutputSampleCount-m_ui64ReceivedSamples;
		if(l_ui32SamplesToCopy>=l_ui32OutputSampleCount-m_ui64ReceivedSamples)
		{
			l_ui32SamplesToCopy=(uint32)(l_ui32OutputSampleCount-m_ui64ReceivedSamples);
			//std::cout<<" /s EPOCHING DONE";
			this->activateOutputTrigger(OVP_Algorithm_StimulationBasedEpoching_OutputTriggerId_EpochingDone, true);
		}

		if(l_ui32SamplesToCopy)
		{
			for(uint32 i=0; i<m_pInputSignal->getDimensionSize(0); i++)
			{
				System::Memory::copy(
					m_pOutputSignal->getBuffer()+i*l_ui32OutputSampleCount+m_ui64ReceivedSamples,
					m_pInputSignal->getBuffer()+i*l_ui32InputSampleCount+m_ui64SamplesToSkip,
					l_ui32SamplesToCopy*sizeof(float64));
			}
		}

		m_ui64ReceivedSamples+=l_ui32SamplesToCopy;
		m_ui64SamplesToSkip=0;
		
		//std::cout<<std::endl;
	}

	return true;
}
