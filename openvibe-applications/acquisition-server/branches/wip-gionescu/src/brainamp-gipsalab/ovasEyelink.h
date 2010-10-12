#ifndef __OpenViBEAcquisitionServer_Eyelink_H__
#define __OpenViBEAcquisitionServer_Eyelink_H__

#include <openvibe/ov_all.h>

#include <iostream>

namespace OpenViBEAcquisitionServer
{
	namespace Eyelink
	{
		typedef enum
		{	NB_SIGNALS			= 4,
			MISSING_DATA		= -32000,
			THRESHOLD_DATA		= -2000,
			INVALID_DATA		= 0x0040,
			MISSING_SAMPLE		= 0x0080,
			SERVER_PORT_FLOAT32	= 700,
		} eyelinkAcq_type;
		
		#pragma pack(push)
		#pragma pack(1)
			typedef struct
			{	OpenViBE::uint16	nbSamples;				// Hz
				OpenViBE::uint16	eyelinkSamplingRate;	// Hz
				OpenViBE::uint16	halfSynchroPeriod;		// millisec
				OpenViBE::uint16	debugMode;				// display the status 
			} eyelinkParams_type;
			
			typedef struct
			{	OpenViBE::float32	leftX, leftY;
				OpenViBE::float32	rightX, rightY;
				OpenViBE::uint16	status;	// tracker status flags
				OpenViBE::uint16	input;	// input word
			} eyelinkEvent_type;
			
			typedef struct
			{	OpenViBE::uint32	blockIndex;
				eyelinkEvent_type	events[1];
			} eyelinkData_type;
		#pragma pack(pop)
	};
};

#endif // __OpenViBEAcquisitionServer_Eyelink_H__
