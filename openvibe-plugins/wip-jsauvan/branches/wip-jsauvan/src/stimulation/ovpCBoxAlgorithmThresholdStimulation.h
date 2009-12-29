#ifndef __OpenViBEPlugins_BoxAlgorithm_ThresholdStimulation_H__
#define __OpenViBEPlugins_BoxAlgorithm_ThresholdStimulation_H__

#include "../ovp_defines.h"
#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>
#include <ovp_global_defines.h>

#include <map>


namespace OpenViBEPlugins
{
	namespace Stimulation
	{
		class CBoxAlgorithmThresholdStimulation : virtual public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
		{

		private:
			OpenViBE::Kernel::IAlgorithmProxy* m_pStreamDecoder;
                        OpenViBE::Kernel::TParameterHandler < const OpenViBE::IMemoryBuffer* > m_oInputMemoryBufferHandle;

			OpenViBE::Kernel::IAlgorithmProxy* m_pStreamEncoder;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMemoryBuffer* > m_oOutputMemoryBufferHandle;

			OpenViBE::uint64 m_ui64StartTime;
			OpenViBE::uint64 m_ui64EndTime;
			OpenViBE::boolean m_bHasSentHeader;
			OpenViBE::CStimulationSet m_oStimulationSet;

			OpenViBE::uint64 m_ui64OnStimulationIdentifier;
			OpenViBE::uint64 m_ui64OffStimulationIdentifier;
			OpenViBE::float64 m_f64Threshold;

			OpenViBE::uint32 m_ui32NbDimensions;
			std::map <OpenViBE::uint32, OpenViBE::uint32> m_lDimensionSize;
			OpenViBE::boolean m_bIsValid;

		public:

			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);
			virtual OpenViBE::boolean process(void);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_ThresholdStimulation);

		protected:

			// ...
		};

		class CBoxAlgorithmThresholdStimulationDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Threshold Stimulation"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Jean-Baptiste Sauvan"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INRIA/IRISA"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Transform a one dim. matrix into a stimulation"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("Transform a one dim. matrix into a stimulation thanks to a threshold"); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Stimulation"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_ThresholdStimulation; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::Stimulation::CBoxAlgorithmThresholdStimulation; }

			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
				rBoxAlgorithmPrototype.addInput("Amplitude", OV_TypeId_StreamedMatrix);

				rBoxAlgorithmPrototype.addOutput("Stimulation",OV_TypeId_Stimulations); 

				rBoxAlgorithmPrototype.addSetting("Threshold", OV_TypeId_Float , "0.0");
				rBoxAlgorithmPrototype.addSetting("ON stimulation", OV_TypeId_Stimulation, "OVTK_GDF_Feedback_Continuous");
                                rBoxAlgorithmPrototype.addSetting("OFF stimulation", OV_TypeId_Stimulation, "OVTK_GDF_End_Of_Trial");

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_ThresholdStimulationDesc);
		};
	};
};

#endif // __OpenViBEPlugins_BoxAlgorithm_ThresholdStimulation_H__
