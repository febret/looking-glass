/********************************************************************************************************************** 
 * THE LOOKING GLASS VISUALIZATION TOOLSET
 *---------------------------------------------------------------------------------------------------------------------
 * Author: 
 *	Alessandro Febretti							Electronic Visualization Laboratory, University of Illinois at Chicago
 * Contact & Web:
 *  febret@gmail.com							http://febretpository.hopto.org
 *---------------------------------------------------------------------------------------------------------------------
 * Looking Glass has been built as part of the ENDURANCE Project (http://www.evl.uic.edu/endurance/).
 * ENDURANCE is supported by the NASA ASTEP program under Grant NNX07AM88G and by the NSF USAP.
 *********************************************************************************************************************/ 
#ifndef VISUALIZATIONMANAGER_H
#define VISUALIZATIONMANAGER_H

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "LookingGlassSystem.h"
#include "VisualizationManagerBase.h"
#include "DataSet.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_PLOT_VIEWS 6
//#define MAX_THRESHOLD_STAGES 4

///////////////////////////////////////////////////////////////////////////////////////////////////
ENUM(ErrorMarkerMode)
	Disabled,
	OldVsSonde,
	OldVsSonar,
	SondeVsSonar
END_ENUM;

///////////////////////////////////////////////////////////////////////////////////////////////////
class VisualizationManager: public VisualizationManagerBase
{
	Q_OBJECT
public:
    /////////////////////////////////////////////////////// Ctor / Dtor.
    VisualizationManager();
    ~VisualizationManager();

    /////////////////////////////////////////////////////// Other Methods.
	// Updates the rendering pipeline and refreshed all the visualiztions.
	void Update();
	// Gets an instance of the dataset object.
	DataSet* GetDataSet();
	// Gets an instance of the color function manager object.
	ColorFunctionManager* GetColorFunctionManager();
	// Gets an instance of the geo data view object.
	GeoDataView* GetGeoDataVew();
	NavigationView* GetNavigationView();
	void SetDepthScale(int value);
	// Gets or Sets the currently selected data field
	void SetSelectedField(int value);
	int GetSelectedField();
	// Sets the point reduction factor
	void SetPointReductionFactor(int);
	// Gets the point reduction factor
	int GetPointReductionFactor();
	// Gets the set of currently selected datapoints.
	QString GetSelectedTag(); 
    void Initialize(DataSet* dataSet);

	void SetMissionRange(time_t startTime, time_t endTime);
	// Clears the sonde drop selection.
	//void ClearSelection();
	// Sets sonde data visibility.
	void SetSondeDataVisibility(bool value);
	// Sets the visibility of bathymetry error markers.
	void SetErrorMarkerMode(ErrorMarkerMode::Enum mode);
	double GetErrorMarkerMean();
	double GetErrorMarkerStdDev();
	int GetNumErrorMarkers() { return myNumErrorMarkers; }
	double* GetErrorMarkerData();
	// Manager interaction with sonde points.
	void OnEndInteraction();
	void OnStartInteraction();
	// Isosurfaces
	//void SetSelectedIsosurfaceField(int i); 
	//void SetSelectedIsosurfaceValue(double value); 
	//void EnableIsosurfaces(bool enable); 

	void SetStatusbarMessage(const QString& msg);

public:
	static const int DefaultDepthScale = 4;

protected slots:
	void OnQuitTrigger(bool);
	void OnPreferencesTrigger(bool);
	void OnSaveSnapshotTrigger(bool);

private:
    void InitSonde();
    void InitErrorMapData();
    void SetupUI();
    void InitPicking();
	//void InitIsosurfaces();
	//void SetSelectedSondeDrop(double X, double Y);

private:
	// Dataset
	DataSet* myDataSet;

	DynamicFilter myTimeFilter;

	// Tools and Windows
	PreferencesWindow* myPreferencesWindow;
	ColorFunctionManager* myColorFunctionManager;
	GeoDataView* myGeoDataView;
	PlotView* myPlotView[MAX_PLOT_VIEWS];
	SliceViewer* mySliceViewer;
	VolumeView* myVolumeView;
	DataViewOptions* myDataViewOptions;
	NavigationView* myNavigationView;
	//DataFieldSettings* myDataFieldSettings;
	TableView* myTableView;

    // Main color scale objects.
    vtkScalarBarActor* myScalarBar;

	// Selected point data and representation.
    vtkDataSetMapper* mySelectionMapper;
    vtkActor* mySelectionActor;
	int myMouseX;
	int myMouseY;

	// Main sonde data view.
	float myDepthScale;
	int myPointReductionFactor;
	vtkMaskPoints* myPointFilter;
	vtkMaskPoints* mySelectedPointFilter;
    vtkDataSetMapper* mySondeMapper;
    vtkActor* mySondeActor;

    vtkRenderer* myRenderer;
    vtkRenderWindow* myRenderWindow;
    int mySelectedField;

	// Picking.
	vtkPointPicker* myPicker;

	// Error markers
	// TODO: Move error markers stuff to GeoDataView.
	int myNumErrorMarkers;
	double* myOldVsSondeDepthError;
	double* myOldVsSonarDepthError;
	double* mySondeVsSonarDepthError;
	ErrorMarkerMode::Enum myErrorMarkerMode;

	// Isosurface stuff.
	//vtkTransformFilter* mySeparationTransform;
	//vtkPolyData* myIsosurfacePolyData;
 //   vtkShepardMethod* myIsosurfaceVolumeBuilder;
	//vtkMarchingContourFilter* myIsosurfaceContour;
	//vtkThreshold* myIsosurfaceThreshold;
 //   vtkDataSetMapper* myIsosurfaceMapper;
	//vtkActor* myIsosurfaceActor;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline ColorFunctionManager* VisualizationManager::GetColorFunctionManager()
{
	return myColorFunctionManager;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline int VisualizationManager::GetPointReductionFactor()
{
	return myPointReductionFactor;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline GeoDataView* VisualizationManager::GetGeoDataVew()
{
	return myGeoDataView;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline NavigationView* VisualizationManager::GetNavigationView()
{
	return myNavigationView;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline double* VisualizationManager::GetErrorMarkerData()
{
	switch(myErrorMarkerMode)
	{
	case ErrorMarkerMode::OldVsSonde:
		return myOldVsSondeDepthError;
	case ErrorMarkerMode::OldVsSonar:
		return myOldVsSonarDepthError;
	case ErrorMarkerMode::SondeVsSonar:
		return mySondeVsSonarDepthError;
	}
	return myOldVsSondeDepthError;
}

#endif // MAINWINDOW_H
