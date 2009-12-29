#include "ovpCVRPNButtonClient.h"
#include <map>
#include <iostream>

using namespace OpenViBE;
using namespace OpenViBE::Plugins;
using namespace OpenViBE::Kernel;
using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::VRPN;
using namespace OpenViBEToolkit;
using namespace std;



CVRPNDeviceInfo::CVRPNDeviceInfo(CString& name, char* cName)
	:m_name(name)
{
	btn = new vrpn_Button_Remote(cName);
}

CVRPNDeviceInfo::~CVRPNDeviceInfo()
{

	delete btn;
}


OpenViBE::boolean CVRPNButtonClient::initialize(void)
{
	
	const IBox * l_pBox=getBoxAlgorithmContext()->getStaticBoxContext();

	m_pStreamEncoder = &getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamEncoder) );
	
	m_pStreamEncoder->initialize();
	m_oMemoryBufferHandle.initialize( m_pStreamEncoder->getOutputParameter(OVP_GD_Algorithm_StimulationStreamEncoder_OutputParameterId_EncodedMemoryBuffer) );

	IStimulationSet* l_pStimulationSet=&m_oStimulationSet;
	m_pStreamEncoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamEncoder_InputParameterId_StimulationSet)->setValue(&l_pStimulationSet);

	m_ui64StartTime=0;
	m_ui64EndTime=0;
	m_bHasSentHeader = false;

	CString l_sServerName;
	l_pBox->getSettingValue(0, l_sServerName);

	addDevice(l_sServerName);
	return true;
}	

OpenViBE::boolean CVRPNButtonClient::uninitialize(void)
{
	m_oMemoryBufferHandle.uninitialize();
	m_pStreamEncoder->uninitialize();
	getAlgorithmManager().releaseAlgorithm(*m_pStreamEncoder);
	m_pStreamEncoder=NULL;
	delete m_pDeviceInfo;
	m_lButtons.clear();
	return true;
}

boolean CVRPNButtonClient::processClock(IMessageClock& rMessageClock)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}


OpenViBE::boolean CVRPNButtonClient::process(void)
{
	getLogManager() << LogLevel_Debug << "Process called\n";
	IBoxIO& l_rDynamicBoxContext = getDynamicBoxContext();
	IPlayerContext& l_rPlayerContext = getPlayerContext();
	
	if(!m_bHasSentHeader)	
	{
		m_oMemoryBufferHandle=l_rDynamicBoxContext.getOutputChunk(0);
		m_pStreamEncoder->process(OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeHeader);
		l_rDynamicBoxContext.markOutputAsReadyToSend(0, m_ui64StartTime, m_ui64EndTime);
		getLogManager() << LogLevel_Debug << "Header sent\n";
		m_bHasSentHeader = true;
	}
	else
	{
		//We call the vrpn button client mainloop
		m_pDeviceInfo->btn->mainloop();
		IStimulationSet* l_pStimulationSet = &m_oStimulationSet;
		//if a button signal is caught for this client, a stimulation is created according to the signal


		if (m_bIsWaiting)
		{

			l_pStimulationSet->setStimulationCount(m_lButtons.size());
			uint32 l_ui32count = 0;
			for (list< pair<uint32, uint32> >::iterator it=m_lButtons.begin(); it!=m_lButtons.end(); ++it)
			{
				getLogManager() << LogLevel_Debug<< "   Button "<<it->first<<" : "<<it->second<<"\n";
				

				l_pStimulationSet->setStimulationIdentifier(l_ui32count, OVTK_StimulationId_Label((it->first+1)*2+it->second ));
				l_pStimulationSet->setStimulationDate(l_ui32count, m_ui64StartTime);
				l_ui32count++;

			}

			m_pStreamEncoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamEncoder_InputParameterId_StimulationSet)->setValue(&l_pStimulationSet);
			m_oMemoryBufferHandle=l_rDynamicBoxContext.getOutputChunk(0);

			m_pStreamEncoder->process(OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeBuffer);


			l_rDynamicBoxContext.markOutputAsReadyToSend(0, m_ui64StartTime, m_ui64EndTime);

			getLogManager() << LogLevel_Debug << "Buffer sent\n";

			m_lButtons.clear();
		}
		//...otherwise, the stimulation is set to nothing.
		else
		{
			l_pStimulationSet->setStimulationCount(0);
			m_pStreamEncoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamEncoder_InputParameterId_StimulationSet)->setValue(&l_pStimulationSet);
			m_oMemoryBufferHandle=l_rDynamicBoxContext.getOutputChunk(0);

			m_pStreamEncoder->process(OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeBuffer);


			l_rDynamicBoxContext.markOutputAsReadyToSend(0, m_ui64StartTime, m_ui64EndTime);

			getLogManager() << LogLevel_Debug << "Buffer sent\n";


		}
			m_bIsWaiting = false;

	}
	m_ui64StartTime=m_ui64EndTime;
	m_ui64EndTime=l_rPlayerContext.getCurrentTime();

	return true;
}

OpenViBE::boolean CVRPNButtonClient::addDevice(CString& name)
{
	OpenViBE::boolean l_bResult;
	const char* l_sConstCName = name.toASCIIString();
	uint32 l_ui32Length = strlen(l_sConstCName);
	char l_sCName[l_ui32Length+1];
	strncpy(l_sCName, l_sConstCName, l_ui32Length+1);

	getLogManager() << LogLevel_Debug << "Connecting to server " << name <<"...\n";
	m_pDeviceInfo = new CVRPNDeviceInfo(name, l_sCName); 
	m_pDeviceInfo->btn->register_change_handler((void*)this, g_handleButton);
	l_bResult = true;
	
	return l_bResult;
}

void CVRPNButtonClient::handleButton(uint32 nbButton, boolean stateButton)
{

	getLogManager() << LogLevel_Debug << "Button Callback called\n";
	getLogManager() << LogLevel_Debug<< "   Button "<<nbButton<<" : "<<stateButton <<"\n";
	m_lButtons.push_back( pair<uint32, uint32>(nbButton, stateButton?1:0) );
	m_bIsWaiting = true;

}

void VRPN_CALLBACK OpenViBEPlugins::VRPN::g_handleButton(
	void *userdata,
	const vrpn_BUTTONCB b)
{

	//a callback inside our box is called
	CVRPNButtonClient* l_pButtonClient = (CVRPNButtonClient*)userdata;
	l_pButtonClient->handleButton(b.button, b.state);

}

