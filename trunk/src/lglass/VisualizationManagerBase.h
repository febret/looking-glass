///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef VISUALIZATIONMANAGERBASE_H
#define VISUALIZATIONMANAGERBASE_H

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "LookingGlassSystem.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
class VisualizationManagerBase: public QObject
{
	Q_OBJECT
public:
    /////////////////////////////////////////////////////// Ctor / Dtor.
    VisualizationManagerBase();
    ~VisualizationManagerBase();

    /////////////////////////////////////////////////////// Other Methods.
    void Initialize();
	// Redraws the content of the main render window.
	void Render();
	// Updates the rendering pipeline and refreshed all the visualizations.
	virtual void Update();
	// Adds a 3D widget to the main render window.
	void Add3DWidget(vtkInteractorObserver* widget);
	// Gets the renderer associated to the main render window.
	vtkRenderer* GetMainRenderer();
	// Gets the unique instance of the main application window.
	QMainWindow* GetMainWindow();
    // Sets the render window that will be used as the main visualization viewport.
	QAction* AddWindowMenuAction(QString name);
	Ui_MainWindow* GetUI() { return myUI; }
	vtkRenderWindow* GetRenderWindow() { return myRenderWindow; }

protected slots:
	void OnQuitTrigger(bool);

private:
	// Reference to UI.
	QMainWindow* myMainWindow;
	Ui_MainWindow* myUI;

    vtkRenderer* myRenderer;
    vtkRenderWindow* myRenderWindow;
};

#endif // MAINWINDOW_H
