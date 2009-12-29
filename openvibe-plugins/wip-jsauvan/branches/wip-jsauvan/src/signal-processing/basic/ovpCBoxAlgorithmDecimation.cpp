#include "ovpCBoxAlgorithmDecimation.h"
#include <iostream>

//===================================================================================================================================
//===================================================================================================================================
//===================================================================================================================================

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;

using namespace std;

boolean CBoxAlgorithmDecimation::initialize(void)
{

	CString l_sSettingValue;

	getStaticBoxContext().getSettingValue(0, l_sSettingValue);
	m_ui64DecimationFactor = static_cast<uint32> (atoi(l_sSettingValue));

	CIdentifier l_oInputTypeIdentifier;
	getStaticBoxContext().getInputType(0, l_oInputTypeIdentifier);
	if(l_oInputTypeIdentifier==OV_TypeId_StreamedMatrix)
	{
		m_pStreamDecoder=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StreamedMatrixStreamDecoder));
		m_pStreamEncoder=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StreamedMatrixStreamEncoder));
	}
	else if(l_oInputTypeIdentifier==OV_TypeId_FeatureVector)
	{
		m_pStreamDecoder=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_FeatureVectorStreamDecoder));
		m_pStreamEncoder=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_FeatureVectorStreamEncoder));
	}
	else if(l_oInputTypeIdentifier==OV_TypeId_Signal)
	{
		m_pStreamDecoder=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SignalStreamDecoder));
		m_pStreamEncoder=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SignalStreamEncoder));
	}
	else if(l_oInputTypeIdentifier==OV_TypeId_Spectrum)
	{
		m_pStreamDecoder=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SpectrumStreamDecoder));
		m_pStreamEncoder=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SpectrumStreamEncoder));
	}
	else
	{
		return false;
	}
	m_pStreamDecoder->initialize();
	m_pStreamEncoder->initialize();


	if(l_oInputTypeIdentifier==OV_TypeId_StreamedMatrix)
	{
	}
	else if(l_oInputTypeIdentifier==OV_TypeId_FeatureVector)
	{
	}
	else if(l_oInputTypeIdentifier==OV_TypeId_Signal)
	{
		/*uint64* l_ui64SamplingRate;
		m_pStreamDecoder->getOutputParameter(OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_SamplingRate)->getValue(l_ui64SamplingRate);
		cerr<<"frequence echantillonage entree :"<<*l_ui64SamplingRate<<endl;
		*l_ui64SamplingRate = *l_ui64SamplingRate/m_ui64DecimationFactor;
		cerr<<"frequence echantillonage :"<<*l_ui64SamplingRate<<endl;
		m_pStreamEncoder->getInputParameter(OVP_GD_Algorithm_SignalStreamEncoder_InputParameterId_SamplingRate)->setValue(l_ui64SamplingRate);*/
	}
	else if(l_oInputTypeIdentifier==OV_TypeId_Spectrum)
	{
		m_pStreamEncoder->getInputParameter(OVP_GD_Algorithm_SpectrumStreamEncoder_InputParameterId_MinMaxFrequencyBands)->setReferenceTarget(m_pStreamDecoder->getOutputParameter(OVP_GD_Algorithm_SpectrumStreamDecoder_OutputParameterId_MinMaxFrequencyBands));
	}




	m_oInputMemoryBufferHandle.initialize(m_pStreamDecoder->getInputParameter(OVP_GD_Algorithm_SignalStreamDecoder_InputParameterId_MemoryBufferToDecode));
	m_oOutputMemoryBufferHandle.initialize( m_pStreamEncoder->getOutputParameter(OVP_GD_Algorithm_SignalStreamEncoder_OutputParameterId_EncodedMemoryBuffer) );	

	m_ui64LastStartTime=0;


	return true;
}

boolean CBoxAlgorithmDecimation::uninitialize(void)
{

	m_oInputMemoryBufferHandle.uninitialize();
	m_oOutputMemoryBufferHandle.uninitialize();
	m_pStreamEncoder->uninitialize();
	m_pStreamDecoder->uninitialize();

	getAlgorithmManager().releaseAlgorithm(*m_pStreamEncoder);
	getAlgorithmManager().releaseAlgorithm(*m_pStreamDecoder);

	

	return true;
}

boolean CBoxAlgorithmDecimation::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

boolean CBoxAlgorithmDecimation::process(void)
{
	IBoxIO& l_rDynamicBoxContext=getDynamicBoxContext();
	IBox& l_rStaticBoxContext=getStaticBoxContext();

	uint32 l_ui32NbDimensions;
	std::vector<uint32> l_vDimensionsSize;

	for(uint32 i=0; i<l_rStaticBoxContext.getInputCount(); i++)
	{
		for(uint32 j=0; j<l_rDynamicBoxContext.getInputChunkCount(i); j++)
		{
			m_oInputMemoryBufferHandle=l_rDynamicBoxContext.getInputChunk(i, j);
			m_oOutputMemoryBufferHandle=l_rDynamicBoxContext.getOutputChunk(i);
			uint64 l_ui64EndTime=m_ui64LastStartTime+l_rDynamicBoxContext.getInputChunkEndTime(i, j)-l_rDynamicBoxContext.getInputChunkStartTime(i, j);

			m_pStreamDecoder->process();
			if(m_pStreamDecoder->isOutputTriggerActive(OVP_GD_Algorithm_SignalStreamDecoder_OutputTriggerId_ReceivedHeader))
			{

				TParameterHandler < IMatrix* > l_oHandleInput(m_pStreamDecoder->getOutputParameter(OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_Matrix));
				TParameterHandler < IMatrix* > l_oHandleOutput(m_pStreamEncoder->getInputParameter(OVP_GD_Algorithm_SignalStreamEncoder_InputParameterId_Matrix));
				TParameterHandler < uint64 > l_oHandleInputSamplingRate(m_pStreamDecoder->getOutputParameter(OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_SamplingRate));
				TParameterHandler < uint64 > l_oHandleOutputSamplingRate(m_pStreamEncoder->getInputParameter(OVP_GD_Algorithm_SignalStreamEncoder_InputParameterId_SamplingRate));

				l_oHandleOutputSamplingRate = l_oHandleInputSamplingRate/m_ui64DecimationFactor;

							OpenViBEToolkit::Tools::Matrix::copyDescription(*l_oHandleOutput, *l_oHandleInput);


				l_ui32NbDimensions = l_oHandleInput->getDimensionCount();
				for (uint32 k=0; k<l_ui32NbDimensions; k++)
				{
					//cerr<<"dimension "<<k<<endl;
					l_vDimensionsSize.push_back(l_oHandleInput->getDimensionSize(k));
					cerr<<"dimension size :"<<l_vDimensionsSize[k]<<endl; 

				}

				l_oHandleOutput->setDimensionSize(1, (l_vDimensionsSize[1]-1)/m_ui64DecimationFactor+1);


				m_pStreamEncoder->process(OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeHeader);
				l_rDynamicBoxContext.markOutputAsReadyToSend(i, l_rDynamicBoxContext.getInputChunkStartTime(i, j),l_rDynamicBoxContext.getInputChunkEndTime(i, j));

			}
			if(m_pStreamDecoder->isOutputTriggerActive(OVP_GD_Algorithm_SignalStreamDecoder_OutputTriggerId_ReceivedBuffer))
			{
				TParameterHandler < IMatrix* > l_oHandleInput(m_pStreamDecoder->getOutputParameter(OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_Matrix));
				TParameterHandler < IMatrix* > l_oHandleOutput(m_pStreamEncoder->getInputParameter(OVP_GD_Algorithm_SignalStreamEncoder_InputParameterId_Matrix));
				

				OpenViBEToolkit::Tools::Matrix::copyDescription(*l_oHandleOutput, *l_oHandleInput);
				OpenViBEToolkit::Tools::Matrix::clearContent(*l_oHandleOutput);

				//cerr<<"frequence d'echantillonage :"<<*l_ui64SamplingRate<<endl;
				if(l_oHandleInput.exists())
				{
					l_ui32NbDimensions = l_oHandleInput->getDimensionCount();
					for (uint32 k=0; k<l_ui32NbDimensions; k++)
					{
						//cerr<<"dimension "<<k<<endl;
						l_vDimensionsSize.push_back(l_oHandleInput->getDimensionSize(k));
						//cerr<<"dimension size :"<<l_vDimensionsSize[k]<<endl; 
						
					}

					l_oHandleOutput->setDimensionSize(1, (l_vDimensionsSize[1]-1)/m_ui64DecimationFactor+1);
					float64* l_pInputMatrixBuffer=l_oHandleInput->getBuffer();
					float64* l_pOutputMatrixBuffer=l_oHandleOutput->getBuffer();

					for(uint32 k=0;k<l_vDimensionsSize[0];k++)
					{
						uint32 l = 0;
						while(l<l_vDimensionsSize[1])
						{
							if(l%m_ui64DecimationFactor==0)
							{
								//cerr<<"buffer :"<<*l_pInputMatrixBuffer<<endl;
								*l_pOutputMatrixBuffer = *l_pInputMatrixBuffer;
								l_pOutputMatrixBuffer++;
							}
							l_pInputMatrixBuffer++;
							l++;
							//cerr<<"l :"<<l<<endl;
						}
					}

				}

				m_pStreamEncoder->process(OVP_GD_Algorithm_SignalStreamEncoder_InputTriggerId_EncodeBuffer);
				l_rDynamicBoxContext.markOutputAsReadyToSend(i, l_rDynamicBoxContext.getInputChunkStartTime(i, j),l_rDynamicBoxContext.getInputChunkEndTime(i, j));
			}
			if(m_pStreamDecoder->isOutputTriggerActive(OVP_GD_Algorithm_SignalStreamDecoder_OutputTriggerId_ReceivedEnd))
			{
				m_pStreamEncoder->process(OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeEnd);
				l_rDynamicBoxContext.markOutputAsReadyToSend(i, l_rDynamicBoxContext.getInputChunkStartTime(i, j),l_rDynamicBoxContext.getInputChunkEndTime(i, j));
			}

			m_ui64LastStartTime=l_rDynamicBoxContext.getInputChunkStartTime(i, j);
			l_rDynamicBoxContext.markInputAsDeprecated(i, j);
		}
	}

	l_vDimensionsSize.clear();

	return true;
}
