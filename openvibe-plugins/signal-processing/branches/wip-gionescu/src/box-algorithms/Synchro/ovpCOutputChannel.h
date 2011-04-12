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

			OpenViBE::boolean initialize(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm>* pTBoxAlgorithm, const OpenViBE::uint32 ui32Channel);
			OpenViBE::boolean uninitialize();
			OpenViBE::boolean hasProcessedHeader() {return m_bHeaderProcessed;}
			void setMatrixPtr(OpenViBE::CMatrix* pMatrix);
			void setSamplingRate(OpenViBE::uint64 freq) {m_ui64SamplingRate = freq;}
			void setChunkTimeStamps(OpenViBE::uint64 start, OpenViBE::uint64 end) {m_ui64InputChunkStartTime = start; m_ui64InputChunkEndTime=end;}
			void sendHeader();

		protected:

			OpenViBE::boolean                                                       m_bHeaderProcessed;
			OpenViBE::CMatrix*                                                      m_oMatrixBuffer;
			OpenViBE::uint64                                                        m_ui64SamplingRate;
			OpenViBE::uint64                                                        m_ui64InputChunkStartTime;
			OpenViBE::uint64                                                        m_ui64InputChunkEndTime;

			// parent memory
			OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm>*     m_pTBoxAlgorithm;
			OpenViBE::uint32														m_ui32Channel;
			
			// signal section
			OpenViBE::Kernel::IAlgorithmProxy*                                      m_pStreamEncoderSignal;
			
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMemoryBuffer* >	    op_pMemoryBufferSignal;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* >				ip_pMatrixSignal;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::uint64 >				ip_ui64SamplingRateSignal;			
		};
	};
};

#endif // __OpenViBEPlugins_OutputChannel_H__
