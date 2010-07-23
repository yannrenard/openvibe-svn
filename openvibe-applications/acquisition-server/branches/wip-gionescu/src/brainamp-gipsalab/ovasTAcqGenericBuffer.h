#ifndef __OpenViBEAcquisitionServer_TAcqCircularBuffer_H__
#define __OpenViBEAcquisitionServer_TAcqCircularBuffer_H__

#include "TAcqFifo.h"

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
	class TAcqCircularBuffer : public TAcqCircularFifo<type>
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
			: TAcqCircularFifo<type>(0)
		{}
		
		/**
		 * \brief Default constructor
		 *
		 * This constructor builds a full FIFO buffer.
		 */
		TAcqCircularBuffer(const size_t channelCount, const size_t channelSize, const size_t bufferCount = 1)
			: TAcqCircularFifo<type>(channelCount, channelSize, bufferCount)
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
		}
		
	protected:		
		virtual bool increaseData(const size_t ui32AppendSize)
		{
			return true;
		}
	};
};

#endif // __OpenViBEAcquisitionServer_TSignals_H__
