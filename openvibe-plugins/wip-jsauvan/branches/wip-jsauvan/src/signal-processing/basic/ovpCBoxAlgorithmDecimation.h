#ifndef __OpenViBEPlugins_SignalProcessing_BoxAlgorithms_Basic_Decimation_H__
#define __OpenViBEPlugins_SignalProcessing_BoxAlgorithms_Basic_Decimation_H__

#include "../../ovp_defines.h"

#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>
#include <ovp_global_defines.h>

namespace OpenViBEPlugins
{
	namespace SignalProcessing
	{
		class CBoxAlgorithmDecimation : public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{
		public:

			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);

			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);
			virtual OpenViBE::boolean process(void);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>, OVP_ClassId_BoxAlgorithm_Decimation)

		protected:

			OpenViBE::Kernel::IAlgorithmProxy* m_pStreamDecoder;
                        OpenViBE::Kernel::TParameterHandler < const OpenViBE::IMemoryBuffer* > m_oInputMemoryBufferHandle;

			OpenViBE::Kernel::IAlgorithmProxy* m_pStreamEncoder;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMemoryBuffer* > m_oOutputMemoryBufferHandle;


			OpenViBE::uint64 m_ui64LastStartTime;

		protected:
			OpenViBE::uint64 m_ui64DecimationFactor;

		};

		class CBoxAlgorithmDecimationDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }
			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Decimation"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Jean-Baptiste Sauvan"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INRIA/IRISA"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Simple decimation of the signal"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("Simple decimation of the signal without filtering"); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Signal processing/Basic"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString(""); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_Decimation; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::SignalProcessing::CBoxAlgorithmDecimation(); }

			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rPrototype) const
			{
				rPrototype.addInput  ("Input signal",    OV_TypeId_Signal);

				rPrototype.addOutput ("Decimated signal", OV_TypeId_Signal);

				rPrototype.addSetting("Decimation factor", OV_TypeId_Integer, "8");

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_DecimationDesc)
		};
	};
};

#endif // __OpenViBEPlugins_SignalProcessing_BoxAlgorithms_Basic_Decimation_H__
