#include <map>
#include <string>

#include <vrpn_Connection.h>
#include <vrpn_Button.h>

#define SSVEP_VRPN_SERVER_PORT 1337

namespace OpenViBESSVEP
{
	class CVRPNServer
	{
		public:
			static CVRPNServer* getInstance();
			~CVRPNServer();
			void processFrame();

			void addButton(std::string sName, int iButtonCount);
			void changeButtonState(std::string sName, int iIndex, int iState);

		private:
			static CVRPNServer* m_poVRPNServerInstance;

			CVRPNServer();

			vrpn_Connection* m_poConnection;
			std::map<std::string, vrpn_Button_Server*> m_oButtonServer;

	};
}


