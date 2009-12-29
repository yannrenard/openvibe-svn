#if defined TARGET_HAS_ThirdPartyFabien && defined TARGET_HAS_ThirdPartyITPP && defined TARGET_HAS_ThirdPartyGSL

#include "ovpCTrialConversion.h"

#include <system/Memory.h>
#include <stdio.h>

using namespace OpenViBE;
using namespace OpenViBE::Plugins;
using namespace OpenViBE::Kernel;
using namespace OpenViBEToolkit;
using namespace std;

#define BCICompetitionIIIbReader_UndefinedClass 0xFFFFFFFFFFLL

namespace TrialConversion
{
	Trial convert(const OpenViBEToolkit::ISignalTrial& oTrial)
	{
		uint32 l_ui32Identifier1;
		uint32 l_ui32Identifier2;

		sscanf(oTrial.getLabelIdentifier().toString(), "(%X, %X)", (unsigned int*)&l_ui32Identifier1, (unsigned int*)&l_ui32Identifier2);

		float64 l_f64Label = (CIdentifier(2) == oTrial.getLabelIdentifier()) ? 0 :(((uint64)l_ui32Identifier1<<32) + l_ui32Identifier2);
		
		Trial l_oResult(oTrial.getChannelCount(),
				oTrial.getSampleCount(),
				oTrial.getSamplingRate(),
				l_f64Label,
				NULL);

		for(uint32 i=0 ; i<oTrial.getChannelCount() ; i++)
		{
			float64 * l_pSourceBuffer = oTrial.getChannelSampleBuffer(i);

			for(uint32 s=0 ; s<oTrial.getSampleCount() ; s++)
			{
				l_oResult[s][i] = l_pSourceBuffer[s];
			}
		}
		
		return l_oResult;
	}

	std::vector<Trial> convert(const OpenViBEToolkit::ISignalTrialSet& oTrialSet)
	{
		std::vector<Trial> l_oResult;
		for(uint32 i=0 ; i<oTrialSet.getSignalTrialCount() ; i++)
		{
			l_oResult.push_back(convert(oTrialSet.getSignalTrial(i)));
		}

		return l_oResult;
	}

	std::vector<Trial> convert(const OpenViBEToolkit::ISignalTrialSet& oTrialSet, OpenViBE::int32 i32Factor, OpenViBE::int32 i32WindowType, OpenViBE::int32  i32WindowSize)
	{
		std::vector<Trial> l_oResult;
		for(uint32 i=0 ; i<oTrialSet.getSignalTrialCount() ; i++)
		{
			Trial l_oTrial = convert(oTrialSet.getSignalTrial(i));
			l_oTrial.downSample(i32Factor, i32WindowType, i32WindowSize);
			l_oResult.push_back(l_oTrial);
		}

		return l_oResult;
	}


}

#endif
