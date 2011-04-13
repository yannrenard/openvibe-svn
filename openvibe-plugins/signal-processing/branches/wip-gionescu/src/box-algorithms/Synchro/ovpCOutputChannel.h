#ifndef __OpenViBEPlugins_OutputChannel_H__
#define __OpenViBEPlugins_OutputChannel_H__

#include "../../ovp_defines.h"
#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>

namespace OpenViBEPlugins
{
	namespace SignalProcessing
	{
		class COutputChannel
		{
		public:

			OpenViBE::boolean initialize(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm>* pTBoxAlgorithm);
			OpenViBE::boolean uninitialize();
			
			void setStimulationReference(OpenViBE::uint64 stimRef) {m_ui64TimeStimulationPosition=stimRef;}
			void sendStimulation(OpenViBE::IStimulationSet* stimset, OpenViBE::uint64 start, OpenViBE::uint64 end);

			OpenViBE::boolean hasProcessedHeader() {return m_bHeaderProcessed;}
			void setMatrixPtr(OpenViBE::CMatrix* pMatrix);
			void setSamplingRate(OpenViBE::uint64 freq) {m_ui64SamplingRate = freq;}
			void setChunkTimeStamps(OpenViBE::uint64 start, OpenViBE::uint64 end) {m_ui64InputChunkStartTime = start; m_ui64InputChunkEndTime=end;}
			void sendHeader();
			void sendSignalChunk();

		protected:

			OpenViBE::boolean                                                       m_bHeaderProcessed;
			OpenViBE::CMatrix*                                                      m_oMatrixBuffer;
			OpenViBE::uint64                                                        m_ui64SamplingRate;
			OpenViBE::uint64                                                        m_ui64InputChunkStartTime;
			OpenViBE::uint64                                                        m_ui64InputChunkEndTime;

			OpenViBE::uint64                                                        m_ui64TimeStimulationPosition;

			// parent memory
			OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm>*     m_pTBoxAlgorithm;
			OpenViBE::uint32														m_ui32SignalChannel;
			OpenViBE::uint32														m_ui32StimulationChannel;
			
			// signal section
			OpenViBE::Kernel::IAlgorithmProxy*                                      m_pStreamEncoderSignal;
			
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMemoryBuffer* >	    op_pMemoryBufferSignal;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* >				ip_pMatrixSignal;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::uint64 >				ip_ui64SamplingRateSignal;			
			
			// stimulation section
			OpenViBE::Kernel::IAlgorithmProxy*                                      m_pStreamEncoderStimulation;

			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMemoryBuffer* >		op_pMemoryBufferStimulation;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IStimulationSet* >		ip_pStimulationSetStimulation;
		};
	};
};

#endif // __OpenViBEPlugins_OutputChannel_H__
