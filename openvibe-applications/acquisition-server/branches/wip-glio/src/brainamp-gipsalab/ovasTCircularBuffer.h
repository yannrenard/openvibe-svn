#ifndef __OpenViBEAcquisitionServer_TCircularBuffer_H__
#define __OpenViBEAcquisitionServer_TCircularBuffer_H__

#include "ovasTFifoBuffer.h"

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class TCircularBuffer
	 * \author Gelu Ionescu (Gipsa-lab)
	 * \date 2010-06-07
	 * \brief Basic §OpenViBE§ signals manipulation
	 * \ingroup Group_Base
	 *
	 * Based on the TFifoBuffer<type> class, this class acts as Circular buffer. 
	 *
	 * Template class that acts as circular buffer. Its main goal is to provide an adaptor 
	 * between an input data flow and an output data flow of different types and
	 * chunk sizes
	 *
	 * This class is faster than the TFifoBuffer<type> ancestor but the internal memory
	 * can not be resized
	 *
	 * \remarks A big constraint deserves to be underlined: at the construction stage 
	 * the channel's size has to have modulo with both : append chunk size & extract chunk size 
	 *
	 * \note <i> BufferSize = 4*AppendChunkSize*ExtractChunkSize <\i> seems to be adequate
	 */
	template <class type>
	class TCircularBuffer : public TFifoBuffer<type>
	{
	public:

		/** \name Constructors */
		//@{

		/**
		 * \brief Default constructor
		 *
		 * Default constructor constructor (an invalid empty buffer will be constructed).
		 *
		 * \note call the build(...) method later 
		 */
		TCircularBuffer(void);
		
		/**
		 * \brief Constructor
		 *
		 * This constructor builds a valid buffer.
		 */
		TCircularBuffer(const OpenViBE::uint32 ui32ChannelCount, const OpenViBE::uint32 ui32ChannelSize);
		
		/**
		 * \brief Destructor
		 *
		 * Releases the internal memory
		 */
		virtual ~TCircularBuffer(void);

		/**
		 * \brief Verify if the extraction section is already filled-up
		 * \return \e true if the extraction section is available.
		 * \return \e false otherwise.
		 */
		virtual OpenViBE::boolean	canExtract(const OpenViBE::uint32 ui32ChannelSize);

		/**
		 * \brief Gets the number of samples of each channel in the buffer 
		 * \return the actual number of samples of each channel in the buffer
		 * \note The value returned by this function is in the interval \b [0..getChannelSize()]
		 * actually this <i> value = m_pEnd - m_pBegin <\i>
		 */
		virtual OpenViBE::uint32 getChannelSamples(void) const;
	protected:
		virtual type*				getChannelPreviousEnd(const OpenViBE::uint32 ui32ChannelIndex, const OpenViBE::uint32 ui32PreviousChunkSize) const;
		virtual	OpenViBE::boolean	increaseData(const OpenViBE::uint32 ui32NbSamples);
		virtual void				increase(const OpenViBE::uint32 ui32NbSamples);
		virtual OpenViBE::boolean	shiftLeft(const OpenViBE::uint32 uint32ShiftSize);

		virtual type*				getExtractPtr(const OpenViBE::uint32 ui32ChannelIndex) const;
		virtual void				adjusteExtract(const OpenViBE::uint32 ui32NbSamples);
	};
};

#endif // __OpenViBEAcquisitionServer_TCircularBuffer_H__
