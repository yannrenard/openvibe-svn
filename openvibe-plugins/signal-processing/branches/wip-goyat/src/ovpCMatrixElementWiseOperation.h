#ifndef __OpenViBEPlugins_MatrixElementWiseOperation_H__
#define __OpenViBEPlugins_MatrixElementWiseOperation_H__

#include "ovp_defines.h"
#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>

#include <map>										// map
#include <boost/spirit/tree/ast.hpp>                  // ast_parse
#include <boost/function.hpp>                         // function<>

typedef boost::spirit::tree_match<const char*>    treematch_t;
typedef treematch_t::tree_iterator tree_iter_t;

namespace OpenViBEPlugins
{	namespace WipGionescu
	{
		class CMatrixElementWiseOperation
		{
		public:
			CMatrixElementWiseOperation(void);

			bool parse(const int nbEntries, const char* formula, const bool debug, OpenViBE::Kernel::ILogManager & log);
			bool evaluate(OpenViBE::IMatrix& rResult, const std::vector<OpenViBE::IMatrix*>& rImputs, const OpenViBE::CString& rFormula,OpenViBE::Kernel::ILogManager & log);
			void debug(const std::vector<OpenViBE::IMatrix*>& rImputs, const OpenViBE::IMatrix& rResult,OpenViBE::Kernel::ILogManager & log);

		private:
		OpenViBE::float64 evaluate(const tree_iter_t& i);
		OpenViBE::float64 evaluate(boost::spirit::tree_parse_info<> t);
		};

	};
};

#endif // __OpenViBEPlugins_MatrixElementWiseOperation_H__
