#include "ovasCDriverCtfVsmMeg.h"
#include "../ovasCConfigurationNetworkGlade.h"

#include <system/Time.h>

#include <math.h>

#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;
using namespace std;



//___________________________________________________________________//
//                                                                   //

CDriverCtfVsmMeg::CDriverCtfVsmMeg(void)
	:m_pConnectionClient(NULL)
	,m_sServerHostName("localhost")
	,m_ui32ServerHostPort(9999)
	,m_pCallback(NULL)
	,m_bInitialized(false)
	,m_bStarted(false)
	,m_ui32SampleCountPerSentBlock(0)
	,m_pSample(NULL)
{
}

CDriverCtfVsmMeg::~CDriverCtfVsmMeg(void)
{
}

const char* CDriverCtfVsmMeg::getName(void)
{
	return "CTF/VSM MEG";
}

//___________________________________________________________________//
//                                                                   //

boolean CDriverCtfVsmMeg::initialize(
	const uint32 ui32RequestedSampleCountPerSentBlock,
	IDriverCallback& rCallback)
{
	char *l_pTmpBuffer;
	
	if(m_bInitialized)
	{
		return false;
	}
	
	// Builds up client connection
	m_pConnectionClient=Socket::createConnectionClient();

	// Tries to connect to server
	m_pConnectionClient->connect(m_sServerHostName, m_ui32ServerHostPort, 10);

	// Checks if connection is correctly established
	if(!m_pConnectionClient->isConnected())
	{
		// In case it is not, try to reconnect
		m_pConnectionClient->connect(m_sServerHostName, m_ui32ServerHostPort, 10);
	}

	if(!m_pConnectionClient->isConnected())
	{
		cout << "Connection problem! Tried 2 times without success! :(" << std::endl;
		cout << "Verify port number and/or Hostname..." << std::endl;
		return false;
	}
	else
	{
		cout << "> Client connected" << std::endl;

		// Get command header
		readCTFCommandHeader();

		if (m_structCommandHeader.m_ui32Command != CTF_HEADER)
		{
			cout << "ERROR : command is " << m_structCommandHeader.m_ui32Command << " . Must be 0 (header)." << std::endl;
			return false;
		}
		
		// Initialize vars for reception
		uint32 l_ui32Received = 0;
		uint32 l_ui32ReqLength = 0;
		uint32 l_ui32Result = 0;
		
		// Receive Fixed Header
		m_ui32FixedHeaderLength = sizeof(Str60)+sizeof(Str32)+sizeof(Str32)+sizeof(int)+(4*sizeof(char))+sizeof(int)+sizeof(Str32)+sizeof(int)+sizeof(Str32)+sizeof(float)+sizeof(int)+(9*sizeof(double))+sizeof(int);

		cout << "> Fixed header size " <<m_ui32FixedHeaderLength<<" bytes"<< std::endl;
		m_pStructFixedHeaderBuffer = (char *)calloc(m_ui32FixedHeaderLength, sizeof(char));
		if (m_pStructFixedHeaderBuffer == NULL)
		{
			cout << "ERROR : memory allocation for receiving fixed header." << std::endl;
			return false;
		}

		l_pTmpBuffer = m_pStructFixedHeaderBuffer;
		
		while(l_ui32Received < m_ui32FixedHeaderLength)
		{
			l_ui32ReqLength = m_ui32FixedHeaderLength -  l_ui32Received;
			l_ui32Result = m_pConnectionClient->receiveBuffer(l_pTmpBuffer, l_ui32ReqLength);

			l_ui32Received += l_ui32Result;
			l_pTmpBuffer += l_ui32Result;
		}
		cout << "> Fixed header received" << std::endl;

		// Pointers to buffer
		bufferToFixedHeaderStruct();

		// Allocation for dynamic part of header
		m_ui32VariableHeaderLength = *m_structFixedHeader.m_i32NumberOfChannels*(sizeof(Str32)+sizeof(int)+sizeof(float)+sizeof(float)+sizeof(float)+sizeof(int));

		m_pStructVariableHeaderBuffer = (char *)calloc(m_ui32VariableHeaderLength, sizeof(char));
		if (m_pStructVariableHeaderBuffer == NULL)
		{
			cout << "ERROR : memory allocation for receiving dynamic header." << std::endl;
			return false;
		}
		
		// Receive Dynamic Header
		l_ui32Received = 0;
		cout << "> Variable header size " <<m_ui32VariableHeaderLength<<" bytes"<< std::endl;

		l_pTmpBuffer = m_pStructVariableHeaderBuffer;
		
		while(l_ui32Received < m_ui32VariableHeaderLength)
		{
			l_ui32ReqLength = m_ui32VariableHeaderLength -  l_ui32Received;
			l_ui32Result = m_pConnectionClient->receiveBuffer(l_pTmpBuffer, l_ui32ReqLength);

			l_ui32Received += l_ui32Result;
			l_pTmpBuffer += l_ui32Result;
		}
		cout << "> Dynamic header received " << std::endl;
				
		// Pointers to buffer
		bufferToVariableHeaderStruct();
		
		// Save Header info into m_oHeader
		//m_oHeader.setExperimentIdentifier();
		//m_oHeader.setExperimentDate();

		//m_oHeader.setSubjectId();
		//m_oHeader.setSubjectName();
		m_oHeader.setSubjectAge(*m_structFixedHeader.m_i32SubjectAge);
		//m_oHeader.setSubjectGender();

		//m_oHeader.setLaboratoryId();
		//m_oHeader.setLaboratoryName();

		//m_oHeader.setTechnicianId();
		//m_oHeader.setTechnicianName();
		
		m_oHeader.setChannelCount((uint32)*m_structFixedHeader.m_i32NumberOfChannels);
		
		for(uint32 i=0; i<m_oHeader.getChannelCount(); i++)
		{
			m_oHeader.setChannelName(i, m_structVariableHeader.m_strChannelLabel[i]);
			m_oHeader.setChannelGain(i, (m_structVariableHeader.m_f32QGain[i]*m_structVariableHeader.m_f32ProperGain[i]*m_structVariableHeader.m_f32IOGain[i]));
		}

		m_oHeader.setSamplingFrequency((uint32)*m_structFixedHeader.m_f32SamplingRate);

		//m_oHeader.setSampleCount(ui32RequestedSampleCountPerSentBlock);

		if(!rCallback.forceSampleCountPerSentBuffer(*m_structFixedHeader.m_ui32NumSamples))
			return false;
//		if(ui32RequestedSampleCountPerSentBlock==0)
//		{
			// Choose a sample count per sent block from acquisition
//			if(!rCallback.forceSampleCountPerSentBuffer(*m_structFixedHeader.m_ui32NumSamples))
//				return false;
//		}

		// Search for stimulation channels
		m_pStimChannelIndex = NULL;
		m_ui32NbStimChannels = 0;
		for (int32 l_ui32iChan=0; l_ui32iChan<*m_structFixedHeader.m_i32NumberOfChannels; l_ui32iChan++)
			{
				if ((strncmp(m_structVariableHeader.m_strChannelLabel[l_ui32iChan], "STIM", strlen("STIM"))==0) ||
					(strncmp(m_structVariableHeader.m_strChannelLabel[l_ui32iChan], "UPPT", strlen("UPPT"))==0) ||
					(strncmp(m_structVariableHeader.m_strChannelLabel[l_ui32iChan], "USPT", strlen("USPT"))==0) ||
					(strncmp(m_structVariableHeader.m_strChannelLabel[l_ui32iChan], "UDIO", strlen("UDIO"))==0))
				{
					m_pStimChannelIndex = (uint32 *)realloc(m_pStimChannelIndex, (m_ui32NbStimChannels+1)*sizeof(uint32));
					if (m_pStimChannelIndex == NULL)
					{
						std::cout << "ERROR : Unable to allocate memory for trigger channels." << std::endl;
					}
					m_pStimChannelIndex[m_ui32NbStimChannels] = l_ui32iChan;
					m_ui32NbStimChannels++;
				}
			}
		
		m_pPrevStimValue = (uint32 *)calloc(m_ui32NbStimChannels, sizeof(uint32));
		if (m_pPrevStimValue == NULL)
		{
			std::cout << "ERROR : Unable to allocate memory for trigger channels." << std::endl;
		}
		
		m_pSample=new float32[m_oHeader.getChannelCount()*(*m_structFixedHeader.m_ui32NumSamples)];

		if(!m_pSample)
		{
			delete [] m_pSample;
			m_pSample=NULL;
			return false;
		}

		// Memory allocation for data buffer
		m_ui32DataBufferLength = (((*m_structFixedHeader.m_i32NumberOfChannels)*(*m_structFixedHeader.m_ui32NumSamples))+1)*sizeof(int32);
		
		m_pStructDataBuffer = (char *)calloc(m_ui32DataBufferLength, sizeof(char));
		if (m_pStructDataBuffer == NULL)
		{
			cout << "ERROR : memory allocation for receiving data buffer." << std::endl;
			return false;
		}
		m_structData.m_i32SampleNumber = (int32 *)m_pStructDataBuffer;
		m_structData.m_pDataBuffer = (int32 *)(m_pStructDataBuffer+sizeof(int32));
		
		m_pCallback=&rCallback;
		m_bInitialized=true;

		m_ui32SampleCountPerSentBlock=*m_structFixedHeader.m_ui32NumSamples;
		
		return true;
	}

}

boolean CDriverCtfVsmMeg::start(void)
{
	if(!m_bInitialized)
	{
		return false;
	}

	if(m_bStarted)
	{
		return false;
	}

	m_bStarted=true;

	return m_bStarted;
}

boolean CDriverCtfVsmMeg::loop(void)
{
	char *l_pTmpBuffer;
	uint32 l_ui32NumChannels;
	
	if(!m_bInitialized)
	{
		return false;
	}

	if(!m_bStarted)
	{
		return false;
	}

	if (!m_pConnectionClient->isReadyToReceive())
	{
		m_bStarted = false;
		return false;
	}
	
	// Initialize var to receive buffer of data
	uint32 l_ui32Received = 0;
	uint32 l_ui32ReqLength = 0;
	uint32 l_ui32Result = 0;

	l_ui32NumChannels = m_oHeader.getChannelCount();
	
	// Get command header
	readCTFCommandHeader();

	if (m_structCommandHeader.m_ui32Command == CTF_STOP)
	{
		cout << "Message received is a STOP!" << std::endl;
		m_bStarted = false;
		return false;
	}
	else if (m_structCommandHeader.m_ui32Command != CTF_DATA)
	{
		cout << "ERROR : command is " << m_structCommandHeader.m_ui32Command << " . Must be 1 (data)." << std::endl;
		return false;
	}
	
	
	// Read a first buffer of data
	l_pTmpBuffer = m_pStructDataBuffer;
	while(l_ui32Received < m_ui32DataBufferLength)
	{
		l_ui32ReqLength = m_ui32DataBufferLength -  l_ui32Received;
		l_ui32Result = m_pConnectionClient->receiveBuffer(l_pTmpBuffer, l_ui32ReqLength);
		l_ui32Received += l_ui32Result;
		l_pTmpBuffer += l_ui32Result;
	}

	// Input flow is equal to output one
	if (m_ui32SampleCountPerSentBlock == *m_structFixedHeader.m_ui32NumSamples)
	{
		// Data
		for (uint32 i=0; i < l_ui32NumChannels; i++)
		{
			for (uint32 j=0; j < m_ui32SampleCountPerSentBlock; j++)
			{
				if ((m_structVariableHeader.m_i32ChannelTypeIndex[i]==0) || (m_structVariableHeader.m_i32ChannelTypeIndex[i]==1) || (m_structVariableHeader.m_i32ChannelTypeIndex[i]==5))
				{
					m_pSample[j + (i*m_ui32SampleCountPerSentBlock)] = (float32)m_structData.m_pDataBuffer[(l_ui32NumChannels*j) + i] * 1e15 / m_oHeader.getChannelGain(i);
				}
				else
				{
					m_pSample[j + (i*m_ui32SampleCountPerSentBlock)] = (float32)m_structData.m_pDataBuffer[(l_ui32NumChannels*j) + i] / m_oHeader.getChannelGain(i);
				}
			}
		}

		// Count for markers and get informations
		CStimulationSet l_oStimulationSet;
		uint32 l_ui32NumberOfMarkers = 0, *l_pEventCode=NULL;
		uint64 *l_pDate=NULL;
		uint32 l_ui32Value;
		for (uint32 l_ui32iStimChan=0; l_ui32iStimChan<m_ui32NbStimChannels; l_ui32iStimChan++)
		{
			for (uint32 j=0; j <m_ui32SampleCountPerSentBlock; j++)
			{
				l_ui32Value = m_structData.m_pDataBuffer[(l_ui32NumChannels*j) + l_ui32iStimChan];
				if ((l_ui32Value != 0) && (m_pPrevStimValue[l_ui32iStimChan] != l_ui32Value))
				{
					l_ui32NumberOfMarkers++;
					m_pPrevStimValue[l_ui32iStimChan] = l_ui32Value;

					// Reallocations
					l_pEventCode = (uint32 *)realloc(l_pEventCode, l_ui32NumberOfMarkers*sizeof(uint32));
					l_pDate = (uint64 *)realloc(l_pDate, l_ui32NumberOfMarkers*sizeof(uint64));
					if ((l_pEventCode == NULL) || (l_pDate == NULL))
					{
						std::cout << "ERROR : memory allocation while decoding stimulations." << std::endl;
					}

					// Get event code and sample
					l_pEventCode[l_ui32NumberOfMarkers-1] = l_ui32Value;
					l_pDate[l_ui32NumberOfMarkers-1] = l_oStimulationSet.sampleIndexToTime((uint32)*m_structFixedHeader.m_f32SamplingRate, (uint64) j);
				}
				if (l_ui32Value == 0)
				{
					m_pPrevStimValue[l_ui32iStimChan] = 0;
				}
			}
		}
		
		l_oStimulationSet.setStimulationCount(l_ui32NumberOfMarkers);
		for (uint32 l_ui32MarkerCount=0; l_ui32MarkerCount<l_ui32NumberOfMarkers; l_ui32MarkerCount++)
		{
			l_oStimulationSet.setStimulationIdentifier(l_ui32MarkerCount, OVTK_StimulationId_Label(l_pEventCode[l_ui32MarkerCount]));
			l_oStimulationSet.setStimulationDate(l_ui32MarkerCount, l_pDate[l_ui32MarkerCount]);
			l_oStimulationSet.setStimulationDuration(l_ui32MarkerCount, 0);
		}

		free(l_pEventCode);
		free(l_pDate);
		
		// send data
		m_pCallback->setSamples(m_pSample);
		// send stimulations
		m_pCallback->setStimulationSet(l_oStimulationSet);
	}
	else
	{
		std::cout<< "ERROR : bad number of samples (" << (uint32)*m_structFixedHeader.m_ui32NumSamples<<", must be "<< m_ui32SampleCountPerSentBlock << ")." << std::endl;
	}

	return true;
}


boolean CDriverCtfVsmMeg::stop(void)
{
	cout << "> Connection stopped" << std::endl;
	
	if(!m_bInitialized)
	{
		return false;
	}

	if(!m_bStarted)
	{
		return false;
	}

	m_bStarted=false;
	return !m_bStarted;
}

boolean CDriverCtfVsmMeg::uninitialize(void)
{
	if(!m_bInitialized)
	{
		return false;
	}

	if(m_bStarted)
	{
		return false;
	}

	m_bInitialized=false;

	// Free memory for fixed part of header
	free(m_pStructFixedHeaderBuffer);
	
	// Free memory for dynamic header part
	free(m_pStructVariableHeaderBuffer);
	
	free(m_pStimChannelIndex);
	m_pStimChannelIndex = NULL;
	free(m_pPrevStimValue);
	m_pPrevStimValue = NULL;
	m_ui32NbStimChannels = 0;
	
	delete [] m_pSample;
	m_pSample=NULL;
	m_pCallback=NULL;

	free(m_pStructDataBuffer);
	m_pStructDataBuffer = NULL;

	// Cleans up client connection
	m_pConnectionClient->close();
	m_pConnectionClient->release();
	m_pConnectionClient=NULL;

	cout << "> Client disconnected" << std::endl;

	return true;
}

//___________________________________________________________________//
//                                                                   //

boolean CDriverCtfVsmMeg::isConfigurable(void)
{
	return true;
}

boolean CDriverCtfVsmMeg::configure(void)
{
	CConfigurationNetworkGlade l_oConfiguration("../share/openvibe-applications/acquisition-server/interface-CtfVsm-Meg.glade");

	l_oConfiguration.setHostName(m_sServerHostName);
	l_oConfiguration.setHostPort(m_ui32ServerHostPort);

	if(l_oConfiguration.configure(m_oHeader))
	{
		m_sServerHostName=l_oConfiguration.getHostName();
		m_ui32ServerHostPort=l_oConfiguration.getHostPort();
		return true;
	}

	return false;
}


// Points structure fields to buffer adresses.
void  CDriverCtfVsmMeg::bufferToFixedHeaderStruct(void)
{
	uint32 l_ui32Offset, l_ui32Length;

	l_ui32Offset = 0;
	l_ui32Length = sizeof(Str60);
	m_structFixedHeader.m_strExperimentId = &m_pStructFixedHeaderBuffer[l_ui32Offset];
	l_ui32Offset += l_ui32Length;

	l_ui32Length = sizeof(Str32);
	m_structFixedHeader.m_strExperimenDate = &m_pStructFixedHeaderBuffer[l_ui32Offset];
	l_ui32Offset += l_ui32Length;
	
	l_ui32Length = sizeof(Str32);
	m_structFixedHeader.m_strSubjectName = &m_pStructFixedHeaderBuffer[l_ui32Offset];
	l_ui32Offset += l_ui32Length;
	
	l_ui32Length = sizeof(int);
	m_structFixedHeader.m_i32SubjectAge = (int32 *)&m_pStructFixedHeaderBuffer[l_ui32Offset];
	l_ui32Offset += l_ui32Length;
	
	l_ui32Length = 4*sizeof(char);
	m_structFixedHeader.m_strSubjectSex = &m_pStructFixedHeaderBuffer[l_ui32Offset];
	l_ui32Offset += l_ui32Length;

	l_ui32Length = sizeof(int);
	m_structFixedHeader.m_i32LabId = (int32 *)&m_pStructFixedHeaderBuffer[l_ui32Offset];
	l_ui32Offset += l_ui32Length;
	
	l_ui32Length = sizeof(Str32);
	m_structFixedHeader.m_strLabName = &m_pStructFixedHeaderBuffer[l_ui32Offset];
	l_ui32Offset += l_ui32Length;
	
	l_ui32Length = sizeof(int);
	m_structFixedHeader.m_i32TechnicianId = (int32 *)&m_pStructFixedHeaderBuffer[l_ui32Offset];
	l_ui32Offset += l_ui32Length;
	
	l_ui32Length = sizeof(Str32);
	m_structFixedHeader.m_strTechnicianName = &m_pStructFixedHeaderBuffer[l_ui32Offset];
	l_ui32Offset += l_ui32Length;
	
	l_ui32Length = sizeof(float32);
	m_structFixedHeader.m_f32SamplingRate = (float *)&m_pStructFixedHeaderBuffer[l_ui32Offset];
	l_ui32Offset += l_ui32Length;
	
	l_ui32Length = sizeof(int);
	m_structFixedHeader.m_i32NumberOfChannels = (int32 *)&m_pStructFixedHeaderBuffer[l_ui32Offset];
	l_ui32Offset += l_ui32Length;

	l_ui32Length = 9*sizeof(double);
	m_structFixedHeader.m_f64PosRefNaLeReDew = (double *)&m_pStructFixedHeaderBuffer[l_ui32Offset];
	l_ui32Offset += l_ui32Length;

	l_ui32Length = sizeof(int);
	m_structFixedHeader.m_ui32NumSamples = (uint32 *)&m_pStructFixedHeaderBuffer[l_ui32Offset];
	l_ui32Offset += l_ui32Length;
}


// Points structure fields to buffer adresses.
void CDriverCtfVsmMeg::bufferToVariableHeaderStruct(void)
{
	uint32 l_ui32Offset, l_ui32Length;
	
	l_ui32Offset = 0;
	l_ui32Length = *m_structFixedHeader.m_i32NumberOfChannels*sizeof(Str32);
	m_structVariableHeader.m_strChannelLabel = (Str32 *)&m_pStructVariableHeaderBuffer[l_ui32Offset];
	l_ui32Offset += l_ui32Length;

	l_ui32Length = *m_structFixedHeader.m_i32NumberOfChannels*sizeof(int32);
	m_structVariableHeader.m_i32ChannelTypeIndex = (int32 *)&m_pStructVariableHeaderBuffer[l_ui32Offset];
	l_ui32Offset += l_ui32Length;
	
	l_ui32Length = *m_structFixedHeader.m_i32NumberOfChannels*sizeof(float);
	m_structVariableHeader.m_f32ProperGain = (float32 *)&m_pStructVariableHeaderBuffer[l_ui32Offset];
	l_ui32Offset += l_ui32Length;
	
	l_ui32Length = *m_structFixedHeader.m_i32NumberOfChannels*sizeof(float);
	m_structVariableHeader.m_f32QGain = (float32 *)&m_pStructVariableHeaderBuffer[l_ui32Offset];
	l_ui32Offset += l_ui32Length;
	
	l_ui32Length = *m_structFixedHeader.m_i32NumberOfChannels*sizeof(float);
	m_structVariableHeader.m_f32IOGain = (float32 *)&m_pStructVariableHeaderBuffer[l_ui32Offset];
	l_ui32Offset += l_ui32Length;
	
	l_ui32Length = *m_structFixedHeader.m_i32NumberOfChannels*sizeof(int32);
	m_structVariableHeader.m_i32GradOrderNum = (int32 *)&m_pStructVariableHeaderBuffer[l_ui32Offset];
	l_ui32Offset += l_ui32Length;
}


// Read CTF/VSM command header
void CDriverCtfVsmMeg::readCTFCommandHeader(void)
{
	uint32 l_ui32CommandHeaderLength, l_ui32Received, l_ui32ReqLength, l_ui32Result;
	char *l_pTmpBuffer;
	
	// Receive Command Header
	l_ui32CommandHeaderLength = 2*sizeof(uint32);
	l_pTmpBuffer = (char *)&m_structCommandHeader;
	l_ui32Received = 0;
	
	while(l_ui32Received < l_ui32CommandHeaderLength)
	{
		l_ui32ReqLength = l_ui32CommandHeaderLength -  l_ui32Received;
		l_ui32Result = m_pConnectionClient->receiveBuffer(l_pTmpBuffer, l_ui32ReqLength);

		l_ui32Received += l_ui32Result;
		l_pTmpBuffer += l_ui32Result;
	}
}

