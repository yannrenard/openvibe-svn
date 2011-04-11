#ifndef __OpenViBEPlugins_BoxAlgorithm_Synchro_H__
#define __OpenViBEPlugins_BoxAlgorithm_Synchro_H__

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

			virtual OpenViBE::boolean initialize(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm>* pTBoxAlgorithm);
			virtual OpenViBE::boolean uninitialize();
			virtual OpenViBE::boolean process(const uint32 ui32Channel);

		protected:

			// parent memory
			OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm>*     m_pTBoxAlgorithm;
			// signal section
			OpenViBE::Kernel::IAlgorithmProxy*                                      m_pStreamDecoderSignal;
			
			OpenViBE::Kernel::TParameterHandler < const OpenViBE::IMemoryBuffer* >	ip_pMemoryBufferSignal;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* >				op_pMatrixSignal;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::uint64 >				op_ui64SamplingRateSignal;
			
			
			// stimulation section
			OpenViBE::Kernel::IAlgorithmProxy*                                     m_pStreamDecoderStimulation;

			OpenViBE::Kernel::TParameterHandler < const OpenViBE::IMemoryBuffer* >	ip_pMemoryBufferStimulation;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IStimulationSet* >		op_pStimulationSetStimulation;
		};
	};
};

#endif // __OpenViBEPlugins_BoxAlgorithm_Synchro_H__
