#ifndef __OpenViBEAcquisitionServer_Eyelink_H__
#define __OpenViBEAcquisitionServer_Eyelink_H__

#include <iostream>

namespace OpenViBEAcquisitionServer
{
	namespace Eyelink
	{
		typedef enum
		{	NB_SAMPLES			= 32,
			NB_SIGNALS			= 5,
			MISSING_DATA		= -32000,
			THRESHOLD_DATA		= -2000,
			INVALID_DATA		= 0x0001,
			MISSING_SAMPLE		= 0x0002,
			SERVER_PORT_FLOAT32	= 700,
		} eyelinkAcq_type;
		
		#pragma pack(push)
		#pragma pack(1)
			typedef struct
			{	OpenViBE::uint16	eyelinkSamplingRate;	// Hz
				OpenViBE::uint16	synchroPeriod;			// millisec
				OpenViBE::uint16	synchroMask;			// synchronisation mask
			} synchro_type;
			
			typedef struct
			{	OpenViBE::float32	leftX, leftY;
				OpenViBE::float32	rightX, rightY;
				OpenViBE::uint16	status;	// tracker status flags
				OpenViBE::uint16	input;	// input word
			} eyelinkEvent_type;
			typedef struct
			{	OpenViBE::uint32	blockIndex;
				eyelinkEvent_type	events[NB_SAMPLES];
			} eyelinkData_type;
			
			typedef struct
			{	OpenViBE::float32	leftX, leftY;
				OpenViBE::float32	rightX, rightY;
				OpenViBE::float32	synchro;
			} outData_type;
		#pragma pack(pop)
	};
};

#endif // __OpenViBEAcquisitionServer_Eyelink_H__
