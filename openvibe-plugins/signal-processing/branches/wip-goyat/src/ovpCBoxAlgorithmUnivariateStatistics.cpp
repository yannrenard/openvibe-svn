#include "ovpCBoxAlgorithmUnivariateStatistics.h"
#include <cstdlib>
#include <iostream>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;

boolean CBoxUnivariateStatistic::initialize(void)
{
	getStaticBoxContext().getInputType(0, m_oInputTypeIdentifier);
	if(m_oInputTypeIdentifier==OV_TypeId_StreamedMatrix)
	{
		m_pStreamDecoder=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StreamedMatrixStreamDecoder));
		m_pStreamEncoderMean=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StreamedMatrixStreamEncoder));
		m_pStreamEncoderVariance=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StreamedMatrixStreamEncoder));
		m_pStreamEncoderRange=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StreamedMatrixStreamEncoder));
		m_pStreamEncoderMedian=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StreamedMatrixStreamEncoder));
		m_pStreamEncoderIQR=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StreamedMatrixStreamEncoder));
		m_pStreamEncoderPercentile=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StreamedMatrixStreamEncoder));
	}
	else if(m_oInputTypeIdentifier==OV_TypeId_FeatureVector)
	{
		m_pStreamDecoder=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_FeatureVectorStreamDecoder));
		m_pStreamEncoderMean=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_FeatureVectorStreamEncoder));
		m_pStreamEncoderVariance=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_FeatureVectorStreamEncoder));
		m_pStreamEncoderRange=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_FeatureVectorStreamEncoder));
		m_pStreamEncoderMedian=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_FeatureVectorStreamEncoder));
		m_pStreamEncoderIQR=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_FeatureVectorStreamEncoder));
		m_pStreamEncoderPercentile=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_FeatureVectorStreamEncoder));

	}
	else if(m_oInputTypeIdentifier==OV_TypeId_Signal)
	{
		m_pStreamDecoder=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SignalStreamDecoder));
		m_pStreamEncoderMean=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SignalStreamEncoder));
		m_pStreamEncoderVariance=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SignalStreamEncoder));
		m_pStreamEncoderRange=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SignalStreamEncoder));
		m_pStreamEncoderMedian=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SignalStreamEncoder));
		m_pStreamEncoderIQR=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SignalStreamEncoder));
		m_pStreamEncoderPercentile=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SignalStreamEncoder));
	}
	else if(m_oInputTypeIdentifier==OV_TypeId_Spectrum)
	{
		m_pStreamDecoder=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SpectrumStreamDecoder));
		m_pStreamEncoderMean=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SpectrumStreamEncoder));
		m_pStreamEncoderVariance=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SpectrumStreamEncoder));
		m_pStreamEncoderRange=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SpectrumStreamEncoder));
		m_pStreamEncoderMedian=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SpectrumStreamEncoder));
		m_pStreamEncoderIQR=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SpectrumStreamEncoder));
		m_pStreamEncoderPercentile=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SpectrumStreamEncoder));
	}
	else
	{
		return false;
	}
	m_pStreamDecoder->initialize();
	m_pStreamEncoderMean->initialize();
	m_pStreamEncoderVariance->initialize();
	m_pStreamEncoderRange->initialize();
	m_pStreamEncoderMedian->initialize();
	m_pStreamEncoderIQR->initialize();
	m_pStreamEncoderPercentile->initialize();

	m_pMatrixStatistic=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_ClassId_AlgoUnivariateStatistic));
	m_pMatrixStatistic->initialize();
	//
	m_pMatrixStatistic->getInputParameter(OVP_Algorithm_UnivariateStatistic_InputParameter)->setReferenceTarget(m_pStreamDecoder->getOutputParameter(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputParameterId_Matrix));
	m_pStreamEncoderMean->getInputParameter(OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputParameterId_Matrix)->setReferenceTarget(m_pMatrixStatistic->getOutputParameter(OVP_Algorithm_UnivariateStatistic_OutputParameter_Mean));
	m_pStreamEncoderVariance->getInputParameter(OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputParameterId_Matrix)->setReferenceTarget(m_pMatrixStatistic->getOutputParameter(OVP_Algorithm_UnivariateStatistic_OutputParameter_Var));
	m_pStreamEncoderRange->getInputParameter(OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputParameterId_Matrix)->setReferenceTarget(m_pMatrixStatistic->getOutputParameter(OVP_Algorithm_UnivariateStatistic_OutputParameter_Range));
	m_pStreamEncoderMedian->getInputParameter(OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputParameterId_Matrix)->setReferenceTarget(m_pMatrixStatistic->getOutputParameter(OVP_Algorithm_UnivariateStatistic_OutputParameter_Med));
	m_pStreamEncoderIQR->getInputParameter(OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputParameterId_Matrix)->setReferenceTarget(m_pMatrixStatistic->getOutputParameter(OVP_Algorithm_UnivariateStatistic_OutputParameter_IQR));
	m_pStreamEncoderPercentile->getInputParameter(OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputParameterId_Matrix)->setReferenceTarget(m_pMatrixStatistic->getOutputParameter(OVP_Algorithm_UnivariateStatistic_OutputParameter_Percent));
	//
	if(m_oInputTypeIdentifier==OV_TypeId_StreamedMatrix)
	{
	}
	else if(m_oInputTypeIdentifier==OV_TypeId_FeatureVector)
	{
	}
	else if(m_oInputTypeIdentifier==OV_TypeId_Signal)
	{
		iop_pSamplingRate.initialize(m_pStreamDecoder->getOutputParameter(OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_SamplingRate));
		m_pStreamEncoderMean->getInputParameter(OVP_GD_Algorithm_SignalStreamEncoder_InputParameterId_SamplingRate)->setReferenceTarget(m_pStreamDecoder->getOutputParameter(OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_SamplingRate));
		m_pStreamEncoderVariance->getInputParameter(OVP_GD_Algorithm_SignalStreamEncoder_InputParameterId_SamplingRate)->setReferenceTarget(m_pStreamDecoder->getOutputParameter(OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_SamplingRate));
		m_pStreamEncoderRange->getInputParameter(OVP_GD_Algorithm_SignalStreamEncoder_InputParameterId_SamplingRate)->setReferenceTarget(m_pStreamDecoder->getOutputParameter(OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_SamplingRate));
		m_pStreamEncoderMedian->getInputParameter(OVP_GD_Algorithm_SignalStreamEncoder_InputParameterId_SamplingRate)->setReferenceTarget(m_pStreamDecoder->getOutputParameter(OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_SamplingRate));
		m_pStreamEncoderIQR->getInputParameter(OVP_GD_Algorithm_SignalStreamEncoder_InputParameterId_SamplingRate)->setReferenceTarget(m_pStreamDecoder->getOutputParameter(OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_SamplingRate));
		m_pStreamEncoderPercentile->getInputParameter(OVP_GD_Algorithm_SignalStreamEncoder_InputParameterId_SamplingRate)->setReferenceTarget(m_pStreamDecoder->getOutputParameter(OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_SamplingRate));
	}
	else if(m_oInputTypeIdentifier==OV_TypeId_Spectrum)
	{
		m_pStreamEncoderMean->getInputParameter(OVP_GD_Algorithm_SpectrumStreamEncoder_InputParameterId_MinMaxFrequencyBands)->setReferenceTarget(m_pStreamDecoder->getOutputParameter(OVP_GD_Algorithm_SpectrumStreamDecoder_OutputParameterId_MinMaxFrequencyBands));
		m_pStreamEncoderVariance->getInputParameter(OVP_GD_Algorithm_SpectrumStreamEncoder_InputParameterId_MinMaxFrequencyBands)->setReferenceTarget(m_pStreamDecoder->getOutputParameter(OVP_GD_Algorithm_SpectrumStreamDecoder_OutputParameterId_MinMaxFrequencyBands));
		m_pStreamEncoderRange->getInputParameter(OVP_GD_Algorithm_SpectrumStreamEncoder_InputParameterId_MinMaxFrequencyBands)->setReferenceTarget(m_pStreamDecoder->getOutputParameter(OVP_GD_Algorithm_SpectrumStreamDecoder_OutputParameterId_MinMaxFrequencyBands));
		m_pStreamEncoderMedian->getInputParameter(OVP_GD_Algorithm_SpectrumStreamEncoder_InputParameterId_MinMaxFrequencyBands)->setReferenceTarget(m_pStreamDecoder->getOutputParameter(OVP_GD_Algorithm_SpectrumStreamDecoder_OutputParameterId_MinMaxFrequencyBands));
		m_pStreamEncoderIQR->getInputParameter(OVP_GD_Algorithm_SpectrumStreamEncoder_InputParameterId_MinMaxFrequencyBands)->setReferenceTarget(m_pStreamDecoder->getOutputParameter(OVP_GD_Algorithm_SpectrumStreamDecoder_OutputParameterId_MinMaxFrequencyBands));
		m_pStreamEncoderPercentile->getInputParameter(OVP_GD_Algorithm_SpectrumStreamEncoder_InputParameterId_MinMaxFrequencyBands)->setReferenceTarget(m_pStreamDecoder->getOutputParameter(OVP_GD_Algorithm_SpectrumStreamDecoder_OutputParameterId_MinMaxFrequencyBands));
		
	}

	op_pfCompression.initialize(m_pMatrixStatistic->getOutputParameter(OVP_Algorithm_UnivariateStatistic_OutputParameter_Compression));
	
	ip_pStatisticMeanActive.initialize(m_pMatrixStatistic->getInputParameter(OVP_Algorithm_UnivariateStatistic_InputParameter_MeanActive));
	ip_pStatisticVarianceActive.initialize(m_pMatrixStatistic->getInputParameter(OVP_Algorithm_UnivariateStatistic_InputParameter_VarActive));
	ip_pStatisticRangeActive.initialize(m_pMatrixStatistic->getInputParameter(OVP_Algorithm_UnivariateStatistic_InputParameter_RangeActive));
	ip_pStatisticMedianActive.initialize(m_pMatrixStatistic->getInputParameter(OVP_Algorithm_UnivariateStatistic_InputParameter_MedActive));
	ip_pStatisticIQRActive.initialize(m_pMatrixStatistic->getInputParameter(OVP_Algorithm_UnivariateStatistic_InputParameter_IQRActive));
	ip_pStatisticPercentileActive.initialize(m_pMatrixStatistic->getInputParameter(OVP_Algorithm_UnivariateStatistic_InputParameter_PercentActive));
	
	CString l_sSettings;
	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(0, l_sSettings);
	ip_pStatisticMeanActive =(l_sSettings == CString("true")? true : false);
	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(1, l_sSettings);
	ip_pStatisticVarianceActive =(l_sSettings == CString("true")? true : false);
	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(2, l_sSettings);
	ip_pStatisticRangeActive =(l_sSettings == CString("true")? true : false);
	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(3, l_sSettings);
	ip_pStatisticMedianActive =(l_sSettings == CString("true")? true : false);
	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(4, l_sSettings);
	ip_pStatisticIQRActive =(l_sSettings == CString("true")? true : false);
	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(5, l_sSettings);
	ip_pStatisticPercentileActive =(l_sSettings == CString("true")? true : false);
	
	getStaticBoxContext().getSettingValue(6, l_sSettings);
	ip_pStatisticParameterValue.initialize(m_pMatrixStatistic->getInputParameter(OVP_Algorithm_UnivariateStatistic_InputParameter_PercentValue));
	ip_pStatisticParameterValue=uint32(::atoi(l_sSettings));	
	return true;
}

boolean CBoxUnivariateStatistic::uninitialize(void)
{
	if(m_oInputTypeIdentifier==OV_TypeId_Signal)
	{
		iop_pSamplingRate.uninitialize();
	}

	ip_pStatisticParameterValue.uninitialize();

	m_pMatrixStatistic->uninitialize();
	m_pStreamEncoderMean->uninitialize();
	m_pStreamEncoderVariance->uninitialize();
	m_pStreamEncoderRange->uninitialize();
	m_pStreamEncoderMedian->uninitialize();
	m_pStreamEncoderIQR->uninitialize();
	m_pStreamEncoderPercentile->uninitialize();
	m_pStreamDecoder->uninitialize();

	getAlgorithmManager().releaseAlgorithm(*m_pMatrixStatistic);
	getAlgorithmManager().releaseAlgorithm(*m_pStreamEncoderMean);
	getAlgorithmManager().releaseAlgorithm(*m_pStreamEncoderVariance);
	getAlgorithmManager().releaseAlgorithm(*m_pStreamEncoderRange);
	getAlgorithmManager().releaseAlgorithm(*m_pStreamEncoderMedian);
	getAlgorithmManager().releaseAlgorithm(*m_pStreamEncoderIQR);
	getAlgorithmManager().releaseAlgorithm(*m_pStreamEncoderPercentile);
	getAlgorithmManager().releaseAlgorithm(*m_pStreamDecoder);

	return true;
}

boolean CBoxUnivariateStatistic::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

boolean CBoxUnivariateStatistic::process(void)
{
	// std::cout<<"process"<<std::endl;
	IBoxIO& l_rDynamicBoxContext=getDynamicBoxContext();
	IBox& l_rStaticBoxContext=getStaticBoxContext();

	
	for(uint32 j=0; j<l_rDynamicBoxContext.getInputChunkCount(0); j++)
	  {
			TParameterHandler < const IMemoryBuffer* > l_oInputMemoryBufferHandle(m_pStreamDecoder->getInputParameter(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_InputParameterId_MemoryBufferToDecode));
			TParameterHandler < IMemoryBuffer* > l_oOutputMemoryBufferHandle_Mean(m_pStreamEncoderMean->getOutputParameter(OVP_GD_Algorithm_StreamedMatrixStreamEncoder_OutputParameterId_EncodedMemoryBuffer));
			TParameterHandler < IMemoryBuffer* > l_oOutputMemoryBufferHandle_Var(m_pStreamEncoderVariance->getOutputParameter(OVP_GD_Algorithm_StreamedMatrixStreamEncoder_OutputParameterId_EncodedMemoryBuffer));
			TParameterHandler < IMemoryBuffer* > l_oOutputMemoryBufferHandle_Range(m_pStreamEncoderRange->getOutputParameter(OVP_GD_Algorithm_StreamedMatrixStreamEncoder_OutputParameterId_EncodedMemoryBuffer));
			TParameterHandler < IMemoryBuffer* > l_oOutputMemoryBufferHandle_Median(m_pStreamEncoderMedian->getOutputParameter(OVP_GD_Algorithm_StreamedMatrixStreamEncoder_OutputParameterId_EncodedMemoryBuffer));
			TParameterHandler < IMemoryBuffer* > l_oOutputMemoryBufferHandle_IQR(m_pStreamEncoderIQR->getOutputParameter(OVP_GD_Algorithm_StreamedMatrixStreamEncoder_OutputParameterId_EncodedMemoryBuffer));
			TParameterHandler < IMemoryBuffer* > l_oOutputMemoryBufferHandle_Percent(m_pStreamEncoderPercentile->getOutputParameter(OVP_GD_Algorithm_StreamedMatrixStreamEncoder_OutputParameterId_EncodedMemoryBuffer));
			l_oInputMemoryBufferHandle=l_rDynamicBoxContext.getInputChunk(0, j);
			l_oOutputMemoryBufferHandle_Mean=l_rDynamicBoxContext.getOutputChunk(0);
			l_oOutputMemoryBufferHandle_Var=l_rDynamicBoxContext.getOutputChunk(1);
			l_oOutputMemoryBufferHandle_Range=l_rDynamicBoxContext.getOutputChunk(2);
			l_oOutputMemoryBufferHandle_Median=l_rDynamicBoxContext.getOutputChunk(3);
			l_oOutputMemoryBufferHandle_IQR=l_rDynamicBoxContext.getOutputChunk(4);
			l_oOutputMemoryBufferHandle_Percent=l_rDynamicBoxContext.getOutputChunk(5);

			m_pStreamDecoder->process();
	
			if(m_pStreamDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputTriggerId_ReceivedHeader))
			{
				m_pMatrixStatistic->process(OVP_Algorithm_UnivariateStatistic_InputTriggerId_Initialize);
				//
				CIdentifier l_id_encodeHeader=OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeHeader;
				if(m_oInputTypeIdentifier==OV_TypeId_FeatureVector)
				  {
				  }
				if(m_oInputTypeIdentifier==OV_TypeId_Signal)	
				  {
					std::cout<<"DownSampling information : "<<iop_pSamplingRate<<"*"<<op_pfCompression<<"=>"<<iop_pSamplingRate*op_pfCompression<<std::endl;
					iop_pSamplingRate=iop_pSamplingRate*op_pfCompression;
					if(iop_pSamplingRate==0)
					  {std::cout<<"nul sampling Rate, it could produce damage in next boxes"<<std::endl;}
					  
					l_id_encodeHeader=OVP_GD_Algorithm_SignalStreamEncoder_InputTriggerId_EncodeHeader;
				  }
				else if(m_oInputTypeIdentifier==OV_TypeId_Spectrum)
				  {
				  }
				
				//
				if(ip_pStatisticMeanActive)
				  {
					m_pStreamEncoderMean->process(l_id_encodeHeader);
					l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, j), l_rDynamicBoxContext.getInputChunkEndTime(0, j));
				  }
				if(ip_pStatisticVarianceActive)
				  {
					m_pStreamEncoderVariance->process(l_id_encodeHeader);
					l_rDynamicBoxContext.markOutputAsReadyToSend(1, l_rDynamicBoxContext.getInputChunkStartTime(0, j), l_rDynamicBoxContext.getInputChunkEndTime(0, j));
				  }
				if(ip_pStatisticRangeActive)
				  {
					m_pStreamEncoderRange->process(l_id_encodeHeader);
					l_rDynamicBoxContext.markOutputAsReadyToSend(2, l_rDynamicBoxContext.getInputChunkStartTime(0, j), l_rDynamicBoxContext.getInputChunkEndTime(0, j));
				  }
				if(ip_pStatisticMedianActive)
				  {
					m_pStreamEncoderMedian->process(l_id_encodeHeader);
					l_rDynamicBoxContext.markOutputAsReadyToSend(3, l_rDynamicBoxContext.getInputChunkStartTime(0, j), l_rDynamicBoxContext.getInputChunkEndTime(0, j));
				  }
				if(ip_pStatisticIQRActive)
				  {
					m_pStreamEncoderIQR->process(l_id_encodeHeader);
					l_rDynamicBoxContext.markOutputAsReadyToSend(4, l_rDynamicBoxContext.getInputChunkStartTime(0, j), l_rDynamicBoxContext.getInputChunkEndTime(0, j));
				  }
				if(ip_pStatisticPercentileActive)
				  {
					m_pStreamEncoderPercentile->process(l_id_encodeHeader);
					l_rDynamicBoxContext.markOutputAsReadyToSend(5, l_rDynamicBoxContext.getInputChunkStartTime(0, j), l_rDynamicBoxContext.getInputChunkEndTime(0, j));
				  }
			}//end header
			if(m_pStreamDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputTriggerId_ReceivedBuffer))
			{
				m_pMatrixStatistic->process(OVP_Algorithm_UnivariateStatistic_InputTriggerId_Process);
				if(m_pMatrixStatistic->isOutputTriggerActive(OVP_Algorithm_UnivariateStatistic_OutputTriggerId_ProcessDone))
  				  {
					CIdentifier l_id_encodeBuffer=OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeBuffer;
					if(m_oInputTypeIdentifier==OV_TypeId_FeatureVector)
					  {
					  }
					if(m_oInputTypeIdentifier==OV_TypeId_Signal)	
					  {
						l_id_encodeBuffer=OVP_GD_Algorithm_SignalStreamEncoder_InputTriggerId_EncodeBuffer;
					  }
					else if(m_oInputTypeIdentifier==OV_TypeId_Spectrum)
					  {
					  }
				  
					if(ip_pStatisticMeanActive)
					  {
						m_pStreamEncoderMean->process(l_id_encodeBuffer);
						l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, j), l_rDynamicBoxContext.getInputChunkEndTime(0, j));
					  }
					if(ip_pStatisticVarianceActive)
					  {
						m_pStreamEncoderVariance->process(l_id_encodeBuffer);
						l_rDynamicBoxContext.markOutputAsReadyToSend(1, l_rDynamicBoxContext.getInputChunkStartTime(0, j), l_rDynamicBoxContext.getInputChunkEndTime(0, j));
					  }
					if(ip_pStatisticRangeActive)
					  {
						m_pStreamEncoderRange->process(l_id_encodeBuffer);
						l_rDynamicBoxContext.markOutputAsReadyToSend(2, l_rDynamicBoxContext.getInputChunkStartTime(0, j), l_rDynamicBoxContext.getInputChunkEndTime(0, j));
					  }
					if(ip_pStatisticMedianActive)
					  {
						m_pStreamEncoderMedian->process(l_id_encodeBuffer);
						l_rDynamicBoxContext.markOutputAsReadyToSend(3, l_rDynamicBoxContext.getInputChunkStartTime(0, j), l_rDynamicBoxContext.getInputChunkEndTime(0, j));
					  }
					if(ip_pStatisticIQRActive)
					  {
						m_pStreamEncoderIQR->process(l_id_encodeBuffer);
						l_rDynamicBoxContext.markOutputAsReadyToSend(4, l_rDynamicBoxContext.getInputChunkStartTime(0, j), l_rDynamicBoxContext.getInputChunkEndTime(0, j));
					  }
					if(ip_pStatisticPercentileActive)
					  {
						m_pStreamEncoderPercentile->process(l_id_encodeBuffer);
						l_rDynamicBoxContext.markOutputAsReadyToSend(5, l_rDynamicBoxContext.getInputChunkStartTime(0, j), l_rDynamicBoxContext.getInputChunkEndTime(0, j));
					  }
				  }
				else
				{std::cout<<"process not activated"<<std::endl;}
			}
			if(m_pStreamDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputTriggerId_ReceivedEnd))
			{
				CIdentifier l_id_encodeEnder=OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeEnd;
				if(m_oInputTypeIdentifier==OV_TypeId_FeatureVector)
				  {
				  }
				if(m_oInputTypeIdentifier==OV_TypeId_Signal)	
				  {
					l_id_encodeEnder=OVP_GD_Algorithm_SignalStreamEncoder_InputTriggerId_EncodeEnd;
				  }
				else if(m_oInputTypeIdentifier==OV_TypeId_Spectrum)
				  {
				  }
					  
				if(ip_pStatisticMeanActive)
				  {
					m_pStreamEncoderMean->process(l_id_encodeEnder);
					l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, j), l_rDynamicBoxContext.getInputChunkEndTime(0, j));
				  }
				if(ip_pStatisticVarianceActive)
				  {
					m_pStreamEncoderVariance->process(l_id_encodeEnder);
					l_rDynamicBoxContext.markOutputAsReadyToSend(1, l_rDynamicBoxContext.getInputChunkStartTime(0, j), l_rDynamicBoxContext.getInputChunkEndTime(0, j));
				  }
				if(ip_pStatisticRangeActive)
				  {
					m_pStreamEncoderRange->process(l_id_encodeEnder);
					l_rDynamicBoxContext.markOutputAsReadyToSend(2, l_rDynamicBoxContext.getInputChunkStartTime(0, j), l_rDynamicBoxContext.getInputChunkEndTime(0, j));
				  }
				if(ip_pStatisticMedianActive)
				  {
					m_pStreamEncoderMedian->process(l_id_encodeEnder);
					l_rDynamicBoxContext.markOutputAsReadyToSend(3, l_rDynamicBoxContext.getInputChunkStartTime(0, j), l_rDynamicBoxContext.getInputChunkEndTime(0, j));
				  }
				if(ip_pStatisticIQRActive)
				  {
					m_pStreamEncoderIQR->process(l_id_encodeEnder);
					l_rDynamicBoxContext.markOutputAsReadyToSend(4, l_rDynamicBoxContext.getInputChunkStartTime(0, j), l_rDynamicBoxContext.getInputChunkEndTime(0, j));
				  }
				if(ip_pStatisticPercentileActive)
				  {
					m_pStreamEncoderPercentile->process(l_id_encodeEnder);
					l_rDynamicBoxContext.markOutputAsReadyToSend(5, l_rDynamicBoxContext.getInputChunkStartTime(0, j), l_rDynamicBoxContext.getInputChunkEndTime(0, j));
				  }
			}

		l_rDynamicBoxContext.markInputAsDeprecated(0, j);
		// std::cout<<"output sent"<<std::endl;
	  }
	// std::cout<<"end process"<<std::endl;
	return true;
}
