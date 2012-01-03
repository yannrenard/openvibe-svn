#include "ovpCBoxAlgorithmBCI2000Reader.h"
#include <iostream>
#include <sstream>
#include <cstdio>
#include <cmath>
#include "bci2000helper.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::FileIO;
using namespace OpenViBEToolkit;

using namespace BCI2000;

boolean CBoxAlgorithmBCI2000Reader::initialize(void)
{
	CString l_sFilename=FSettingValueAutoCast(*this->getBoxAlgorithmContext(),0);
	m_pB2KReaderHelper=new BCI2000Reader(l_sFilename);
	if (!m_pB2KReaderHelper->is_good())
	{
		this->getLogManager() << LogLevel_ImportantWarning << "Could not open file [" << l_sFilename << "]\n";
		m_pB2KReaderHelper = NULL;
		return false;
	}
	else
	{
		std::stringstream l_sStream;
		m_pB2KReaderHelper->print_info(l_sStream);
		this->getLogManager() << LogLevel_Trace << "Metadata from [" << l_sFilename << "] :\n" << l_sStream.str().c_str() << "\n";
	}

	m_bHeaderSent=false;
	m_ui32Rate=m_pB2KReaderHelper->get_rate();
	m_ui32ChannelCount=m_pB2KReaderHelper->get_channels();
	m_ui32SampleCountPerBuffer=(uint64)FSettingValueAutoCast(*this->getBoxAlgorithmContext(),1);
	m_ui32SamplesSent=0;
	m_pBuffer=new float64[m_ui32ChannelCount*m_ui32SampleCountPerBuffer];
	m_pStates=new uint32[m_pB2KReaderHelper->get_state_vector_size()*m_ui32SampleCountPerBuffer];

	m_oSignalEncoder.initialize(*this);
	m_pSignalOutputMatrix=m_oSignalEncoder.getInputMatrix();
	m_pSignalOutputMatrix->setDimensionCount(2);
	m_pSignalOutputMatrix->setDimensionSize(0,m_ui32ChannelCount);
	m_pSignalOutputMatrix->setDimensionSize(1,m_ui32SampleCountPerBuffer);
	for (uint32 i=0; i<m_ui32ChannelCount; i++)
	{
		std::stringstream l_sName;
		l_sName << "Channel " << i;
		m_pSignalOutputMatrix->setDimensionLabel(0,i,l_sName.str().c_str());
	}

	m_oStateEncoder.initialize(*this);
	m_pStateOutputMatrix=m_oStateEncoder.getInputMatrix();
	m_pStateOutputMatrix->setDimensionCount(2);
	m_pStateOutputMatrix->setDimensionSize(0,m_pB2KReaderHelper->get_state_vector_size());
	m_pStateOutputMatrix->setDimensionSize(1,m_ui32SampleCountPerBuffer);
	for (int i=0; i<m_pB2KReaderHelper->get_state_vector_size(); i++)
	{
		m_pStateOutputMatrix->setDimensionLabel(0,i,m_pB2KReaderHelper->get_state_name(i).c_str());
	}
	m_oSignalEncoder.getInputSamplingRate()=m_ui32Rate;
	m_oStateEncoder.getInputSamplingRate()=m_ui32Rate;
	for (int i=0; i<m_pB2KReaderHelper->get_state_vector_size(); i++)
	{
		this->getLogManager() << LogLevel_Trace << "BCI2000 state var "<< i << " is : " << m_pB2KReaderHelper->get_state_name(i).c_str() << "\n";
	}
	return true;
}

boolean CBoxAlgorithmBCI2000Reader::uninitialize(void)
{
	if(m_pB2KReaderHelper != NULL)
	{
		delete m_pB2KReaderHelper;
		delete[] m_pBuffer;
		delete[] m_pStates;
	}
	
	m_oSignalEncoder.uninitialize();
	m_oStateEncoder.uninitialize();

	return true;
}

boolean CBoxAlgorithmBCI2000Reader::processClock(IMessageClock& rMessageClock)
{
	if (m_pB2KReaderHelper->get_samples_left()>0)
	{
		getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
		return true;
	}

	return false;
}

uint64 CBoxAlgorithmBCI2000Reader::getClockFrequency(void)
{
	return ((uint64)m_ui32Rate<<32)/m_ui32SampleCountPerBuffer;
}

void CBoxAlgorithmBCI2000Reader::sendHeader(void)
{
	m_oSignalEncoder.encodeHeader(0);
	m_oStateEncoder.encodeHeader(1);
	m_bHeaderSent=true;
	getDynamicBoxContext().markOutputAsReadyToSend(0,0,0);
	getDynamicBoxContext().markOutputAsReadyToSend(1,0,0);
}

boolean CBoxAlgorithmBCI2000Reader::process(void)
{
	if (!m_bHeaderSent)
	{
		sendHeader();
	}

	//prepare data
	int32 l_ui32SamplesRead=m_pB2KReaderHelper->read_samples(m_pBuffer,m_pStates,m_ui32SampleCountPerBuffer);
	if (l_ui32SamplesRead>0)
	{
		// padding. TODO: is it necessary ? or even dangerous ?
		for (uint32 i=l_ui32SamplesRead; i<m_ui32SampleCountPerBuffer; i++)
		{
			for (uint32 j=0; j<m_ui32ChannelCount; j++)
			{
				m_pBuffer[i*m_ui32ChannelCount+j]=0.0;
			}
		}
		// transpose (yeah, I know... ugly)
		for (uint32 i=0; i<m_ui32SampleCountPerBuffer; i++)
		{
			for (uint32 j=0; j<m_ui32ChannelCount; j++)
			{
				m_pSignalOutputMatrix->getBuffer()[j*m_ui32SampleCountPerBuffer+i]=m_pBuffer[i*m_ui32ChannelCount+j];
			}
		}
		m_oSignalEncoder.encodeBuffer(0);
		uint64 StartTime;
		uint64 EndTime;
		StartTime=(((uint64)(m_ui32SamplesSent))<<32)/m_ui32Rate;
		EndTime=(((uint64)(m_ui32SamplesSent+m_ui32SampleCountPerBuffer))<<32)/m_ui32Rate;
		m_ui32SamplesSent+=l_ui32SamplesRead;
		if (m_pB2KReaderHelper->get_samples_left()==0)
		{
			m_oSignalEncoder.encodeEnd(0);
		}

		getDynamicBoxContext().markOutputAsReadyToSend(0,StartTime,EndTime);

		// padding. TODO: is it necessary ? or even dangerous ?
		for (uint32 i=l_ui32SamplesRead; i<m_ui32SampleCountPerBuffer; i++)
		{
			for (int j=0; j<m_pB2KReaderHelper->get_state_vector_size(); j++)
			{
				m_pStates[i*m_pB2KReaderHelper->get_state_vector_size()+j]=0;
			}
		}
		// transpose (yeah, I know... ugly)
		for (uint32 i=0; i<m_ui32SampleCountPerBuffer; i++)
		{
			for (int j=0; j<m_pB2KReaderHelper->get_state_vector_size(); j++)
			{
				m_pStateOutputMatrix->getBuffer()[j*m_ui32SampleCountPerBuffer+i]=
					m_pStates[i*m_pB2KReaderHelper->get_state_vector_size()+j];
			}
		}
		m_oStateEncoder.encodeBuffer(1);

		if (m_pB2KReaderHelper->get_samples_left()==0)
		{
			m_oSignalEncoder.encodeEnd(1);
		}

		getDynamicBoxContext().markOutputAsReadyToSend(1,StartTime,EndTime);
	}
	return true;
}
