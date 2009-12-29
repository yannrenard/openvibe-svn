#ifndef __OpenViBEPlugins_Algorithm_MatrixOperation_H__
#define __OpenViBEPlugins_Algorithm_MatrixOperation_H__

#include <vector>
#include <algorithm>

#include "ovp_defines.h"
#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>

#include "ovpCMatrixOperation.h"

// TODO:
// - please move the identifier definitions in ovp_defines.h
// - please include your desciptor in ovp_main.cpp

namespace OpenViBEPlugins
{	namespace WipGionescu
	{
		class CAlgorithmMatrixOperation : virtual public OpenViBEToolkit::TAlgorithm < OpenViBE::Plugins::IAlgorithm >
		{
		public:
			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
			virtual OpenViBE::boolean process(void);

			virtual OpenViBE::boolean evaluate(OpenViBE::IMatrix& rResult, const std::vector<OpenViBE::IMatrix*>& rImputs, const OpenViBE::CString& rFormula);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TAlgorithm < OpenViBE::Plugins::IAlgorithm >, OVP_ClassId_Algorithm_MatrixOperation);

		protected:

			OpenViBE::Kernel::TParameterHandler < void* >                       ip_pInputs;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::CString* >			ip_pGrammar;

			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* >          op_pResult;
		};

		class CAlgorithmMatrixOperationDesc : virtual public OpenViBE::Plugins::IAlgorithmDesc
		{
		public:
			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Matrix Operation"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Gelu Ionescu"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("Gipsa-lab"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("execute an operation betwin two matrixes"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("execute an operation betwin two matrixes element by element"); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Mathematic Tools"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("0.1"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_Algorithm_MatrixOperation; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::WipGionescu::CAlgorithmMatrixOperation; }

			virtual OpenViBE::boolean getAlgorithmPrototype(
				OpenViBE::Kernel::IAlgorithmProto& rAlgorithmPrototype) const
			{
				rAlgorithmPrototype.addInputParameter  (OVP_Algorithm_MatrixOperation_InputParameterId_Inputs,   "Inputs",     OpenViBE::Kernel::ParameterType_Pointer);
				rAlgorithmPrototype.addInputParameter  (OVP_Algorithm_MatrixOperation_InputParameterId_Grammar,  "Grammar",    OpenViBE::Kernel::ParameterType_String);
		
				rAlgorithmPrototype.addOutputParameter (OVP_Algorithm_MatrixOperation_OutputParameterId_Result,  "Result",     OpenViBE::Kernel::ParameterType_Matrix);
			
				rAlgorithmPrototype.addInputTrigger    (OVP_Algorithm_MatrixOperation_InputTriggerId_Evaluate,   "Evaluate");

				rAlgorithmPrototype.addOutputTrigger   (OVP_Algorithm_MatrixOperation_OutputTriggerId_Success,   "Success");
				rAlgorithmPrototype.addOutputTrigger   (OVP_Algorithm_MatrixOperation_OutputTriggerId_Fail,      "Fail");

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IAlgorithmDesc, OVP_ClassId_Algorithm_MatrixOperationDesc);
		};
	};
};

#endif // __OpenViBEPlugins_Algorithm_MatrixOperation_H__
