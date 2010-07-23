#ifndef __OpenViBEPlugins_SynchroEngine_H__
#define __OpenViBEPlugins_SynchroEngine_H__

#include "ovpCSynchroBuffer.h"

namespace OpenViBEPlugins
{
	namespace SignalProcessing
	{
		class CSynchroEngine
		{
			
		public:
			CSynchroEngine();

			void				Build(const OpenViBE::uint32 group, const OpenViBE::uint32 samplingRate, const OpenViBE::uint32 otherSamplingRate, const OpenViBE::uint32 dimChannels, const OpenViBE::uint32 dimSamples, const OpenViBE::uint32 durationBuffer, const OpenViBE::uint32 interpolationMode = CSynchroBuffer::INTERPOLATION_LINEAR); 
			void				Push(const OpenViBE::uint32 group, OpenViBE::IMatrix& data);
			OpenViBE::boolean	IsInitialized() const							{	return m_SynchroBuffer1.IsInitialized() &&  m_SynchroBuffer2.IsInitialized();			}
			OpenViBE::uint32	NbChannels(const OpenViBE::uint32 group)		{	return group ?  m_SynchroBuffer2.dimChannels() : m_SynchroBuffer1.dimChannels();			}
			OpenViBE::uint32	NbChunkSamples(const OpenViBE::uint32 group)	{	return group ?  m_SynchroBuffer2.NbChunkSamples() : m_SynchroBuffer1.NbChunkSamples();	}
			OpenViBE::boolean	IsDetected(const OpenViBE::uint32 group) const	{	return group ?  m_SynchroBuffer2.IsDetected() : m_SynchroBuffer1.IsDetected();			}
			bool				GetResult(OpenViBE::IMatrix& result);

		private:
			OpenViBE::boolean	HasSynchro(OpenViBE::uint32& dim1, OpenViBE::uint32& dim2) const
								{	return m_SynchroBuffer1.HasSynchro(dim1) &&  m_SynchroBuffer2.HasSynchro(dim2);	}

		private:
			OpenViBE::boolean	m_inTransfer;
			OpenViBE::uint32	m_nbChunks;
			OpenViBE::uint32	m_chunkIndex;
			CSynchroBuffer		m_SynchroBuffer1;
			CSynchroBuffer		m_SynchroBuffer2;
		};

	};
};

#endif // __OpenViBEPlugins_SynchroBuffer_H__
