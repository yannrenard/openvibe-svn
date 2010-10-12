#ifndef __OpenViBE_AcquisitionServer_CAcqServerBrainampSocketDataInputStream_H__
#define __OpenViBE_AcquisitionServer_CAcqServerBrainampSocketDataInputStream_H__

#include "../ovasCAcqServerSocketDataInputStream.h"

#include "ovasBrainAmp.h"
using namespace OpenViBEAcquisitionServer::BrainAmp;

namespace OpenViBEAcquisitionServer
{
	class CAcqServerBrainampSocketDataInputStream : public CAcqServerSocketDataInputStream
	{
	public:
		CAcqServerBrainampSocketDataInputStream(OpenViBE::CString& sServerHostName, OpenViBE::uint32& ui32ServerHostPort);
		virtual ~CAcqServerBrainampSocketDataInputStream(void);

		virtual	OpenViBE::boolean	read();
	};
};

#endif // __OpenViBE_AcquisitionServer_CAcqServerBrainampSocketDataInputStream_H__
