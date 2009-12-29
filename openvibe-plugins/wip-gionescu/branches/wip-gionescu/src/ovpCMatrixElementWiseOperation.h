#ifndef __OpenViBEPlugins_MatrixElementWiseOperation_H__
#define __OpenViBEPlugins_MatrixElementWiseOperation_H__

#include "ovp_defines.h"
#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>

// TODO:
// - please move the identifier definitions in ovp_defines.h
// - please include your desciptor in ovp_main.cpp

namespace OpenViBEPlugins
{	namespace WipGionescu
	{
		class CMatrixElementWiseOperation
		{
		public:
			CMatrixElementWiseOperation(void);

			static bool parse(const int nbEntries, const char* formula, const bool debug = false);
			static bool evaluate(OpenViBE::IMatrix& rResult, const std::vector<OpenViBE::IMatrix*>& rImputs, const OpenViBE::CString& rFormula);
			static void debug(const std::vector<OpenViBE::IMatrix*>& rImputs, const OpenViBE::IMatrix& rResult);
		};

	};
};

#endif // __OpenViBEPlugins_MatrixElementWiseOperation_H__
