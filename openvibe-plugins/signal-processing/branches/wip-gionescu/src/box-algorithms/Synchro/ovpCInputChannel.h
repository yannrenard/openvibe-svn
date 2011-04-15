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
			OpenViBE::boolean initialize(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm>* pTBoxAlgorithm);
			OpenViBE::boolean uninitialize();

			OpenViBE::boolean isInitialized() { return m_bInitialized; }
			OpenViBE::boolean getStimulationStart();
			OpenViBE::boolean getStimulation();
			void flushInputStimulation();
			void flushLateSignal();
			OpenViBE::uint64 getStimulationReference() {return m_ui64TimeStimulationPosition;}
			OpenViBE::IStimulationSet* getStimulationSet() {return m_oIStimulationSet;}
			OpenViBE::uint64 getTimeStampStartStimulation() {return m_ui64TimeStampStartStimulation;}
			OpenViBE::uint64 getTimeStampEndStimulation() {return m_ui64TimeStampEndStimulation;}

			OpenViBE::boolean hasProcessedHeader() {return m_bHeaderProcessed;}
			OpenViBE::boolean getHeaderParams();
			OpenViBE::CMatrix* getMatrixPtr();
			void switchMatrixPtr() {m_ui64PtrMatrixIndex++;}
			OpenViBE::uint64 getSamplingRate() {return op_ui64SamplingRateSignal;}
			OpenViBE::uint64 getTimeStampStart() {return m_ui64TimeStampStartSignal;}
			OpenViBE::uint64 getTimeStampEnd() {return m_ui64TimeStampEndSignal;}

			OpenViBE::boolean hasSignalChunk();
			OpenViBE::boolean fillData();
			OpenViBE::boolean calculateSampleOffset();
			void copyData(const OpenViBE::boolean copyFirstBlock, OpenViBE::uint64 matrixIndex);

		public:
			OpenViBE::uint32                                                        m_ui32LoopStimulationChunkIndex;
			OpenViBE::uint32                                                        m_ui32LoopSignalChunkIndex;
		protected:

			OpenViBE::uint64                                                        m_ui64StartStimulation;
			OpenViBE::boolean                                                       m_bInitialized;
			OpenViBE::uint64                                                        m_ui64TimeStimulationPosition;
			OpenViBE::uint64                                                        m_ui64TimeStampStartStimulation;
			OpenViBE::uint64                                                        m_ui64TimeStampEndStimulation;

			OpenViBE::boolean                                                       m_bHeaderProcessed;
			OpenViBE::CMatrix*                                                      m_oMatrixBuffer[2];
			OpenViBE::uint64                                                        m_ui64PtrMatrixIndex;
			OpenViBE::uint64                                                        m_ui64FirstBlock;
			OpenViBE::uint64                                                        m_ui64SecondBlock;
			OpenViBE::uint64                                                        m_ui64NbSamples;
			OpenViBE::uint64                                                        m_ui64NbChannels;
			OpenViBE::boolean                                                       m_bFirstChunk;
			OpenViBE::uint64                                                        m_ui64TimeStampStartSignal;
			OpenViBE::uint64                                                        m_ui64TimeStampEndSignal;

			OpenViBE::IStimulationSet*                                              m_oIStimulationSet;

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
