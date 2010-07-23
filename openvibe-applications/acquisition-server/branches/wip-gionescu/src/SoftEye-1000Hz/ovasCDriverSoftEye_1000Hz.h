#ifndef __OpenViBE_AcquisitionServer_CDriverSoftEye_1000Hz_H__
#define __OpenViBE_AcquisitionServer_CDriverSoftEye_1000Hz_H__

#include "../ovasIDriver.h"
#include "../ovasCHeader.h"


//) Eyelink/SoftEye standard data struct.
//"""""""""""""""""""""""""""""""""""""""
namespace Standard_SoftEye_1000Hz
{

typedef struct
    {    unsigned short            halfPeriod;                // millisec
         unsigned short            eyelinkPeriod;             // millisec
         unsigned short            samplesPerHalfPeriod;      // actually halfPeriod/eyelinkPeriod
         unsigned short            parallelPortMask;          // synchronisation mask
    } synchro_type;

	typedef struct
			{   unsigned short    softeyeStatus;
				unsigned short    eyelinkFlags;    // flags to indicate contents
				unsigned short    eyelinkStatus;    // tracker status flags
				unsigned short    eyelinkInput;    // extra (input word)
				unsigned long    eyelinkTime;
				float    leftX, leftY;
				float    rightX, rightY;
			} eyelink_type;
	  
	typedef enum
           {    PARALLEL_PORT_SYNCHRO    = 0x0001,
				TASK_SYNCHRO            = 0x0002,
				INVALID_DATA            = 0x0004,
				MISSING_SAMPLE            = 0x0008,
           } mask_status_type; 

typedef struct
			{

				OpenViBE::float32 Eyelink_TIME;

				OpenViBE::float32 SoftEye_PARALLEL_PORT_SYNCHRO;
				OpenViBE::float32 SoftEye_TASK_SYNCHRO;
				OpenViBE::float32 SoftEye_INVALID_DATA;
				OpenViBE::float32 SoftEye_MISSING_SAMPLE;
	
				OpenViBE::float32 Eyelink_FLAGS;
				OpenViBE::float32 Eyelink_STATUS;
				OpenViBE::float32 Eyelink_INPUT;

				OpenViBE::float32 Eye_X_left;
				OpenViBE::float32 Eye_Y_left;
				OpenViBE::float32 Eye_X_right;
				OpenViBE::float32 Eye_Y_right;

				OpenViBE::float32 Eye_Synchro;


			} openvibe_type;

};
using namespace Standard_SoftEye_1000Hz;

//) Driver Class
//""""""""""""""

namespace OpenViBEAcquisitionServer
{


	class CDriverSoftEye_1000Hz : public OpenViBEAcquisitionServer::IDriver
	{
	public:

		CDriverSoftEye_1000Hz(OpenViBEAcquisitionServer::IDriverContext& rDriverContext);//modif new Idriver
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

		void dump(openvibe_type* pSample,	eyelink_type* pEyelink);
		OpenViBE::float32 Last_synchro_ok;
		int cpt_0_1;
		//int cpt_1;

	protected:

		OpenViBEAcquisitionServer::IDriverCallback* m_pCallback;
		OpenViBEAcquisitionServer::CHeader m_oHeader;

		OpenViBE::boolean m_bInitialized;
		OpenViBE::boolean m_bStarted;
		OpenViBE::uint32 m_ui32SampleCountPerSentBlock;
		OpenViBE::float32* m_pSample;
		//openvibe_type* m_pSample;

		OpenViBE::uint32 m_ui32SampleIndex;

		OpenViBE::uint32 m_ui32StartTime;
		OpenViBE::uint64 m_ui64SampleCountTotal;
		OpenViBE::uint64 m_ui64AutoAddedSampleCount;
		OpenViBE::uint64 m_ui64AutoRemovedSampleCount;
	};

	


};


#endif // __OpenViBE_AcquisitionServer_CDriverSoftEye_1000Hz_H__
