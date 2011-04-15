#ifndef __OpenViBEAcquisitionServer_Eyelink_H__
#define __OpenViBEAcquisitionServer_Eyelink_H__

#include <openvibe/ov_all.h>

#include <iostream>

namespace OpenViBEAcquisitionServer
{
	namespace Eyelink
	{
		#pragma pack(push)
		#pragma pack(1)
			typedef struct
			{	OpenViBE::uint16	ui16NbSamples;				// samples in the data block
				OpenViBE::uint16	ui16AcquiredEyes;			// 1 - left (2 channels); 2 - right (2 channels); 3 - both (4 channels)
				OpenViBE::uint16	ui16SamplingRate;			// Hz
			} header_type;
		#pragma pack(pop)
	};
};

#endif // __OpenViBEAcquisitionServer_Eyelink_H__
