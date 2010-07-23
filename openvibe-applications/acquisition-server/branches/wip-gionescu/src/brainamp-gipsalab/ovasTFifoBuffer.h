#ifndef __OpenViBEAcquisitionServer_TFifoBuffer_H__
#define __OpenViBEAcquisitionServer_TFifoBuffer_H__

#include <openvibe/ov_all.h>

#include <string>
#include <sstream>

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class TFifoBuffer
	 * \author Gelu Ionescu (Gipsa-lab)
	 * \date 2010-06-07
	 * \brief Basic §OpenViBE§ signals manipulation
	 * \ingroup Group_Base
	 *
	 * Template class that acts as a FIFO buffer. Its main goal is to provide an adaptor 
	 * between an input data flow and an output data flow of different types and
	 * chunk sizes
	 *
	 * \note Every time a data chunk is appended to the buffer, an internal check is done and the memory 
	 * will be increased if the internal available memory isn't sufficient 
	 *
	 \remarks Three internal pointers will be maintained
	 - m_pBuffer	: points to the inner memory buffer; it shouldn't be modified 
	 - m_pBegin	    : points to the beginning of the first channel (generally m_pBegin = m_pBuffer)
	 - m_pEnd		: points to the end of the first channel
	 - m_pExtract	: points to extraction zone
	 *
	 * An user who wishes to leave a space at the beginning of the buffer for various reasons 
	 * (eg. for cubic interpolation the former samples are necessary), will consequently have 
	 * to move the three pointers m_pBegin, m_pEnd, m_pExtract towards the right  
	 */
	template <class type>
	class TFifoBuffer
	{
	public:

		/** \name Constructors */
		//@{

		/**
		 * \brief Default constructor
		 *
		 * Default constructor (an invalid empty buffer will be constructed).
		 *
		 * \note call the build(...) method later 
		 */
		TFifoBuffer(void);
		
		/**
		 * \brief Constructor
		 *
		 * This constructor builds a valid buffer.
		 */
		TFifoBuffer(const OpenViBE::uint32 ui32ChannelCount, const OpenViBE::uint32 ui32ChannelSize);
		
		/**
		 * \brief Destructor
		 *
		 * Releases the internal memory
		 */
		virtual ~TFifoBuffer(void);

		/**
		 * \brief Verify the buffer validity
		 * \return \e true if the buffer is not empty.
		 * \return \e false if the buffer is empty.
		 */		
		OpenViBE::boolean isValid() const;

		/**
		 * \brief Builds an internal memory space organized as a sequence of \i ui32ChannelCount channels;
		 * Each channel has \i ui32ChannelSize size
		 * \return \e true in case of success.
		 * \return \e false in case of error.
		 */
		virtual OpenViBE::boolean build(const OpenViBE::uint32 ui32ChannelCount, const OpenViBE::uint32 ui32ChannelSize);

		/**
		 * \brief Gets the number of channels in the buffer 
		 * \return the number of channels in the buffer
		 */
		OpenViBE::uint32 getChannelCount(void) const;

		/**
		 * \brief Gets the size of each channel in the buffer 
		 * \return the actual reserved space of each channel in the buffer
		 */
		OpenViBE::uint32 getChannelSize(void) const;

		/**
		 * \brief Gets the number of samples of each channel in the buffer 
		 * \return the actual number of samples of each channel in the buffer
		 * \note The value returned by this function is in the interval \b [0..getChannelSize()]
		 * actually this <i> value = m_pEnd - m_pBegin <\i>
		 */
		virtual OpenViBE::uint32 getChannelSamples(void) const;
		
		/**
		 * \brief Gets total number of samples available in the internal buffer 
		 * \return the size of the internal buffer
		 * \note actually getChannelSize()*getChannelCount()
		 */
		OpenViBE::uint32 getBufferElementCount(void) const;
		
		/**
		 * \brief Gets the size of the internal buffer 
		 * \return the size of the internal buffer in terms of number of bytes
		 * \note The value returned by this function is \i getBufferElementCount()*sizeof(type)
		 * \note The second variant will return \i ui32NbElements*sizeof(type)
		 */
		OpenViBE::uint32 getSizeof(void) const;
		
		/**
		 * \brief Gets a pointer to the inner buffer 
		 * \return the pointer to the inner buffer 
		 * \note It shouldn't be modified externally
		 */
		type*	getBuffer() const;

		/**
		 * \brief Gets a pointer to the beginning of the inner buffer 
		 * \return the pointer to the inner buffer 
		 */
		type*	getBegin() const;

		/**
		 * \brief Gets a pointer to the beginning of the channel's buffer 
		 * \return the pointer to the channel buffer 
		 * \note If the \i m_pBegin pointer wasn't modified by the user, it returns 
		 * the same value as \i getChannelBegin() method
		 */
		type*	getChannelBuffer(const OpenViBE::uint32 ui32ChannelIndex) const;
		
		/**
		 * \brief Gets a pointer to the beginning of the channel 
		 * \return the pointer to the beginning of the channel 
		 * \note If the \i m_pBegin pointer wasn't modified by the user, it returns 
		 * the same value as \i getChannelBuffer() method
		 */
		type*	getChannelBegin(const OpenViBE::uint32 ui32ChannelIndex) const;
	
		/**
		 * \brief Gets a pointer to the end of the channel buffer 
		 * \return the pointer end of to the channel buffer 
		 */
		type*	getChannelEnd(const OpenViBE::uint32 ui32ChannelIndex) const;

		/**
		 * \brief Verify if the extraction section is already filled-up
		 * \return \e true if the extraction section is available.
		 * \return \e false otherwise.
		 */
		virtual OpenViBE::boolean	canExtract(const OpenViBE::uint32 ui32ChannelSize);
		
		/**
		 * \brief All those methods extract data from the FIFO buffer
		 \note   - Because the internal memory has a FIFO behaviour the data is extracted  always from the beginning of the buffer
				 - In the Circular buffer variant the data is extracted from the \i m_pExtract buffer; in this case, 
				 be shure that the internal buffer size is modulo the extraction chunk size
		 \remark - the \i dest TFifoBuffer<type> buffer should be constructed properly
		 */
		virtual OpenViBE::boolean	extractData(OpenViBE::float32* pDest, const OpenViBE::uint32 ui32ChannelCount, const OpenViBE::uint32 ui32ChannelSize, const OpenViBE::uint32 ui32SrcChannelindex = 0);
		virtual OpenViBE::boolean	extractData(TFifoBuffer<type>& dest, const OpenViBE::uint32 ui32SrcChannelindex = 0);

		/**
		 * \brief All those methods add data to the end of the FIFO buffer
		 * \return \e true if the buffer is not empty.
		 * \return \e false if the buffer is empty.
		 \note   - Because the internal memory has a FIFO behaviour is added at the end of the buffer (if not enough memory is available, the buffer will be resized)
				 - In the Circular buffer variant the data is added in a circular way (be shure that the enough memory is available);
				 be shure that the internal buffer size is modulo the append chunk size
		 */
		virtual OpenViBE::boolean	appendData(const type* pSrc, const OpenViBE::uint32 ui32ChannelCount, const OpenViBE::uint32 ui32ChannelSize, const OpenViBE::uint32 ui32DestChannelindex = 0);
		virtual OpenViBE::boolean	appendData(const TFifoBuffer<type>& src, const OpenViBE::uint32 ui32DestChannelindex = 0);

		/**
		 * \brief This methods add transposed data to the end of the FIFO buffer
		 * \return \e true if the buffer is not empty.
		 * \return \e false if the buffer is empty.
		 \note   - Because the internal memory has a FIFO behaviour is added at the end of the buffer (if not enough memory is available, the buffer will be resized)
				 - In the Circular buffer variant the data is added in a circular way (be shure that the enough memory is available);
				 be shure that the internal buffer size is modulo the append chunk size
		 */
		template <class type1> OpenViBE::boolean appendTranspozedData(type1* pSrc, const OpenViBE::uint32 ui32ChannelCount, const OpenViBE::uint32 ui32ChannelSize, const OpenViBE::uint32 ui32DestChannelindex = 0)
		{
			if(!isValid())
			{	if(!build(ui32DestChannelindex + ui32ChannelCount, ui32ChannelSize))
					return false;
			}
			else
			{	if((ui32DestChannelindex + ui32ChannelCount) > m_channelCount)
					return false;

				if(!increaseData(ui32ChannelSize))
					return false;
			}

			for(OpenViBE::uint32 iChan=0; iChan < ui32ChannelCount; iChan++)
			{	type*	_pDest	= getChannelEnd(iChan + ui32DestChannelindex);
				type1*	_pSrc	= pSrc + iChan;
				
				for(OpenViBE::uint32 iSample=0; iSample < ui32ChannelSize; iSample++, _pDest++, _pSrc+=ui32ChannelCount)
					*_pDest	= type(*_pSrc);
			}
			
			increase(ui32ChannelSize);

			return true;
		}
		/**
		 * \brief Dump the internal status
		 * \return a std::string containing the internal status. 
		 */
		virtual std::string	dump();

	protected:		
		OpenViBE::uint32			getSizeof(const OpenViBE::uint32 ui32NbElements) const;
		virtual type*				getChannelPreviousEnd(const OpenViBE::uint32 ui32ChannelIndex, const OpenViBE::uint32 ui32PreviousChunkSize) const;
		virtual	void				clean(void);
		virtual OpenViBE::uint32	getChannelRoom(void) const;
		virtual	OpenViBE::boolean	increaseData(const OpenViBE::uint32 ui32NbSamples);
		virtual void				increase(const OpenViBE::uint32 ui32NbSamples);
		virtual void				decrease(const OpenViBE::uint32 ui32NbSamples);
		virtual OpenViBE::boolean	shiftLeft(const OpenViBE::uint32 uint32ShiftSize);
	
		// extraction zone
		virtual type*				getExtractPtr(const OpenViBE::uint32 ui32ChannelIndex) const;
		virtual void				adjusteExtract(const OpenViBE::uint32 ui32NbSamples);
		
		static	OpenViBE::boolean	copyData(const OpenViBE::float32* pDest, const OpenViBE::uint32 uint32DestStride, const OpenViBE::float32* const pSrc, const OpenViBE::uint32 uint32SrcStride, const OpenViBE::uint32 ui32nbChannels, const OpenViBE::uint32 ui32nbSamples);
	
	protected:
		OpenViBE::uint32			m_channelCount;		//!< number of channels in the internal buffer
		OpenViBE::uint32			m_channelSize;		//!< curent channel size
		type*						m_pBegin;			//!< points to the beginning of the first channel (generally = m_pBegin)
		type*						m_pEnd;				//!< points to the end of the first channel
		type*						m_pExtract;			//!< points to the next extraction zone
	private:
		type*						m_pBuffer;			//!< private pointer to the internal memory buffer
		OpenViBE::uint32			m_overflow;			//!< keeps trace of the overflow status in debug mode
	};
};

#endif // __OpenViBEAcquisitionServer_TFifoBuffer_H__
