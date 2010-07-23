using namespace OpenViBEAcquisitionServer;

// public section
template <class type> TFifoBuffer<type>::TFifoBuffer(void)
	: m_channelCount(0)
	, m_channelSize(0)
	, m_pBuffer(0)
	, m_pBegin(0)
	, m_pEnd(0)
	, m_overflow(0)
{
}
		
template <class type> TFifoBuffer<type>::TFifoBuffer(const OpenViBE::uint32 ui32ChannelCount, const OpenViBE::uint32 ui32ChannelSize)
	: m_channelCount(0)
	, m_channelSize(0)
	, m_pBuffer(0)
	, m_pBegin(0)
	, m_pEnd(0)
	, m_overflow(0)
{
	build(ui32ChannelCount, ui32ChannelSize);
}
		
template <class type> TFifoBuffer<type>::~TFifoBuffer(void)
{	
	clean();
}

template <class type> OpenViBE::boolean TFifoBuffer<type>::isValid() const
{
	return m_pBuffer != 0;
}

template <class type> OpenViBE::boolean TFifoBuffer<type>::build(const OpenViBE::uint32 ui32ChannelCount, const OpenViBE::uint32 ui32ChannelSize)
{
	clean();

	m_pBuffer	= new type[ui32ChannelCount*ui32ChannelSize];

	if(isValid())
	{	m_pBegin		= m_pBuffer;
		m_pEnd			= m_pBuffer;
		m_pExtract		= m_pBuffer;
		m_channelCount	= ui32ChannelCount;
		m_channelSize	= ui32ChannelSize;
		
		::memset(m_pBuffer, 0, getSizeof());
	}

	return isValid();
}

template <class type> OpenViBE::uint32 TFifoBuffer<type>::getChannelCount(void) const
{
	return m_channelCount;
}

template <class type> OpenViBE::uint32 TFifoBuffer<type>::getChannelSize(void) const
{
	return m_channelSize;
}

template <class type> OpenViBE::uint32 TFifoBuffer<type>::getChannelSamples(void) const
{
	return m_pEnd - m_pExtract;
}

template <class type> OpenViBE::boolean TFifoBuffer<type>::extractData(OpenViBE::float32* pDest, const OpenViBE::uint32 ui32ChannelCount, const OpenViBE::uint32 ui32ChannelSize, const OpenViBE::uint32 ui32SrcChannelindex /*= 0*/)
{
	if(!isValid())
		return false;

	if(pDest == 0)
		return false;

	if((ui32SrcChannelindex + ui32ChannelCount) > m_channelCount)
		return false;

	if(!canExtract(ui32ChannelSize))
		return false;

	if(!copyData(pDest, ui32ChannelSize, getExtractPtr(ui32SrcChannelindex), m_channelSize, ui32ChannelCount, ui32ChannelSize))
		return false;

	if(!shiftLeft(ui32ChannelSize))
		return false;

	adjusteExtract(ui32ChannelSize);

	return true;
}

template <class type> OpenViBE::boolean TFifoBuffer<type>::extractData(TFifoBuffer<type>& dest, const OpenViBE::uint32 ui32SrcChannelindex /*= 0*/)
{	
	return extractData(dest.getBuffer(), dest.getChannelCount(), dest.getChannelSize(), ui32SrcChannelindex);
}

template <class type> OpenViBE::boolean TFifoBuffer<type>::appendData(const type* pSrc, const OpenViBE::uint32 ui32ChannelCount, const OpenViBE::uint32 ui32ChannelSize, const OpenViBE::uint32 ui32DestChannelindex /*= 0*/)
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

	if(!copyData(getChannelEnd(ui32DestChannelindex), m_channelSize, pSrc, ui32ChannelSize, ui32ChannelCount, ui32ChannelSize))
		return false;

	increase(ui32ChannelSize);

	return true;
}

template <class type> OpenViBE::boolean TFifoBuffer<type>::appendData(const TFifoBuffer<type>& src, const OpenViBE::uint32 ui32DestChannelindex /*= 0*/)
{	
	return appendData(src.getBuffer(), src.getChannelCount(), src.getChannelSize(), ui32DestChannelindex);
}

// protected section
template <class type> OpenViBE::uint32 TFifoBuffer<type>::getBufferElementCount(void) const
{
	return m_channelCount*m_channelSize;
}

template <class type> OpenViBE::uint32 TFifoBuffer<type>::getSizeof(const OpenViBE::uint32 ui32NbElements) const
{
	return ui32NbElements*sizeof(type);
}

template <class type> OpenViBE::uint32 TFifoBuffer<type>::getSizeof(void) const
{
	return getSizeof(getBufferElementCount());
}

template <class type> type* TFifoBuffer<type>::getBuffer() const
{
	return m_pBuffer;
}

template <class type> type* TFifoBuffer<type>::getBegin() const
{
	return m_pBegin;
}

template <class type> type* TFifoBuffer<type>::getChannelBuffer(const OpenViBE::uint32 ui32ChannelIndex) const
{
	return m_pBuffer + ui32ChannelIndex*m_channelSize;
}
		
template <class type> type* TFifoBuffer<type>::getChannelBegin(const OpenViBE::uint32 ui32ChannelIndex) const
{
	return m_pBegin + ui32ChannelIndex*m_channelSize;
}
	
template <class type> type* TFifoBuffer<type>::getChannelEnd(const OpenViBE::uint32 ui32ChannelIndex) const
{
	return m_pEnd + ui32ChannelIndex*m_channelSize;
}

template <class type> type* TFifoBuffer<type>::getChannelPreviousEnd(const OpenViBE::uint32 ui32ChannelIndex, const OpenViBE::uint32 ui32PreviousChunkSize) const
{
	return getChannelEnd(ui32ChannelIndex) - ui32PreviousChunkSize;
}

template <class type> void TFifoBuffer<type>::clean(void)
{
	delete m_pBuffer;
	
	m_pBuffer		= 0;
	m_pBegin		= 0;
	m_pEnd			= 0;
	m_pExtract		= 0;
	m_channelCount	= 0;
	m_channelSize	= 0;
}

template <class type> OpenViBE::uint32 TFifoBuffer<type>::getChannelRoom(void) const
{
	return m_channelSize - (m_pEnd - m_pBegin);
}

template <class type> OpenViBE::boolean TFifoBuffer<type>::increaseData(const OpenViBE::uint32 ui32NbSamples)
{
	if(!isValid())
		return false;
	
	if(getChannelRoom() >= ui32NbSamples)
		return true;

	OpenViBE::uint32	l_offBegin	= m_pBegin	- m_pBuffer;
	OpenViBE::uint32	l_offEnd	= m_pEnd	- m_pBuffer;

	TFifoBuffer<type>	l_oldSignals;
	if(l_offEnd)
	{	if(!l_oldSignals.build(m_channelCount, l_offEnd))
			return false;
		
		if(!copyData(l_oldSignals.getBegin(), l_offEnd, m_pBuffer, m_channelSize, m_channelCount, l_offEnd))
			return false;
	}
		
	if(!TFifoBuffer<type>::build(m_channelCount, m_channelSize + ui32NbSamples))
		return false;
	
	if(l_offEnd)
	{	if(!copyData(m_pBuffer, m_channelSize, l_oldSignals.getBuffer(), l_offEnd, m_channelCount, l_offEnd))
			return false;
	}
		
	m_pBegin	= m_pBuffer + l_offBegin;
	m_pEnd		= m_pBuffer + l_offEnd;

	return true;
}

template <class type> void TFifoBuffer<type>::increase(const OpenViBE::uint32 ui32NbSamples)
{
	OpenViBE::boolean less = m_pEnd < m_pExtract;

	m_pEnd	+= ui32NbSamples;

	if(less && (m_pEnd >= m_pExtract))
		m_overflow++;
}

template <class type> void TFifoBuffer<type>::decrease(const OpenViBE::uint32 ui32NbSamples)
{
	m_pEnd	-= ui32NbSamples;
}

template <class type> OpenViBE::boolean TFifoBuffer<type>::shiftLeft(const OpenViBE::uint32 uint32ShiftSize)
{
	OpenViBE::uint32	l_uint32NbSamples = (m_pEnd - m_pBuffer) - uint32ShiftSize;
	if(!copyData(m_pBuffer, m_channelSize, m_pEnd - l_uint32NbSamples, m_channelSize, m_channelCount, l_uint32NbSamples))
		return false;
				
	decrease(uint32ShiftSize);

	return true;
}

template <class type> OpenViBE::boolean TFifoBuffer<type>::canExtract(const OpenViBE::uint32 ui32ChannelSize)
{
	return getChannelSamples() >= ui32ChannelSize;
}

template <class type> type*	TFifoBuffer<type>::getExtractPtr(const OpenViBE::uint32 ui32ChannelIndex) const
{
	return getChannelBegin(ui32ChannelIndex);
}
		
template <class type> void TFifoBuffer<type>::adjusteExtract(const OpenViBE::uint32 /*ui32NbSamples*/)
{
}

template <class type> OpenViBE::boolean TFifoBuffer<type>::copyData(const OpenViBE::float32* pDest, const OpenViBE::uint32 uint32DestStride, const OpenViBE::float32* const pSrc, const OpenViBE::uint32 uint32SrcStride, const OpenViBE::uint32 ui32nbChannels, const OpenViBE::uint32 ui32nbSamples)
{
	if(!pSrc || !pDest)
		return false;

	if(ui32nbSamples)
	{	for(OpenViBE::uint32 iChan=0; iChan < ui32nbChannels; iChan++)
			memcpy((void*) (pDest + iChan*uint32DestStride), pSrc + iChan*uint32SrcStride, ui32nbSamples*sizeof(OpenViBE::float32));
	}

	return true;
}

template <class type> std::string TFifoBuffer<type>::dump()
{
	std::ostringstream	l_oss;
	
	l_oss	<< "; channelCount = "		<< m_channelCount
			<< "; channelSize = "		<< m_channelSize
			<< "; channelSamples = "	<< getChannelSamples()
			<< "; appendRoom = "		<< m_channelSize - (m_pEnd - m_pBegin)
			<< "; extractRoom = "		<< m_channelSize - (m_pExtract - m_pBegin)
			<< "; overflow = "			<< m_overflow
			;

	return l_oss.str();
}
