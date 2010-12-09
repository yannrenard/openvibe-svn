#include "ovpCBoxAlgorithmOnlineStatistics.h"

#include <cstdlib>
#include <cstdio>
#include <cmath>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;
using namespace std;

boolean CBoxAlgorithmOnlineStatistics::initialize(void)
{
	//setting up the signal decoder and its input/outputs
	m_pSignalDecoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SignalStreamDecoder));
	m_pSignalDecoder->initialize();
	ip_pMemoryBufferToDecode.initialize(m_pSignalDecoder->getInputParameter(OVP_GD_Algorithm_SignalStreamDecoder_InputParameterId_MemoryBufferToDecode));
	op_ui64SamplingRate.initialize(m_pSignalDecoder->getOutputParameter(OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_SamplingRate));
	op_pDecodedMatrix.initialize(m_pSignalDecoder->getOutputParameter(OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_Matrix));
	
	//setting up the MEAN signal encoder and its input/outputs
	m_pMeanSignalEncoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SignalStreamEncoder));
	m_pMeanSignalEncoder->initialize();
	ip_ui64MeanSamplingRate.initialize(m_pMeanSignalEncoder->getInputParameter(OVP_GD_Algorithm_SignalStreamEncoder_InputParameterId_SamplingRate));
	ip_pMeanMatrixToEncode.initialize(m_pMeanSignalEncoder->getInputParameter(OVP_GD_Algorithm_SignalStreamEncoder_InputParameterId_Matrix));
	op_pMeanEncodedMemoryBuffer.initialize(m_pMeanSignalEncoder->getOutputParameter(OVP_GD_Algorithm_SignalStreamEncoder_OutputParameterId_EncodedMemoryBuffer));
	
	//setting up the VARIANCE signal encoder and its input/outputs
	m_pVarianceSignalEncoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SignalStreamEncoder));
	m_pVarianceSignalEncoder->initialize();
	ip_ui64VarianceSamplingRate.initialize(m_pVarianceSignalEncoder->getInputParameter(OVP_GD_Algorithm_SignalStreamEncoder_InputParameterId_SamplingRate));
	ip_pVarianceMatrixToEncode.initialize(m_pVarianceSignalEncoder->getInputParameter(OVP_GD_Algorithm_SignalStreamEncoder_InputParameterId_Matrix));
	op_pVarianceEncodedMemoryBuffer.initialize(m_pVarianceSignalEncoder->getOutputParameter(OVP_GD_Algorithm_SignalStreamEncoder_OutputParameterId_EncodedMemoryBuffer));

	return true;
}

boolean CBoxAlgorithmOnlineStatistics::uninitialize(void)
{
	//uninitializing parameters and algorithms
	ip_pMeanMatrixToEncode.uninitialize();
	ip_ui64MeanSamplingRate.uninitialize();
	m_pMeanSignalEncoder->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pMeanSignalEncoder);

	ip_pVarianceMatrixToEncode.uninitialize();
	ip_ui64VarianceSamplingRate.uninitialize();
	m_pVarianceSignalEncoder->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pVarianceSignalEncoder);

	op_pDecodedMatrix.uninitialize();
	op_ui64SamplingRate.uninitialize();
	m_pSignalDecoder->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pSignalDecoder);

	return true;
}

boolean CBoxAlgorithmOnlineStatistics::processInput(uint32 ui32InputIndex)
{
	//if input is arrived, processing it
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}

boolean CBoxAlgorithmOnlineStatistics::process(void)
{
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

	//for each chunk on the input 0
	for(uint32 i=0; i<l_rDynamicBoxContext.getInputChunkCount(0); i++)
	{
		ip_pMemoryBufferToDecode=l_rDynamicBoxContext.getInputChunk(0, i);
		op_pMeanEncodedMemoryBuffer=l_rDynamicBoxContext.getOutputChunk(0);
		op_pVarianceEncodedMemoryBuffer=l_rDynamicBoxContext.getOutputChunk(1);

		m_pSignalDecoder->process();

		//HEADER
		if(m_pSignalDecoder->isOutputTriggerActive(OVP_GD_Algorithm_SignalStreamDecoder_OutputTriggerId_ReceivedHeader))
		{
			//create the matrices : only one value per chunk (mean or variance)
			OpenViBEToolkit::Tools::Matrix::copyDescription(*ip_pMeanMatrixToEncode, *op_pDecodedMatrix);
			OpenViBEToolkit::Tools::Matrix::copyDescription(*ip_pVarianceMatrixToEncode, *op_pDecodedMatrix);
			ip_pMeanMatrixToEncode->setDimensionSize(1,1);
			ip_pVarianceMatrixToEncode->setDimensionSize(1,1);
			uint32 l_ui32NbSamplesPerBuffer = op_pDecodedMatrix->getDimensionSize(1);

			ip_ui64MeanSamplingRate = ip_ui64MeanSamplingRate/l_ui32NbSamplesPerBuffer;
			ip_ui64VarianceSamplingRate = ip_ui64VarianceSamplingRate/l_ui32NbSamplesPerBuffer;

			//encode the headers 
			m_pMeanSignalEncoder->process(OVP_GD_Algorithm_SignalStreamEncoder_InputTriggerId_EncodeHeader);
			m_pVarianceSignalEncoder->process(OVP_GD_Algorithm_SignalStreamEncoder_InputTriggerId_EncodeHeader);
			
			//send the headers
			l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));
			l_rDynamicBoxContext.markOutputAsReadyToSend(1, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));
		}
		//BUFFER 
		if(m_pSignalDecoder->isOutputTriggerActive(OVP_GD_Algorithm_SignalStreamDecoder_OutputTriggerId_ReceivedBuffer))
		{
			//compute mean and variance, encode it and mark the output
			uint32 l_ui32NbChannels = op_pDecodedMatrix->getDimensionSize(0);
			uint32 l_ui32NbSamplesPerBuffer = op_pDecodedMatrix->getDimensionSize(1);

			float64* l_pBuffer = op_pDecodedMatrix->getBuffer();

			//for each channel, we compute the mean and variance
			for(uint32 c=0; c<l_ui32NbChannels; c++)
			{
				float64 l_f64Sum = 0;
				float64 l_f64SqrSum = 0;
				// we iterate over the samples
				for(uint32 s=0; s<l_ui32NbSamplesPerBuffer; s++)
				{
					l_f64Sum += l_pBuffer[c*l_ui32NbSamplesPerBuffer+s];
					l_f64SqrSum += (l_pBuffer[c*l_ui32NbSamplesPerBuffer+s]*l_pBuffer[c*l_ui32NbSamplesPerBuffer+s]);
				}

				float64 l_f64Mean = l_f64Sum/(float64)l_ui32NbSamplesPerBuffer;
				float64 l_f64Variance = l_f64SqrSum/(float64)l_ui32NbSamplesPerBuffer - l_f64Mean*l_f64Mean;

				ip_pMeanMatrixToEncode->getBuffer()[c] = l_f64Mean;
				ip_pVarianceMatrixToEncode->getBuffer()[c] = l_f64Variance;
			}

			m_pMeanSignalEncoder->process(OVP_GD_Algorithm_SignalStreamEncoder_InputTriggerId_EncodeBuffer);
			m_pVarianceSignalEncoder->process(OVP_GD_Algorithm_SignalStreamEncoder_InputTriggerId_EncodeBuffer);

			l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));
			l_rDynamicBoxContext.markOutputAsReadyToSend(1, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));
		}
		if(m_pSignalDecoder->isOutputTriggerActive(OVP_GD_Algorithm_SignalStreamDecoder_OutputTriggerId_ReceivedEnd))
		{
			m_pMeanSignalEncoder->process(OVP_GD_Algorithm_SignalStreamEncoder_InputTriggerId_EncodeEnd);
			m_pVarianceSignalEncoder->process(OVP_GD_Algorithm_SignalStreamEncoder_InputTriggerId_EncodeEnd);

			l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));
		}

		l_rDynamicBoxContext.markInputAsDeprecated(0, i);
	}

	return true;
}
