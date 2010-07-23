#include "ovpCSynchroEngine.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;

CSynchroEngine::CSynchroEngine()
	: m_SynchroBuffer1(0)
	, m_SynchroBuffer2(1)
	, m_inTransfer(false)
{
}

void CSynchroEngine::Build(const OpenViBE::uint32 group, const OpenViBE::uint32 samplingRate, const OpenViBE::uint32 otherSamplingRate, const OpenViBE::uint32 dimChannels, const OpenViBE::uint32 dimSamples, const OpenViBE::uint32 durationBuffer, const OpenViBE::uint32 interpolationMode /*= CSynchroBuffer::INTERPOLATION_LINEAR*/)
{
	if(group == 0)
		m_SynchroBuffer1.Build(samplingRate, otherSamplingRate, dimChannels, dimSamples, durationBuffer, interpolationMode);
	else
		m_SynchroBuffer2.Build(samplingRate, otherSamplingRate, dimChannels, dimSamples, durationBuffer, interpolationMode);
}

void CSynchroEngine::Push(const OpenViBE::uint32 group, OpenViBE::IMatrix& data)
{
	if(group == 0)
		m_SynchroBuffer1.Push(data);
	else
		m_SynchroBuffer2.Push(data);
}

bool CSynchroEngine::GetResult(OpenViBE::IMatrix& result)
{
	OpenViBE::uint32 outputChunkSize = result.getDimensionSize(1);

	if(!m_inTransfer)
	{	OpenViBE::uint32 nbSamples1, nbSamples2, nbSamples11, nbSamples22;
		if(!HasSynchro(nbSamples1, nbSamples2))
			return false;

		if(m_SynchroBuffer1.IsSlower())
		{	nbSamples11		= m_SynchroBuffer1.PrepareTransfer(nbSamples2);
			nbSamples22		= m_SynchroBuffer2.PrepareTransfer(nbSamples2);
		}
		else if(m_SynchroBuffer2.IsSlower())
		{	nbSamples11		= m_SynchroBuffer1.PrepareTransfer(nbSamples1);
			nbSamples22		= m_SynchroBuffer2.PrepareTransfer(nbSamples1);
		}
		else
		{	if(nbSamples2 < nbSamples1)
				nbSamples1	= nbSamples2;
		
			nbSamples11		= m_SynchroBuffer1.PrepareTransfer(nbSamples1);
			nbSamples22		= m_SynchroBuffer2.PrepareTransfer(nbSamples1);
		}
			
		m_nbChunks		= nbSamples11/outputChunkSize;
		m_chunkIndex	= 0;
		m_inTransfer	= true;
	}

	if(m_chunkIndex < m_nbChunks)
	{	m_SynchroBuffer1.GetResult(result, 0);
		m_SynchroBuffer2.GetResult(result, m_SynchroBuffer1.dimChannels());
		m_chunkIndex++;
	}
	else		
	{	m_SynchroBuffer1.SetAsDeprecated();
		m_SynchroBuffer2.SetAsDeprecated();
		m_inTransfer = false;
	}

	return m_inTransfer;
}



