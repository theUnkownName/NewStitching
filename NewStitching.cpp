#include "NewStitching.h"

#define widhtGrid 3
#define heightGrid 3
#define heightCell 20
#define widthCell 20


NewStitching::NewStitching()
  : mShutdown(false),
    mRoot(0),
    mCameraFront(0),
    mSceneMgr(0),
    mWindow(0),
    mResourcesCfg(Ogre::StringUtil::BLANK),
    mPluginsCfg(Ogre::StringUtil::BLANK),
    mCameraMan(0),
    mMouse(0),
    mKeyboard(0),
    mInputMgr(0)
{
}
 
NewStitching::~NewStitching()
{
  if (mCameraMan) delete mCameraMan;
 
  Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
  windowClosed(mWindow);
 
  delete mRoot;
}
 
void NewStitching::go()
{
  #ifdef _DEBUG
    mResourcesCfg = "resources_d.cfg";
    mPluginsCfg = "plugins_d.cfg";
  #else
    mResourcesCfg = "resources.cfg";
    mPluginsCfg = "plugins.cfg";
  #endif
 
  if (!setup())
    return;
 
  mRoot->startRendering();
 
  destroyScene();
}
 
bool NewStitching::frameRenderingQueued(const Ogre::FrameEvent& fe)
{
  if (mKeyboard->isKeyDown(OIS::KC_ESCAPE))
	  NewStitching::mShutdown = true;
 
  if (mShutdown)
    return false;
 
  if (mWindow->isClosed())
    return false;
 
    //Fix for 1.9
	//Need to capture/update each device
    /*mKeyboard->capture();
    mMouse->capture();*/
	mInputContext.capture();

 
  mCameraMan->frameRenderingQueued(fe);
 
  //CEGUI::System::getSingleton().injectTimePulse(fe.timeSinceLastFrame);
 
  if (!processUnbufferedInput(fe))
    return false;

  return true;
}

bool NewStitching::keyPressed(const OIS::KeyEvent& ke)
{
  // CEGUI::GUIContext& context = CEGUI::System::getSingleton().getDefaultGUIContext();
  // context.injectKeyDown((CEGUI::Key::Scan)ke.key);
  // context.injectChar((CEGUI::Key::Scan)ke.text);
 
  mCameraMan->injectKeyDown(ke);

  return true;
}
 
bool NewStitching::keyReleased(const OIS::KeyEvent& ke)
{
  // CEGUI::GUIContext& context = CEGUI::System::getSingleton().getDefaultGUIContext();
  // context.injectKeyUp((CEGUI::Key::Scan)ke.key);
 
  mCameraMan->injectKeyUp(ke);
 
  return true;
}
 
bool NewStitching::mouseMoved(const OIS::MouseEvent& me)
{
  // CEGUI::GUIContext& context = CEGUI::System::getSingleton().getDefaultGUIContext();
  // context.injectMouseMove(me.state.X.rel, me.state.Y.rel);
 
  mCameraMan->injectMouseMove(me);
 
  return true;
}
  
bool NewStitching::mousePressed(const OIS::MouseEvent& me, OIS::MouseButtonID id)
{
  // CEGUI::GUIContext& context = CEGUI::System::getSingleton().getDefaultGUIContext();
  // context.injectMouseButtonDown(convertButton(id));
 
  mCameraMan->injectMouseDown(me, id);
 
  return true;
}
 
bool NewStitching::mouseReleased(const OIS::MouseEvent& me, OIS::MouseButtonID id)
{
  // CEGUI::GUIContext& context = CEGUI::System::getSingleton().getDefaultGUIContext();
  // context.injectMouseButtonUp(convertButton(id));
 
  mCameraMan->injectMouseUp(me, id);
 
  return true;
}
 
void NewStitching::windowResized(Ogre::RenderWindow* rw)
{
  unsigned int width, height, depth;
  int left, top;
  rw->getMetrics(width, height, depth, left, top);
 
  const OIS::MouseState& ms = mMouse->getMouseState();
  ms.width = width;
  ms.height = height;
}
 
void NewStitching::windowClosed(Ogre::RenderWindow* rw)
{
  if (rw == mWindow)
  {
    if (mInputMgr)
    {
      mInputMgr->destroyInputObject(mMouse);
      mInputMgr->destroyInputObject(mKeyboard);
 
      OIS::InputManager::destroyInputSystem(mInputMgr);
      mInputMgr = 0;
    }
  }
}
 
bool NewStitching::setup()
{
  mRoot = new Ogre::Root(mPluginsCfg);
 
  setupResources();
 
  if (!configure())
    return false;
 
  chooseSceneManager();
  createCamera();
  createViewports();
 
  Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
 
  createResourceListener();
  loadResources();
 
  //setupCEGUI();
 
  createScene();
 
  createFrameListener();
 
  return true;
}
 
bool NewStitching::configure()
{
  if (!(mRoot->restoreConfig() || mRoot->showConfigDialog()))
  {
    return false;
  }
 
  mWindow = mRoot->initialise(true, "Triangles");
 
  return true;
}
 
void NewStitching::chooseSceneManager()
{
  mSceneMgr = mRoot->createSceneManager(Ogre::ST_EXTERIOR_CLOSE);



  //Fix for 1.9
  //Needed cause since 1.9 OverlaySystem is its own component
  mOverlaySystem = new Ogre::OverlaySystem();
    mSceneMgr->addRenderQueueListener(mOverlaySystem);
}
 
void NewStitching::createCamera()
{
  mCameraFront = mSceneMgr->createCamera("FrontCam");
  mCameraFront->setPosition(Ogre::Vector3(30, 30, 160));
  mCameraFront->lookAt(Ogre::Vector3(30, 30, 0));
  mCameraFront->setNearClipDistance(5);

  mCameraSideView = mSceneMgr->createCamera("SideView");
  mCameraSideView->setPosition(Ogre::Vector3(-50, 30, 0 ));
  mCameraSideView->lookAt(Ogre::Vector3(30,30,0));
  mCameraSideView->setNearClipDistance(5);

  mCameraMan = new OgreBites::SdkCameraMan(mCameraFront);

}
 
void NewStitching::createScene()
{
	mSceneMgr->setAmbientLight(Ogre::ColourValue(.25, .25, .25)); //Turn on the lights
	
	Ogre::Light* pointLight = mSceneMgr->createLight("PointLight");
	pointLight->setType(Ogre::Light::LT_POINT);
	pointLight->setPosition(250, 150, 250);
	pointLight->setDiffuseColour(Ogre::ColourValue::White);
    pointLight->setSpecularColour(Ogre::ColourValue::White);

	createGrid(heightGrid, widhtGrid);
	createTemplate();
	createPatches();
	clickM = false;

}

void NewStitching::createGrid(int height_grid, int width_grid)
{
	Ogre::ManualObject* line = mSceneMgr->createManualObject("line");
	line->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_LIST);

	//int numberOfCells = height * width;
	//Vertical lines
	line->position(0, 0, 0);
	line->position(0, 60, 0);
	line->position(20, 0, 0);
	line->position(20, 60, 0);
	line->position(40, 0, 0);
	line->position(40, 60, 0);
	line->position(60, 0, 0);
	line->position(60, 60, 0);
	
	//Horizontal lines
	line->position(0, 0, 0);
	line->position(60, 0, 0);
	line->position(0, 20, 0);
	line->position(60, 20, 0);
	line->position(0, 40, 0);
	line->position(60, 40, 0);
	line->position(0, 60, 0);
	line->position(60, 60, 0);
	
	line->end();
	Ogre::SceneNode* grid = mSceneMgr->getRootSceneNode()->createChildSceneNode("grid");
	grid->attachObject(line);

}

void NewStitching::createTemplate()
{
	Ogre::Quaternion rotation(Ogre::Degree(-180), Ogre::Vector3::UNIT_Y); 
	Ogre::Quaternion rotation2(Ogre::Degree(180), Ogre::Vector3::UNIT_Z); 
	Ogre::Vector3 scale(0.5, 0.5, 0.5);

	Ogre::Entity *targetPatch = mSceneMgr->createEntity("target", "mm.mesh");
	Ogre::SceneNode* targetNode = mSceneMgr->getSceneNode("grid")->createChildSceneNode();
	targetNode->scale(scale);
	targetNode->translate(30,30,0);
	targetNode->rotate(rotation, Ogre::Node::TransformSpace::TS_LOCAL); //Because Blender export them with a different orientation
	targetNode->rotate(rotation2, Ogre::Node::TransformSpace::TS_LOCAL);
	targetNode->attachObject(targetPatch);

}

void NewStitching::createPatches()
{	
	Ogre::SceneNode* patches = mSceneMgr->getRootSceneNode()->createChildSceneNode("patches");

	for (int numberOfPatch = 0; numberOfPatch < 8; numberOfPatch++) // Create 7 patches
	{
		Patch* patch = new Patch(false, numberOfPatch, mSceneMgr);
		_patches.push_back(patch);
	}
}

bool NewStitching::processUnbufferedInput(const Ogre::FrameEvent& fe)
{
	static bool mouseDownLastFrame = false;
	mouseFlag = false;
	bool leftMouseDown = mMouse->getMouseState().buttonDown(OIS::MB_Left);

	if (leftMouseDown && !mouseDownLastFrame && !mouseFlag)
	{
		startAnimation();
	}
	mouseDownLastFrame = leftMouseDown;	  
	return true;
}

void NewStitching::startAnimation()
{
	int numberOfTotalCells = widhtGrid * heightGrid;
	int numberOfPossibleCells = numberOfTotalCells-1;
	Grid* grid = new Grid(widhtGrid, heightGrid);												//Create the grid
	Patch* target = new Patch(true, mSceneMgr->getEntity("target"));							//Create the template
	Patch* bestPatch;
	bestErrorOfPatch bestPatchInGrid;

	for (int cells_checked = 0; cells_checked < numberOfPossibleCells; cells_checked++)
	{
		for (int patchId = 0; patchId < _patches.size(); patchId++)									//Are all patches checked?
		{
			if (_patches[patchId]->available == true)												//If this patch is not in place already
				grid->transverseGrid(_patches[patchId], target, mSceneMgr, mRoot, mDetailsPanel, patchId, numberOfTotalCells-1);
		}		
		bestPatchInGrid = grid->bestFitInGrid(grid->_bestFitOfPatch);								//Retrieves the best patch for all the grid
		bestPatch = _patches[bestPatchInGrid.patchId];												//I cant put the patch directly in the struct (dont know why) so i look for the identifier of the patch
		bestPatch->translatePatchDeffinitve(mSceneMgr, bestPatchInGrid,bestPatchInGrid.cell->c_centerX, bestPatchInGrid.cell->c_centerY);	//Translate the patch to the best position
		mRoot->renderOneFrame();
		bestPatchInGrid.cell->updateCell(bestPatch);
	}
	mouseFlag = true;

}

std::pair<int, int> NewStitching::retrieveXY(int x, int y)
{
	switch (x)	{	
	case(0):x = 10;	break;
	case(1):x = 30; break;
	case(2):x = 50;	break;
	default:x= 0; break;
	}

	switch (y)	{
	case(0):y = 10;break;
	case(1):y = 30;break;
	case(2):y = 50;break;
	default:y = 0;break;
	}

	return std::make_pair(x, y);
}

void NewStitching::destroyScene()
{
}
 
void NewStitching::createFrameListener(void)
{
  Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
  OIS::ParamList pl;
  size_t windowHnd = 0;
  std::ostringstream windowHndStr;
 
  mWindow->getCustomAttribute("WINDOW", &windowHnd);
  windowHndStr << windowHnd;
  pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
 
  mInputMgr = OIS::InputManager::createInputSystem(pl);
 
  mKeyboard = static_cast<OIS::Keyboard*>(mInputMgr->createInputObject(OIS::OISKeyboard, true));
  mMouse = static_cast<OIS::Mouse*>(mInputMgr->createInputObject(OIS::OISMouse, true));

  //Fix for 1.9
  mInputContext.mKeyboard = mKeyboard;
  mInputContext.mMouse = mMouse;

  mKeyboard->setEventCallback(this);
  mMouse->setEventCallback(this);
 
  windowResized(mWindow);
 
  Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);  

  
  //Error panel
  mTrayMgr = new OgreBites::SdkTrayManager("InterfaceName", mWindow, mInputContext, this);
  mTrayMgr->hideCursor();

  // create a panel to dsiplay the error and angle
  Ogre::StringVector items;
  items.push_back("Patch Side:  ");
  items.push_back("Target Side: ");
  items.push_back("Error:       ");
  items.push_back("Camera Position:       ");
  items.push_back("Camera Orientation:       ");
  mDetailsPanel = mTrayMgr->createParamsPanel(OgreBites::TL_NONE, "DetailsPanel", 200, items);
  mRoot->addFrameListener(this);
   	
}

void NewStitching::createViewports()
{
	//Ogre::Viewport* vp = mWindow->addViewport(mCameraFront,1,0.5,0,0.5,0.5);
	Ogre::Viewport* vp = mWindow->addViewport(mCameraFront);
	vp->setBackgroundColour(Ogre::ColourValue(0, 0, 0));
	    mCameraFront->setAspectRatio(
		Ogre::Real(vp->getActualWidth()) /
		Ogre::Real(vp->getActualHeight()));


	Ogre::Viewport* sideLeftVP = mWindow->addViewport(mCameraSideView,2,0.1,0.2,0.1,0.8); //Side Camera
	sideLeftVP->setBackgroundColour(Ogre::ColourValue(0,0,0));
	sideLeftVP->setOverlaysEnabled(false); 
	mCameraSideView->setAspectRatio(
		Ogre::Real(sideLeftVP->getActualWidth()) /
		Ogre::Real(sideLeftVP->getActualHeight()));

}

//void NewStitching::dualViewPort(Ogre::SceneManager *primarySceneMgr, Ogre::SceneManager *secondarySceneMgr)
//{
//    mWindow->removeAllViewports();
// 
//    //Ogre::Viewport *vp = 0;
//   
//	Ogre::Viewport* vp = mWindow->addViewport(mCameraFront);
//	Ogre::Camera *cam = primarySceneMgr->getCamera("PlayerCam");
//    vp = mWindow->addViewport(cam);
//    vp->setBackgroundColour(Ogre::ColourValue(0,0,0));
//    cam->setAspectRatio(Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));
// 
//    cam = secondarySceneMgr->getCamera("SideView");
//    vp = mWindow->addViewport(cam, 1, 0.8, 0, 1, 1);
//    vp->setBackgroundColour(Ogre::ColourValue(1,1,1));
//    cam->setAspectRatio(Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));
//}

 
void NewStitching::setupResources()
{
  Ogre::ConfigFile cf;
  cf.load(mResourcesCfg);
 
  Ogre::String secName, typeName, archName;
  Ogre::ConfigFile::SectionIterator secIt = cf.getSectionIterator();
 
  while (secIt.hasMoreElements())
  {
    secName = secIt.peekNextKey();
    Ogre::ConfigFile::SettingsMultiMap* settings = secIt.getNext();
    Ogre::ConfigFile::SettingsMultiMap::iterator setIt;
 
    for (setIt = settings->begin(); setIt != settings->end(); ++setIt)
    {
      typeName = setIt->first;
      archName = setIt->second;
      Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
	archName, typeName, secName);
    }
  }
}
 
void NewStitching::createResourceListener()
{
}
 
void NewStitching::loadResources()
{
  Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}
 
/*bool NewStitching::setupCEGUI()
{
  Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing CEGUI ***");
 
  mRenderer = &CEGUI::OgreRenderer::bootstrapSystem();
 
  CEGUI::ImageManager::setImagesetDefaultResourceGroup("Imagesets");
  CEGUI::Font::setDefaultResourceGroup("Fonts");
  CEGUI::Scheme::setDefaultResourceGroup("Schemes");
  CEGUI::WidgetLookManager::setDefaultResourceGroup("LookNFeel");
  CEGUI::WindowManager::setDefaultResourceGroup("Layouts");
 
  CEGUI::SchemeManager::getSingleton().createFromFile("TaharezLook.scheme");
  CEGUI::FontManager::getSingleton().createFromFile("DejaVuSans-10.font");
 
  CEGUI::GUIContext& context = CEGUI::System::getSingleton().getDefaultGUIContext();
 
  context.setDefaultFont("DejaVuSans-10");
  context.getMouseCursor().setDefaultImage("TaharezLook/MouseArrow");
 
  Ogre::LogManager::getSingletonPtr()->logMessage("Finished");
 
  return true;
}*/
  
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
  #define WIN32_LEAN_AND_MEAN
  #include "windows.h"
#endif
 
#ifdef __cplusplus
  extern "C" {
#endif
 
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
  INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
  int main(int argc, char *argv[])
#endif
{
  NewStitching app;

  try
  {
    app.go();
  }
  catch(Ogre::Exception& e)
  {
    #if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
      MessageBox(
	NULL,
	e.getFullDescription().c_str(),
	"An exception has occured!",
	MB_OK | MB_ICONERROR | MB_TASKMODAL);
    #else
      std::cerr << "An exception has occured: " <<
	e.getFullDescription().c_str() << std::endl;
    #endif
  }
 
  return 0;
}
 
#ifdef __cplusplus
  }
#endif
