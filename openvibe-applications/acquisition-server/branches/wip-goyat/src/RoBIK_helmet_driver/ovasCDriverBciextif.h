#ifndef __OpenViBE_AcquisitionServer_CDriverBciextif_H__
#define __OpenViBE_AcquisitionServer_CDriverBciextif_H__

#include <string>

#include "../ovasIDriver.h"
#include "../ovasCHeader.h"
#include "DataRecord.h"

namespace OpenViBEAcquisitionServer
{

	class CDriverBciextif : public OpenViBEAcquisitionServer::IDriver
	{
	public:

		CDriverBciextif(OpenViBEAcquisitionServer::IDriverContext& rDriverContext);
        virtual ~CDriverBciextif();
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

		void NewDataRecieved ( char* channelID, int iStartTick, int iDataCount, double Data[], int iLostTickCount, int LostTicks[] );
		
	protected:

        	void DoAddDataToReader( int iDataCount, double Data[], int channel );

		OpenViBEAcquisitionServer::IDriverCallback* m_pCallback;
		OpenViBEAcquisitionServer::CHeader m_oHeader;

		OpenViBE::uint32 m_ui32StartTime;
		OpenViBE::uint32 m_ui32SampleCountPerSentBlock;
        	OpenViBE::uint32 m_ui32SamplePerBciextifRead;
		OpenViBE::float32* m_pSample;

		double *m_pdbSample;
		int *m_puilost;
		DataReader reader;
		std::string m_sBCIFilePath;

        bool DefSet( char* sVar, char* sValue, bool bFlush );	
	};
	
};

#endif // __OpenViBE_AcquisitionServer_CDriverBciextif_H__
