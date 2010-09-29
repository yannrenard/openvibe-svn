#ifndef __OpenViBEAcquisitionServer_CSignalsAndStimulation_H__
#define __OpenViBEAcquisitionServer_CSignalsAndStimulation_H__

#include "ovasCSignals.h"

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
	 * It provides also an extra channel to manage the synchronisation between the 
	 * acquired data and the acquired stimulations
	 *
	 */
	template <class strategy = TCircularBuffer<OpenViBE::float32> >
	class  TSignalsAndStimulation : public strategy
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
		TSignalsAndStimulation(void)
			: strategy()
		{}	
		
		/**
		 * \brief Constructor
		 *
		 * This constructor builds a valid buffer.
		 */
		TSignalsAndStimulation(const OpenViBE::uint32 ui32ChannelCount, const OpenViBE::uint32 ui32ChannelSize)
			: strategy(ui32ChannelCount, ui32ChannelSize)
		{}	
		
		/**
		 * \brief Builds an internal memory space organized as a sequence of \i ui32ChannelCount channels
		 * \return \e true in case of success.
		 * \return \e false in case of error.
		 * \note An extra channel for stimulations will be reserved.
		 */
		virtual OpenViBE::boolean build(const OpenViBE::uint32 ui32ChannelCount, const OpenViBE::uint32 ui32ChannelSize)
		{
			return strategy::build(ui32ChannelCount + 1, ui32ChannelSize);
		}
		
		/**
		 * \brief Gets the number of stimulations in the stimulations chunk 
		 * \return the number of stimulations in the buffer chunk
		 * \note This method is useful in the extraction step
		 */
		OpenViBE::uint32 getStimulationCount(const OpenViBE::uint32 ui32ChunkSize) const
		{
			OpenViBE::uint32	l_ui32StimulationCount	= 0;
			OpenViBE::uint32	l_ui32ChunkSize			= ui32ChunkSize;
			OpenViBE::float32*	pData					= getStimulationBegin();

			while(l_ui32ChunkSize--)
			{	if(*pData++ != 0)
					l_ui32StimulationCount++;
			}

			return l_ui32StimulationCount;
		}
		
		/**
		 * \brief Gets a stimulation from the stimulation list 
		 * \return the indexed stimulation from the list
		 * \note if the stimulation does not exist, -1 is returned
		 */
		OpenViBE::uint32 getStimulation(OpenViBE::uint32& ui32StimuilationPosition, const OpenViBE::uint32 ui32StimuilationIndex, const OpenViBE::uint32 ui32ChunkSize) const
		{
			ui32StimuilationPosition					= 0;

			OpenViBE::uint32	l_ui32StimulationIndex	= 0;
			OpenViBE::float32*	pData					= getStimulationBegin();
			OpenViBE::float32*	pDataEnd				= pData + ui32ChunkSize;

			while(pData < pDataEnd)
			{	if(*pData != 0)
				{	if(l_ui32StimulationIndex == ui32StimuilationIndex)
						return OpenViBE::uint32(*pData);
				
					l_ui32StimulationIndex++;
				}

				pData++;
				ui32StimuilationPosition++;
			}

			return -1;
		}
		
		/**
		 * \brief Add a stimulation at the specified position 
		 * \return \e true if the is successful.
		 * \return \e false otherwise.
		 */
		OpenViBE::boolean addStimulation(const OpenViBE::uint32 uint32Position, const OpenViBE::uint32 ui32LastChunkSize, const OpenViBE::float32 float32Stimulation)
		{

			if(uint32Position >= ui32LastChunkSize)
				return false;

			*(getStimulationPreviousEnd(ui32LastChunkSize) + uint32Position) = float32Stimulation;
			
			return true;
		}
		
		/**
		 * \brief Amplify the selected channel 
		 */
		void amplifyData(const OpenViBE::uint32 ui32ChannelIndex, const OpenViBE::uint32 ui32Samples, const OpenViBE::float32 float32Gain)
		{
			OpenViBE::float32*	pData = getChannelPreviousEnd(ui32ChannelIndex, ui32Samples);

			OpenViBE::uint32 l_ui32Samples = ui32Samples;
			while(l_ui32Samples--)
				*pData++	*= float32Gain;
		}

	protected:
		OpenViBE::float32* getStimulationBegin(void) const
		{
			return m_pExtract + (m_channelCount-1)*m_channelSize;
		}
		OpenViBE::float32* getStimulationEnd(void) const
		{
			return getChannelEnd(m_channelCount-1);
		}
		OpenViBE::float32* getStimulationPreviousEnd(const OpenViBE::uint32 ui32ChunkSize) const
		{
			return getChannelPreviousEnd(m_channelCount-1, ui32ChunkSize);
		}	
		void increase(const OpenViBE::uint32 ui32NbSamples)
		{
			memset(getStimulationEnd(), 0, getSizeof(ui32NbSamples));
			
			strategy::increase(ui32NbSamples);
		}
	};

	class  CSignalsAndStimulationCircular : public TSignalsAndStimulation<>
	{
	public:
		CSignalsAndStimulationCircular(void)
			: TSignalsAndStimulation<>()
			{}
		CSignalsAndStimulationCircular(const OpenViBE::uint32 ui32ChannelCount, const OpenViBE::uint32 ui32ChannelSize)
			: TSignalsAndStimulation<>(ui32ChannelCount, ui32ChannelSize)
			{}
	};

	class  CSignalsAndStimulationFifo : public TSignalsAndStimulation< TFifoBuffer<OpenViBE::float32> >
	{
	public:
		CSignalsAndStimulationFifo(void)
			: TSignalsAndStimulation< TFifoBuffer<OpenViBE::float32> >()
			{}
		CSignalsAndStimulationFifo(const OpenViBE::uint32 ui32ChannelCount, const OpenViBE::uint32 ui32ChannelSize)
			: TSignalsAndStimulation< TFifoBuffer<OpenViBE::float32> >(ui32ChannelCount, ui32ChannelSize)
			{}
	};
};

#endif // __OpenViBEAcquisitionServer_CSignalsAndStimulation_H__
