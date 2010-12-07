#include "ovassvepCApplication.h"

using namespace OpenViBESSVEP;
using namespace OpenViBE::Kernel;

	CApplication::CApplication()
: m_bContinueRendering( true ),
	m_ui8CurrentFrame( 0 ),
	m_roGUIRenderer( NULL )
{
}

CApplication::~CApplication()
{

	if (m_poPainter != NULL)
	{
		(*m_poLogManager) << LogLevel_Debug << "- m_poPainter\n";
		delete m_poPainter;
		m_poPainter = NULL;
	}

	for (std::vector<CCommand*>::iterator it = m_oCommands.begin();
			it != m_oCommands.end(); ++it) 
	{
		(*m_poLogManager) << LogLevel_Debug << "- CCommand\n";
		if (*it != NULL)
		{
			delete *it;
			*it = NULL;
		}
	}

	
	(*m_poLogManager) << LogLevel_Debug << "- m_poRoot\n";
	if (m_poRoot != NULL)
	{
		delete m_poRoot;
		m_poRoot = NULL;
	}
	
}

bool CApplication::setup(OpenViBE::Kernel::IKernelContext* poKernelContext)
{
	m_poKernelContext = poKernelContext;
	m_poLogManager = &(m_poKernelContext->getLogManager());

	(*m_poLogManager) << LogLevel_Info << "  * CApplication::setup()\n";

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

	// Create LogManager to stop Ogre flooding the console and creating random files
	
	Ogre::LogManager* l_poLogManager = new Ogre::LogManager();
	l_poLogManager->createLog("Ogre.log", true, false, true );

	// Root creation
	(*m_poLogManager) << LogLevel_Debug << "+ m_poRoot = new Ogre::Root(...)\n";
	m_poRoot = new Ogre::Root(l_oPluginsPath, "ogre.cfg","ogre.log");

	// Resource handling
	this->setupResources();

	// Configuration from file or dialog window if needed
	if (!this->configure())
	{
		(*m_poLogManager) << LogLevel_Error << "[FAILED] The configuration process ended unexpectedly.\n";
		return false;
	}

	m_poWindow = m_poRoot->initialise(true);

	m_poSceneManager = m_poRoot->createSceneManager(Ogre::ST_GENERIC);
	m_poCamera = m_poSceneManager->createCamera("SSVEPApplicationCamera");
	m_poViewport = m_poWindow->addViewport(m_poCamera);

	m_poSceneNode = m_poSceneManager->getRootSceneNode()->createChildSceneNode("SSVEPApplicationNode");

	// initialize the paiter object
	(*m_poLogManager) << LogLevel_Debug << "+ m_poPainter = new CBasicPainter(...)\n";
	m_poPainter = new CBasicPainter( m_poSceneManager );

	this->initCEGUI();

	return true;
}

bool CApplication::configure()
{
	if(! m_poRoot->restoreConfig())
	{
		if( ! m_poRoot->showConfigDialog() )
		{
			(*m_poLogManager) << LogLevel_Error << "[FAILED] No configuration created from the dialog window.\n";
			return false;
		}
	}

	// Set hard-coded parameters, VSync in particular
	m_poRoot->getRenderSystem()->setConfigOption("VSync", "True");
	m_poRoot->getRenderSystem()->setConfigOption("Full Screen","No");  
	m_poRoot->getRenderSystem()->setConfigOption("Video Mode","640 x 480 @ 16-bit colour");


	return true;
}


void CApplication::initCEGUI()
{
	m_roGUIRenderer = &(CEGUI::OgreRenderer::bootstrapSystem());
	CEGUI::SchemeManager::getSingleton().create((CEGUI::utf8*)"TaharezLook-ov.scheme");

	m_poGUIWindowManager = CEGUI::WindowManager::getSingletonPtr();
	m_poSheet = m_poGUIWindowManager->createWindow("DefaultWindow", "Sheet");

	CEGUI::System::getSingleton().setGUISheet(m_poSheet);
}


void CApplication::setupResources()
{
	Ogre::ResourceGroupManager::getSingleton().createResourceGroup("SSVEP");
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("../share/openvibe-applications/ssvep-demo/resources", "FileSystem", "SSVEP");
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("../share/openvibe-applications/ssvep-demo/resources/Trainer", "FileSystem", "SSVEPTrainer");
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("../share/openvibe-applications/ssvep-demo/resources/GUI", "FileSystem", "CEGUI");
	Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("SSVEP");
	Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("SSVEPTrainer");
	Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("CEGUI");
}

bool CApplication::frameRenderingQueued(const Ogre::FrameEvent &evt)
{
	return (m_bContinueRendering && !m_poWindow->isClosed());
}

bool CApplication::frameStarted(const Ogre::FrameEvent &evt)
{
	m_ui8CurrentFrame = ++m_ui8CurrentFrame % SSVEP_FPS;

	this->processFrame(m_ui8CurrentFrame);

	for (OpenViBE::uint8 i = 0; i < m_oCommands.size(); i++)
	{
		m_oCommands[i]->processFrame();
	}

	return true;
}

void CApplication::go()
{
	m_poRoot->addFrameListener(this);
	m_poRoot->startRendering();
}

void CApplication::addCommand(CCommand* pCommand)
{
	m_oCommands.push_back(pCommand);
}

void CApplication::startExperiment()
{
	(*m_poLogManager) << LogLevel_Info << "[!] Experiment starting\n";
}

void CApplication::stopExperiment()
{
	(*m_poLogManager) << LogLevel_Info << "[!] Experiment halting\n";
	this->exit();
}

