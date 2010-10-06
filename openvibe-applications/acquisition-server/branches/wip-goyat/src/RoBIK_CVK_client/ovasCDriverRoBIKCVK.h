#ifndef __OpenViBE_AcquisitionServer_CDriverRoBIKCVK_H__
#define __OpenViBE_AcquisitionServer_CDriverRoBIKCVK_H__

#include "../ovasIDriver.h"
#include "../ovasCHeader.h"

namespace OpenViBEAcquisitionServer
{
	class CDriverRoBIKCVK : public OpenViBEAcquisitionServer::IDriver
	{
	public:

		CDriverRoBIKCVK(OpenViBEAcquisitionServer::IDriverContext& rDriverContext);//modif new Idriver
		virtual void release(void);
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

		//??? virtual void processData (cf neXus)

	protected:

		OpenViBEAcquisitionServer::IDriverCallback* m_pCallback;
		OpenViBEAcquisitionServer::CHeader m_oHeader;

		OpenViBE::uint32 m_ui32SampleCountPerSentBlock;
		OpenViBE::float32* m_pSample;

		OpenViBE::uint32 m_ui32SampleIndex;
		OpenViBE::boolean m_bConnected;

	};
};

#endif // __OpenViBE_AcquisitionServer_CDriverRoBIKCVK_H__
