#ifndef __OpenViBEPlugins_Algorithm_MatrixElementWiseOperation_H__
#define __OpenViBEPlugins_Algorithm_MatrixElementWiseOperation_H__

#include <vector>
#include <algorithm>

#include "ovp_defines.h"
#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>

#include "ovpCMatrixElementWiseOperation.h"

namespace OpenViBEPlugins
{	
	namespace WipGionescu
	{
		class CAlgorithmMatrixElementWiseOperation : virtual public OpenViBEToolkit::TAlgorithm < OpenViBE::Plugins::IAlgorithm >
		{
		public:
			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
			virtual OpenViBE::boolean process(void);

			virtual OpenViBE::boolean evaluate(OpenViBE::IMatrix& rResult, const std::vector<OpenViBE::IMatrix*>& rImputs, const OpenViBE::CString& rFormula);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TAlgorithm < OpenViBE::Plugins::IAlgorithm >, OVP_ClassId_Algorithm_MatrixElementWiseOperation);

		protected:

			OpenViBE::Kernel::TParameterHandler < void* >                       ip_pInputs;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::CString* >			ip_pGrammar;
			OpenViBE::Kernel::TParameterHandler < CMatrixElementWiseOperation* >ip_pParser;
			CMatrixElementWiseOperation*	m_parser;

			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* >          op_pResult;
			
		private:
		
			//static bool parse(const int nbEntries, const char* formula, const bool debug = false);
			//static bool evaluate(OpenViBE::IMatrix& rResult, const std::vector<OpenViBE::IMatrix*>& rImputs, const OpenViBE::CString& rFormula);
			//static void debug(const std::vector<OpenViBE::IMatrix*>& rImputs, const OpenViBE::IMatrix& rResult);

		};

		class CAlgorithmMatrixElementWiseOperationDesc : virtual public OpenViBE::Plugins::IAlgorithmDesc
		{
		public:
			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Matrix Operation"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Gelu Ionescu + Matthieu Goyat"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("Gipsa-lab"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("execute an operation betwin two matrixes"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("execute an operation betwin two matrixes element by element"); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Signal processing/Mathematic Tools"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("0.1"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_Algorithm_MatrixElementWiseOperation; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::WipGionescu::CAlgorithmMatrixElementWiseOperation; }

			virtual OpenViBE::boolean getAlgorithmPrototype(
				OpenViBE::Kernel::IAlgorithmProto& rAlgorithmPrototype) const
			{
				rAlgorithmPrototype.addInputParameter  (OVP_Algorithm_MatrixElementWiseOperation_InputParameterId_Inputs,   "Inputs",     OpenViBE::Kernel::ParameterType_Pointer);
				rAlgorithmPrototype.addInputParameter  (OVP_Algorithm_MatrixElementWiseOperation_InputParameterId_Grammar,  "Grammar",    OpenViBE::Kernel::ParameterType_String);
				rAlgorithmPrototype.addInputParameter  (OVP_Algorithm_MatrixElementWiseOperation_InputParameterId_ParserOperator,   "parser",     OpenViBE::Kernel::ParameterType_Pointer);  	
		
				rAlgorithmPrototype.addOutputParameter (OVP_Algorithm_MatrixElementWiseOperation_OutputParameterId_Result,  "Result",     OpenViBE::Kernel::ParameterType_Matrix);
			
				rAlgorithmPrototype.addInputTrigger    (OVP_Algorithm_MatrixElementWiseOperation_InputTriggerId_Evaluate,   "Evaluate");

				rAlgorithmPrototype.addOutputTrigger   (OVP_Algorithm_MatrixElementWiseOperation_OutputTriggerId_Success,   "Success");
				rAlgorithmPrototype.addOutputTrigger   (OVP_Algorithm_MatrixElementWiseOperation_OutputTriggerId_Fail,      "Fail");

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IAlgorithmDesc, OVP_ClassId_Algorithm_MatrixElementWiseOperationDesc);
		};
	};
};

#endif // __OpenViBEPlugins_Algorithm_MatrixElementWiseOperation_H__
