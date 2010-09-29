#ifndef __OpenViBEAcquisitionServer_TAcqSignalsAndStimulation_H__
#define __OpenViBEAcquisitionServer_TAcqSignalsAndStimulation_H__

#include "ovasTAcqCircularBuffer.h"

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class TAcqSignalsAndStimulation
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
	template < class strategy = TAcqCircularBuffer< OpenViBE::float32> >
	class TAcqSignalsAndStimulation : public strategy
	{
	public:

		/** \name Constructors */
		//@{

		/**
		 * \brief Default constructor
		 *
		 * This constructor builds the internal implementation of this matrix.
		 */
		TAcqSignalsAndStimulation(void)
			: strategy()
		{}
		
		/**
		 * \brief Default constructor
		 *
		 * This constructor builds a full FIFO buffer.
		 */
		TAcqSignalsAndStimulation(const OpenViBE::uint32 channelCount, const OpenViBE::uint32 channelSize, const OpenViBE::uint32 bufferCount = 1)
		{
			build(channelCount + 1, channelSize, bufferCount);
		}
		
		virtual bool build(const OpenViBE::uint32 channelCount, const OpenViBE::uint32 channelSize, const OpenViBE::uint32 bufferCount = 1)
		{
			return strategy::build(channelCount + 1, channelSize, bufferCount);
		}

		void amplifyData(const OpenViBE::uint32 channelIndex, const OpenViBE::uint32 lastAddedSamples, const OpenViBE::float32 gain)
		{
			if(channelIndex == 0)
				zeroLastAddedChannel(lastAddedSamples, getChannelCount());

			OpenViBE::uint32 appendIndex, appendPosition;
			getLastAddedPosition(appendIndex, appendPosition, lastAddedSamples);

			OpenViBE::uint32	nbSamples = lastAddedSamples;

			while(nbSamples)
			{	OpenViBE::float32*	pData	= getIndexedChannelPtr(appendIndex, appendPosition, channelIndex);
				OpenViBE::uint32	samples	= appendPosition ? m_channelSize - appendPosition : m_channelSize;

				if(samples > nbSamples)
					samples		= nbSamples;

				OpenViBE::uint32	nb		= samples;
				while(nb--)
					*pData++	*= gain;

				nbSamples		-= samples;
				appendIndex++;
				appendIndex		%= m_bufferCount;
				appendPosition	 = 0;
			}
		}
		
		void addStimulation(const OpenViBE::uint32 stimulationPosition, const OpenViBE::uint32 lastAddedSamples, const OpenViBE::float32 stimulationValue)
		{
			OpenViBE::uint32 appendIndex, appendPosition;
			getLastAddedPosition(appendIndex, appendPosition, lastAddedSamples - stimulationPosition);

			*getIndexedChannelPtr(appendIndex, appendPosition, getChannelCount())	= stimulationValue;
		}

		OpenViBE::uint32 getStimulationCount()
		{
			OpenViBE::float32*	pData				= getIndexedChannelPtr(m_extractIndex, 0, getChannelCount());
			OpenViBE::uint32	samples				= m_channelSize;
			OpenViBE::uint32	stimulationCount	= 0;
			while(samples--)
			{	if(*pData++ != OpenViBE::float32(0))
					stimulationCount++;
			}

			return stimulationCount;
		}

		OpenViBE::uint32 getStimulation(OpenViBE::uint32& stimuilationPosition, const OpenViBE::uint32 stimulationIndex)
		{
			OpenViBE::float32*	pData				= getIndexedChannelPtr(m_extractIndex, 0, getChannelCount());
			OpenViBE::uint32	samples				= m_channelSize;
			OpenViBE::uint32	stimulationCount	= 0;
			stimuilationPosition					= 0;
			while(samples--)
			{	if(*pData != OpenViBE::float32(0))
				{	if(stimulationCount++ == stimulationIndex)
						return OpenViBE::uint32(*pData);
				}

				pData++;
				stimuilationPosition++;
			}

			return 0;
		}

	protected:
		virtual OpenViBE::uint32 getChannelCount(void) const
		{
			return m_channelCount - 1;
		}
	};

	class  CSignalsAndStimulationCircular : public TAcqSignalsAndStimulation<>
	{
	public:
		//CSignalsAndStimulationCircular(void)
		//	: TSignalsAndStimulation<>()
		//	{}
		//CSignalsAndStimulationCircular(const OpenViBE::uint32 channelCount, const OpenViBE::uint32 channelSize, const OpenViBE::uint32 bufferCount = 1)
		//	: TAcqSignalsAndStimulation<>(channelCount, channelSize, bufferCount)
		//	{}
	};

	class  CSignalsAndStimulationFifo : public TAcqSignalsAndStimulation< TAcqFifo<OpenViBE::float32> >
	{
	public:
		//CSignalsAndStimulationFifo(void)
		//	: TSignalsAndStimulation< TAcqFifo<OpenViBE::float32> >()
		//	{}
		//CSignalsAndStimulationFifo(const OpenViBE::uint32 channelCount, const OpenViBE::uint32 channelSize, const OpenViBE::uint32 bufferCount = 1)
		//	: TSignalsAndStimulation< TAcqFifo<OpenViBE::float32> >(channelCount, channelSize, bufferCount)
		//	{}
	};
};

#endif // __OpenViBEAcquisitionServer_TAcqSignalsAndStimulation_H__
