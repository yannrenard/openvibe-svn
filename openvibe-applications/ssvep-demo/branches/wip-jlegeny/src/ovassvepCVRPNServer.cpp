#include "ovassvepCVRPNServer.h"

using namespace OpenViBESSVEP;

CVRPNServer* CVRPNServer::m_poVRPNServerInstance = NULL;

CVRPNServer::CVRPNServer()
{
	m_poConnection = vrpn_create_server_connection(SSVEP_VRPN_SERVER_PORT);
}

CVRPNServer* CVRPNServer::getInstance()
{
	if (m_poVRPNServerInstance == NULL)
	{
		m_poVRPNServerInstance = new CVRPNServer();
	}

	return m_poVRPNServerInstance;
}

void CVRPNServer::addButton(std::string sName, int iButtonCount)
{
	m_oButtonServer.insert(std::pair<std::string, vrpn_Button_Server*>(sName, new vrpn_Button_Server(sName.data(), m_poConnection, iButtonCount)));
	m_oButtonCache[sName].clear();
	m_oButtonCache[sName].resize(iButtonCount);
}

void CVRPNServer::processFrame()
{
	for (std::map<std::string, vrpn_Button_Server*>::iterator it = m_oButtonServer.begin(); it != m_oButtonServer.end(); ++it)
	{
		it->second->mainloop();
	}

	m_poConnection->mainloop();
}

void CVRPNServer::changeButtonState(std::string sName, int iIndex, int iState)
{
	m_oButtonServer[sName]->set_button(iIndex, iState);
	m_oButtonCache[sName][iIndex] = iState;
}

int CVRPNServer::getButtonState(std::string sName, int iIndex)
{
	return m_oButtonCache[sName][iIndex];
}
