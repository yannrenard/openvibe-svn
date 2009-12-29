#ifndef __Offline_TrialConversion_H__
#define __Offline_TrialConversion_H__

#if defined TARGET_HAS_ThirdPartyFabien && defined TARGET_HAS_ThirdPartyITPP && defined TARGET_HAS_ThirdPartyGSL

#include <openvibe-toolkit/ovtk_all.h>
#include <openvibe-toolkit/ovtkTTrainingBoxAlgorithm.h>

#include <Trial.h>

#include <vector>

namespace TrialConversion
{

	/**
	 * Converts a single OpenViBE trial into a libFab trial :D
	 * \param oTrial The trial to convert.
	 * \return The resulting trial.
	 * */
	Trial convert(const OpenViBEToolkit::ISignalTrial& oTrial);

	/**
	 * Converts a whole OpenViBE trialset into a vector of libFab's trials.
	 * \param oTrialSet The trialset to convert.
	 * \return The resulting trialset.
	 * */
	std::vector<Trial> convert(const OpenViBEToolkit::ISignalTrialSet& oTrialSet);

	/**
	 * Converts a whole OpenViBE trialset into a vector of libFab's trials and downsamples each trial.
	 * \param oTrialSet The trialset to convert.
	 * \param i32Factor The downsampling factor.
	 * \param i32WindowType The kind of window to use for the downsampling (see libFab documentation).
	 * \param i32WindowSize The window's size.
	 * \return The resulting downsampled trialset.
	 * */
	std::vector<Trial> convert(const OpenViBEToolkit::ISignalTrialSet& oTrialSet, OpenViBE::int32 i32Factor, OpenViBE::int32 i32WindowType, OpenViBE::int32 i32WindowSize);

};

#endif

#endif // __Offline_TrialConversion_H__
