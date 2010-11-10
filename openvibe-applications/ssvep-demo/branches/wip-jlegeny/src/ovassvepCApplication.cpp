#include "ovassvepCApplication.h"

using namespace OpenViBESSVEP;

CApplication::CApplication()
	: m_bContinueRendering( true ),
	m_ui8CurrentFrame( 0 )
{
}

CApplication::~CApplication()
{

	delete m_poRoot;

	if (m_poPainter != NULL)
	{
		delete m_poPainter;
	}
	
}

bool CApplication::setup()
{
	CLog::log << "Setting up base SSVEP Application"; 

	// Plugin config path setup
	Ogre::String l_oPluginsPath;

#if defined OVA_OS_Windows
#if defined OVA_BUILDTYPE_Debug
	l_oPluginsPath = std::string(getenv("OGRE_HOME")) + std::string("/bin/debug/plugins_d.cfg");
#else
	l_oPluginsPath = std::string(getenv("OGRE_HOME")) + std::string("/bin/release/plugins.cfg");
#endif
#elif defined OVA_OS_Linux
	l_oPluginsPath = Ogre::String(getenv("OGRE_HOME")) + Ogre::String("/lib/OGRE/Plugins.cfg");
#else
#error "No OS defined."
#endif

	// Root creation
	m_poRoot = new Ogre::Root(l_oPluginsPath, "ogre.cfg","ogre.log");

	// Resource handling
	this->setupResources();

	// Configuration from file or dialog window if needed
	if (!this->configure())
	{
		std::cerr<<"[FAILED] The configuration process ended unexpectedly."<< std::endl;
		return false;
	}

	m_poWindow = m_poRoot->initialise(true);

	m_poSceneManager = m_poRoot->createSceneManager(Ogre::ST_GENERIC);
	m_poCamera = m_poSceneManager->createCamera("SSVEPApplicationCamera");
	m_poViewport = m_poWindow->addViewport(m_poCamera);

	m_poSceneNode = m_poSceneManager->getRootSceneNode()->createChildSceneNode("SSVEPApplicationNode");

	// initialize the paiter object
	m_poPainter = new CBasicPainter( m_poSceneManager );
}

/**
 */
bool CApplication::configure()
{
	if(! m_poRoot->restoreConfig())
	{
		if( ! m_poRoot->showConfigDialog() )
		{
			std::cerr<<"[FAILED] No configuration created from the dialog window."<< std::endl;
				return false;
		}
	}
	
	// Set hard-coded parameters, VSync in particular
	m_poRoot->getRenderSystem()->setConfigOption("VSync", "True");
	m_poRoot->getRenderSystem()->setConfigOption("Full Screen","No");  
	m_poRoot->getRenderSystem()->setConfigOption("Video Mode","300 x 300 @ 16-bit colour");

		
	return true;
}

/**
*/
void CApplication::setupResources()
{
	Ogre::ResourceGroupManager::getSingleton().createResourceGroup("SSVEP");
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("../share/openvibe-applications/ssvep-demo/resources", "FileSystem", "SSVEP");
	Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("SSVEP");
}

/**
*/
bool CApplication::frameRenderingQueued(const Ogre::FrameEvent &evt)
{
	return (m_bContinueRendering && !m_poWindow->isClosed());
}

/**
*/
bool CApplication::frameStarted(const Ogre::FrameEvent &evt)
{
	m_ui8CurrentFrame = ++m_ui8CurrentFrame % SSVEP_FPS;

	this->processFrame(m_ui8CurrentFrame);

	for (int i = 0; i < m_oCommands.size(); i++)
	{
		m_oCommands[i]->processFrame();
	}

	return true;
}

/**
*/
void CApplication::go()
{
	m_poRoot->addFrameListener(this);
	m_poRoot->startRendering();
}

/**
 */
void CApplication::addCommand(CCommand* command)
{
}
