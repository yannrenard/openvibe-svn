#ifndef __OpenViBE_AcquisitionServer_CDriverBrainampGipsalab_H__
#define __OpenViBE_AcquisitionServer_CDriverBrainampGipsalab_H__

#include "../ovasCAcqServerPipe.h"

#include "ovasBrainAmp.h"
using namespace OpenViBEAcquisitionServer::BrainAmp;

namespace OpenViBEAcquisitionServer
{
	class CDriverBrainampGipsalab : public CAcqServerPipe
	{
	public:

		CDriverBrainampGipsalab(OpenViBEAcquisitionServer::IDriverContext& rDriverContext);
		virtual ~CDriverBrainampGipsalab(void);

		virtual	OpenViBE::boolean	setAcquisitionParams();
		virtual	OpenViBE::boolean	processDataAndStimulations();
		
	protected:
		virtual	void				clean();
		void						dumpData();
		void						dumpMarker(const RDA_Marker& marker, const OpenViBE::int32 stimulation);

	private:
		RDA_MessageStart*	m_pStructRDA_MessageStart;
		RDA_MessageHeader*	m_pStructRDA_MessageHeader;
		RDA_MessageData*	m_pStructRDA_MessageData;
		RDA_MessageData32*	m_pStructRDA_MessageData32;
	};
};

#endif // __OpenViBE_AcquisitionServer_CDriverBrainampGipsalab_H__
