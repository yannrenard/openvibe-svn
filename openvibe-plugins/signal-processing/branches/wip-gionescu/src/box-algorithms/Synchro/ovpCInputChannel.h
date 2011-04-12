#ifndef __OpenViBEPlugins_InputChannel_H__
#define __OpenViBEPlugins_InputChannel_H__

#include "../../ovp_defines.h"
#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>

namespace OpenViBEPlugins
{
	namespace SignalProcessing
	{
		class CInputChannel
		{
		public:

			~CInputChannel();
			OpenViBE::boolean initialize(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm>* pTBoxAlgorithm, const OpenViBE::uint32 ui32Channel);
			OpenViBE::boolean uninitialize();
			OpenViBE::boolean isInitialized() { return m_bInitialized; }
			OpenViBE::boolean getStimulationStart();
			void flushInputStimulation();
			void flushLateSignal();
			OpenViBE::boolean hasProcessedHeader() {return m_bHeaderProcessed;}
			OpenViBE::boolean getHeaderParams();
			OpenViBE::CMatrix* getMatrixPtr() {return m_oMatrixBuffer;}
			OpenViBE::uint64 getSamplingRate() {return op_ui64SamplingRateSignal;}
			OpenViBE::uint64 getTimeStampStart() {return m_ui64InputChunkStartTime;}
			OpenViBE::uint64 getTimeStampEnd() {return m_ui64InputChunkEndTime;}

		protected:

			OpenViBE::uint64                                                        m_ui64StartStimulation;
			OpenViBE::boolean                                                       m_bInitialized;
			OpenViBE::uint64                                                        m_ui64TimeStimulationPosition;
			OpenViBE::uint64                                                        m_ui64TimeStampStart;

			OpenViBE::boolean                                                       m_bHeaderProcessed;
			OpenViBE::CMatrix*                                                      m_oMatrixBuffer;
			OpenViBE::uint64                                                        m_ui64InputChunkStartTime;
			OpenViBE::uint64                                                        m_ui64InputChunkEndTime;

			// parent memory
			OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm>*     m_pTBoxAlgorithm;
			OpenViBE::uint32														m_ui32SignalChannel;
			OpenViBE::uint32														m_ui32StimulationChannel;
			
			// signal section
			OpenViBE::Kernel::IAlgorithmProxy*                                      m_pStreamDecoderSignal;
			
			OpenViBE::Kernel::TParameterHandler < const OpenViBE::IMemoryBuffer* >	ip_pMemoryBufferSignal;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* >				op_pMatrixSignal;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::uint64 >				op_ui64SamplingRateSignal;
			
			
			// stimulation section
			OpenViBE::Kernel::IAlgorithmProxy*                                      m_pStreamDecoderStimulation;

			OpenViBE::Kernel::TParameterHandler < const OpenViBE::IMemoryBuffer* >	ip_pMemoryBufferStimulation;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IStimulationSet* >		op_pStimulationSetStimulation;
		};
	};
};

#endif // __OpenViBEPlugins_InputChannel_H__
