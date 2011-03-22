#ifndef __OpenViBEToolkit_TEncoder_H__
#define __OpenViBEToolkit_TEncoder_H__

#ifdef TARGET_HAS_ThirdPartyOpenViBEPluginsGlobalDefines

#include "../../ovtk_base.h"

#include "../ovtkTCodec.h"

namespace OpenViBEToolkit
{
	template <class T>
	class TEncoderLocal : public T
	{
	protected:

		OpenViBE::Kernel::TParameterHandler < OpenViBE::IMemoryBuffer* > m_pOutputMemoryBuffer;

		using T::initialize;
		using T::m_pCodec;
		using T::m_pBoxAlgorithm;

		virtual void setOutputChunk(OpenViBE::IMemoryBuffer * pOutputChunkMemoryBuffer)
		{
			m_pOutputMemoryBuffer = pOutputChunkMemoryBuffer;
		}

		virtual OpenViBE::Kernel::TParameterHandler < OpenViBE::IMemoryBuffer* >& getOutputMemoryBuffer()
		{
			return m_pOutputMemoryBuffer;
		}

		virtual OpenViBE::boolean isOutputTriggerActive(OpenViBE::CIdentifier oTrigger)
		{
			return m_pCodec->isOutputTriggerActive(oTrigger);
		}

		virtual OpenViBE::boolean process(OpenViBE::CIdentifier &oTrigger)
		{
			return m_pCodec->process(oTrigger);
		}
		virtual OpenViBE::boolean process(void)
		{
			return m_pCodec->process();
		}

		// The functions that need to be specified by the encoders (specific Trigger ID)
		virtual OpenViBE::boolean encodeHeader() = 0;
		virtual OpenViBE::boolean encodeBuffer() = 0;
		virtual OpenViBE::boolean encodeEnd() = 0;

	public:

		/* 
		These public methods handle every aspect of the encoding process : 
		- prepare the output memory buffer with a chunk
		- encode (specific for each encoder)
		- mark output as ready to be sent
		*/

		OpenViBE::boolean encodeHeader(OpenViBE::uint32 ui32OutputIndex, OpenViBE::uint64 ui64StartTime, OpenViBE::uint64 ui64EndTime)
		{
			this->setOutputChunk(m_pBoxAlgorithm->getDynamicBoxContext().getOutputChunk(ui32OutputIndex));
			if(!this->encodeHeader()) return false;
			return m_pBoxAlgorithm->getDynamicBoxContext().markOutputAsReadyToSend(ui32OutputIndex, ui64StartTime, ui64EndTime);
		}

		OpenViBE::boolean encodeBuffer(OpenViBE::uint32 ui32OutputIndex, OpenViBE::uint64 ui64StartTime, OpenViBE::uint64 ui64EndTime)
		{
			this->setOutputChunk(m_pBoxAlgorithm->getDynamicBoxContext().getOutputChunk(ui32OutputIndex));
			if(!this->encodeBuffer()) return false;
			return m_pBoxAlgorithm->getDynamicBoxContext().markOutputAsReadyToSend(ui32OutputIndex, ui64StartTime, ui64EndTime);
		}

		OpenViBE::boolean encodeEnd(OpenViBE::uint32 ui32OutputIndex, OpenViBE::uint64 ui64StartTime, OpenViBE::uint64 ui64EndTime)
		{
			this->setOutputChunk(m_pBoxAlgorithm->getDynamicBoxContext().getOutputChunk(ui32OutputIndex));
			if(!this->encodeEnd()) return false;
			return m_pBoxAlgorithm->getDynamicBoxContext().markOutputAsReadyToSend(ui32OutputIndex, ui64StartTime, ui64EndTime);
		}
	};

	/*
	This class provides an access to the superclass TEncoder.
	Use case : iterating over a vector of TEncoder, calling encodeHeader() each time. 
	You don't need to know which type of encoder is in the vector.
	*/
	template <class T>
	class TEncoder : public TEncoderLocal < TCodec < T > >
	{
	protected:
		// constructor is protected, ensuring we can't instanciate a TEncoder
		TEncoder()
		{
		}
	};
};

#endif // TARGET_HAS_ThirdPartyOpenViBEPluginsGlobalDefines

#endif //__OpenViBEToolkit_TEncoder_H__
