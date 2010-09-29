#ifndef __OpenViBEAcquisitionServer_CAcqServerCircularBuffer_H__
#define __OpenViBEAcquisitionServer_CAcqServerCircularBuffer_H__

#include <openvibe/ov_all.h>

#include <boost/thread/mutex.hpp>

#include <string>
#include <vector>

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class CAcqServerCircularBuffer
	 * \author Gelu Ionescu (Gipsa-lab)
	 * \date 2010-07-23
	 * \brief Basic §OpenViBE§ signals manipulation
	 * \ingroup Group_Base
	 *
	 * This class that acts as a extensible circular buffer. Its main goal is to provide an adaptor 
	 * between an input data flow and an output data flow of different types and chunk sizes
	 *
	 * \note Every time a data chunk is appended to the buffer, an internal check is done and the memory 
	 * will be increased if the internal available memory isn't sufficient 
	 *
	 * \remarks Four internal pointers will be maintained :
	 * - m_pBegin	    : points to the beginning of the circular buffer
	 * - m_pEnd			: points to the end of the circular buffer
	 * - m_pAppend		: points to the append zone 
	 * - m_pExtract		: points to theextraction zone
	 *
	 */
	class CAcqServerCircularBuffer
	{
	public:
		/**
		 * \class CStimulation
		 *
		 * Internal class that defines the stimulation parameters
		 *
		 * \remarks this class is useful for \i append and \i extract operations
		 */
		class CStimulation
		{	
		public:
			/**
			 * \brief constructor
			 *
			 * this constructor acts as a default constructor also
			 *
			 */
			CStimulation(const OpenViBE::uint32 position=0, const OpenViBE::uint32 value=0)
				: position(position)
				, value(value)
				{}

			OpenViBE::uint32	position;	//!< stimulation position
			OpenViBE::uint32	value;		//!< stimulation value
		};
	public:

		/** \name Constructors */
		//@{

		/**
		 * \brief Default constructor
		 *
		 * Default constructor (an invalid empty buffer will be constructed).
		 *
		 * \remarks Three internal pointers will be maintained
		 * \note call the build(...) method later to construct properly the buffer
		 */
		CAcqServerCircularBuffer(void);
				
		/**
		 * \brief True constructor
		 *
		 * This constructor build a starting buffer.
		 *
		 * \note it simply calls the build(...) method
		 */
		CAcqServerCircularBuffer(const OpenViBE::uint32 uint32ChannelCount, const OpenViBE::uint32 uint32AppendSize, const OpenViBE::uint32 uint32EextractSize, const OpenViBE::uint32 uint32Multiplier = 8);

		/**
		 * \brief Destructor
		 *
		 * The internal implementation is released.
		 */
		~CAcqServerCircularBuffer();

		/**
		 * \brief Verify the buffer validity
		 * \return \e true in case of success.
		 * \return \e false in case of error.
		 */		
		OpenViBE::boolean isValid() const;

		/**
		 * \brief Builds an internal memory space organized according to the following rules  : 
		 * - the memory space is contigu
		 * - the given data are append as a succession of samples S1 S2 ... Sn where each sample is
		 *   a succession of channels S1[C1, C2, ..., Cm] S2[C1, C2, ..., Cm] ... Sn[C1, C2, ..., Cm]
		 * - At the extraction step, the data will be transpozed; therefor the data looks as a succession of
		 *   channels C1 C2 ... Cm, each containing a succession of samples 
		 *   C1[S1, S2, ..., Sn] C2[S1, S2, ..., Sn] ... Cm[S1, S2, ..., Sn]
		 * - the memory size is modulo \i appendSize
		 * - the memory size is modulo \i extractSize
		 * - an extra channel is affected to keep track of the stimulations
		 *
		 * \return \e true in case of success.
		 * \return \e false in case of error.
		 *
		 * \note To avoid further memory manipulations, one can play with the \i multiplier parameter
		 */
		OpenViBE::boolean build(const OpenViBE::uint32 uint32ChannelCount, const OpenViBE::uint32 uint32AppendSize, const OpenViBE::uint32 uint32EextractSize, const OpenViBE::uint32 uint32Multiplier = 8);

		/**
		 * \brief Gets a pointer to the inner buffer 
		 * \return the pointer to the inner buffer 
		 * \note It shouldn't be modified externally
		 */
		OpenViBE::float32* getBuffer() const;

		/**
		 * \brief Verify if the extraction section is already filled-up
		 * \return \e true if the extraction section is available.
		 * \return \e false otherwise.
		 */
		OpenViBE::boolean canExtract(void) const;

		/**
		 * \brief This template method adds any kind of data type as explained for \i build() method (i.e. S1[C1, C2, ..., Cm] S2[C1, C2, ..., Cm] ... Sn[C1, C2, ..., Cm])
		 * It will also fill-up the stimulations channel from the \i vStimulations vector
		 * \return \e true if the buffer is not empty.
		 * \return \e false if the buffer is empty.
		 * \note - If not enough memory is available, the buffer will be resized)
		 * \remarks - This concurrent operation is a guarded by \i m_guard mutex; it can occurs safely
		 */
		template <class type> OpenViBE::boolean append(const type* pSrc, const std::vector<CStimulation>& vStimulations)
		{
			boost::mutex::scoped_lock l_scopedMutex(m_guard);
			
			if(!increaseSize())
				return false;

			OpenViBE::uint32	l_appendSize	= m_appendSize;
			OpenViBE::float32*	l_pAppend		= m_pAppend;
			while(l_appendSize--)
			{	OpenViBE::uint32 l_channelCount	= m_channelCount;			
				while(l_channelCount--)
					*l_pAppend++ = OpenViBE::float32(*pSrc++);

				*l_pAppend++ = OpenViBE::float32(0);
			}

			for(std::vector<CStimulation>::const_iterator it=vStimulations.begin(); it != vStimulations.end(); it++)
				*(m_pAppend + m_channelCount + it->position*(m_channelCount + 1))	= OpenViBE::float32(it->value);

			m_pAppend	= l_pAppend;

			if(m_pAppend >= m_pEnd)
				m_pAppend	= m_pBegin;

			return true;
		}

		/**
		 * \brief This method extracts data in a transpozed way as explained for \i build() method (i.e. C1[S1, S2, ..., Sn] C2[S1, S2, ..., Sn] ... Cm[S1, S2, ..., Sn])
		 * It will also fill-up the  \i vStimulations vector from the stimulations channel
		 * \return \e true if the buffer is not empty.
		 * \return \e false if the buffer is empty.
		 *
		 * \note The best way to extract data is to follow the next steps :
		 *       - build once an extraction buffer <i> CAcqServerCircularBuffer extractionBuffer(channelCount, 1, extractSize, 1) <\i>
		 *       - define a stimulation buffer <i> std::vector<CStimulation> vStimulations <\i>
		 *       - extract data <i> myBuffer.extract(extractionBuffer.getBuffer(), vStimulations); <\i>    
		 * \remarks - This concurrent operation is a guarded by \i m_guard mutex; it can occurs safely
		 */
		OpenViBE::boolean extract(OpenViBE::float32* pfloat32DestBuffer, std::vector<CStimulation>& vStimulations);

		/**
		 * \brief This method amplify the data channel by channel
		 *
		 * \note VERY IMPORTANT : this method should be called only on the extraction buffer
		 * (see the note for \i extract() method)
		 */
		void amplifyData(const OpenViBE::uint32 uint32ChannelIndex, const OpenViBE::float32 float32Gain);

		/**
		 * \brief This method dumps the internal status in a text form for debug purposes
		 *
		 * \return a formatted \i std::string string.
		 */
		std::string dump() const;

	private:
		void					clean();
		OpenViBE::boolean		increaseSize();
		OpenViBE::uint32		getSizeof(const OpenViBE::uint32 nbElements) const;

	private:
		OpenViBE::float32*		m_pBegin;				//!< internal buffer address
		OpenViBE::float32*		m_pEnd;					//!< address of the internal buffer limit
	
		OpenViBE::uint32		m_channelCount;			//!< number of channels in the internal buffer

		OpenViBE::float32*		m_pAppend;				//!< pointer to the append zone
		OpenViBE::uint32		m_appendSize;			//!< input chunk size in terms of samples per channel
		OpenViBE::uint32		m_appendBufferSize;		//!< actually = m_channelCount * m_appendSize
		
		OpenViBE::float32*		m_pExtract;
		OpenViBE::uint32		m_extractSize;			//!< output chunk size in terms of samples per channel
		OpenViBE::uint32		m_extractBufferSize;	//!< actually = m_channelCount * m_extractSize
		OpenViBE::uint32		m_overflow;				//!< keeps trace of the overflow status in debug mode
		boost::mutex			m_guard;				//!< input / output operations guard
	};
};
#endif // __OpenViBEAcquisitionServer_CAcqServerCircularBuffer_H__
