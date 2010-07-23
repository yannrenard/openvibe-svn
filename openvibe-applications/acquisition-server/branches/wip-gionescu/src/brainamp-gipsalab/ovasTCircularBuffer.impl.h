#include "ovasTFifoBuffer.impl.h"

using namespace OpenViBEAcquisitionServer;

// public zone

template <class type> TCircularBuffer<type>::TCircularBuffer(void)
	: TFifoBuffer<type>()
{
}
		
template <class type> TCircularBuffer<type>::TCircularBuffer(const OpenViBE::uint32 ui32ChannelCount, const OpenViBE::uint32 ui32ChannelSize)
	: TFifoBuffer<type>(ui32ChannelCount, m_channelSize)
{
	build(ui32ChannelCount, ui32ChannelSize);
}
		
template <class type> TCircularBuffer<type>::~TCircularBuffer(void)
{	
}

// protected zone

template <class type> type* TCircularBuffer<type>::getChannelPreviousEnd(const OpenViBE::uint32 ui32ChannelIndex, const OpenViBE::uint32 ui32PreviousChunkSize) const
{
	return (m_pEnd == m_pBegin) ? getChannelBegin(ui32ChannelIndex) + (m_channelSize - ui32PreviousChunkSize) : TFifoBuffer<type>::getChannelPreviousEnd(ui32ChannelIndex, ui32PreviousChunkSize);
}

template <class type> OpenViBE::boolean TCircularBuffer<type>::increaseData(const OpenViBE::uint32 ui32NbSamples)
{
	return true;
}

template <class type> void TCircularBuffer<type>::increase(const OpenViBE::uint32 ui32NbSamples)
{
	TFifoBuffer<type>::increase(ui32NbSamples);
	
	if(getChannelRoom() == 0)
		m_pEnd	= m_pBegin;
}

template <class type> OpenViBE::boolean TCircularBuffer<type>::shiftLeft(const OpenViBE::uint32 /*ui32NbSamples*/)
{
	return true;
}

template <class type> OpenViBE::boolean TCircularBuffer<type>::canExtract(const OpenViBE::uint32 ui32ChannelSize)
{
	if(m_pExtract > m_pEnd)
		return true;

	return (m_pExtract + ui32ChannelSize) <= m_pEnd;
}

template <class type> OpenViBE::uint32 TCircularBuffer<type>::getChannelSamples(void) const
{
	return (m_pEnd > m_pExtract) ?
		m_pEnd - m_pExtract :
		m_channelSize - (m_pExtract - m_pEnd);
}

template <class type> type*	TCircularBuffer<type>::getExtractPtr(const OpenViBE::uint32 ui32ChannelIndex) const
{
	return getChannelBegin(ui32ChannelIndex) + (m_pExtract - m_pBegin);
}

template <class type> void TCircularBuffer<type>::adjusteExtract(const OpenViBE::uint32 ui32NbSamples)
{
	m_pExtract	+= ui32NbSamples;
	
	if(m_pExtract >= (m_pBegin + m_channelSize))
		m_pExtract	= m_pBegin;
}

