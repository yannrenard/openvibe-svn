#include "ovpCSynchroBuffer.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;

#ifndef NDEBUG
	std::ofstream		theEngineDebug("c:/tmp/CSynchroEngine.txt");
#endif


CMatrix2d::CMatrix2d()
{
}

CMatrix2d::CMatrix2d(const OpenViBE::uint32 dimChannels, const OpenViBE::uint32 dimSamples)
{
	build(dimChannels, dimSamples);
}

void CMatrix2d::build(const OpenViBE::uint32 dimChannels, const OpenViBE::uint32 dimSamples)
{
	setDimensionCount(2);
	setDimensionSize(0, dimChannels);
	setDimensionSize(1, dimSamples);
	
	m_beginPtr	= getBuffer();
	m_endPtr	= m_beginPtr;
	
	::memset(getBuffer(), 0, size_t(dimChannels*dimSamples*sizeof(OpenViBE::float64)));
}

void CMatrix2d::copy(OpenViBE::float64*	dest, OpenViBE::float64* src, const OpenViBE::uint32 srcStride, const OpenViBE::uint32 nbSample)
{
	for(OpenViBE::uint32 iChan=0; iChan < dimChannels(); iChan++)
		memcpy(dest + iChan*dimSamples(), src + iChan*srcStride, nbSample*sizeof(OpenViBE::float64));
}

void CMatrix2d::extract(OpenViBE::float64* dest, const OpenViBE::uint32 destStride, const OpenViBE::uint32 nbSample)
{
	for(OpenViBE::uint32 iChan=0; iChan < dimChannels(); iChan++)
		memcpy(dest + iChan*destStride, begin() + iChan*dimSamples(), nbSample*sizeof(OpenViBE::float64));
	
	shiftLeft(nbSample);
}

void CMatrix2d::shiftLeft(const OpenViBE::uint32 shift)
{
	OpenViBE::uint32	nbSample = (m_endPtr - getBuffer()) - shift;
	copy(getBuffer(), m_endPtr - nbSample, dimSamples(), nbSample);
				
	decrease(shift);

	DumpM("Shift    ");
}

void CMatrix2d::shiftLeft(const OpenViBE::uint32 shift, const OpenViBE::uint32 decreaseVal)
{
	OpenViBE::uint32	nbSample = (m_endPtr - m_beginPtr) - shift;
	copy(getBuffer(), m_endPtr - nbSample, dimSamples(), nbSample);
				
	decrease(decreaseVal);

	DumpM("Shift    ");
}

void CMatrix2d::Dump(std::ofstream& ofs, const std::string& tag)
{
	if(ofs.is_open())
		ofs	<< Dump(tag) << std::endl << std::flush;
}

std::string CMatrix2d::Dump(const std::string& tag)
{
	std::ostringstream	oss;

	oss	<< tag << ' '\
		<< std::setw(4) << int(m_endPtr - m_beginPtr)			<< ' '
		<< *m_beginPtr											<< ' '
		<< *(m_endPtr - 1)										<< ' ';

	std::string debug = oss.str();

	return debug;
}

CSynchroBuffer::CSynchroBuffer(const OpenViBE::uint32 groupId, const OpenViBE::uint32 offset /*= 5*/)
	: m_offset(offset)
	, m_groupId(groupId)
	, m_samplingRate(500)
	, m_otherSamplingRate(500)
	, m_interpolationMode(INTERPOLATION_LINEAR)
	, m_initialized(false)
	, m_inSynchro(false)
	, m_detected(false)
{
}

void CSynchroBuffer::Build(const OpenViBE::uint32 samplingRate, const OpenViBE::uint32 otherSamplingRate, const OpenViBE::uint32 nbChannel, const OpenViBE::uint32 nbChunkSamples, const OpenViBE::uint32 durationBuffer, const OpenViBE::uint32 interpolationMode /*= INTERPOLATION_LINEAR*/)
{
	m_initialized			= true;

	CMatrix2d::build(nbChannel, OpenViBE::uint32(samplingRate*durationBuffer));

	m_samplingRate			= samplingRate;
	m_otherSamplingRate		= otherSamplingRate;
	m_interpolationMode		= interpolation_type(interpolationMode);

	m_nbChankSamples		= nbChunkSamples;

	m_beginPtr				= getBuffer() + m_offset;
	m_endPtr				= m_beginPtr;
	m_limitPtr				= getBuffer() + dimSamples() - m_nbChankSamples;
	
	m_beginSynchroPtr		= m_beginPtr + (dimChannels() - 1)*dimSamples();
	m_endSynchroPtr			= m_beginSynchroPtr;
	m_synchroPtr			= m_beginSynchroPtr;

	m_transferBuffer.build(dimChannels(), IsSlower() ? 3*m_otherSamplingRate : dimSamples());
}

void CSynchroBuffer::Push(OpenViBE::IMatrix& data)
{
	append(data);
	
	m_endSynchroPtr	+= data.getDimensionSize(1);
	
	FindEdge();
}

void CSynchroBuffer::FindEdge()
{
	if(m_endPtr >= m_limitPtr)
	{	DumpM("Exit     ");
		exit(-10);
	}

	if(m_detected)
	{	DumpM("Wait     ");
		return;
	}

	OpenViBE::float64*	pSynchroOld	= m_inSynchro ? m_beginSynchroPtr : m_beginSynchroPtr - 1;
	OpenViBE::float64*	pSynchro	= pSynchroOld + 1;
	OpenViBE::boolean	found		= false;
	
	while(pSynchro < m_endSynchroPtr)
	{	if((*pSynchroOld == 0) && (*pSynchro != 0))
		{	found	= true;
			break;
		}

		pSynchroOld++;
		pSynchro++;
	}

	if(found)
	{	if(!m_inSynchro)
		{	shiftLeft(pSynchro - m_beginSynchroPtr);
			
			m_inSynchro		= true;			
		}
		else if(uint32(m_endSynchroPtr - pSynchro) > m_offset)
		{	m_detected		= true;
			m_synchroPtr	= pSynchro;
			DumpM("FindEdge ");
	}	}
	else if(!m_inSynchro)
	{	m_endPtr			= m_beginPtr;
		m_endSynchroPtr		= m_beginSynchroPtr;
	}
}

OpenViBE::uint32 CSynchroBuffer::PrepareTransfer(const OpenViBE::uint32 nbSample)
{
	if(IsSlower())
		Interpolate(nbSample);
	else
		m_transferBuffer.append(m_beginPtr, dimSamples(), nbSample);

	return m_transferBuffer.nbSamples();
}

void CSynchroBuffer::SetAsDeprecated()
{
	OpenViBE::uint32 shift			= m_synchroPtr - m_beginSynchroPtr;

	shiftLeft(shift);

	m_endSynchroPtr	-= shift;
	m_synchroPtr	 = m_beginSynchroPtr;
	m_detected		 = false;
}

void CSynchroBuffer::GetResult(OpenViBE::IMatrix& result, const OpenViBE::uint32 offChannels)
{
	m_transferBuffer.extract(result, offChannels);
}

void CSynchroBuffer::Interpolate(const OpenViBE::uint32 nbSample)
{
	OpenViBE::float64	step	= OpenViBE::float64(m_samplingRate)/m_otherSamplingRate;
	
	switch(m_interpolationMode)
	{	case INTERPOLATION_LINEAR:
			for(OpenViBE::uint32 iChan=0; iChan < dimChannels(); iChan++)
			{	OpenViBE::float64	pos		= 0;
				OpenViBE::float64*	pDest	= m_transferBuffer.end() + iChan*m_transferBuffer.dimSamples();
				OpenViBE::float64*	pSrc	= m_beginPtr + iChan*dimSamples();
				for(OpenViBE::uint32 ii=0; ii < nbSample; ii++, pos+=step, pDest++)
				{	OpenViBE::uint32	offset	= OpenViBE::uint32(pos);
					OpenViBE::float64	k		= pos - offset;
					*pDest						= (1 - k) * *(pSrc + offset) + k * *(pSrc + offset + 1);
			}	}
			break;
	}
	
	m_transferBuffer.increase(nbSample);		
}

void CSynchroBuffer::Dump(std::ofstream& ofs, const std::string& tag)
{
	if(ofs.is_open())
		ofs	<< Dump(tag) << std::endl << std::flush;
}

std::string CSynchroBuffer::Dump(const std::string& tag)
{
	std::ostringstream	oss;

	oss	<< m_groupId << ' ' << tag << ' '\
		<< std::setw(4) << int(m_endPtr			- m_beginPtr)			<< ' '
		<< std::setw(4) << int(m_endSynchroPtr	- m_beginSynchroPtr)	<< ' '
		<< std::setw(4) << int(m_synchroPtr		- m_beginSynchroPtr)	<< ' '
		<< int(*(m_beginSynchroPtr-1))	<<  int(*m_beginSynchroPtr)		<< ' '
		<< int(*(m_synchroPtr-1))		<<  int(*m_synchroPtr)			<< ' '
		<< *m_beginPtr													<< ' '
		<< *(m_endPtr - 1)												<< ' ';

	std::string debug = oss.str();

	return debug;
}