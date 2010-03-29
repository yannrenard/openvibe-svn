#ifndef __OpenViBE_AcquisitionServer_CAcquisitionServerThread_H__
#define __OpenViBE_AcquisitionServer_CAcquisitionServerThread_H__

#include "ovasCAcquisitionServerGUI.h"
#include "ovasCAcquisitionServer.h"

#include <system/Time.h>

namespace OpenViBEAcquisitionServer
{
	class CAcquisitionServerThread
	{
	public:

		enum
		{
			Status_Idle,
			Status_Connected,
			Status_Started,
			Status_Finished
		};

		CAcquisitionServerThread(const OpenViBE::Kernel::IKernelContext& rKernelContext, CAcquisitionServerGUI& rGUI, CAcquisitionServer& rAcquisitionServer)
			:m_rKernelContext(rKernelContext)
			,m_rGUI(rGUI)
			,m_rAcquisitionServer(rAcquisitionServer)
			,m_ui32Status(Status_Idle)
		{
		}

		void main(void)
		{
			m_ui64InnerLoopWaitMilli=m_rKernelContext.getConfigurationManager().expandAsInteger("${AcquisitionServer_InnerLoopWaitMilliseconds}", 5);
			m_ui64OuterLoopWaitMilli=m_rKernelContext.getConfigurationManager().expandAsInteger("${AcquisitionServer_InnerLoopWaitMilliseconds}", 100);
			m_rKernelContext.getLogManager() << OpenViBE::Kernel::LogLevel_Trace << "Loop wait set to [inner:" << m_ui64InnerLoopWaitMilli << "ms][outer:" << m_ui64OuterLoopWaitMilli << "]\n";

			while(m_ui32Status!=Status_Finished)
			{

				OpenViBE::boolean l_bShouldSleep=false;

				{
					boost::mutex::scoped_lock m_oLock(m_rAcquisitionServer.m_oMutex);

					// m_rKernelContext.getLogManager() << OpenViBE::Kernel::LogLevel_Info << "CAcquisitionServerThread::main()\n";
					switch(m_ui32Status)
					{
						case Status_Idle:
							l_bShouldSleep=true;
							break;

						case Status_Connected:
						case Status_Started:
							{
								if(!m_rAcquisitionServer.loop())
								{
									m_rAcquisitionServer.stop();
									m_rAcquisitionServer.disconnect();
									m_ui32Status=Status_Finished;
								}
							}
							break;

						default:
							break;
					}
				}

				if(l_bShouldSleep)
				{
					System::Time::sleep(m_ui64OuterLoopWaitMilli);
				}
				else
				{
					System::Time::sleep(m_ui64InnerLoopWaitMilli);
				}
			}
		}

		OpenViBE::boolean connect(void)
		{
			boost::mutex::scoped_lock m_oLock(m_rAcquisitionServer.m_oMutex);

			m_rKernelContext.getLogManager() << OpenViBE::Kernel::LogLevel_Trace << "CAcquisitionServerThread::connect()\n";

			if(!m_rAcquisitionServer.connect(m_rGUI.getDriver(), m_rGUI.getSampleCountPerBuffer(), m_rGUI.getTCPPort()))
			{
				m_rAcquisitionServer.disconnect();
				return false;
			}
			else
			{
				m_ui32Status=Status_Connected;
			}
			return true;
		}

		OpenViBE::boolean start(void)
		{
			boost::mutex::scoped_lock m_oLock(m_rAcquisitionServer.m_oMutex);

			m_rKernelContext.getLogManager() << OpenViBE::Kernel::LogLevel_Trace << "CAcquisitionServerThread::start()\n";
			if(!m_rAcquisitionServer.start())
			{
				m_rAcquisitionServer.stop();
				return false;
			}
			else
			{
				m_ui32Status=Status_Started;
			}
			return true;
		}

		OpenViBE::boolean stop(void)
		{
			boost::mutex::scoped_lock m_oLock(m_rAcquisitionServer.m_oMutex);

			m_rKernelContext.getLogManager() << OpenViBE::Kernel::LogLevel_Trace << "CAcquisitionServerThread::stop()\n";
			m_rAcquisitionServer.stop();
			m_ui32Status=Status_Connected;
			return true;
		}

		OpenViBE::boolean disconnect(void)
		{
			boost::mutex::scoped_lock m_oLock(m_rAcquisitionServer.m_oMutex);

			m_rKernelContext.getLogManager() << OpenViBE::Kernel::LogLevel_Trace << "CAcquisitionServerThread::disconnect()\n";

			if(m_ui32Status==Status_Started)
			{
				m_rAcquisitionServer.stop();
			}

			m_rAcquisitionServer.disconnect();
			m_ui32Status=Status_Idle;
			return true;
		}

		OpenViBE::boolean terminate(void)
		{
			boost::mutex::scoped_lock m_oLock(m_rAcquisitionServer.m_oMutex);

			m_rKernelContext.getLogManager() << OpenViBE::Kernel::LogLevel_Trace << "CAcquisitionServerThread::terminate()\n";

			switch(m_ui32Status)
			{
				case Status_Started:
					m_rAcquisitionServer.stop();
				case Status_Connected:
					m_rAcquisitionServer.disconnect();
			}

			m_ui32Status=Status_Finished;
			return true;
		}
/*
		uint32 getStatus(void)
		{
			boost::mutex::scoped_lock m_oLock(m_rAcquisitionServer.m_oMutex);

			return m_ui32Status;
		}
*/
	protected:

		const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
		OpenViBEAcquisitionServer::CAcquisitionServerGUI& m_rGUI;
		OpenViBEAcquisitionServer::CAcquisitionServer& m_rAcquisitionServer;
		OpenViBE::uint32 m_ui32Status;
		OpenViBE::uint64 m_ui64InnerLoopWaitMilli;
		OpenViBE::uint64 m_ui64OuterLoopWaitMilli;
	};

	class CAcquisitionServerThreadHandle
	{
	public:

		CAcquisitionServerThreadHandle(OpenViBEAcquisitionServer::CAcquisitionServerThread& rAcquisitionServerThread)
			:m_rAcquisitionServerThread(rAcquisitionServerThread)
		{
		}

		void operator() (void)
		{
			m_rAcquisitionServerThread.main();
		}

	protected:

		OpenViBEAcquisitionServer::CAcquisitionServerThread& m_rAcquisitionServerThread;
	};
}

#endif // __OpenViBE_AcquisitionServer_CAcquisitionServerThread_H__
