#include "ovpCBipolarChannels.h"
#include <system/Memory.h>

using namespace OpenViBE;
using namespace OpenViBE::Plugins;
using namespace OpenViBE::Kernel;
using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;
using namespace OpenViBEToolkit;
#include <iostream>
using namespace std;

namespace OpenViBEPlugins
{
	namespace SignalProcessing
	{
		void CBipolarChannels::setChannelCount(const OpenViBE::uint32 ui32ChannelCount)
		{
			m_oChannelNames.resize(ui32ChannelCount);
		}

		void CBipolarChannels::setChannelName(const OpenViBE::uint32 ui32ChannelIndex, const char* sChannelName)
		{
			m_oChannelNames[ui32ChannelIndex] = sChannelName;
		}

		void CBipolarChannels::setSampleCountPerBuffer(const OpenViBE::uint32 ui32SampleCountPerBuffer)
		{
			m_ui32SamplesPerBuffer = ui32SampleCountPerBuffer;

			//allocate matrix
			m_ui64MatrixBufferSize = m_ui32SamplesPerBuffer * ((uint32)m_oChannelNames.size() + (uint32)m_oChannelPairs.size());
			m_pMatrixBuffer = new float64[m_ui64MatrixBufferSize];

			//send header
			m_pSignalOutputWriterHelper->setSamplingRate(m_ui32SamplingFrequency);
			m_pSignalOutputWriterHelper->setChannelCount((uint32)m_oChannelNames.size() + (uint32)m_oChannelPairs.size());

			for(size_t i=0 ; i<m_oChannelNames.size(); i++)
			{
				m_pSignalOutputWriterHelper->setChannelName((uint32)i, m_oChannelNames[i].c_str());
			}

			m_pSignalOutputWriterHelper->setSampleCountPerBuffer((uint32) m_ui32SamplesPerBuffer);
			m_pSignalOutputWriterHelper->setSampleBuffer(m_pMatrixBuffer);

			m_pSignalOutputWriterHelper->writeHeader(*m_pWriter);

			getBoxAlgorithmContext()->getDynamicBoxContext()->markOutputAsReadyToSend(0, m_ui64LastChunkStartTime, m_ui64LastChunkEndTime);

		}

		void CBipolarChannels::setSamplingRate(const OpenViBE::uint32 ui32SamplingFrequency)
		{
			m_ui32SamplingFrequency = ui32SamplingFrequency;
		}

		void CBipolarChannels::setSampleBuffer(const OpenViBE::float64* pBuffer)
		{
			//copy original data
			System::Memory::copy(m_pMatrixBuffer, pBuffer, m_ui32SamplesPerBuffer * sizeof(float64) * ((uint32)m_oChannelNames.size() + (uint32)m_oChannelPairs.size()));

			//append new resulting channels
			float64 * l_pNewChannels = m_pMatrixBuffer + (m_ui32SamplesPerBuffer * (uint32)m_oChannelNames.size() );

			for(size_t i=0 ; i<m_oChannelPairs.size() ; i++)
			{
				for(uint32 j=0 ; j<m_ui32SamplesPerBuffer ; j++)
				{
					l_pNewChannels[(i*m_ui32SamplesPerBuffer) + j] = 
						pBuffer[(m_oChannelPairs[i].first * m_ui32SamplesPerBuffer) + j] -
						pBuffer[(m_oChannelPairs[i].second * m_ui32SamplesPerBuffer) + j];
				}
			}

			//the matrix is ready to be sent
			m_pSignalOutputWriterHelper->writeBuffer(*m_pWriter);
			getBoxAlgorithmContext()->getDynamicBoxContext()->markOutputAsReadyToSend(0, m_ui64LastChunkStartTime, m_ui64LastChunkEndTime);

		}

		void CBipolarChannels::writeSignalOutput(const void* pBuffer, const EBML::uint64 ui64BufferSize)
		{
			appendOutputChunkData<0>(pBuffer, ui64BufferSize);
		}

		CBipolarChannels::CBipolarChannels() :
			m_pReader(NULL),
			m_pSignalReaderCallBack(NULL),
			m_ui64LastChunkStartTime(0),
			m_ui64LastChunkEndTime(0),
			m_pWriter(NULL),
			m_oSignalOutputWriterCallbackProxy(
					*this,
					&CBipolarChannels::writeSignalOutput),
			m_pSignalOutputWriterHelper(NULL),
			m_ui32SamplingFrequency(0),
			m_ui32SamplesPerBuffer(0),
			m_bError(false),
			m_pMatrixBuffer(NULL),
			m_ui64MatrixBufferSize(0)
		{
		}

		OpenViBE::boolean CBipolarChannels::initialize()
		{
			CString l_sSettings;
			getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(0, l_sSettings);

			//read values
			int l_iNumberOfValues = 0;
			uint32 l_ui32FirstChannel = 0;
			uint32 l_ui32SecondChannel = 0;
			FILE * l_pIn = fopen((const char*) l_sSettings, "r");
			
			fscanf(l_pIn, "%d\n", &l_iNumberOfValues);
			
			for(int32 i=0 ; i<l_iNumberOfValues ; i++)
			{
				fscanf(l_pIn, "%d %d\n", (int*)&l_ui32FirstChannel, (int*)&l_ui32SecondChannel);
				m_oChannelPairs.push_back(pair<uint32, uint32>(l_ui32FirstChannel, l_ui32SecondChannel));
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

		OpenViBE::boolean CBipolarChannels::uninitialize()
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

		OpenViBE::boolean CBipolarChannels::processInput(OpenViBE::uint32 ui32InputIndex)
		{
			//don't process anything if an error has occured
			if(m_bError)
			{
				return false;
			}

			getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
			return true;
		}

		OpenViBE::boolean CBipolarChannels::process()
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

