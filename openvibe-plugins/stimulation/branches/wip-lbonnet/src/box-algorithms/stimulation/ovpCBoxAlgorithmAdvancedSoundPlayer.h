#ifndef __OpenViBEPlugins_BoxAlgorithm_AdvancedSoundPlayer_H__
#define __OpenViBEPlugins_BoxAlgorithm_AdvancedSoundPlayer_H__

#include "../../ovp_defines.h"
#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>

#include <AL/alut.h>
#include <vector>

#define OVP_ClassId_BoxAlgorithm_AdvancedSoundPlayer  OpenViBE::CIdentifier(0x7AC2396F, 0x7EE52EFE)
#define OVP_ClassId_BoxAlgorithm_AdvancedSoundPlayerDesc  OpenViBE::CIdentifier(0x6FD040EF, 0x7E2F1284)

namespace OpenViBEPlugins
{
	namespace Stimulation
	{
		class CBoxAlgorithmAdvancedSoundPlayer : public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
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

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_AdvancedSoundPlayer);

		protected:

			OpenViBE::Kernel::IAlgorithmProxy* m_pStreamDecoder;
			OpenViBE::boolean m_bLoop;
			OpenViBE::uint64 m_ui64PlayTrigger;
			OpenViBE::uint64 m_ui64StopTrigger;
			OpenViBE::CString m_sFileName;

			std::vector<ALuint> m_vOpenALSources;
			ALuint m_uiSoundBufferHandle;
		};

		class CBoxAlgorithmAdvancedSoundPlayerDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Advanced sound player"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Laurent Bonnet"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INRIA"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Stimulation"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_AdvancedSoundPlayer; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::Stimulation::CBoxAlgorithmAdvancedSoundPlayer; }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-media-play"); }

			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
				rBoxAlgorithmPrototype.addInput  ("Stimulation stream", OV_TypeId_Stimulations);
				rBoxAlgorithmPrototype.addSetting("PLAY trigger", OV_TypeId_Stimulation,"OVTK_StimulationId_Label_00");
				rBoxAlgorithmPrototype.addSetting("STOP trigger", OV_TypeId_Stimulation, "OVTK_StimulationId_Label_01");
				rBoxAlgorithmPrototype.addSetting("File to play", OV_TypeId_Filename, "../share/openvibe-plugins/stimulation/ov_beep.wav");
				rBoxAlgorithmPrototype.addSetting("Loop", OV_TypeId_Boolean, "False");
				rBoxAlgorithmPrototype.addFlag   (OpenViBE::Kernel::BoxFlag_IsUnstable);

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_AdvancedSoundPlayerDesc);
		};
	};
};

#endif // __OpenViBEPlugins_BoxAlgorithm_AdvancedSoundPlayer_H__