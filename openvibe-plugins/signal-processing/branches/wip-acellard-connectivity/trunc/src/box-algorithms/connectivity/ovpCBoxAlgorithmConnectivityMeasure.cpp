#if defined(TARGET_HAS_ThirdPartyEIGEN)

#include "ovpCBoxAlgorithmConnectivityMeasure.h"
#include <system/Memory.h>
#include <cstdio>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;

using namespace OpenViBEToolkit;

namespace
{
	inline uint32 _find_channel_(const IMatrix& rMatrix, const CString& rChannel, const CIdentifier& rMatchMethodIdentifier, uint32 uiStart=0)
	{
		uint32 i, l_ui32Result=uint32(-1);

		if(rMatchMethodIdentifier==OVP_TypeId_MatchMethod_Name)
		{
			for(i=uiStart; i<rMatrix.getDimensionSize(0); i++)
			{
				if(OpenViBEToolkit::Tools::String::isAlmostEqual(rMatrix.getDimensionLabel(0, i), rChannel, false))
				{
					l_ui32Result=i;
				}
			}
		}
		else if(rMatchMethodIdentifier==OVP_TypeId_MatchMethod_Index)
		{
			unsigned int value;
			if(::sscanf(rChannel.toASCIIString(), "%u", &value)==1)
			{
				value--; // => makes it 0-indexed !
				if(uiStart <= uint32(value) && uint32(value) < rMatrix.getDimensionSize(0))
				{
					l_ui32Result=uint32(value);
				}
			}
		}
		else if(rMatchMethodIdentifier==OVP_TypeId_MatchMethod_Smart)
		{
			if(l_ui32Result==uint32(-1)) l_ui32Result=_find_channel_(rMatrix, rChannel, OVP_TypeId_MatchMethod_Name, uiStart);
			if(l_ui32Result==uint32(-1)) l_ui32Result=_find_channel_(rMatrix, rChannel, OVP_TypeId_MatchMethod_Index, uiStart);
		}

		return l_ui32Result;
	}
};



boolean CBoxAlgorithmConnectivityMeasure::initialize(void)
{
	IBox& l_rStaticBoxContext=this->getStaticBoxContext();

	// Retrieve algorithm chosen by the user
	CIdentifier l_oConnectivityAlgorithmClassIdentifier;
//	CString l_sConnectivityAlgorithmClassIdentifier;
//	l_rStaticBoxContext.getSettingValue(0, l_sConnectivityAlgorithmClassIdentifier);
	CString l_sConnectivityAlgorithmClassIdentifier = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
	l_oConnectivityAlgorithmClassIdentifier=this->getTypeManager().getEnumerationEntryValueFromName(OVP_TypeId_ConnectivityAlgorithm, l_sConnectivityAlgorithmClassIdentifier);
	// Display an error message if the algorithm is not recognized
	if(l_oConnectivityAlgorithmClassIdentifier==OV_UndefinedIdentifier)
	{
		this->getLogManager() << LogLevel_ImportantWarning << "Unknown Connectivity algorithm [" << l_sConnectivityAlgorithmClassIdentifier << "]\n";
		return false;
	}

	m_pConnectivityAlgo=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(l_oConnectivityAlgorithmClassIdentifier));
	m_pConnectivityAlgo->initialize();

	// Signal stream decoder
	m_oAlgo0_SignalDecoder.initialize(*this);
	// Signal stream encoder
	m_oAlgo1_SignalEncoder.initialize(*this);
	
	CString l_sChannelList=FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1);
	uint64 l_ui64MatchMethodIdentifier=FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 2);
	m_vChannelTable.clear();

	//______________________________________________________________________________________________________________________________________
	//
	// Splits the channel list in order to identify channel pairs to process
	//_______________________________________________________________________________________________________________________________________
	//


	std::vector < CString > l_sPairs;
	uint32 l_ui32PairsCount = Tools::String::split(l_sChannelList, Tools::String::TSplitCallback < std::vector < CString > >(l_sPairs), OV_Value_EnumeratedStringSeparator);
	for(uint32 j=0; j<l_ui32PairsCount; j++)
	{
		std::vector < CString > l_sChannel;

		uint32 l_ui32ChannelCount = Tools::String::split(l_sPairs, Tools::String::TSplitCallback < std::vector < CString > >(l_sChannel), OVP_Value_CoupledStringSeparator);
		for(uint32 i=0; i<l_ui32ChannelCount; i++)
		{
			std::vector < CString > l_sSubChannel;
			// Checks if the channel designation is a range
			if(OpenViBEToolkit::Tools::String::split(l_sChannel[i], OpenViBEToolkit::Tools::String::TSplitCallback < std::vector < CString > >(l_sSubChannel), OV_Value_RangeStringSeparator)==2)
			{
				// Finds the first & second part of the range (only index based)
				uint32 l_ui32RangeStartIndex=::_find_channel_(*m_pInputMatrix, l_sSubChannel[0], OVP_TypeId_MatchMethod_Index);
				uint32 l_ui32RangeEndIndex=::_find_channel_(*m_pInputMatrix, l_sSubChannel[1], OVP_TypeId_MatchMethod_Index);

				// When first or second part is not found but associated token is empty, don't consider this as an error
				if(l_ui32RangeStartIndex==uint32(-1) && l_sSubChannel[0]==CString("")) l_ui32RangeStartIndex=0;
				if(l_ui32RangeEndIndex  ==uint32(-1) && l_sSubChannel[1]==CString("")) l_ui32RangeEndIndex=m_pInputMatrix->getDimensionSize(0)-1;

				// After these corections, if either first or second token were not found, or if start index is greater than start index, consider this an error and invalid range
				if(l_ui32RangeStartIndex==uint32(-1) || l_ui32RangeEndIndex  ==uint32(-1) || l_ui32RangeStartIndex>l_ui32RangeEndIndex)
				{
					this->getLogManager() << LogLevel_Warning << "Invalid channel range [" << l_sChannel[i] << "] - splitted as [" << l_sSubChannel[0] << "][" << l_sSubChannel[1] << "]\n";
				}
				else
				{
					// The range is valid so selects all the channels in this range
					this->getLogManager() << LogLevel_Trace << "For range [" << l_sChannel[i] << "] :\n";
					for(uint32 k=l_ui32RangeStartIndex; k<=l_ui32RangeEndIndex; k++)
					{
						m_vChannelTable.push_back(k);
						this->getLogManager() << LogLevel_Trace << "  Selected channel [" << k+1 << "]\n";
					}
				}
			}
			// This is not a range, identify the channel
			else
			{
				uint32 l_bFound=false;
				uint32 l_ui32Index=uint32(-1);

				// Looks for all the channels with this name
				while((l_ui32Index=::_find_channel_(*ip_pMatrix, l_sChannel[i], l_ui64MatchMethodIdentifier, l_ui32Index+1))!=uint32(-1))
				{
					l_bFound=true;
					m_vChannelTable.push_back(l_ui32Index);
					this->getLogManager() << LogLevel_Trace << "Selected channel [" << l_ui32Index+1 << "]\n";
				}

				// When no channel was found, consider it a missing channel
				if(!l_bFound)
				{
					this->getLogManager() << LogLevel_Warning << "Invalid channel [" << l_sPairs[j] << "]\n";
					m_vChannelTable.push_back(uint32(-1));
				}
			}

		}
	}


	// insert algo target


	return true;
}
/*******************************************************************************/

boolean CBoxAlgorithmConnectivityMeasure::uninitialize(void)
{
	m_pConnectivityAlgo->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pConnectivityAlgo);
	m_oAlgo0_SignalDecoder.uninitialize();
	m_oAlgo1_SignalEncoder.uninitialize();

	return true;
}


boolean CBoxAlgorithmConnectivityMeasure::processInput(uint32 ui32InputIndex)
{
	// some pre-processing code if needed...

	// ready to process !
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}
/*******************************************************************************/

boolean CBoxAlgorithmConnectivityMeasure::process(void)
{
	
	// the static box context describes the box inputs, outputs, settings structures
	IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	// the dynamic box context describes the current state of the box inputs and outputs (i.e. the chunks)
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

	// we decode the input signal chunks
	for(uint32 i=0; i<l_rDynamicBoxContext.getInputChunkCount(0); i++)
	{

		m_oAlgo0_SignalDecoder.decode(0,i);

		// If header is received
		if(m_oAlgo0_SignalDecoder.isHeaderReceived())
		{
			// Start the initialization process
			m_pConnectivityAlgo->process(OVTK_Algorithm_Connectivity_InputTriggerId_Initialize)
			// Make sure the algo initialization was successful
			if(!m_pARBurgMethodAlgorithm->process(OVP_Algorithm_ARBurgMethod_InputTriggerId_Initialize))
		    {
				cout << "initialization was unsuccessful" <<endl;
				return false;
		    }

			// Pass the header to the next boxes, by encoding a header on the output 0:
			m_oAlgo1_SignalEncoder.encodeHeader(0);
			// send the output chunk containing the header. The dates are the same as the input chunk:
			l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));
		}
		if(m_oAlgo0_SignalDecoder.isBufferReceived())
		{
			// Buffer received. For example the signal values
			// Access to the buffer can be done thanks to :
			IMatrix* l_pMatrix = m_oSignalDecoder.getOutputMatrix(); // the StreamedMatrix of samples.
			uint64 l_uiSamplingFrequency = m_oSignalDecoder.getOutputSamplingRate(); // the sampling rate of the signal
			
			m_pConnectivityAlgo->process(OVTK_Algorithm_Connectivity_InputTriggerId_Process)

			if(m_pConnectivityAlgo->isOutputTriggerActive(OVTK_Algorithm_Connectivity_InputTriggerId_ProcessDone))
			{
				// Encode the output buffer :
				m_oAlgo1_SignalEncoder.encodeBuffer(0);
				// and send it to the next boxes :
				l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));
			}

		}
		if(m_oSignalDecoder.isEndReceived())
		{
			// End of stream received. This happens only once when pressing "stop". Just pass it to the next boxes so they receive the message :
			m_oAlgo1_SignalEncoder.encodeEnd(0);
			l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));
		}

		// The current input chunk has been processed, and automaticcaly discarded
	}

	return true;
}

#endif //#TARGET_HAS_ThirdPartyEIGEN
