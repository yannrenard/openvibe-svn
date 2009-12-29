#ifndef __OpenViBEPlugins_Offline_COfflineBestClassifierTraining_H__
#define __OpenViBEPlugins_Offline_COfflineBestClassifierTraining_H__

#if defined TARGET_HAS_ThirdPartyFabien && defined TARGET_HAS_ThirdPartyITPP && defined TARGET_HAS_ThirdPartyGSL

#include "../ovp_defines.h"

#include <openvibe-toolkit/ovtk_all.h>
#include <openvibe-toolkit/ovtkTTrainingBoxAlgorithm.h>

#include <ebml/CReader.h>
#include <ebml/CReaderHelper.h>

#include "ovpCTrialConversion.h"

#include <Trial.h>
#include <TrialSetParser.h>
#include <LogBP_IIR.h>
#include <LDAClassif.h>

namespace OpenViBEPlugins
{
	namespace Offline
	{
		class COfflineBestClassifierTraining : virtual public OpenViBEToolkit::TTrainingBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{
		public:

			COfflineBestClassifierTraining(void);
			virtual ~COfflineBestClassifierTraining(void);

			virtual OpenViBE::boolean initialize();

			virtual void release(void);
			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);
			virtual OpenViBE::boolean process(void);

			virtual OpenViBE::CIdentifier getStimulationIdentifierTrialStart(void);
			virtual OpenViBE::CIdentifier getStimulationIdentifierTrialEnd(void);
			virtual OpenViBE::CIdentifier getStimulationIdentifierTrialLabelRangeStart(void);
			virtual OpenViBE::CIdentifier getStimulationIdentifierTrialLabelRangeEnd(void);
			virtual OpenViBE::CIdentifier getStimulationIdentifierTrain(void);
			virtual OpenViBE::boolean train(OpenViBEToolkit::ISignalTrialSet& rTrialSet);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TTrainingBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>, OVP_ClassId_OfflineBestClassifierTraining)

			OpenViBEToolkit::IBoxAlgorithmSignalInputReaderCallback::TCallbackProxy1<OpenViBEToolkit::TTrainingBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm> > m_oSignalReaderCallbackProxy;
			OpenViBEToolkit::IBoxAlgorithmStimulationInputReaderCallback::TCallbackProxy1<OpenViBEToolkit::TTrainingBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm> > m_oStimulationReaderCallbackProxy;

			OpenViBEToolkit::IBoxAlgorithmSignalInputReaderCallback* m_pSignalReaderCallback;
			OpenViBEToolkit::IBoxAlgorithmStimulationInputReaderCallback* m_pStimulationReaderCallback;

			EBML::CReader m_oSignalReader;
			EBML::CReader m_oStimulationReader;

			OpenViBE::uint64 m_ui64SignalLatestSampleTime;

			LogBP_IIR m_oBandPower;

			OpenViBE::CString m_oOutputFilename;

			OpenViBE::float64 m_f64FeedbackStart;
			OpenViBE::float64 m_f64FeedbackEnd;

			OpenViBE::float64 m_f64WindowWidth;
			OpenViBE::float64 m_f64WindowStep;

			OpenViBE::CIdentifier m_oTrialStartIdentifier;
			OpenViBE::CIdentifier m_oTrialEndIdentifier;
			OpenViBE::CIdentifier m_oLabelStartIdentifier;
			OpenViBE::CIdentifier m_oLabelEndIdentifier;
			OpenViBE::CIdentifier m_oTrainIdentifier;

			OpenViBE::CString m_oOutputConfiguration;

			OpenViBE::uint32 m_ui32DownsamplingFactor;

		};

		class COfflineBestClassifierTrainingDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }
			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Offline best classifier training"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Bruno Renier"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INRIA/IRISA"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Selects the best classifier possible."); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("Trains and computes the accuracy of classifiers trained on different time windows, and selects the best one."); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Offline"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("0.7"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_OfflineBestClassifierTraining; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::Offline::COfflineBestClassifierTraining(); }

			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rPrototype) const
			{
				// Adds box inputs
				rPrototype.addInput("Signal stream", OV_TypeId_Signal);
				rPrototype.addInput("Stimulation stream", OV_TypeId_Stimulations);

				// Adds box outputs

				// Adds box settings
				rPrototype.addSetting("Feature extractor", OVTK_TypeId_Filename, "../../BandPowerConf.txt");
				rPrototype.addSetting("Output filename", OVTK_TypeId_Filename, "../../Classifier.xml");

				rPrototype.addSetting("Feedback start", OV_TypeId_Float, "4");
				rPrototype.addSetting("Feedback end", OV_TypeId_Float, "8");

				rPrototype.addSetting("Window width", OV_TypeId_Float, "1");
				rPrototype.addSetting("Window step", OV_TypeId_Float, "0.2");

				rPrototype.addSetting("Trial start stimulation", OV_TypeId_Stimulation, "OVTK_GDF_Cross_On_Screen");
				rPrototype.addSetting("Trial end stimulation", OV_TypeId_Stimulation, "OVTK_GDF_End_Of_Trial");
				rPrototype.addSetting("Trial label range start stimulation", OV_TypeId_Stimulation, "OVTK_GDF_Left");
				rPrototype.addSetting("Trial label range end stimulation", OV_TypeId_Stimulation, "OVTK_GDF_Right");
				rPrototype.addSetting("Train stimulation", OV_TypeId_Stimulation, "OVTK_GDF_End_Of_Session");

				rPrototype.addSetting("Output configuration", OVTK_TypeId_Filename, "../../Classifier.txt");
				rPrototype.addSetting("Downsampling factor", OV_TypeId_Integer, "1");

				return true;
			}

			virtual OpenViBE::CString getStockItemName(void) const
			{
				return OpenViBE::CString("gtk-execute");
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_OfflineBestClassifierTrainingDesc)
		};
	};
};

#endif

#endif // __OpenViBEPlugins_Offline_COfflineBestClassifierTraining_H__
