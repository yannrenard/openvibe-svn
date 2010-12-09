#ifndef __OpenViBEPlugins_BoxAlgorithm_OnlineStatistics_H__
#define __OpenViBEPlugins_BoxAlgorithm_OnlineStatistics_H__

#include "../../ovp_defines.h"
#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>
#include <vector>

#define OVP_ClassId_BoxAlgorithm_OnlineStatistics OpenViBE::CIdentifier(0x6458395C, 0x30175384)
#define OVP_ClassId_BoxAlgorithm_OnlineStatisticsDesc OpenViBE::CIdentifier(0x20B94D87, 0x71A90691)

// The online statistics box computes in real time the mean and variance of each incoming chunk.
// Use epoching boxes to compute these statistics on relevant signal windows.

namespace OpenViBEPlugins
{
	namespace SignalProcessing
	{
		class CBoxAlgorithmOnlineStatistics : public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
		{
		public:

			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);
			virtual OpenViBE::boolean process(void);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_OnlineStatistics);

		protected:

			//the input signal decoder
			OpenViBE::Kernel::IAlgorithmProxy* m_pSignalDecoder;
			OpenViBE::Kernel::TParameterHandler < const OpenViBE::IMemoryBuffer* > ip_pMemoryBufferToDecode;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::uint64 > op_ui64SamplingRate;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* > op_pDecodedMatrix;

			//The mean signal encoder
			OpenViBE::Kernel::IAlgorithmProxy* m_pMeanSignalEncoder;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::uint64 > ip_ui64MeanSamplingRate;  // directly built in the box
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* > ip_pMeanMatrixToEncode; // directly built in the box
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMemoryBuffer* > op_pMeanEncodedMemoryBuffer;

			//The variance signal encoder
			OpenViBE::Kernel::IAlgorithmProxy* m_pVarianceSignalEncoder;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::uint64 > ip_ui64VarianceSamplingRate;  // directly built in the box
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* > ip_pVarianceMatrixToEncode; // directly built in the box
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMemoryBuffer* > op_pVarianceEncodedMemoryBuffer;

		};

		class CBoxAlgorithmOnlineStatisticsDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Online statistics"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Laurent Bonnet"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INRIA"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Computes the mean and variance of each incoming signal chunk."); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("The statistics are computed for each channel, in real time.\n Use epoching boxes (time-based or event-based) to compute these statistics on relevant signal windows. "); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Signal processing/Statistics"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("0.1"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_OnlineStatistics; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::SignalProcessing::CBoxAlgorithmOnlineStatistics; }

			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
				rBoxAlgorithmPrototype.addInput("Input signal", OV_TypeId_Signal);
				rBoxAlgorithmPrototype.addOutput("Output mean", OV_TypeId_Signal);
				rBoxAlgorithmPrototype.addOutput("Output variance", OV_TypeId_Signal);
				rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_IsUnstable);
				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_OnlineStatisticsDesc);
		};
	};
};

#endif // __OpenViBEPlugins_BoxAlgorithm_OnlineStatistics_H__
