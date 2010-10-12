#ifndef __OpenViBE_AcquisitionServer_CAcqServerEyelinkSocketDataInputStream_H__
#define __OpenViBE_AcquisitionServer_CAcqServerEyelinkSocketDataInputStream_H__

#include "../ovasCAcqServerSocketDataInputStream.h"

#include "ovasEyelink.h"
using namespace OpenViBEAcquisitionServer::Eyelink;

namespace OpenViBEAcquisitionServer
{
	class CAcqServerEyelinkSocketDataInputStream : public CAcqServerSocketDataInputStream
	{
	public:
		CAcqServerEyelinkSocketDataInputStream(OpenViBE::CString& sServerHostName, OpenViBE::uint32& ui32ServerHostPort);
		virtual ~CAcqServerEyelinkSocketDataInputStream(void);

		virtual	OpenViBE::boolean	read();
	};
};

#endif // __OpenViBE_AcquisitionServer_CAcqServerEyelinkSocketDataInputStream_H__
