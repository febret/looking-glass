/**************************************************************************************************
 * THE LOOKING GLASS VISUALIZATION TOOLSET
 *-------------------------------------------------------------------------------------------------
 * Author: 
 *	Alessandro Febretti		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Contact & Web:
 *  febret@gmail.com		http://febretpository.hopto.org
 *-------------------------------------------------------------------------------------------------
 * Looking Glass has been built as part of the ENDURANCE Project (http://www.evl.uic.edu/endurance/).
 * ENDURANCE is supported by the NASA ASTEP program under Grant NNX07AM88G and by the NSF USAP.
 *-------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2011, Electronic Visualization Laboratory, University of Illinois at Chicago
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, are permitted 
 * provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this list of conditions 
 * and the following disclaimer. Redistributions in binary form must reproduce the above copyright 
 * notice, this list of conditions and the following disclaimer in the documentation and/or other 
 * materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF 
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *************************************************************************************************/ 
#ifndef VISUALIZATIONMANAGER_H
#define VISUALIZATIONMANAGER_H

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "LookingGlassSystem.h"
#include "VisualizationManagerBase.h"
#include "DataSet.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_PLOT_VIEWS 4
#define MAX_SECTION_VIEWS 2

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
	LineTool* GetLineTool();
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
	void OnEndInteraction();
	void OnStartInteraction();
	void SetStatusbarMessage(const QString& msg);

public:
	static const int DefaultDepthScale = 4;

protected slots:
	void OnQuitTrigger(bool);
	void OnPreferencesTrigger(bool);
	void OnSaveSnapshotTrigger(bool);

private:
    void InitSonde();
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
	SectionView* mySectionView[MAX_SECTION_VIEWS];
	SliceViewer* mySliceViewer;
	VolumeView* myVolumeView;
	DataViewOptions* myDataViewOptions;
	NavigationView* myNavigationView;
	//DataFieldSettings* myDataFieldSettings;
	TableView* myTableView;
	LineTool* myLineTool;

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
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline ColorFunctionManager* VisualizationManager::GetColorFunctionManager()
{ return myColorFunctionManager; }

///////////////////////////////////////////////////////////////////////////////////////////////////
inline int VisualizationManager::GetPointReductionFactor()
{ return myPointReductionFactor; }

///////////////////////////////////////////////////////////////////////////////////////////////////
inline GeoDataView* VisualizationManager::GetGeoDataVew()
{ return myGeoDataView; }

///////////////////////////////////////////////////////////////////////////////////////////////////
inline NavigationView* VisualizationManager::GetNavigationView()
{ return myNavigationView; }

///////////////////////////////////////////////////////////////////////////////////////////////////
inline LineTool* VisualizationManager::GetLineTool()
{ return myLineTool; }

#endif // MAINWINDOW_H
