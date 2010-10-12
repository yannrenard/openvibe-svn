#include <sstream>

#include "ovasCAcqServerCircularBuffer.h"

using namespace OpenViBEAcquisitionServer;

CAcqServerCircularBuffer::CAcqServerCircularBuffer(void)
	: m_pBegin(0)
	, m_overflow(0)
{
}
		
CAcqServerCircularBuffer::CAcqServerCircularBuffer(const OpenViBE::uint32 uint32ChannelCount, const OpenViBE::uint32 uint32AppendSize, const OpenViBE::uint32 uint32EextractSize, const OpenViBE::uint32 uint32Multiplier /*= 8*/)
	: m_pBegin(0)
	, m_overflow(0)
{
	build(uint32ChannelCount, uint32AppendSize, uint32EextractSize, uint32Multiplier);
}

CAcqServerCircularBuffer::~CAcqServerCircularBuffer()
{
	clean();
}

OpenViBE::boolean CAcqServerCircularBuffer::isValid() const
{
	return m_pBegin != 0;
}

OpenViBE::boolean CAcqServerCircularBuffer::build(const OpenViBE::uint32 uint32ChannelCount, const OpenViBE::uint32 uint32AppendSize, const OpenViBE::uint32 uint32EextractSize, const OpenViBE::uint32 uint32Multiplier /*= 8*/)
{
	clean();

	OpenViBE::uint32 l_size	= uint32Multiplier*(uint32ChannelCount + 1)*uint32AppendSize*uint32EextractSize;
	m_pBegin				= new OpenViBE::float32[l_size];

	if(isValid())
	{	m_pEnd				= m_pBegin + l_size;

		m_channelCount		= uint32ChannelCount;

		m_pAppend			= m_pBegin;
		m_appendSize		= uint32AppendSize;
		m_appendBufferSize	= (m_channelCount + 1)*m_appendSize;

		m_pExtract			= m_pBegin;
		m_extractSize		= uint32EextractSize;
		m_extractBufferSize	= (m_channelCount + 1)*m_extractSize;
	}

	return isValid();
}

OpenViBE::float32* CAcqServerCircularBuffer::getBuffer() const
{
	return m_pBegin;
}

OpenViBE::boolean CAcqServerCircularBuffer::canExtract(void) const
{
	return (m_pExtract > m_pAppend) || ((m_pExtract + m_extractBufferSize) <= m_pAppend);
}

OpenViBE::boolean CAcqServerCircularBuffer::extract(OpenViBE::float32* pfloat32DestBuffer, std::vector<CStimulation>& vStimulations)
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

void CAcqServerCircularBuffer::amplifyData(const OpenViBE::uint32 uint32ChannelIndex, const OpenViBE::float32 float32Gain)
{
	OpenViBE::float32*	l_pChannel  = m_pExtract + uint32ChannelIndex*m_extractSize;

	OpenViBE::uint32	l_extractSize = m_extractSize;
	while(l_extractSize--)
		*l_pChannel++	*= float32Gain;
}

std::string CAcqServerCircularBuffer::dump() const
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

void CAcqServerCircularBuffer::clean()
{
	delete m_pBegin;

	m_pBegin	= 0;
}

OpenViBE::boolean CAcqServerCircularBuffer::increaseSize()
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
	
	::memcpy(l_pBegin,							m_pBegin,	getSizeof(l_toAppend));
	::memcpy(l_pBegin + l_size - l_toExtract,	m_pExtract, getSizeof(l_toExtract));

	clean();

	m_pBegin	= l_pBegin;
	m_pEnd		= m_pBegin	+ l_size;
	m_pAppend	= m_pBegin	+ l_toAppend;
	m_pExtract	= m_pEnd	- l_toExtract;

	m_overflow++;
	
	return isValid();
}

OpenViBE::uint32 CAcqServerCircularBuffer::getSizeof(const OpenViBE::uint32 uint32NbElements) const
{
	return uint32NbElements*sizeof(OpenViBE::float32);
}
