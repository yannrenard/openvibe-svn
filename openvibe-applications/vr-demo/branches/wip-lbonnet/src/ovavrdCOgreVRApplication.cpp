#include "ovavrdCOgreVRApplication.h"

#include <iostream>

using namespace OpenViBEVRDemos;
using namespace Ogre;

COgreVRApplication::COgreVRApplication()
{
}

COgreVRApplication::~COgreVRApplication()
{
}
			
void COgreVRApplication::go(void)
{
	if (!this->setup()) { return; }
	std::cout<<std::endl<< "START RENDERING..."<<std::endl;
    m_poRoot->startRendering();
}

bool COgreVRApplication::setup()
{
	// Plugin config path setup
	Ogre::String pluginsPath;
	pluginsPath = "../share/openvibe-applications/vr-demo/common/Plugins.cfg";
	//plugins_d for the debug ??

	// Root creation
	m_poRoot = new Ogre::Root(pluginsPath, "ogre.cfg","Ogre.log");
	// Resource handling
	this->setupResources();
	if (!this->configure()) 
	{ 
		return false; 
	}
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups(); // load ressources

	// Scene graph and rendering initialisation
	m_poSceneManager = m_poRoot->createSceneManager("TerrainSceneManager", "DefaultSceneManager");

	m_poCamera = m_poSceneManager->createCamera("DefaultCamera");
	m_poCamera->setNearClipDistance(0.1f);
	m_poCamera->setFarClipDistance(300.0f);
	m_poCamera->setRenderingDistance(0.01f);

	// Create one viewport, entire window
    Ogre::Viewport* l_poViewPort = m_poWindow->addViewport(m_poCamera);
    l_poViewPort->setBackgroundColour(Ogre::ColourValue(0,0,0));
    // Alter the camera aspect ratio to match the viewport
    m_poCamera->setAspectRatio(Ogre::Real(l_poViewPort->getActualWidth()) / Ogre::Real(l_poViewPort->getActualHeight()));


	// Set default mipmap level (NB some APIs ignore this)
	Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

	m_poRoot->addFrameListener(this);
	this->initOIS();

	return true;
}

bool COgreVRApplication::configure()
{
	if(! m_poRoot->restoreConfig())
	{
		if( ! m_poRoot->showConfigDialog() )
		{
			return false;
		}
	}

	m_poWindow = m_poRoot->initialise(true,"VR/BCI Application - powered by OpenViBE");

	return true;
}


void COgreVRApplication::setupResources()
{
	 // Load resource paths from config file
    Ogre::ConfigFile l_oConfigFile;
    l_oConfigFile.load("../share/openvibe-applications/vr-demo/common/resources.cfg");
    // Go through all sections & settings in the file
    Ogre::ConfigFile::SectionIterator l_oSectionIterator = l_oConfigFile.getSectionIterator();

    Ogre::String l_sSecName, l_sTypeName, l_sArchName;
    while (l_oSectionIterator.hasMoreElements())
    {
        l_sSecName = l_oSectionIterator.peekNextKey();
		Ogre::ConfigFile::SettingsMultiMap *settings = l_oSectionIterator.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            l_sTypeName = i->first;
            l_sArchName = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(l_sArchName, l_sTypeName, l_sSecName);
        }
    }
}

bool COgreVRApplication::initOIS() 
{
	OIS::ParamList l_oParamList;
	size_t windowHnd = 0;
	std::ostringstream windowHndStr;
	m_bContinue = true;

	// Retrieve the rendering window
	RenderWindow* window = Ogre::Root::getSingleton().getAutoCreatedWindow();
	window->getCustomAttribute("WINDOW", &windowHnd);
	windowHndStr << windowHnd;
	l_oParamList.insert(make_pair(std::string("WINDOW"), windowHndStr.str()));
	l_oParamList.insert(std::make_pair(std::string("x11_mouse_grab"), std::string("false")));
    l_oParamList.insert(std::make_pair(std::string("x11_keyboard_grab"), std::string("false")));

	// Create the input manager
	m_poInputManager = OIS::InputManager::createInputSystem( l_oParamList );

	//Create all devices (TODO: create joystick)
	m_poKeyboard = static_cast<OIS::Keyboard*>(m_poInputManager->createInputObject( OIS::OISKeyboard, true ));
	m_poMouse = static_cast<OIS::Mouse*>(m_poInputManager->createInputObject( OIS::OISMouse, true ));

	m_poKeyboard->setEventCallback(this);
	m_poMouse->setEventCallback(this);

	std::cout<<"OIS initialised"<<std::endl;

	return true;
}

bool COgreVRApplication::keyPressed(const OIS::KeyEvent& evt)
{
	if(evt.key == OIS::KC_ESCAPE)
	{
		std::cout<<"Ending Application..."<<std::endl;
		m_bContinue = false;
	}

	return true;
}

bool COgreVRApplication::frameStarted(const FrameEvent& evt)
{
	m_poKeyboard->capture();
	m_poMouse->capture();

	return m_bContinue;
}