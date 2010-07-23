#ifndef __OpenViBEAcquisitionServer_TAcqFifo_H__
#define __OpenViBEAcquisitionServer_TAcqFifo_H__

#include <openvibe/ov_all.h>

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class TAcqFifo
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
	class TAcqFifo
	{
	public:

		/** \name Constructors */
		//@{

		/**
		 * \brief Default constructor
		 *
		 * This constructor builds the internal implementation of this matrix.
		 */
		TAcqFifo(void)
			: m_pBuffer(0)
			, m_bufferCount(0)
			, m_channelCount(0)
			, m_channelSize(0)
			, m_extractIndex(0)
			, m_appendIndex(0)
			, m_appendPosition(0)
			, m_overflow(0)
		{}
		
		/**
		 * \brief Default constructor
		 *
		 * This constructor builds a full FIFO buffer.
		 */
		TAcqFifo(const OpenViBE::uint32 channelCount, const OpenViBE::uint32 channelSize, const OpenViBE::uint32 bufferCount = 1)
			: m_pBuffer(0)
			, m_bufferCount(0)
			, m_channelCount(0)
			, m_channelSize(0)
			, m_extractIndex(0)
			, m_appendIndex(0)
			, m_appendPosition(0)
			, m_overflow(0)
		{
			Build(channelCount, channelSize, bufferCount);
		}
		
		/**
		 * \brief Destructor
		 *
		 * The internal implementation is released.
		 */
		virtual ~TAcqFifo()
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
			return m_pBuffer != 0;
		}

		/**
		 * \brief Builds a 2D CMatrix organized as a sequence of \i channelCount channels
		 * \return \e true in case of success.
		 * \return \e false in case of error.
		 */
		virtual bool build(const OpenViBE::uint32 channelCount, const OpenViBE::uint32 channelSize, const OpenViBE::uint32 bufferCount = 1)
		{
			clean();

			m_pBuffer	= new type[channelCount*channelSize*bufferCount];

			if(isValid())
			{	m_channelCount		= channelCount;
				m_channelSize		= channelSize;
				m_bufferCount		= bufferCount;
				
				::memset(m_pBuffer, 0, getSizeof());
			}

			return isValid();
		}

		/**
		 * \brief Gets a pointer to the inner buffer 
		 * \return the pointer to the inner buffer 
		 * \note It shouldn't be modified externally
		 */
		type* getBuffer() const
		{
			return m_pBuffer;
		}

		bool canExtract(void) const
		{
			return m_appendIndex != m_extractIndex;
		}

		type* getExtractPtr(void)
		{
			return canExtract() ? m_pBuffer + m_extractIndex*extractChunkSize() : 0;
		}

		virtual void adjusteExtract(void)
		{
			shiftLeft();
		}

		template <class type1> bool appendData(const type1* pSrc, const OpenViBE::uint32 channelSize)
		{
			if(increaseData(channelSize))
				return append(pSrc, channelSize, false);

			return false;
		}

		template <class type1> bool appendTranspozedData(type1* pSrc, const OpenViBE::uint32 channelSize)
		{
			if(increaseData(channelSize))
				return append(pSrc, channelSize, true);

			return false;
		}
		
		/**
		 * \brief Dump the internal status
		 * \return a std::string containing the internal status. 
		 */
		virtual std::string	dump()
		{
			std::ostringstream	l_oss;
			
			l_oss	<< "; bufferCount = "		<< m_bufferCount
					<< "; channelCount = "		<< m_channelCount
					<< "; channelSize = "		<< m_channelSize
					<< "; extractIndex = "		<< m_extractIndex
					<< "; appendIndex = "		<< m_appendIndex
					<< "; appendPosition = "	<< m_appendPosition
					<< "; overflow = "			<< m_overflow
					;

			return l_oss.str();
		}
	protected:
		void clean()
		{
			delete m_pBuffer;

			m_pBuffer			= 0;
			m_bufferCount		= 0;

			m_channelCount		= 0;
			m_channelSize		= 0;

			m_extractIndex		= 0;

			m_appendIndex		= 0;
			m_appendPosition	= 0;
		}
		
		OpenViBE::uint32 getElementCount(void) const
		{
			return m_channelCount*m_channelSize*m_bufferCount;
		}

		virtual OpenViBE::uint32 getChannelCount(void) const
		{
			return m_channelCount;
		}

		OpenViBE::uint32 getSizeof(void) const
		{
			return getSizeof(getElementCount());
		}

		OpenViBE::uint32 getSizeof(const OpenViBE::uint32 nbElements) const
		{
			return nbElements*sizeof(type);
		}

		OpenViBE::uint32 extractChunkSize()
		{
			return m_channelCount*m_channelSize;
		}

		virtual bool increaseData(const OpenViBE::uint32 appendSize)
		{
			if(!isValid())
				return false;

			OpenViBE::uint32 roomSize			= (m_bufferCount - m_appendIndex)*m_channelSize - m_appendPosition;
			if(roomSize >= appendSize)
				return true;

			OpenViBE::uint32	extraBuffers	= (appendSize - roomSize)/m_channelSize + 1;
			type*	pBuffer						= new type[m_channelCount*m_channelSize*(m_bufferCount + extraBuffers)];
			if(pBuffer == 0)
				return false;

			memset(pBuffer, 0, getSizeof(m_channelCount*m_channelSize*(m_bufferCount + extraBuffers)));
			memcpy(pBuffer, m_pBuffer, getSizeof());

			delete m_pBuffer;
			
			m_pBuffer		 = pBuffer;
			m_bufferCount	+= extraBuffers;
			
			return true;
		}

		type* getChannelPtr(const OpenViBE::uint32 channelIndex)
		{
			return getIndexedChannelPtr(m_appendIndex, m_appendPosition, channelIndex);
		}

		type* getIndexedChannelPtr(const OpenViBE::uint32 appendIndex, const OpenViBE::uint32 appendPosition, const OpenViBE::uint32 channelIndex)
		{
			return m_pBuffer + appendIndex*extractChunkSize() + channelIndex*m_channelSize + appendPosition;
		}

		virtual void getLastAddedPosition(OpenViBE::uint32& appendIndex, OpenViBE::uint32& appendPosition, const OpenViBE::uint32 lastAddedSamples)
		{
			OpenViBE::uint32 oldPosition  = m_appendIndex*m_channelSize + m_appendPosition - lastAddedSamples;

			appendIndex			= oldPosition / m_channelSize;
			appendPosition		= oldPosition % m_channelSize;
		}

		void zeroLastAddedChannel(const OpenViBE::uint32 lastAddedSamples, const OpenViBE::uint32 channelIndex)
		{
			OpenViBE::uint32 appendIndex, appendPosition;
			getLastAddedPosition(appendIndex, appendPosition, lastAddedSamples);

			OpenViBE::uint32	nbSamples = lastAddedSamples;

			while(nbSamples)
			{	type*	pData				= getIndexedChannelPtr(appendIndex, appendPosition, channelIndex);
				OpenViBE::uint32	samples = appendPosition ? m_channelSize - appendPosition : m_channelSize;

				if(samples > nbSamples)
					samples		= nbSamples;

				memset(pData, 0, getSizeof(samples));

				nbSamples		-= samples;
				appendIndex++;
				appendIndex		%= m_bufferCount;
				appendPosition	 = 0;
			}
		}

		template <class type1> const type1* getSrcChannelPtr(const type1* pSrc, const OpenViBE::uint32 channelCount, const OpenViBE::uint32 channelSize, const OpenViBE::uint32 channelIndex, const OpenViBE::uint32 channelPosition, const bool transpozed)
		{
			return	transpozed ?
					pSrc + channelPosition*channelCount + channelIndex :
					pSrc + channelIndex*channelSize + channelPosition;
		}
		
		template <class type1> bool append(type1* pSrc, const OpenViBE::uint32 channelSize, const bool transpozed)
		{
			OpenViBE::uint32	srcIndex		= 0;
			OpenViBE::uint32	srcChannelSize	= channelSize;
			OpenViBE::uint32	channelCount	= getChannelCount();
			while(srcChannelSize)
			{	OpenViBE::uint32	nbElements	= (m_appendPosition == 0) ? m_channelSize : m_channelSize - m_appendPosition;
				
				if(nbElements > srcChannelSize)
					nbElements	= srcChannelSize;
				
				for(OpenViBE::uint32 ii=0; ii < channelCount; ii++)
				{		  type*		ptrDest	= getChannelPtr(ii);
					const type1*	ptrSrc	= getSrcChannelPtr(pSrc, channelCount, channelSize, ii, srcIndex, transpozed);
					OpenViBE::uint32			nb		= nbElements;
					
					if(transpozed)
					{	while(nb--)
						{	*ptrDest++	 = type(*ptrSrc);
							ptrSrc		+= channelCount;
					}	}
					else
					{	while(nb--)
							*ptrDest++	= type(*ptrSrc++);
				}	}

				srcChannelSize		-= nbElements;
				srcIndex			+= nbElements;
				
				adjusteAppendParams(nbElements + m_appendPosition);
			}

			return true;
		}

		void shiftLeft()
		{
			memcpy(m_pBuffer, m_pBuffer + extractChunkSize(), getSizeof(m_appendIndex*extractChunkSize()));

			m_appendIndex--;
		}
	
		virtual void adjusteAppendParams(const OpenViBE::uint32 nbElements)
		{
			m_appendPosition	 = nbElements % m_channelSize;
			m_appendIndex		+= nbElements / m_channelSize;
		}
		
	protected:
		type*					m_pBuffer;
		OpenViBE::uint32		m_bufferCount;

		OpenViBE::uint32		m_channelCount;
		OpenViBE::uint32		m_channelSize;
		
		OpenViBE::uint32		m_extractIndex;
		
		OpenViBE::uint32		m_appendIndex;
		OpenViBE::uint32		m_appendPosition;
		OpenViBE::uint32		m_overflow;			//!< keeps trace of the overflow status in debug mode
	};
};

#endif // __OpenViBEAcquisitionServer_TAcqFifo_H__
