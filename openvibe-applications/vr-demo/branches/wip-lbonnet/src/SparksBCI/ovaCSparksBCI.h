#ifndef __OpenViBEApplication_CSparksBCI_H__
#define __OpenViBEApplication_CSparksBCI_H__

#include "../ovavrdCOgreVRApplication.h"

namespace OpenViBEVRDemos {

	/**
	 * \class CSparksBCI
	 * \author Laurent Bonnet (INRIA/IRISA)
	 * \date 2010-02-16
	 * \brief Ogre application for the SSSEP demonstrator.
	 *
	 * \details The SparksBCI application is based on SSSEP. 
	 * The user is stimulated with a tactile vibrator embedded in a mouse.
	 * The SSSEP response is detected in the brain, and triggers a particle animation in the application.
	 * 
	 */
	class CSparksBCI : public COgreVRApplication
	{
		public:

			/**
			* \brief constructor.
			*/
			CSparksBCI();

		private:

			/**
			* \brief Initializes the scene, camera, lights and GUI.
			* \return \em true if the scene is successfully set up.
			*/
			virtual bool initialise(void);
			
			//----- SCENE COMPONENTS -----//
			/**
			* \brief Loads the GUI.
			*/
			void loadGUI(void);
			
			
			/**
			* \brief Lifts the barrels and spaceship according to the feedback received from the analog server.
			*/
			virtual bool process(double timeSinceLastProcess);
			
			bool m_bStimulated;
			bool m_bStimulationDetected;
			bool m_bFocusDetected;
			bool m_bCalibrationInProgress;
			bool m_bCalibrated;
			// the last value received
			double m_dAnalogValue_meanNotStimulated; 
			double m_dAnalogValue_meanStimulated;
			double m_dAnalogValue_varianceNotStimulated;
			double m_dAnalogValue_varianceStimulated;
			
			double m_dLastAnalogValue_meanNotStimulated; 
			double m_dLastAnalogValue_meanStimulated;
			double m_dLastAnalogValue_varianceNotStimulated;
			double m_dLastAnalogValue_varianceStimulated;
			
			double m_dAnalogValue_power;

			//double m_dFeedback_stimulation;                //!<The current feedback value received from the VRPN server.
			//double m_dMaxFeedback_stimulation;             //!<The max feedback value ever received from the VRPN server.
			//double m_dLastFeedback_stimulation;            //!<Previous feedback value.

			//double m_dFeedback_focus;                //!<The current feedback value received from the VRPN server.
			//double m_dMaxFeedback_focus;             //!<The max feedback value ever received from the VRPN server.
			//double m_dLastFeedback_focus;            //!<Previous feedback value.


			double m_dStat_SparksAnimationTime;
			
			
			/** 
			* \brief Key pressed callback, launched when a key is pressed.
			* \return \em true if the rendering engine should continue.
			*/
			bool keyPressed(const OIS::KeyEvent& evt);
		
	};
};
#endif //__OpenViBEApplication_CSparksBCI_H__