#ifndef __OpenViBEPlugins_Stimulation_CrazyInterpreter_H__
#define __OpenViBEPlugins_Stimulation_CrazyInterpreter_H__

#include "../ovp_defines.h"
#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>
#include <ovp_global_defines.h>
#include <iostream>
#include <vector>
#include <map>
#include <time.h>

namespace OpenViBEPlugins
{
	namespace Stimulation
	{
		class CCrazyInterpreter : virtual public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
		{
		public:

			virtual void release(void) { delete this; }

			// virtual OpenViBE::uint64 getClockFrequency(void);
			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
			// virtual OpenViBE::boolean processEvent(OpenViBE::CMessageEvent& rMessageEvent);
			// virtual OpenViBE::boolean processSignal(OpenViBE::CMessageSignal& rMessageSignal);
			// virtual OpenViBE::boolean processClock(OpenViBE::CMessageClock& rMessageClock);
			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);
			virtual OpenViBE::boolean process(void);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_CrazyInterpreter);

		protected:

			typedef struct
			{
				OpenViBE::uint64 m_ui64Identifier;
				OpenViBE::uint64 m_ui64Date;
				OpenViBE::uint64 m_ui64Duration;
			} SStimulation;

			std::vector < OpenViBE::Kernel::IAlgorithmProxy* > m_vStreamDecoder;
			std::vector < OpenViBE::uint64 > m_vStreamDecoderEndTime;
			OpenViBE::Kernel::IAlgorithmProxy* m_pStreamEncoder;

			OpenViBE::uint64 m_ui64StartTime;
			OpenViBE::uint64 m_ui64EndTime;
			OpenViBE::boolean m_bHasSentHeader;
			OpenViBE::boolean m_bEndOfTrial;
			OpenViBE::boolean m_bHasReceivedChoice;

			OpenViBE::uint64 m_ui64EndOfTrialIdentifier;
			OpenViBE::uint64 m_ui64P300StimuIdentifier;
			OpenViBE::uint64 m_ui64InterFlashTime;
			OpenViBE::float64 m_f64ErrorProbability;

			std::vector < CCrazyInterpreter::SStimulation > m_vEvents;
			CCrazyInterpreter::SStimulation m_oChoice;
		};

		class CCrazyInterpreterDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Crazy Interpreter"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Jean-Baptiste Sauvan"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INRIA/IRISA"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Stimulation"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("0.1"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_CrazyInterpreter; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::Stimulation::CCrazyInterpreter; }

			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
				rBoxAlgorithmPrototype.addInput ("Event stimulations",       OV_TypeId_Stimulations);
				rBoxAlgorithmPrototype.addInput ("Choice stimulations",       OV_TypeId_Stimulations);
				rBoxAlgorithmPrototype.addOutput("Interpreted choice", OV_TypeId_Stimulations);

				rBoxAlgorithmPrototype.addSetting("Probability of errors", OV_TypeId_Float, "0.0");
				//rBoxAlgorithmPrototype.addSetting("Choice stimulation", OV_TypeId_Stimulation, "OVTK_GDF_Feedback_Continuous");

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_CrazyInterpreterDesc);
		};
	};
};

#endif // __OpenViBEPlugins_Stimulation_CrazyInterpreter_H__
