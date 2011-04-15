#ifndef __OpenViBE_AcquisitionServer_CDriverEyelink1_H__
#define __OpenViBE_AcquisitionServer_CDriverEyelink1_H__

#include "../ovasIDriver.h"
#include "../ovasCHeader.h"

#include <openvibe/ov_all.h>

#include <vector>

#include <socket/IConnectionClient.h>

#include "ovasEyelink.h"
using namespace OpenViBEAcquisitionServer::Eyelink;

namespace OpenViBEAcquisitionServer
{
	class CDriverEyelink1 : public OpenViBEAcquisitionServer::IDriver
	{
	public:

		CDriverEyelink1(OpenViBEAcquisitionServer::IDriverContext& rDriverContext);
		virtual ~CDriverEyelink1(void);
		virtual const char* getName(void);

		virtual OpenViBE::boolean initialize(
			const OpenViBE::uint32 ui32SampleCountPerSentBlock,
			OpenViBEAcquisitionServer::IDriverCallback& rCallback);
		virtual OpenViBE::boolean uninitialize(void);

		virtual OpenViBE::boolean start(void);
		virtual OpenViBE::boolean stop(void);
		virtual OpenViBE::boolean loop(void);

		virtual OpenViBE::boolean isConfigurable(void);
		virtual OpenViBE::boolean configure(void);

		virtual const OpenViBEAcquisitionServer::IHeader* getHeader(void) { return &m_oHeader; }

	protected:

		OpenViBEAcquisitionServer::IDriverCallback* m_pCallback;
		Socket::IConnectionClient* m_pConnectionClient;
		OpenViBE::CString m_sServerHostName;
		OpenViBE::uint32 m_ui32ServerHostPort;

		OpenViBEAcquisitionServer::CHeader m_oHeader;

		OpenViBE::uint32               m_ui32SampleCountPerSentBlock;
		header_type                    m_structHeader;
		OpenViBE::uint16               m_ui16NbChannels;
		OpenViBE::uint32               m_ui32InputDataBlockSize;
		std::vector<OpenViBE::float32> m_vInputData;
		std::vector<OpenViBE::float32> m_vOutputData;
	private:
		OpenViBE::boolean readBlock(const void* pData, const OpenViBE::uint32 uint32DimData);
	};
};

#endif // __OpenViBE_AcquisitionServer_CDriverEyelink_H__
