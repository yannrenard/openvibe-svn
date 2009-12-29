#include "ovpCChannelCentering.h"
#include <system/Memory.h>

using namespace OpenViBE;
using namespace OpenViBE::Plugins;
using namespace OpenViBE::Kernel;
using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;
using namespace OpenViBEToolkit;

namespace OpenViBEPlugins
{
	namespace SignalProcessing
	{
		void CChannelCentering::setChannelCount(const OpenViBE::uint32 ui32ChannelCount)
		{
			m_oChannelNames.resize(ui32ChannelCount);

			if(m_oChannelTranslation.size() != ui32ChannelCount)
			{
				m_bError = true;
				getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning <<"Wrong number of base values!\n";
			}
		}

		void CChannelCentering::setChannelName(const OpenViBE::uint32 ui32ChannelIndex, const char* sChannelName)
		{
			m_oChannelNames[ui32ChannelIndex] = sChannelName;
		}

		void CChannelCentering::setSampleCountPerBuffer(const OpenViBE::uint32 ui32SampleCountPerBuffer)
		{
			m_ui32SamplesPerBuffer = ui32SampleCountPerBuffer;

			//allocate matrix
			m_ui64MatrixBufferSize = m_ui32SamplesPerBuffer * (uint32)m_oChannelNames.size();
			m_pMatrixBuffer = new float64[m_ui64MatrixBufferSize];

			//send header
			m_pSignalOutputWriterHelper->setSamplingRate(m_ui32SamplingFrequency);
			m_pSignalOutputWriterHelper->setChannelCount((uint32)m_oChannelNames.size());

			for(size_t i=0 ; i<m_oChannelNames.size(); i++)
			{
				m_pSignalOutputWriterHelper->setChannelName((uint32)i, m_oChannelNames[i].c_str());
			}

			m_pSignalOutputWriterHelper->setSampleCountPerBuffer((uint32) m_ui32SamplesPerBuffer);
			m_pSignalOutputWriterHelper->setSampleBuffer(m_pMatrixBuffer);

			m_pSignalOutputWriterHelper->writeHeader(*m_pWriter);

			getBoxAlgorithmContext()->getDynamicBoxContext()->markOutputAsReadyToSend(0, m_ui64LastChunkStartTime, m_ui64LastChunkEndTime);

		}

		void CChannelCentering::setSamplingRate(const OpenViBE::uint32 ui32SamplingFrequency)
		{
			m_ui32SamplingFrequency = ui32SamplingFrequency;
		}

		void CChannelCentering::setSampleBuffer(const OpenViBE::float64* pBuffer)
		{

			for(size_t i=0 ; i<m_oChannelNames.size() ; i++)
			{
				//substract
				for(uint32 j=0 ; j<m_ui32SamplesPerBuffer ; j++)
				{
					uint64 l_ui64Index = i*m_ui32SamplesPerBuffer + j;
					m_pMatrixBuffer[l_ui64Index] = pBuffer[l_ui64Index] - m_oChannelTranslation[i];
				}
			}

			//the matrix is ready to be sent
			m_pSignalOutputWriterHelper->writeBuffer(*m_pWriter);
			getBoxAlgorithmContext()->getDynamicBoxContext()->markOutputAsReadyToSend(0, m_ui64LastChunkStartTime, m_ui64LastChunkEndTime);

		}

		void CChannelCentering::writeSignalOutput(const void* pBuffer, const EBML::uint64 ui64BufferSize)
		{
			appendOutputChunkData<0>(pBuffer, ui64BufferSize);
		}

		CChannelCentering::CChannelCentering() :
			m_pReader(NULL),
			m_pSignalReaderCallBack(NULL),
			m_ui64LastChunkStartTime(0),
			m_ui64LastChunkEndTime(0),
			m_pWriter(NULL),
			m_oSignalOutputWriterCallbackProxy(
				*this,
				&CChannelCentering::writeSignalOutput),
			m_pSignalOutputWriterHelper(NULL),
			m_ui32SamplingFrequency(0),
			m_ui32SamplesPerBuffer(0),
			m_bError(false),
			m_pMatrixBuffer(NULL),
			m_ui64MatrixBufferSize(0)
		{
		}

		OpenViBE::boolean CChannelCentering::initialize()
		{
			CString l_sSettings;
			getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(0, l_sSettings);

			//read values
			unsigned int l_uiNumberOfValues = 0;
			FILE * l_pIn = fopen((const char*) l_sSettings, "r");
			fscanf(l_pIn, "%d\n", (int*)&l_uiNumberOfValues);

			m_oChannelTranslation.resize(l_uiNumberOfValues);

			for(uint32 i=0 ; i<l_uiNumberOfValues ; i++)
			{
				fscanf(l_pIn, "%lf\n", (double*)&m_oChannelTranslation[i]);
			}
			fclose(l_pIn);

			// Prepares EBML reader
			m_pSignalReaderCallBack = createBoxAlgorithmSignalInputReaderCallback(*this);
			m_pReader=EBML::createReader(*m_pSignalReaderCallBack);

			// Prepares EBML writer
			m_pSignalOutputWriterHelper=createBoxAlgorithmSignalOutputWriter();
			m_pWriter=EBML::createWriter(m_oSignalOutputWriterCallbackProxy);

			return true;
		}

		OpenViBE::boolean CChannelCentering::uninitialize()
		{
			if(m_pMatrixBuffer)
			{
				delete[] m_pMatrixBuffer;
			}

			// Cleans up EBML writer
			m_pWriter->release();
			m_pWriter=NULL;

			releaseBoxAlgorithmSignalOutputWriter(m_pSignalOutputWriterHelper);
			m_pSignalOutputWriterHelper=NULL;

			// Cleans up EBML reader
			releaseBoxAlgorithmSignalInputReaderCallback(m_pSignalReaderCallBack);

			m_pReader->release();
			m_pReader=NULL;

			return true;
		}

		OpenViBE::boolean CChannelCentering::processInput(OpenViBE::uint32 ui32InputIndex)
		{
			//don't process anything if an error has occured
			if(m_bError)
			{
				return false;
			}

			getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
			return true;
		}

		OpenViBE::boolean CChannelCentering::process()
		{
			IBoxIO* l_pBoxIO = getBoxAlgorithmContext()->getDynamicBoxContext();

			// Process input data
			for(uint32 i=0; i<l_pBoxIO->getInputChunkCount(0); i++)
			{

				uint64 l_ui64ChunkSize;
				const uint8* l_pBuffer;
				l_pBoxIO->getInputChunk(0, i, m_ui64LastChunkStartTime, m_ui64LastChunkEndTime, l_ui64ChunkSize, l_pBuffer);
				l_pBoxIO->markInputAsDeprecated(0, i);
				m_pReader->processData(l_pBuffer, l_ui64ChunkSize);
			}
			return true;
		}

	};
};

