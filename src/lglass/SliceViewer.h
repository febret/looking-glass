///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef SLICEVIEW_H
#define SLICEVIEW_H

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "LookingGlassSystem.h"
#include "ui_SliceViewerWindow.h"
#include "DockedTool.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Contains objects and data used to visualize a single slice.
class Slice
{
public:
    /////////////////////////////////////////////////////// Ctor / Dtor.
	Slice(SliceViewer* owner);
	~Slice();

    /////////////////////////////////////////////////////// Other methods.
	void Initialize();
	void Update();
	void SetInput(vtkDataObject* input);
	vtkRenderer* GetRenderer() { return myRenderer; }
	void SetDepth(float depth);

private:
	SliceViewer* myOwner;
	vtkRenderer* myRenderer;
    vtkPlane* myPlane;
    vtkCutter* myPlaneProbe;
    vtkDataSetMapper* myProbeMapper;
    vtkActor* myBathyActor;
    vtkActor* myProbeActor;

	// Slice contours.
	vtkContourFilter* myContours;
	vtkPolyDataMapper* myContourMapper;
	vtkActor* myContourActor;

	// Caption
	vtkTextActor* myCaption;
	char myCaptionText[256];
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class SliceViewer: public QWidget
{
	Q_OBJECT
public:
	/////////////////////////////////////////////////////// Public constats.
	static const int MAX_SLICES = 9;

    /////////////////////////////////////////////////////// Ctor / Dtor.
    SliceViewer(VisualizationManager* msg);
    ~SliceViewer();

    /////////////////////////////////////////////////////// Other Methods.
    void Initialize();
	void UpdateDepth();
	int GetSelectedField() { return mySelectedField; }
	bool GetContoursEnabled() { return myContoursEnabled; }
	int GetNumContours() { return myNumContours; }
	VisualizationManager* GetVisualizationManager() { return myVizMng; }
	vtkPolyDataMapper* GetBathyContourMapper() { return myBathyContourMapper; }
    //void SetColorFunction(vtkColorTransferFunction* fx);
	//void SetDepthScale(int value);

protected slots:
	void Show();
	void SetNumSlices(int slices);
	void SetStartDepth(int depth);
	void SetEndDepth(int depth);
	void SetNumContours(int depth);
	void OnContoursButtonToggle(bool enabled);
	void OnSelectedFieldChanged(int i);
	void OnSliceButtonToggle(bool);

private:
	void SetupUI();

private:
    /////////////////////////////////////////////////////// Private fields.
	Ui_SliceViewerWindow* myUI;
	VisualizationManager* myVizMng;
	QAction* myMenuAction;
	int mySelectedField;
	int myNumSlices;
	float myStartDepth;
	float myEndDepth;

	// Color legend (shown on first slice)
    vtkScalarBarActor* myScalarBar;

	Slice* mySlices[MAX_SLICES];

	vtkTransformFilter* mySeparationTransform;
    vtkShepardMethod* mySondeVolumeBuilder;

	// Bathymetry data.
	vtkContourFilter* myBathyContour;
	vtkPolyDataMapper* myBathyContourMapper;

	bool myContoursEnabled;
	int myNumContours;
};

#endif 
