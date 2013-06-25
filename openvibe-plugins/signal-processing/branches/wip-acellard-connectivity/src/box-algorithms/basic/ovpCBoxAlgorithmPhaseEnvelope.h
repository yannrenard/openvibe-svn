#ifndef __OpenViBEPlugins_BoxAlgorithm_PhaseEnvelope_H__
#define __OpenViBEPlugins_BoxAlgorithm_PhaseEnvelope_H__


#include "../../ovp_defines.h"

#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>

// The unique identifiers for the box and its descriptor.
// Identifier are randomly chosen by the skeleton-generator.
//#define OVP_ClassId_BoxAlgorithm_PhaseEnvelope OpenViBE::CIdentifier(0x7878A47F, 0x9A8FE349)
//#define OVP_ClassId_BoxAlgorithm_PhaseEnvelopeDesc OpenViBE::CIdentifier(0x2DB54E2F, 0x435675EF)

namespace OpenViBEPlugins
{
	namespace SignalProcessingBasic
	{
		/**
		 * \class CBoxAlgorithmPhaseEnvelope
		 * \author Alison Cellard (Inria)
		 * \date Thu Jun  6 13:47:53 2013
		 * \brief The class CBoxAlgorithmPhaseEnvelope describes the box Phase and Envelope.
		 *
		 */
		class CBoxAlgorithmPhaseEnvelope : virtual public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
		{
		public:
			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);

			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);
			
			virtual OpenViBE::boolean process(void);

			// As we do with any class in openvibe, we use the macro below 
			// to associate this box to an unique identifier. 
			// The inheritance information is also made available, 
			// as we provide the superclass OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_PhaseEnvelope);

		protected:

			// Signal stream decoder
			OpenViBEToolkit::TSignalDecoder < CBoxAlgorithmPhaseEnvelope > m_oAlgo0_SignalDecoder;
			// Signal stream encoder
			OpenViBEToolkit::TSignalEncoder < CBoxAlgorithmPhaseEnvelope > m_oAlgo1_SignalEncoder;
			OpenViBEToolkit::TSignalEncoder < CBoxAlgorithmPhaseEnvelope > m_oAlgo2_SignalEncoder;

			OpenViBE::Kernel::IAlgorithmProxy* m_pHilbertAlgo;

			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* > ip_pSignal_Matrix;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* > op_pEnvelope_Matrix;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* > op_pPhase_Matrix;

		};

		

		/**
		 * \class CBoxAlgorithmPhaseEnvelopeDesc
		 * \author Alison Cellard (Inria)
		 * \date Thu Jun  6 13:47:53 2013
		 * \brief Descriptor of the box Phase and Envelope.
		 *
		 */
		class CBoxAlgorithmPhaseEnvelopeDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Phase and Envelope"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Alison Cellard"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("Inria"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Phase and envelope from discrete-time analytic signal using Hilbert transform"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("Return phase and envelope of the input signal using Hilbert transform and analytic signal"); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Signal processing/Basic"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-new"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_PhaseEnvelope; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::SignalProcessingBasic::CBoxAlgorithmPhaseEnvelope; }
			
			
//			virtual OpenViBE::Plugins::IBoxListener* createBoxListener(void) const               { return new CBoxAlgorithmPhaseEnvelopeListener; }
//			virtual void releaseBoxListener(OpenViBE::Plugins::IBoxListener* pBoxListener) const { delete pBoxListener; }
			
			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
				rBoxAlgorithmPrototype.addInput("EEG Signal",OV_TypeId_Signal);

				//rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_CanModifyInput);
				//rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_CanAddInput);
				
				rBoxAlgorithmPrototype.addOutput("Envelope",OV_TypeId_Signal);
				rBoxAlgorithmPrototype.addOutput("Phase",OV_TypeId_Signal);


				//rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_CanModifyOutput);
				//rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_CanAddOutput);
				
				//No setting specified.To add settings use :
				//rBoxAlgorithmPrototype.addSetting("Setting Name",OV_TypeId_XXXX,"default value");

				//rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_CanAddSetting);
				
				rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_IsUnstable);
				
				return true;
			}
			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_PhaseEnvelopeDesc);
		};
	};
};

#endif // __OpenViBEPlugins_BoxAlgorithm_PhaseEnvelope_H__
