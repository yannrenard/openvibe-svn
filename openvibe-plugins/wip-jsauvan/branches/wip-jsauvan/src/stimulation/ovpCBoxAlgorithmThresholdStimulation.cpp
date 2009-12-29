#include "ovpCBoxAlgorithmThresholdStimulation.h"
#include <iostream>
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::Stimulation;
using namespace std;

boolean CBoxAlgorithmThresholdStimulation::initialize(void)
{

	const IBox * l_pBox=getBoxAlgorithmContext()->getStaticBoxContext();

	m_pStreamDecoder = &getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StreamedMatrixStreamDecoder));
	m_pStreamEncoder = &getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamEncoder) );

        m_pStreamEncoder->initialize();
        m_oOutputMemoryBufferHandle.initialize( m_pStreamEncoder->getOutputParameter(OVP_GD_Algorithm_StimulationStreamEncoder_OutputParameterId_EncodedMemoryBuffer) );

	m_pStreamDecoder->initialize();
	m_oInputMemoryBufferHandle.initialize(m_pStreamDecoder->getInputParameter(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_InputParameterId_MemoryBufferToDecode));

	CString l_sOnStimulationIdentifier;
	CString l_sOffStimulationIdentifier;
	CString l_sThreshold;

	l_pBox->getSettingValue(0, l_sThreshold);
	l_pBox->getSettingValue(1, l_sOnStimulationIdentifier);
	l_pBox->getSettingValue(2, l_sOffStimulationIdentifier);

	m_f64Threshold = static_cast<float64> (atof(l_sThreshold));

	m_ui64OnStimulationIdentifier = getBoxAlgorithmContext()->getPlayerContext()->getTypeManager().getEnumerationEntryValueFromName(OVTK_TypeId_Stimulation, l_sOnStimulationIdentifier);
	m_ui64OffStimulationIdentifier = getBoxAlgorithmContext()->getPlayerContext()->getTypeManager().getEnumerationEntryValueFromName(OVTK_TypeId_Stimulation, l_sOffStimulationIdentifier);

	IStimulationSet* l_pStimulationSet=&m_oStimulationSet;

        m_pStreamEncoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamEncoder_InputParameterId_StimulationSet)->setValue(&l_pStimulationSet);

	m_ui64StartTime=0;
        m_ui64EndTime=0;
        m_bHasSentHeader = false;
	m_bIsValid = false;

	return true;
}

boolean CBoxAlgorithmThresholdStimulation::uninitialize(void)
{

	m_oInputMemoryBufferHandle.uninitialize();
	m_pStreamDecoder->uninitialize();
	getAlgorithmManager().releaseAlgorithm(*m_pStreamDecoder);
	m_pStreamDecoder=NULL;

	m_oOutputMemoryBufferHandle.uninitialize();
        m_pStreamEncoder->uninitialize();
        getAlgorithmManager().releaseAlgorithm(*m_pStreamEncoder);
        m_pStreamEncoder=NULL;

	return true;
}


boolean CBoxAlgorithmThresholdStimulation::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

boolean CBoxAlgorithmThresholdStimulation::process(void)
{
	getLogManager() << LogLevel_Debug<< "process called\n";
	IBoxIO& l_rDynamicBoxContext=getDynamicBoxContext();
	IPlayerContext& l_rPlayerContext = getPlayerContext();

	float64 l_f64Buffer;

	if(!m_bHasSentHeader)
	{
		m_oOutputMemoryBufferHandle=l_rDynamicBoxContext.getOutputChunk(0);
		m_pStreamEncoder->process(OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeHeader);
		l_rDynamicBoxContext.markOutputAsReadyToSend(0, m_ui64StartTime, m_ui64EndTime);
		getLogManager() << LogLevel_Debug << "Header sent\n";
		m_bHasSentHeader = true;
	}

	for(uint32 j=0; j<l_rDynamicBoxContext.getInputChunkCount(0); j++)
	{
		m_oInputMemoryBufferHandle=l_rDynamicBoxContext.getInputChunk(0, j);
		m_pStreamDecoder->process();

		//When we receive a header...
		if(m_pStreamDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputTriggerId_ReceivedHeader))
		{
			getLogManager() << LogLevel_Debug<< "Matrix Header received\n";
			TParameterHandler < IMatrix* > l_oHandle(m_pStreamDecoder->getOutputParameter(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputParameterId_Matrix));
			if(l_oHandle.exists())
			{
				//for now, the dimension of the input matrix has to be (1,1), otherwise buffers are not taken into account
				m_ui32NbDimensions = l_oHandle->getDimensionCount();
				for (uint32 k=0; k<m_ui32NbDimensions; k++)
				{
					m_lDimensionSize[k] = l_oHandle->getDimensionSize(k);
				}
				if(m_ui32NbDimensions == 2 && m_lDimensionSize[0] == 1 && m_lDimensionSize[1] == 1)
				{
					m_bIsValid = true;
					getLogManager() << LogLevel_Debug<<"Matrix type is valid\n";
				}
				else
				{
					m_bIsValid = false;
					getLogManager() << LogLevel_Debug<< "Matrix Type is not valid : \n";
					getLogManager() << LogLevel_Debug<< "      Dimensions : "<< m_ui32NbDimensions <<"\n";
					getLogManager() << LogLevel_Debug<< "      Size : ("<<m_lDimensionSize[0]<<","<<m_lDimensionSize[1]<<")\n";
				}
			}
		}


		//When we receive a buffer which is considered valid (see above)....
		if(m_pStreamDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputTriggerId_ReceivedBuffer) && m_bIsValid)
		{
			getLogManager() << LogLevel_Debug<< "A Buffer has been received and is valid\n";
			TParameterHandler < IMatrix* > l_oHandle(m_pStreamDecoder->getOutputParameter(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputParameterId_Matrix));
			if(l_oHandle.exists())
			{
				IStimulationSet* l_pStimulationSet = &m_oStimulationSet;
				l_f64Buffer = *l_oHandle->getBuffer();
				if(l_f64Buffer >= m_f64Threshold)
				{
					//if the float inside the input matrix is above our threshold, we send a ON stimulation to the output
					getLogManager() << LogLevel_Debug<< "Input is above threshold\n";
					l_pStimulationSet->setStimulationCount(1);
					l_pStimulationSet->setStimulationIdentifier(0, m_ui64OnStimulationIdentifier);
					l_pStimulationSet->setStimulationDate(0, m_ui64EndTime);

				}
				else
				{
					//if the float inside the input matrix is under our threshold, we send a OFF stimulation to the output
					getLogManager() << LogLevel_Debug<< "Input is under threshold\n";
					l_pStimulationSet->setStimulationCount(1);
					l_pStimulationSet->setStimulationIdentifier(0, m_ui64OffStimulationIdentifier);
					l_pStimulationSet->setStimulationDate(0, m_ui64EndTime);
				}

				m_pStreamEncoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamEncoder_InputParameterId_StimulationSet)->setValue(&l_pStimulationSet);
				m_oOutputMemoryBufferHandle=l_rDynamicBoxContext.getOutputChunk(0);
				m_pStreamEncoder->process(OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeBuffer);
				l_rDynamicBoxContext.markOutputAsReadyToSend(0, m_ui64StartTime, m_ui64EndTime);
			}
		}

		//When we receive a end of stream...
		if(m_pStreamDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputTriggerId_ReceivedEnd))
		{
			getLogManager() << LogLevel_Debug<< "End of stream received\n";
			m_bIsValid = false;
		}

		l_rDynamicBoxContext.markInputAsDeprecated(0, j);
	}

	m_ui64StartTime=m_ui64EndTime;
	m_ui64EndTime=l_rPlayerContext.getCurrentTime();

	return true;
}
