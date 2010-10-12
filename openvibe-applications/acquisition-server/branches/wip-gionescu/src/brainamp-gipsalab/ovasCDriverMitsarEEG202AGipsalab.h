#ifndef __OpenViBE_AcquisitionServer_CDriverMitsarEEG202AGipsalab_H__
#define __OpenViBE_AcquisitionServer_CDriverMitsarEEG202AGipsalab_H__

#include "../ovasCAcqServerPipe.h"

namespace OpenViBEAcquisitionServer
{
	class CDriverMitsarEEG202AGipsalab : public CAcqServerPipe
	{
	private:
		typedef enum
		{	NB_SAMPLES			= 32,
			NB_SIGNALS			= 33,
			CH_Event_INDEX		= NB_SIGNALS - 2,
			Bio1_INDEX			= NB_SIGNALS - 1,
		} acquisition_type;
	public:

		CDriverMitsarEEG202AGipsalab(OpenViBEAcquisitionServer::IDriverContext& rDriverContext);
		virtual ~CDriverMitsarEEG202AGipsalab(void);

		virtual	OpenViBE::boolean	setAcquisitionParams();
		virtual	OpenViBE::boolean	processDataAndStimulations();

	private:
		void						parseTriggers(OpenViBE::uint16& triggers, OpenViBE::uint16& synchro, const OpenViBE::float32 value);
	
	private:
		OpenViBE::uint16			m_uint16OldStimulation;
	};
};

#endif // __OpenViBE_AcquisitionServer_CDriverMitsarEEG202AGipsalab_H__
