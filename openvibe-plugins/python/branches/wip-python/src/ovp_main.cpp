#if defined TARGET_HAS_ThirdPartyPython
#include "ovpCBoxAlgorithmPython.h"

#ifdef OVP_OS_Windows
#include <windows.h>
#endif

class CPythonInitializer
{
public:
	CPythonInitializer(void);
	~CPythonInitializer(void);
	OpenViBE::boolean IsPythonAvailable(void);
private:
	//		PyThreadState *m_pMainPyThreadState;
	OpenViBE::boolean m_python_available;
};

CPythonInitializer::CPythonInitializer(void)
{
	m_python_available = false;
	//m_pMainPyThreadState = NULL;

#ifdef OVP_OS_Windows
	__try
	{
		if (!Py_IsInitialized())
		{
			//		PyEval_InitThreads();
			Py_Initialize();
			//	m_pMainPyThreadState = PyEval_SaveThread();
			m_python_available = true;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{

	}
#else
	if (!Py_IsInitialized())
	{
		//	PyEval_InitThreads();
		Py_Initialize();
		//	m_pMainPyThreadState = PyEval_SaveThread();
	}
	m_python_available = true;
#endif

	PyRun_SimpleString(
				"import sys\n"
				"sys.path.append('../share/openvibe-plugins/python')\n"
				"import openvibe\n"
				"from StimulationsCodes import *\n"
				);

}

CPythonInitializer::~CPythonInitializer(void)
{
	if (m_python_available)
	{
		//	if(m_pMainPyThreadState)
		//{
		//	PyEval_AcquireThread(m_pMainPyThreadState);
		//PyEval_AcquireLock();
		// adding openvibe.py path to the python path

		Py_Finalize();

		//	m_pMainPyThreadState = NULL;
		//}
	}
}

OpenViBE::boolean CPythonInitializer::IsPythonAvailable(void)
{
	return m_python_available;
}

OVP_Declare_Begin();
static CPythonInitializer python_initializer;
if (python_initializer.IsPythonAvailable())
{
	OVP_Declare_New(OpenViBEPlugins::Python::CBoxAlgorithmPythonDesc);
}
OVP_Declare_End();
#endif // TARGET_HAS_ThirdPartyPython
