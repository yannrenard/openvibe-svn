#ifndef __OpenViBEAcquisitionServer_TAcqCircularBuffer_H__
#define __OpenViBEAcquisitionServer_TAcqCircularBuffer_H__

#include "ovasTAcqFifo.h"

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class TAcqCircularBuffer
	 * \author Gelu Ionescu (Gipsa-lab)
	 * \date 2010-06-07
	 * \brief Basic §OpenViBE§ signals manipulation
	 * \ingroup Group_Base
	 *
	 * Based on the OpenViBE::CMatrix, this class acts as FIFO buffer. 
	 *
	 * Instances of this class use an internal implementation of the OpenViBE::CMatrix
	 * interface and redirect their calls to this implementation.
	 */
	template <class type>
	class TAcqCircularBuffer : public TAcqFifo<type>
	{
	public:

		/** \name Constructors */
		//@{

		/**
		 * \brief Default constructor
		 *
		 * This constructor builds the internal implementation of this matrix.
		 */
		TAcqCircularBuffer(void)
			: TAcqFifo<type>()
		{}
		
		/**
		 * \brief Default constructor
		 *
		 * This constructor builds a full FIFO buffer.
		 */
		TAcqCircularBuffer(const OpenViBE::uint32 channelCount, const OpenViBE::uint32 channelSize, const OpenViBE::uint32 bufferCount = 1)
			: TAcqFifo<type>(channelCount, channelSize, bufferCount)
		{
		}
		
		/**
		 * \brief Destructor
		 *
		 * The internal implementation is released.
		 */
		virtual ~TAcqCircularBuffer()
		{
		}

		virtual void adjusteExtract(void)
		{
			m_extractIndex++;
			m_extractIndex	%= m_bufferCount;
		}
		
	protected:		
		virtual bool increaseData(const OpenViBE::uint32 ui32AppendSize)
		{
			return true;
		}
		
		virtual void getLastAddedPosition(OpenViBE::uint32& appendIndex, OpenViBE::uint32& appendPosition, const OpenViBE::uint32 lastAddedSamples)
		{
			OpenViBE::uint32 oldPosition  = m_appendIndex*m_channelSize + m_appendPosition + (m_bufferCount*m_channelSize - lastAddedSamples);

			appendIndex			= oldPosition / m_channelSize;
			appendIndex		   %= m_bufferCount;
			appendPosition		= oldPosition % m_channelSize;
		}
		
		virtual void adjusteAppendParams(const OpenViBE::uint32 nbElements)
		{
			OpenViBE::boolean less = m_appendIndex < m_extractIndex;
			TAcqFifo<type>::adjusteAppendParams(nbElements);

			m_appendIndex		%= m_bufferCount;

			if(less && (m_appendIndex >= m_extractIndex))
				m_overflow++;
		}
	};
};

#endif // __OpenViBEAcquisitionServer_TAcqCircularBuffer_H__
