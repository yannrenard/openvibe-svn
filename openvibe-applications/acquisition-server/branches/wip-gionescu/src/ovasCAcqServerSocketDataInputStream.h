#ifndef __OpenViBE_AcquisitionServer_CAcqServerSocketDataInputStream_H__
#define __OpenViBE_AcquisitionServer_CAcqServerSocketDataInputStream_H__

#include <socket/IConnectionClient.h>

#include "ovasCAcqServerDataInputStreamAbstract.h"

namespace OpenViBEAcquisitionServer
{
	class CAcqServerSocketDataInputStream : public CAcqServerDataInputStreamAbstract
	{
	public:
		CAcqServerSocketDataInputStream(OpenViBE::CString& sServerHostName, OpenViBE::uint32& ui32ServerHostPort);
		virtual ~CAcqServerSocketDataInputStream(void);

		virtual	OpenViBE::boolean	open();
		virtual	OpenViBE::boolean	close();

	protected:
		virtual	OpenViBE::boolean	readBlock(const void* dataPtr, const OpenViBE::uint32 uint32DimData);
		
	protected:
		Socket::IConnectionClient*	m_pConnectionClient;	
		OpenViBE::CString&			m_sServerHostName;
		OpenViBE::uint32&			m_ui32ServerHostPort;
	};
};

#endif // __OpenViBE_AcquisitionServer_CAcqServerSocketDataInputStream_H__
