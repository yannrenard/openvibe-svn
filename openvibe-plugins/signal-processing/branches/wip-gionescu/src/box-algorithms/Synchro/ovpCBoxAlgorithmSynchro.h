#ifndef __OpenViBEPlugins_BoxAlgorithm_Synchro_H__
#define __OpenViBEPlugins_BoxAlgorithm_Synchro_H__

#include "../../ovp_defines.h"
#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>

#include <string>
#include <vector>

#define OVP_ClassId_BoxAlgorithm_Synchro     OpenViBE::CIdentifier(0x7D8C1A18, 0x4C273A91)
#define OVP_ClassId_BoxAlgorithm_SynchroDesc OpenViBE::CIdentifier(0x4E806E5E, 0x5035290D)

namespace OpenViBEPlugins
{
	namespace SignalProcessing
	{
		class CBoxAlgorithmSynchro : public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
		{
		public:

			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);
			virtual OpenViBE::boolean process(void);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_Synchro);

		protected:

			// first input params
			OpenViBE::Kernel::IAlgorithmProxy* m_pStreamDecoderSignal1;
			OpenViBE::Kernel::IAlgorithmProxy* m_pStreamDecoderStimulation1;
			
			OpenViBE::Kernel::TParameterHandler < const OpenViBE::IMemoryBuffer* >	ip_pMemoryBufferSignal1;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* >				op_pMatrixSignal1;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::uint64 >				op_ui64SamplingRateSignal1;
			OpenViBE::Kernel::TParameterHandler < const OpenViBE::IMemoryBuffer* >	ip_pMemoryBufferStimulation1;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IStimulationSet* >		op_pStimulationSetStimulation1;

			// second input params
			OpenViBE::Kernel::IAlgorithmProxy* m_pStreamDecoderSignal2;
			OpenViBE::Kernel::IAlgorithmProxy* m_pStreamDecoderStimulation2;
			
			OpenViBE::Kernel::TParameterHandler < const OpenViBE::IMemoryBuffer* >	ip_pMemoryBufferSignal2;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* >				op_pMatrixSignal2;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::uint64 >				op_ui64SamplingRateSignal2;
			OpenViBE::Kernel::TParameterHandler < const OpenViBE::IMemoryBuffer* >	ip_pMemoryBufferStimulation2;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IStimulationSet* >		op_pStimulationSetStimulation2;
	
			// output params
			OpenViBE::Kernel::IAlgorithmProxy* m_pStreamEncoderSignal1;
			OpenViBE::Kernel::IAlgorithmProxy* m_pStreamEncoderSignal2;
			OpenViBE::Kernel::IAlgorithmProxy* m_pStreamEncoderStimulation;

			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* >				ip_pMatrixSignal1;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::uint64 >				ip_ui64SamplingRateSignal1;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMemoryBuffer* >		op_pMemoryBufferSignal1;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* >				ip_pMatrixSignal2;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::uint64 >				ip_ui64SamplingRateSignal2;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMemoryBuffer* >		op_pMemoryBufferSignal2;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IStimulationSet* >		ip_pStimulationSetStimulation;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMemoryBuffer* >		op_pMemoryBufferStimulation;

			//Intern ressources
			OpenViBE::boolean m_bStimulationReceivedStart;
		};

		class CBoxAlgorithmSynchroDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Synchro"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Gelu Ionescu & Matthieu Goyat"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("GIPSA-lab"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Synchronize two acq servers"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Signal processing/Synchro"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-missing-image"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_Synchro; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::SignalProcessing::CBoxAlgorithmSynchro; }
			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
				rBoxAlgorithmPrototype.addInput  ("Input1 signal", OV_TypeId_Signal);
				rBoxAlgorithmPrototype.addInput  ("Input1 stimulation", OV_TypeId_Stimulations);
				rBoxAlgorithmPrototype.addInput  ("Input2 signal", OV_TypeId_Signal);
				rBoxAlgorithmPrototype.addInput  ("Input2 stimulation", OV_TypeId_Stimulations);
				rBoxAlgorithmPrototype.addOutput ("Output1 signal", OV_TypeId_Signal);
				rBoxAlgorithmPrototype.addOutput ("Output2 signal", OV_TypeId_Signal);
				rBoxAlgorithmPrototype.addOutput ("Output stimulation", OV_TypeId_Stimulations);
				rBoxAlgorithmPrototype.addSetting("New channel names", OV_TypeId_String, "Channel 1;Channel 2");
				// rBoxAlgorithmPrototype.addFlag   (OpenViBE::Kernel::BoxFlag_CanModifyInput);

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_SynchroDesc);
		};
	};
};

#endif // __OpenViBEPlugins_BoxAlgorithm_Synchro_H__
