#ifndef __OpenViBEDesigner_CLogListenerDesigner_H__
#define __OpenViBEDesigner_CLogListenerDesigner_H__

#include "ovd_base.h"

#include <vector>
#include <map>


namespace OpenViBEDesigner
{
	class CLogListenerDesigner : public OpenViBE::Kernel::ILogListener
	{
		public:

			CLogListenerDesigner(const OpenViBE::Kernel::IKernelContext& rKernelContext, ::GtkBuilder* pBuilderInterface);

			virtual OpenViBE::boolean isActive(OpenViBE::Kernel::ELogLevel eLogLevel);
			virtual OpenViBE::boolean activate(OpenViBE::Kernel::ELogLevel eLogLevel, OpenViBE::boolean bActive);
			virtual OpenViBE::boolean activate(OpenViBE::Kernel::ELogLevel eStartLogLevel, OpenViBE::Kernel::ELogLevel eEndLogLevel, OpenViBE::boolean bActive);
			virtual OpenViBE::boolean activate(OpenViBE::boolean bActive);

			virtual void log(const OpenViBE::uint64 ui64Value);
			virtual void log(const OpenViBE::uint32 ui32Value);
			virtual void log(const OpenViBE::uint16 ui16Value);
			virtual void log(const OpenViBE::uint8 ui8Value);

			virtual void log(const OpenViBE::int64 i64Value);
			virtual void log(const OpenViBE::int32 i32Value);
			virtual void log(const OpenViBE::int16 i16Value);
			virtual void log(const OpenViBE::int8 i8Value);

			virtual void log(const OpenViBE::float64 f64Value);
			virtual void log(const OpenViBE::float32 f32Value);

			virtual void log(const OpenViBE::boolean bValue);

			virtual void log(const OpenViBE::CIdentifier& rValue);
			virtual void log(const OpenViBE::CString& rValue);
			virtual void log(const char* pValue);

			virtual void log(const OpenViBE::Kernel::ELogLevel eLogLevel);
			virtual void log(const OpenViBE::Kernel::ELogColor eLogColor);

			_IsDerivedFromClass_Final_(OpenViBE::Kernel::ILogListener, OV_UndefinedIdentifier);

			void clearMessages();
			void focusMessageWindow();

		protected:

			std::map<OpenViBE::Kernel::ELogLevel, OpenViBE::boolean> m_vActiveLevel;

		private:
			::GtkBuilder* m_pBuilderInterface;
			::GtkBuilder* m_pAlertBuilder;
			::GtkTextView* m_pTextView;
			::GtkTextBuffer* m_pBuffer;

			::GtkWindow* m_pAlertWindow;

			OpenViBE::boolean m_bIngnoreMessages;

			OpenViBE::uint32 m_ui32CountMessages;
			OpenViBE::uint32 m_ui32CountWarnings;
			OpenViBE::uint32 m_ui32CountErrors;

			void updateMessageCounts();


	};
};

#endif // __OpenViBEDesigner_CLogListenerDesigner_H__
