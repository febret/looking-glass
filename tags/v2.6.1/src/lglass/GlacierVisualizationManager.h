///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef GLACIERVISUALIZATIONMANAGER_H
#define GLACIERVISUALIZATIONMANAGER_H

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "LookingGlassSystem.h"
#include "VisualizationManagerBase.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_PLOT_VIEWS 4
#define MAX_THRESHOLD_STAGES 4

///////////////////////////////////////////////////////////////////////////////////////////////////
class GlacierVisualizationManager: public VisualizationManagerBase
{
	Q_OBJECT
public:
    /////////////////////////////////////////////////////// Ctor / Dtor.
    GlacierVisualizationManager();
    ~GlacierVisualizationManager();

    /////////////////////////////////////////////////////// Other Methods.
    void Initialize();
	// Updates the rendering pipeline and refreshed all the visualizations.
	void Update();
	// Sets the depth scale.
	void SetDepthScale(int value);
    // Sets the render window that will be used as the main visualization viewport.
	QAction* AddWindowMenuAction(QString name);
    // Sets the render window that will be used as the main visualization viewport.
    //void SetRenderWindow(vtkRenderWindow* renderWindow) {myRenderWindow = renderWindow;}
	void SetSurfaceOpacity(int value);
	void SetSonarDataVisibility(bool visible);

protected slots:
	void OnQuitTrigger(bool);

private:
    void SetupUI();
	void LoadSonarData();
	void LoadSurfaceData();
	void GenerateSurfaceData();

private:
	// Reference to UI.
	//QMainWindow* myMainWindow;
	//Ui_MainWindow* myUI;

	// Tools and Windows
	GlacierViewOptions* myViewOptions;
	vtkTransform* myTransformDef;
	vtkTransform* mySurfaceTransformDef;

	// Main sonde data view.
	vtkUnstructuredGridReader* myGlacierPointsReader;
	vtkTransformFilter* mySonarTransform;
    vtkDataSetMapper* myGlacierPointsMapper;
    vtkActor* myGlacierPointsActor;

	// Glacier surface.
	vtkPolyDataReader* myGlacierSurfaceReader;
    vtkPolyDataMapper* myGlacierSurfaceMapper;
	vtkTransformFilter* mySurfaceTransform;
    vtkActor* myGlacierSurfaceActor;

    vtkRenderer* myRenderer;
    vtkRenderWindow* myRenderWindow;
};

#endif // MAINWINDOW_H
