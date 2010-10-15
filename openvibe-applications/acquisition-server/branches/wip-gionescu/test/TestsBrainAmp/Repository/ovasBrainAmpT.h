#ifndef __OpenViBEAcquisitionServer_BrainAmp_H__
#define __OpenViBEAcquisitionServer_BrainAmp_H__

#include <iostream>

namespace OpenViBEAcquisitionServer
{
	namespace BrainAmp
	{
		#pragma pack(push)
		#pragma pack(1)

		typedef enum
		{	BLOCK_BEFORE	= 0,
			BLOCK_START		= 1,
			BLOCK_DATA		= 2,
			BLOCK_STOP		= 3,
			BLOCK_DATA32	= 4,
			BLOCK_INVALID	= 1000,
			BLOCK_AFTER
		};
			
		typedef struct
		{	OpenViBE::uint32	Data1;
			unsigned short		Data2;
			unsigned short		Data3;
			unsigned char		Data4[8];
		} GUID;

		class RDA_Marker
		//; A single marker in the marker array of RDA_MessageData
		{
		public:
			OpenViBE::uint32	nSize;              // Size of this marker.
			OpenViBE::uint32	nPosition;          // Relative position in the data block.
			OpenViBE::uint32	nPoints;            // Number of points of this marker
			OpenViBE::int32		nChannel;           // Associated channel number (-1 = all channels).
			char				sTypeDesc[1];       // Type, description in ASCII delimited by '\0'.
			//virtual void	Dump()
			//		{	std::cout	<< "  nSize      = " << nSize
			//						<< "  nPosition  = " << nPosition
			//						<< "  nPoints    = " << nPoints
			//						<< "  nChannel   = " << nChannel
			//						<< "  Descr      = " << sTypeDesc
			//						<< "  Info       = " << &sTypeDesc[strlen(sTypeDesc) + 1]
			//						<< std::endl;		
			//		}
		};

		class RDA_MessageHeader
		//; Message header
		{	
		public:
			GUID				guid;               // Always GUID_RDAHeader
			OpenViBE::uint32	nSize;              // Size of the message block in bytes including this header
			OpenViBE::uint32	nType;              // Message type.

			bool	IsStart()	const {	return nType == BLOCK_START;		}
			bool	IsData16()	const {	return nType == BLOCK_DATA;			}
			bool	IsData32()	const {	return nType == BLOCK_DATA32;		}
			bool	IsData()	const {	return IsData16() || IsData32();	}
			bool	IsStop()	const {	return nType == BLOCK_STOP;			}
			//virtual void	Dump()
			//	{	std::cout	<< "  nSize    = " << nSize
			//					<< "  nType    = " << nType;		
			//	}
		};

		// **** Messages sent by the RDA server to the clients. ****
		class RDA_MessageStart : public RDA_MessageHeader
		//; Setup / Start infos, Header -> nType = 1
		{	
		public:
			OpenViBE::uint32	nChannels;          // Number of channels
			OpenViBE::float64	dSamplingInterval;  // Sampling interval in microseconds
			OpenViBE::float64	dResolutions[1];    // Array of channel resolutions -> double dResolutions[nChannels] coded in microvolts. i.e. RealValue = resolution * A/D value
			char				sChannelNames[1];   // Channel names delimited by '\0'. The real size is larger than 1.
		};

		class RDA_MessageStop : public RDA_MessageHeader
		//; Data acquisition has been stopped. // Header -> nType = 3
		{
		public:
		};

		class RDA_MessageData : public RDA_MessageHeader
		//; Block of 32-bit floating point data, Header -> nType = 4, sent only from port 51244
		{	
		public:
			OpenViBE::uint32	nBlock;             // Block number, i.e. acquired blocks since acquisition started.
			OpenViBE::uint32	nPoints;            // Number of data points in this block
			OpenViBE::uint32	nMarkers;           // Number of markers in this data block
			OpenViBE::int16		nData[1];           // Data array -> float fData[nChannels * nPoints], multiplexed
			RDA_Marker			Markers[1];         // Array of markers -> RDA_Marker Markers[nMarkers]
			//virtual void	Dump()
			//		{	std::cout	<< "  nBlock   = " << nBlock
			//						<< "  nPoints  = " << nPoints
			//						<< "  nMarkers = " << nMarkers
			//						<< std::endl;		
			//		}
			};

		class RDA_MessageData32 : public RDA_MessageHeader
		//; Block of 32-bit floating point data, Header -> nType = 4, sent only from port 51244
		{	
		public:
			OpenViBE::uint32	nBlock;             // Block number, i.e. acquired blocks since acquisition started.
			OpenViBE::uint32	nPoints;            // Number of data points in this block
			OpenViBE::uint32	nMarkers;           // Number of markers in this data block
			OpenViBE::float32	fData[1];           // Data array -> float fData[nChannels * nPoints], multiplexed
			RDA_Marker			Markers[1];         // Array of markers -> RDA_Marker Markers[nMarkers]
			//virtual void	Dump()
			//		{	std::cout	<< "  nBlock   = " << nBlock
			//						<< "  nPoints  = " << nPoints
			//						<< "  nMarkers = " << nMarkers
			//						<< std::endl;		
			//		}
		};
		#pragma pack(pop)
	};
};

#ifndef DEFINE_GUIDD
	#define DEFINE_GUIDD(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
		OpenViBEAcquisitionServer::BrainAmp::GUID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#endif

#ifndef COMPARE_GUIDD
	#define COMPARE_GUIDD(name1, name2) \
		(	name1.Data1 == name2.Data1 && name1.Data2 == name2.Data2 && \
			name1.Data3 == name2.Data3 && name1.Data4[0] == name2.Data4[0] && \
			name1.Data4[1] == name2.Data4[1] && name1.Data4[2] == name2.Data4[2] &&\
			name1.Data4[3] == name2.Data4[3] && name1.Data4[4] == name2.Data4[4] &&\
			name1.Data4[5] == name2.Data4[5] && name1.Data4[6] == name2.Data4[6] &&\
			name1.Data4[7] == name2.Data4[7] \
		)
#endif

#endif // __OpenViBEAcquisitionServer_BrainAmp_H__
