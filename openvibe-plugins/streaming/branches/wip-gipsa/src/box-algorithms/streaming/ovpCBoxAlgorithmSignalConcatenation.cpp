#include "ovpCBoxAlgorithmSignalConcatenation.h"

#include <iostream>
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::Local;

boolean CBoxAlgorithmSignalConcatenation::initialize(void)
{
	m_pSignalDecoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SignalStreamDecoder));
    m_pSignalDecoder->initialize();
    ip_pMemoryBufferToDecode1.initialize(m_pSignalDecoder->getInputParameter(OVP_GD_Algorithm_SignalStreamDecoder_InputParameterId_MemoryBufferToDecode));
    op_pDecodedMatrix1.initialize(m_pSignalDecoder->getOutputParameter(OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_Matrix));
	op_ui64SamplingRate1.initialize(m_pSignalDecoder->getOutputParameter(OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_SamplingRate));

	m_pSignalEncoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SignalStreamEncoder));
    m_pSignalEncoder->initialize();
    ip_pMatrixToEncode.initialize(m_pSignalEncoder->getInputParameter(OVP_GD_Algorithm_SignalStreamEncoder_InputParameterId_Matrix));
    op_pEncodedMemoryBuffer.initialize(m_pSignalEncoder->getOutputParameter(OVP_GD_Algorithm_SignalStreamEncoder_OutputParameterId_EncodedMemoryBuffer));
    ip_ui64SamplingRate.initialize(m_pSignalEncoder->getInputParameter(OVP_GD_Algorithm_SignalStreamEncoder_InputParameterId_SamplingRate));

	ip_ui64SamplingRate.setReferenceTarget(op_ui64SamplingRate1);
	return true;
}

boolean CBoxAlgorithmSignalConcatenation::uninitialize(void)
{
	op_pDecodedMatrix1.uninitialize();
    ip_pMemoryBufferToDecode1.uninitialize();
	op_ui64SamplingRate1.uninitialize();
	m_pSignalDecoder->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pSignalDecoder);

	ip_pMatrixToEncode.uninitialize();
	op_pEncodedMemoryBuffer.uninitialize();
    ip_ui64SamplingRate.uninitialize();
	m_pSignalEncoder->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pSignalEncoder);

	return true;
}

boolean CBoxAlgorithmSignalConcatenation::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}


boolean CBoxAlgorithmSignalConcatenation::process(void)
{
	IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	IBoxIO* l_pDynamicBoxContext=getBoxAlgorithmContext()->getDynamicBoxContext();

	uint32 l_uiminChunkInputCount=l_pDynamicBoxContext->getInputChunkCount(0);
	for(uint32 i=1; i<l_rStaticBoxContext.getInputCount(); i++)
	  {l_uiminChunkInputCount=std::min(l_pDynamicBoxContext->getInputChunkCount(i),l_uiminChunkInputCount);}
	
	for(uint32 j=0; j<l_uiminChunkInputCount; j++)
	  {
		uint32 l_uidecoderType=0;
		int64 i64samplingRate=-1;
		uint32 l_uiChannelCount=0;
		uint32 l_uiSampleCount=0;
		uint64 l_ui64IdxCount=0;
		uint64 l_ui64StartTime=0,l_ui64EndTime=0;
		for(uint32 i=0; i<l_rStaticBoxContext.getInputCount(); i++)
		  {
			//décodage
			ip_pMemoryBufferToDecode1=l_pDynamicBoxContext->getInputChunk(i, j);
			m_pSignalDecoder->process();

			//uniformisation
			if(l_uidecoderType<4)
			  {
				if(m_pSignalDecoder->isOutputTriggerActive(OVP_GD_Algorithm_SignalStreamDecoder_OutputTriggerId_ReceivedHeader))
				  {
					l_uidecoderType=(l_uidecoderType==0 ? 1 : (l_uidecoderType!=1 ? 10 : l_uidecoderType));
				  }
				if(m_pSignalDecoder->isOutputTriggerActive(OVP_GD_Algorithm_SignalStreamDecoder_OutputTriggerId_ReceivedBuffer))
				  {
					l_uidecoderType=(l_uidecoderType==0 ? 2 : (l_uidecoderType!=2 ? 10 : l_uidecoderType));
				  }
				if(m_pSignalDecoder->isOutputTriggerActive(OVP_GD_Algorithm_SignalStreamDecoder_OutputTriggerId_ReceivedEnd))
				  {
					l_uidecoderType=(l_uidecoderType==0 ? 3 : (l_uidecoderType!=3 ? 10 : l_uidecoderType));
				  }
			  }
			
			//action
			if(l_uidecoderType==1) //header
			  {
				// Test of entry flux sampling rate coherency
				if(i64samplingRate==-1) {i64samplingRate=op_ui64SamplingRate1;} 
				if(i64samplingRate!=op_ui64SamplingRate1) 
				  {
					getLogManager() << LogLevel_Warning << "different Sampling : "<<i64samplingRate<<"Hz | "<<op_ui64SamplingRate1<<"Hz.\n";
					return false;
				  }
				
				//// Test chunk size coherency
				if(l_uiSampleCount==0) {l_uiSampleCount=op_pDecodedMatrix1->getDimensionSize(1);}
				if(l_uiSampleCount!=op_pDecodedMatrix1->getDimensionSize(1))
				  {
					getLogManager() << LogLevel_Warning << "different Epoch size : "<<l_uiSampleCount<<" | "<<op_pDecodedMatrix1->getDimensionSize(1)<<".\n";
					return false;
				  }

				//Memory reservation
				m_vuiChannelsCount.push_back(op_pDecodedMatrix1->getDimensionSize(0));
				l_uiChannelCount+=op_pDecodedMatrix1->getDimensionSize(0);
				ip_pMatrixToEncode->setDimensionCount(2);
				ip_pMatrixToEncode->setDimensionSize(0,l_uiChannelCount);
				ip_pMatrixToEncode->setDimensionSize(1,l_uiSampleCount);
				//Channel names
				const char* l_pChannelLabel;
				for (uint32 a=l_uiChannelCount-op_pDecodedMatrix1->getDimensionSize(0); a<l_uiChannelCount; a++)
					{
						l_pChannelLabel=op_pDecodedMatrix1->getDimensionLabel(0,a);
						ip_pMatrixToEncode->setDimensionLabel(0,a,l_pChannelLabel);
					}
					
				//send data
				if(i==l_rStaticBoxContext.getInputCount()-1)
				  {
					op_pEncodedMemoryBuffer=l_pDynamicBoxContext->getOutputChunk(0);
					m_pSignalEncoder->process(OVP_GD_Algorithm_SignalStreamEncoder_InputTriggerId_EncodeHeader);
					l_ui64StartTime=l_pDynamicBoxContext->getInputChunkStartTime(0, j);
					l_ui64EndTime=l_pDynamicBoxContext->getInputChunkEndTime(0, j);
					l_pDynamicBoxContext->markOutputAsReadyToSend(0,l_ui64StartTime ,l_ui64EndTime );
				  }
			  }
			//
			if(l_uidecoderType==2)//buffer
			  {
				//real time security
				if( (l_ui64StartTime !=0 && l_ui64StartTime!=l_pDynamicBoxContext->getInputChunkStartTime(i, j))
					|| (l_ui64EndTime!=0 && l_ui64EndTime!=l_pDynamicBoxContext->getInputChunkEndTime(i, j)) )
				  {
					getLogManager() << LogLevel_Warning << "different Time Chunk : "<<l_ui64StartTime<<" | "<<l_pDynamicBoxContext->getInputChunkStartTime(i, j)
																					<<l_ui64EndTime<<" | "<<l_pDynamicBoxContext->getInputChunkEndTime(i, j)<<".\n";
					return false;
				  }
					
				l_ui64StartTime=l_pDynamicBoxContext->getInputChunkStartTime(i, j);
				l_ui64EndTime=l_pDynamicBoxContext->getInputChunkEndTime(i, j);
					
				for(uint32 a=0; a<m_vuiChannelsCount.at(i)*op_pDecodedMatrix1->getDimensionSize(1); a++)
				  {
					ip_pMatrixToEncode->getBuffer()[l_ui64IdxCount+a]=op_pDecodedMatrix1->getBuffer()[a];
				  }
				l_ui64IdxCount+=m_vuiChannelsCount.at(i)*op_pDecodedMatrix1->getDimensionSize(1);
				if(l_ui64IdxCount>ip_pMatrixToEncode->getDimensionSize(0)*ip_pMatrixToEncode->getDimensionSize(1))
				  {
					getLogManager() << LogLevel_Warning <<"overMemory : "<<l_ui64IdxCount<<" / "<<ip_pMatrixToEncode->getDimensionSize(0)*ip_pMatrixToEncode->getDimensionSize(1)<<"\n";
					return false;
				  }
				
				//send data
				if(i==l_rStaticBoxContext.getInputCount()-1)
				  {
					op_pEncodedMemoryBuffer=l_pDynamicBoxContext->getOutputChunk(0);
					m_pSignalEncoder->process(OVP_GD_Algorithm_SignalStreamEncoder_InputTriggerId_EncodeBuffer);
					l_pDynamicBoxContext->markOutputAsReadyToSend(0,l_ui64StartTime ,l_ui64EndTime );
				  }
			  }
			//
			if(l_uidecoderType==3)//ender
			  {
			  }

			l_pDynamicBoxContext->markInputAsDeprecated(i,j);
		  }
	  }

	return true;
}
