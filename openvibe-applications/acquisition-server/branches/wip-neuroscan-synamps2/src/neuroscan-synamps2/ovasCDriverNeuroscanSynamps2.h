#ifndef __OpenViBE_AcquisitionServer_CDriverNeuroscanSynamps2_H__
#define __OpenViBE_AcquisitionServer_CDriverNeuroscanSynamps2_H__

#include "../ovasIDriver.h"
#include "../ovasCHeader.h"

#include <openvibe/ov_all.h>

#include <socket/IConnectionClient.h>

#ifndef ULONG
typedef unsigned long ULONG;
#endif

typedef unsigned long DWORD;	// These typedef are necessary to use (D)WORD
typedef unsigned short WORD;

namespace OpenViBEAcquisitionServer
{
	class CDriverNeuroscanSynamps2 : public OpenViBEAcquisitionServer::IDriver
	{
	public:

		CDriverNeuroscanSynamps2(void);
		virtual ~CDriverNeuroscanSynamps2(void);
		virtual void release(void);
		virtual const char* getName(void);

		virtual OpenViBE::boolean isFlagSet(
			const OpenViBEAcquisitionServer::EDriverFlag eFlag) const
		{
			return eFlag==DriverFlag_IsUnstable;
		}

		virtual OpenViBE::boolean initialize(	// starts the "build up connection" & "connect"
			const OpenViBE::uint32 ui32SampleCountPerSentBlock,
			OpenViBEAcquisitionServer::IDriverCallback& rCallback);
		virtual OpenViBE::boolean uninitialize(void);

		virtual OpenViBE::boolean start(void);
		virtual OpenViBE::boolean stop(void);
		virtual OpenViBE::boolean loop(void);

		virtual OpenViBE::boolean isConfigurable(void);
		virtual OpenViBE::boolean configure(void);
		virtual const OpenViBEAcquisitionServer::IHeader* getHeader(void) { return &o_pHeader; }

		Socket::IConnectionClient* m_pConnectionClient;		// builds up connection
		OpenViBE::CString m_sServerHostName;
		OpenViBE::uint32 m_ui32ServerHostPort;

	protected:

		OpenViBEAcquisitionServer::IDriverCallback* m_pCallback;
		//OpenViBEAcquisitionServer::IHeader* m_pHeader;
		OpenViBEAcquisitionServer::CHeader o_pHeader;

#define m_pHeader (&o_pHeader)

		OpenViBE::boolean m_bInitialized;
		OpenViBE::boolean m_bStarted;
		OpenViBE::uint32 m_ui32SampleCountPerSentBlock;
		OpenViBE::float32* m_pSample;

		OpenViBE::uint32 m_ui32IndexIn;
		OpenViBE::uint32 m_ui32IndexOut;
		OpenViBE::uint32 m_ui32BuffDataIndex;

		OpenViBE::uint32 m_ui32DataOffset;
		OpenViBE::uint32 m_ui32MarkerCount;

				OpenViBE::uint32 m_ui32TotalSampleCount;
		OpenViBE::uint32 m_ui32StartTime;

		#pragma pack(push,1)
		struct RDA_Marker
		//; A single marker in the marker array of RDA_MessageData
		{
			ULONG				nSize;				// Size of this marker.						//! don't mix up with m_dwSize!
			ULONG				nPosition;			// Relative position in the data block.
			ULONG				nPoints;			// Number of points of this marker
			long				nChannel;			// Associated channel number (-1 = all channels).
			char				sTypeDesc[1];		// Type, description in ASCII delimited by '\0'.
		};

		//; PacketHeader definition of the NeuroScan 4.3 software (Acquisition)
		typedef struct{
			char	m_chId[4];	// ID string, no trailing '\0'(*)
			WORD	m_wCode;	// Code(*)
			WORD	m_wRequest;	// Request(*)
			DWORD	m_dwSize;	// Body size (in bytes)(*)
								// (*)= for definition look up table on p.159 in Acquire4.3 doc
		}RDA_MessageHeader;		// changed from PacketHeader to RDA_MessageHeader because this implicates less changes/confusion

		// **** Messages sent by the RDA server to the clients. ****
		struct RDA_MessageStart : RDA_MessageHeader	// inheritance from RDA_MessageHeader
		//; Setup / Start infos, Header -> nType = 1
		{
			ULONG				nChannels;			// Number of channels
			double				dSamplingInterval;	// Sampling interval in microseconds
			double				dResolutions[1];	// Array of channel resolutions -> double dResolutions[nChannels]
													// coded in microvolts. i.e. RealValue = resolution * A/D value
			char 				sChannelNames[1];	// Channel names delimited by '\0'. The real size is
													// larger than 1.
		};

		struct RDA_MessageStop : RDA_MessageHeader	// inheritance from RDA_MessageHeader
		//; Data acquisition has been stopped. // Header -> nType = 3
		{
		};

		struct RDA_MessageData32 : RDA_MessageHeader	// inheritance from RDA_MessageHeader
		//; Block of 32-bit floating point data, Header -> nType = 4, sent only from port 51244
		{
			//NeuroScan has no nBlock variables (and highly probalbe also no Markers to send -> comment them out later)
			//ULONG				nBlock;				// Block number, i.e. acquired blocks since acquisition started.
			//ULONG				nPoints;			// Number of data points in this block
			//ULONG				nMarkers;			// Number of markers in this data block
			float				fData[1];			// Data array -> float fData[nChannels * nPoints], multiplexed
			//RDA_Marker			Markers[1];			// Array of markers -> RDA_Marker Markers[nMarkers]
		};

		//Big-Endinan <-> Little-Endian, Infos , Controll of DATA
		CDriverNeuroscanSynamps2:: RDA_MessageHeader	swapIt			(RDA_MessageHeader l_structRDA_MessageHeader);
		void									showIds			(RDA_MessageHeader l_structRDA_MessageHeader);
		char*									checkIt			(RDA_MessageHeader l_structRDA_MessageHeader);
		char*									sendIt			(RDA_MessageHeader l_structRDA_MessageHeader);
		char*									receiveIt		(RDA_MessageHeader l_structRDA_MessageHeader);

		// Initialisations:
		RDA_MessageHeader* m_pStructRDA_MessageHeader;
		char* m_pcharStructRDA_MessageHeader;
		RDA_MessageStart* m_pStructRDA_MessageStart;
		RDA_MessageStop* m_pStructRDA_MessageStop;
		RDA_MessageData32* m_pStructRDA_MessageData32;
		RDA_Marker* m_pStructRDA_Marker;

	};
};
// To solve the error message "warning C4103 filename : alignment changed after including header" solution http://msdn.microsoft.com/en-us/library/t4d0762d(VS.80).aspx
#pragma pack(pop)
#endif // __OpenViBE_AcquisitionServer_CDriverNeuroscanSynamps2_H__
