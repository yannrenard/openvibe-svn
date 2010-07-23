#ifndef __OpenViBEAcquisitionServer_CAcqServerCircularBuffer_H__
#define __OpenViBEAcquisitionServer_CAcqServerCircularBuffer_H__

#include <openvibe/ov_all.h>

#include <boost/thread/mutex.hpp>

#include <string>
#include <sstream>

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class CAcqServerCircularBuffer
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
	class CAcqServerCircularBuffer
	{
	public:
		class CStimulation
		{	
		public:
			CStimulation(const OpenViBE::uint32 position=0, const OpenViBE::uint32 value=0)
				: position(position)
				, value(value)
				{}

			OpenViBE::uint32	position;
			OpenViBE::uint32	value;
		};
	public:

		/** \name Constructors */
		//@{

		/**
		 * \brief Default constructor
		 *
		 * This constructor builds the internal implementation of this matrix.
		 */
		CAcqServerCircularBuffer(void)
			: m_pBegin(0)
			, m_overflow(0)
		{}
				
		CAcqServerCircularBuffer(const OpenViBE::uint32 channelCount, const OpenViBE::uint32 appendSize, const OpenViBE::uint32 extractSize, const OpenViBE::uint32 multiplier = 8)
			: m_pBegin(0)
			, m_overflow(0)
		{
			build(channelCount, appendSize, extractSize, multiplier);
		}
		/**
		 * \brief Destructor
		 *
		 * The internal implementation is released.
		 */
		~CAcqServerCircularBuffer()
		{
			clean();
		}

		/**
		 * \brief Verify the buffer validity
		 * \return \e true in case of success.
		 * \return \e false in case of error.
		 */		
		bool isValid() const
		{
			return m_pBegin != 0;
		}

		/**
		 * \brief Builds a 2D CMatrix organized as a sequence of \i channelCount channels
		 * \return \e true in case of success.
		 * \return \e false in case of error.
		 */
		bool build(const OpenViBE::uint32 channelCount, const OpenViBE::uint32 appendSize, const OpenViBE::uint32 extractSize, const OpenViBE::uint32 multiplier = 8)
		{
			clean();

			OpenViBE::uint32 l_size	= multiplier*(channelCount + 1)*appendSize*extractSize;
			m_pBegin				= new OpenViBE::float32[l_size];

			if(isValid())
			{	m_pEnd				= m_pBegin + l_size;

				m_channelCount		= channelCount;

				m_pAppend			= m_pBegin;
				m_appendSize		= appendSize;
				m_appendBufferSize	= (m_channelCount + 1)*m_appendSize;

				m_pExtract			= m_pBegin;
				m_extractSize		= extractSize;
				m_extractBufferSize	= (m_channelCount + 1)*m_extractSize;
			}

			return isValid();
		}

		OpenViBE::float32*	getBuffer() const
		{
			return m_pBegin;
		}

		bool canExtract(void) const
		{
			return (m_pExtract > m_pAppend) || ((m_pExtract + m_extractBufferSize) <= m_pAppend);
		}

		template <class type> bool append(const type* pSrc, const std::vector<CStimulation>& vStimulations)
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

		bool extract(OpenViBE::float32* pfloat32DestBuffer, std::vector<CStimulation>& vStimulations)
		{
			if(!canExtract())
				return false;

			boost::mutex::scoped_lock l_scopedMutex(m_guard);

			for(OpenViBE::uint32 ii=0; ii < m_extractSize; ii++)
			{	OpenViBE::float32* l_pDest = pfloat32DestBuffer + ii;
				for(OpenViBE::uint32 jj=0; jj <= m_channelCount; jj++, l_pDest+=m_extractSize)
					*l_pDest = *m_pExtract++;
			}

			vStimulations.resize(0);
			OpenViBE::float32* l_pDest = pfloat32DestBuffer + m_channelCount*m_extractSize;
			for(OpenViBE::uint32 ii=0; ii < m_extractSize; ii++, l_pDest++)
			{	if(*l_pDest != OpenViBE::float32(0))
					vStimulations.push_back(CStimulation(ii, OpenViBE::uint32(*l_pDest)));
			}
				
			if(m_pExtract >= m_pEnd)
				m_pExtract	= m_pBegin;
		
			return true;
		}

		void amplifyData(const OpenViBE::uint32 uint32ChannelIndex, const OpenViBE::float32 float32Gain)
		{
			OpenViBE::float32*	l_pChannel  = m_pExtract + uint32ChannelIndex*m_extractSize;

			OpenViBE::uint32	l_extractSize = m_extractSize;
			while(l_extractSize--)
				*l_pChannel++	*= float32Gain;
		}

		std::string dump()
		{
			std::ostringstream	l_oss;
			
			l_oss	<< "; channelCount = "		<< m_channelCount + 1
					<< "; appendBuffers = "		<< (m_pEnd - m_pBegin)/m_appendBufferSize
					<< "; appendIndex = "		<< (m_pAppend - m_pBegin)/m_appendBufferSize
					<< "; extractBuffers = "	<< (m_pEnd - m_pBegin)/m_extractBufferSize
					<< "; extractIndex = "		<< (m_pExtract - m_pBegin)/m_extractBufferSize
					<< "; overflow = "			<< m_overflow
					;

			return l_oss.str();
		}

	private:
		void clean()
		{
			delete m_pBegin;

			m_pBegin	= 0;
		}
		
		bool increaseSize()
		{
			if((m_pAppend >= m_pExtract) || ((m_pAppend + m_appendBufferSize) <= m_pExtract))
				return true;

			OpenViBE::uint32	l_extraSize	= (m_pEnd - m_pBegin);
			OpenViBE::uint32	l_size		= 2*l_extraSize;
			OpenViBE::float32*	l_pBegin	= new OpenViBE::float32[l_size];

			if(l_pBegin == 0)
				return false;

			OpenViBE::uint32	l_toAppend	= m_pAppend - m_pBegin;
			OpenViBE::uint32	l_toExtract	= m_pEnd - m_pExtract;
			
			memcpy(l_pBegin,						m_pBegin,	getSizeof(l_toAppend));
			memcpy(l_pBegin + l_size - l_toExtract,	m_pExtract, getSizeof(l_toExtract));
		
			clean();

			m_pBegin	= l_pBegin;
			m_pEnd		= m_pBegin	+ l_size;
			m_pAppend	= m_pBegin	+ l_toAppend;
			m_pExtract	= m_pEnd	- l_toExtract;

			m_overflow++;
			
			return isValid();
		}

		OpenViBE::uint32 getSizeof(const OpenViBE::uint32 nbElements) const
		{
			return nbElements*sizeof(OpenViBE::float32);
		}

	private:
		OpenViBE::float32*		m_pBegin;
		OpenViBE::float32*		m_pEnd;
	
		OpenViBE::uint32		m_channelCount;

		OpenViBE::float32*		m_pAppend;
		OpenViBE::uint32		m_appendSize;
		OpenViBE::uint32		m_appendBufferSize;
		
		OpenViBE::float32*		m_pExtract;
		OpenViBE::uint32		m_extractSize;
		OpenViBE::uint32		m_extractBufferSize;
		OpenViBE::uint32		m_overflow;			//!< keeps trace of the overflow status in debug mode
		boost::mutex			m_guard;
	};
};

#endif // __OpenViBEAcquisitionServer_CAcqServerCircularBuffer_H__
