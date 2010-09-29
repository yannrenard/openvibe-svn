#ifndef __OpenViBEAcquisitionServer_TGenericBuffer_H__
#define __OpenViBEAcquisitionServer_TGenericBuffer_H__

#include "ovasTCircularBuffer.h"
#include "ovasTCircularBuffer.impl.h"

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class TGenericBuffer
	 * \author Gelu Ionescu (Gipsa-lab)
	 * \date 2010-06-07
	 * \brief Basic §OpenViBE§ signals manipulation
	 * \ingroup Group_Base
	 *
	 * Helper Template classe that allows to setup the type of the data flow and the
	 * internal buffer behaviour (FIFO or Circular) as well 
	 *
	 */

	template <class type = OpenViBE::float32, class strategy = TCircularBuffer<type> >
	class  TGenericBuffer : public strategy
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
		TGenericBuffer(void)
			: strategy()
		{}	
		
		/**
		 * \brief Constructor
		 *
		 * This constructor builds a valid buffer.
		 */
		TGenericBuffer(const OpenViBE::uint32 ui32ChannelCount, const OpenViBE::uint32 ui32ChannelSize)
			: strategy(ui32ChannelCount, ui32ChannelSize)
		{}	
	};
};

#endif // __OpenViBEAcquisitionServer_TGenericBuffer_H__
