///////////////////////////////////////////////////////////////////////////////////////////////////
#include "VisualizationManagerBase.h"
#include "ui_MainWindow.h"
#include "PreferencesWindow.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
VisualizationManagerBase::VisualizationManagerBase():
	myRenderWindow(NULL)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
VisualizationManagerBase::~VisualizationManagerBase()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QAction* VisualizationManagerBase::AddWindowMenuAction(QString name)
{
	QAction* action = myUI->menuWindows->addAction(name);
	action->setCheckable(true);
	return action;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void VisualizationManagerBase::Initialize()
{
	myUI = new Ui_MainWindow();
	myMainWindow = new QMainWindow();
	myUI->setupUi(myMainWindow);
	myMainWindow->setCaption("Glacier Toolset - Looking Glass " LOOKING_GLASS_VERSION);

	myRenderWindow = myUI->vtkView->GetRenderWindow();
	myRenderer = vtkRenderer::New();
	myRenderWindow->AddRenderer(myRenderer);

	// Disable preferences window in glacier mode.
	myUI->actionPreferences->setEnabled(false);

	connect(myUI->actionQuit, SIGNAL(triggered(bool)),
		SLOT(OnQuitTrigger(bool)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void VisualizationManagerBase::Update()
{
	Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void VisualizationManagerBase::Render()
{
	myUI->vtkView->GetRenderWindow()->Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void VisualizationManagerBase::Add3DWidget(vtkInteractorObserver* widget)
{
	widget->SetInteractor(myUI->vtkView->GetRenderWindow()->GetInteractor());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
vtkRenderer* VisualizationManagerBase::GetMainRenderer() 
{ 
	return myRenderer; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QMainWindow* VisualizationManagerBase::GetMainWindow() 
{ 
	return myMainWindow; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void VisualizationManagerBase::OnQuitTrigger(bool)
{
	QApplication::quit();
}
