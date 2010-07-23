#ifndef __OpenViBEAcquisitionServer_CSignals_H__
#define __OpenViBEAcquisitionServer_CSignals_H__

#include "ovasTGenericBuffer.h"

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class CSignalsCircular32 & CSignalsFifo32
	 * \author Gelu Ionescu (Gipsa-lab)
	 * \date 2010-06-07
	 * \brief Basic §OpenViBE§ signals manipulation
	 * \ingroup Group_Base
	 *
	 * Helper classes that allow the specialisation of the TGenericBuffer<> class
	 * toward the current utilisation
	 *
	 */

	typedef	TGenericBuffer<> TGenericCircular32;

	class  CSignalsCircular32 : public TGenericCircular32
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
		CSignalsCircular32()
			: TGenericCircular32()
			{}
		
		/**
		 * \brief Constructor
		 *
		 * This constructor builds a valid buffer.
		 */
		CSignalsCircular32(const OpenViBE::uint32 ui32ChannelCount, const OpenViBE::uint32 ui32ChannelSize)
			: TGenericCircular32(ui32ChannelCount, ui32ChannelSize)
			{}

	};

	typedef	TGenericBuffer<OpenViBE::float32, TFifoBuffer<OpenViBE::float32> > TGenericFifo32;
	
	class  CSignalsFifo32 : public TGenericFifo32
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
		CSignalsFifo32()
			: TGenericFifo32()
			{}
		
		/**
		 * \brief Constructor
		 *
		 * This constructor builds a valid buffer.
		 */
		CSignalsFifo32(const OpenViBE::uint32 ui32ChannelCount, const OpenViBE::uint32 ui32ChannelSize)
			: TGenericFifo32(ui32ChannelCount, ui32ChannelSize)
			{}

	};
};

#endif // __OpenViBEAcquisitionServer_CSignals_H__
