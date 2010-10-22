#include "ovasCConfigurationMitsarEEG202ABuilder.h"

#include "ovasCDriverMitsarEEG202AGipsalab.h"
#include "ovasCAcqServerMitsarEEG202ADataInputStream.h"

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;

#define DEVICE_NAME			"GIPSA-Lab :: Mitsar EEG202A"
#define DEVICE_CONFIG_NAME	"../share/openvibe-applications/acquisition-server/interface-Gipsalab-MitsarEEG202A.ui"

CDriverMitsarEEG202AGipsalab::CDriverMitsarEEG202AGipsalab(IDriverContext& rDriverContext)
	: CAcqServerPipe(rDriverContext, DEVICE_NAME)
{
	CConfigurationMitsarEEG202Builder* l_pConfigurationBuilder	= new CConfigurationMitsarEEG202Builder(rDriverContext, DEVICE_CONFIG_NAME);
	m_pConfigurationBuilder										= l_pConfigurationBuilder;
	m_pDataInputStream											= new CAcqServerMitsarEEG202ADataInputStream(l_pConfigurationBuilder->refIndex());
}

CDriverMitsarEEG202AGipsalab::~CDriverMitsarEEG202AGipsalab(void)
{
}

OpenViBE::boolean CDriverMitsarEEG202AGipsalab::setAcquisitionParams()
{
	m_sAcquisitionParams.m_vecChannelNames.push_back("FP1");
	m_sAcquisitionParams.m_vecChannelNames.push_back("FPz");
	m_sAcquisitionParams.m_vecChannelNames.push_back("FP2");
	m_sAcquisitionParams.m_vecChannelNames.push_back("F7");
	m_sAcquisitionParams.m_vecChannelNames.push_back("F3");
	m_sAcquisitionParams.m_vecChannelNames.push_back("Fz");
	m_sAcquisitionParams.m_vecChannelNames.push_back("F4");
	m_sAcquisitionParams.m_vecChannelNames.push_back("F8");
	m_sAcquisitionParams.m_vecChannelNames.push_back("FT7");
	m_sAcquisitionParams.m_vecChannelNames.push_back("FC3");
	m_sAcquisitionParams.m_vecChannelNames.push_back("FCz");
	m_sAcquisitionParams.m_vecChannelNames.push_back("FC4");
	m_sAcquisitionParams.m_vecChannelNames.push_back("FT8");
	m_sAcquisitionParams.m_vecChannelNames.push_back("T3");
	m_sAcquisitionParams.m_vecChannelNames.push_back("C3");
	m_sAcquisitionParams.m_vecChannelNames.push_back("Cz");
	m_sAcquisitionParams.m_vecChannelNames.push_back("C4");
	m_sAcquisitionParams.m_vecChannelNames.push_back("T4");
	m_sAcquisitionParams.m_vecChannelNames.push_back("TP7");
	m_sAcquisitionParams.m_vecChannelNames.push_back("CP3");
	m_sAcquisitionParams.m_vecChannelNames.push_back("CPz");
	m_sAcquisitionParams.m_vecChannelNames.push_back("CP4");
	m_sAcquisitionParams.m_vecChannelNames.push_back("TP8");
	m_sAcquisitionParams.m_vecChannelNames.push_back("T5");
	m_sAcquisitionParams.m_vecChannelNames.push_back("P3");
	m_sAcquisitionParams.m_vecChannelNames.push_back("Pz");
	m_sAcquisitionParams.m_vecChannelNames.push_back("P4");
	m_sAcquisitionParams.m_vecChannelNames.push_back("T6");
	m_sAcquisitionParams.m_vecChannelNames.push_back("O1");
	m_sAcquisitionParams.m_vecChannelNames.push_back("Oz");
	m_sAcquisitionParams.m_vecChannelNames.push_back("O2");
	m_sAcquisitionParams.m_vecChannelNames.push_back("Bio1");
	m_sAcquisitionParams.m_vecChannelNames.push_back(m_pConfigurationBuilder->getSynchroMask() ? "Synchro" : "CH_Event");
		
	m_sAcquisitionParams.m_dataType					= AcquisitionParams::type_float32;	
	m_sAcquisitionParams.m_ui32ChannelCount			= NB_SIGNALS;
	m_sAcquisitionParams.m_ui32SamplingFrequency	= OpenViBE::uint32(SAMPLING_RATE);
	m_sAcquisitionParams.m_ui32SampleCount			= NB_SAMPLES;
	m_sAcquisitionParams.m_pData					= m_pDataInputStream->getBuffer();

	m_uint16OldStimulation							= 0;

	m_rDriverContext.getLogManager() << LogLevel_Info << "CDriverMitsarEEG202AGipsalab::setAcquisitionParams OK\n";

	return true;
}

void CDriverMitsarEEG202AGipsalab::parseTriggers(OpenViBE::uint16& CH_EventOut, OpenViBE::uint16& synchro, const OpenViBE::float32 CH_EventIn)
{
	if(CH_EventIn < 0.03)
		CH_EventOut	= 0xc0;
	else if(CH_EventIn < 0.1)
		CH_EventOut	= 0x80;
	else if(CH_EventIn < 0.16)
		CH_EventOut	= 0x40;
	else
		CH_EventOut	= 0;

	if(m_pConfigurationBuilder->getSynchroMask())
	{	synchro		 = CH_EventOut & m_pConfigurationBuilder->getSynchroMask();
		CH_EventOut	&= ~m_pConfigurationBuilder->getSynchroMask();
	}
	else
		synchro		 = 0;		
}

OpenViBE::boolean CDriverMitsarEEG202AGipsalab::processDataAndStimulations()
{
	OpenViBE::uint16	l_CH_EventOut;
	OpenViBE::uint16	l_synchro;
	OpenViBE::float32*	l_pCH_Event	= (OpenViBE::float32*) m_sAcquisitionParams.m_pData + CH_Event_INDEX;
	OpenViBE::float32*	l_pBio1		= (OpenViBE::float32*) m_sAcquisitionParams.m_pData + Bio1_INDEX;
	
	m_sAcquisitionParams.m_stimulations.resize(0);
	for(OpenViBE::uint32 i = 0; i < NB_SAMPLES; i++, l_pCH_Event += NB_SIGNALS, l_pBio1 += NB_SIGNALS)
	{	parseTriggers(l_CH_EventOut, l_synchro, *l_pCH_Event);
		
		*l_pCH_Event	= *l_pBio1;
		*l_pBio1		= m_pConfigurationBuilder->getSynchroMask() ? l_synchro : l_CH_EventOut;

		if(l_CH_EventOut > m_uint16OldStimulation)
			m_sAcquisitionParams.m_stimulations.push_back(CAcqServerCircularBuffer::CStimulation(i, l_CH_EventOut));

		m_uint16OldStimulation	= l_CH_EventOut;
	}

	return true;
}
