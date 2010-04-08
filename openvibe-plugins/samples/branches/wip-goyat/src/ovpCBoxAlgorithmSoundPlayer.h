#ifndef __OpenViBEPlugins_BoxAlgorithm_SoundPlayer_H__
#define __OpenViBEPlugins_BoxAlgorithm_SoundPlayer_H__

#include "ovp_defines.h"
#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>

namespace OpenViBEPlugins
{
	namespace Samples
	{
		class CBoxAlgorithmSoundPlayer : public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
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

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_SoundPlayer);

		protected:
			std::vector<std::pair<OpenViBE::uint64, OpenViBE::CString> > m_soundmap;

		private : 
			OpenViBE::Kernel::IAlgorithmProxy* m_pSequenceStimulationDecoder;
			OpenViBE::Kernel::TParameterHandler<const OpenViBE::IMemoryBuffer*> ip_pSequenceMemoryBuffer;
			OpenViBE::Kernel::TParameterHandler<OpenViBE::IStimulationSet*> op_pSequenceStimulationSet;
		};
		
		class CSoundPlayerListener : public OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >
		{
		public:

			OpenViBE::boolean check(OpenViBE::Kernel::IBox& rBox)
			{
				char l_sName[1024];
				OpenViBE::uint32 i;

				for(i=0; i<rBox.getSettingCount(); i++)
				{
					if(i%2==0)
					  {
						sprintf(l_sName, "Sound Trigger %u ", i/2);
						rBox.setSettingName(i, l_sName);
						rBox.setSettingType(i, OV_TypeId_Stimulation);
					  }
					if(i%2==1)
					  {
						sprintf(l_sName, "Sound Path %u", i/2);
						rBox.setSettingName(i, l_sName);
						rBox.setSettingType(i, OV_TypeId_Filename);
					  }
				}

				return true;
			}

			virtual OpenViBE::boolean onSettingRemoved(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index)
			{
				//supprime l'un par defaut
				
				//supprime le deuxième
				OpenViBE::uint32 index=ui32Index;
				if(index%2==1) {index--;}
				rBox.removeSetting(index);

				return this->check(rBox);
			};
			
			virtual OpenViBE::boolean onSettingAdded(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index)
			{
				//retirer l'ajouté par defaut
				rBox.removeSetting(ui32Index);
				///ajouter les deux nouveaux settings
				rBox.addSetting("", OV_TypeId_Stimulation, "OVTK_GDF_Feedback_Continuous");
				rBox.addSetting("", OV_TypeId_Filename, "sound.wav");

				return this->check(rBox);
			};

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >, OV_UndefinedIdentifier);
		};
		
		class CBoxAlgorithmSoundPlayerDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("SoundPlayer"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Matthieu Goyat"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("Gipsa-lab"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("This box play Sounds"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Samples"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-about"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_SoundPlayer; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::Samples::CBoxAlgorithmSoundPlayer; }
			virtual OpenViBE::Plugins::IBoxListener* createBoxListener(void) const               { return new CSoundPlayerListener; }
			virtual void releaseBoxListener(OpenViBE::Plugins::IBoxListener* pBoxListener) const { delete pBoxListener; }
			
			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
				rBoxAlgorithmPrototype.addInput  ("Stimulations Input", OVTK_TypeId_Stimulations);
				rBoxAlgorithmPrototype.addSetting("sound trigger",  OV_TypeId_Stimulation, "OVTK_GDF_Feedback_Continuous");
				rBoxAlgorithmPrototype.addSetting("sound path", OV_TypeId_Filename, "sound.wav");
				rBoxAlgorithmPrototype.addFlag   (OpenViBE::Kernel::BoxFlag_CanAddSetting);
				rBoxAlgorithmPrototype.addFlag   (OpenViBE::Kernel::BoxFlag_IsUnstable);
				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_SoundPlayerDesc);
		};
	};
};

#endif // __OpenViBEPlugins_BoxAlgorithm_SoundPlayer_H__
