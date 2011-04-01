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
#include "AppConfig.h"
#include "VisualizationManager.h"
#include "DataSet.h"
#include "DataFieldSettings.h"
#include "DataViewOptions.h"
#include "GeoDataView.h"
#include "ui_MainWindow.h"
#include "PlotView.h"
#include "Preferences.h"
#include "PreferencesWindow.h"
#include "SliceViewer.h"
#include "ColorFunctionManager.h"
#include "NavigationView.h"
#include "TableView.h"
#include "Utils.h"
#include "VtkDataManager.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
VTK_CALLBACK(StartInteractionCallback, VisualizationManager, OnStartInteraction());
VTK_CALLBACK(EndInteractionCallback, VisualizationManager, OnEndInteraction());

///////////////////////////////////////////////////////////////////////////////////////////////////
VisualizationManager::VisualizationManager():
	myDepthScale(DefaultDepthScale),
	myDataSet(NULL),
	myRenderWindow(NULL),
	mySelectedField(0),
	myPointReductionFactor(2)
{
	for(int i = 0; i < MAX_PLOT_VIEWS; i++)
	{
		myPlotView[i] = NULL;
	}

	myTimeFilter.Type = DynamicFilter::TimeFilter;
	myTimeFilter.Enabled = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
VisualizationManager::~VisualizationManager()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void VisualizationManager::SetupUI()
{
	myPreferencesWindow = new PreferencesWindow(this);

	GetUI()->statusbar->setVisible(true);

	connect(GetUI()->actionQuit, SIGNAL(triggered(bool)), SLOT(OnQuitTrigger(bool)));

	connect(GetUI()->actionSaveSnapshot, SIGNAL(triggered(bool)), SLOT(OnSaveSnapshotTrigger(bool)));

	connect(GetUI()->actionPreferences, SIGNAL(triggered(bool)), SLOT(OnPreferencesTrigger(bool)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void VisualizationManager::SetSondeDataVisibility(bool value)
{
	mySondeActor->SetVisibility(value);
	Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString VisualizationManager::GetSelectedTag()
{
	return QString();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void VisualizationManager::Initialize(DataSet* dataSet)
{
	VisualizationManagerBase::Initialize();

	// Disable preferences window in glacier mode.
	GetUI()->actionPreferences->setEnabled(true);

	myDataSet = dataSet;
	GetMainWindow()->setCaption("Sonde and Bathymetry Toolset - Looking Glass " LOOKING_GLASS_VERSION);

	// Setup the main render window
	myRenderer = GetMainRenderer();
	myRenderer->SetBackground(0.15f, 0.16f, 0.2f);
	myRenderWindow = GetRenderWindow();

	myColorFunctionManager = new ColorFunctionManager(this);
	myColorFunctionManager->Initialize();

    InitSonde();
	InitPicking();
	//InitIsosurfaces();

	myTableView = new TableView(this);
	myTableView->Initialize();
	myTableView->Enable();

	for(int i = 0; i < MAX_PLOT_VIEWS; i++)
	{
		myPlotView[i] = new PlotView(this, i + 1);
		myPlotView[i]->Initialize();
	}

	myPlotView[0]->Enable();

	myDataViewOptions = new DataViewOptions(this);
	myDataViewOptions->Initialize();
	myDataViewOptions->Enable();

	// Initialize the view objects.
	myGeoDataView = new GeoDataView(this);
	myGeoDataView->Initialize();
	//myGeoDataView->Enable();

	// Init error markers data (must be done AFTER geo data view initialization)
	InitErrorMapData();

	// myGeoDataView->SetupBathyErrorMap();

	myNavigationView = new NavigationView(this);
	myNavigationView->Initialize();

	mySliceViewer = new SliceViewer(this);
	mySliceViewer->Initialize();

	/*myDataFieldSettings = new DataFieldSettings(this);
	myDataFieldSettings->Initialize();
	myDataFieldSettings->Enable();*/

	// Set the main view interactor to terrain.
	vtkInteractorStyleTerrain* terrainInteractor = vtkInteractorStyleTerrain::New();
	GetUI()->vtkView->GetInteractor()->AddObserver(vtkCommand::StartInteractionEvent, new StartInteractionCallback(this));
	GetUI()->vtkView->GetInteractor()->AddObserver(vtkCommand::EndInteractionEvent, new EndInteractionCallback(this));
    GetUI()->vtkView->GetInteractor()->SetInteractorStyle(terrainInteractor);

	SetupUI();

	// Initialize the depth scale for all objects.
	SetDepthScale(myDepthScale);

	myRenderer->GetActiveCamera()->ParallelProjectionOn();
	myRenderer->ResetCamera();
	myRenderer->GetActiveCamera()->SetParallelScale(1000);

	myDataSet->AddFilter(&myTimeFilter);

	this->GetMainWindow()->layout();

	// Load window state from preferences.
	Preferences* prefs = AppConfig::GetInstance()->GetPreferences();
	if(!prefs->GetWindowState().isNull())
	{
		GetMainWindow()->restoreState(prefs->GetWindowState());
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void VisualizationManager::InitPicking()
{
	myPicker = vtkPointPicker::New();

	mySelectedPointFilter = vtkMaskPoints::New();
	mySelectedPointFilter->SetInput(VtkDataManager::GetInstance()->GetPointSet(DataSet::SelectedData));
	mySelectedPointFilter->GenerateVerticesOn();
	mySelectedPointFilter->SetOnRatio(myPointReductionFactor);

	// Setup selected point visualization
    mySelectionMapper = vtkDataSetMapper::New();
	mySelectionMapper->SetInput(mySelectedPointFilter->GetOutput());

    mySelectionActor = vtkActor::New();
    mySelectionActor->GetProperty()->SetPointSize(12.0f);
    mySelectionActor->SetMapper(mySelectionMapper);
	mySelectionActor->GetProperty()->SetRepresentationToPoints();

    myRenderer->AddActor(mySelectionActor);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void VisualizationManager::InitSonde()
{
    SetInitMessage("Initializing Sonde Data...");

	myPointFilter = vtkMaskPoints::New();
	myPointFilter->SetInput(VtkDataManager::GetInstance()->GetPointSet(DataSet::FilteredData));
	myPointFilter->GenerateVerticesOn();
	myPointFilter->SetOnRatio(myPointReductionFactor);

	mySondeMapper = vtkDataSetMapper::New();
	mySondeMapper->SetInput( myPointFilter->GetOutput());
    mySondeMapper->Update();

	// Setup the scalar color bar.
    myScalarBar = vtkScalarBarActor::New();
    myScalarBar->GetPositionCoordinate()->SetCoordinateSystemToDisplay();
    myScalarBar->GetPositionCoordinate()->SetValue(30, 5);
	myScalarBar->GetPosition2Coordinate()->SetCoordinateSystemToDisplay();
    myScalarBar->GetPosition2Coordinate()->SetValue(280, 50);
	myScalarBar->SetOrientationToHorizontal();
    myScalarBar->GetTitleTextProperty()->ShadowOff();

    mySondeActor = vtkActor::New();
    mySondeActor->GetProperty()->SetPointSize(5.0f);
    mySondeActor->SetMapper(mySondeMapper);
	mySondeActor->GetProperty()->SetRepresentationToPoints();
    mySondeActor->GetProperty()->BackfaceCullingOff();
    mySondeActor->GetProperty()->FrontfaceCullingOff();
    mySondeActor->PickableOn();

    myRenderer->AddActor(mySondeActor);
    myRenderer->AddActor2D(myScalarBar);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void VisualizationManager::InitErrorMapData()
{
	myNumErrorMarkers = myDataSet->GetSondeBathyData()->GetNumberOfPoints();
	
	// Allocate arro marker arrays.
	myOldVsSondeDepthError = new double[myNumErrorMarkers];
	myOldVsSonarDepthError = new double[myNumErrorMarkers];
	mySondeVsSonarDepthError = new double[myNumErrorMarkers];

	for(int i = 0; i < myNumErrorMarkers; i++)
	{
		// Compute the error marker values.
		double* pt = myDataSet->GetSondeBathyData()->GetPoint(i);
		double oldDepth = myGeoDataView->GetDepthAt(pt[0], pt[2], false);
		double sonarDepth = myGeoDataView->GetDepthAt(pt[0], pt[2], true);

		myOldVsSondeDepthError[i] = pt[1] - oldDepth;
		myOldVsSonarDepthError[i] = sonarDepth - oldDepth;
		mySondeVsSonarDepthError[i] = sonarDepth - pt[1];
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void VisualizationManager::Update()
{
	Render();
	myTableView->Update();
	for(int i = 0; i < MAX_PLOT_VIEWS; i++)
	{
		if(myPlotView[i]->IsEnabled())
		{
			myPlotView[i]->Update();
			myPlotView[i]->Update();
			//myPlotView[i]->Render();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void VisualizationManager::SetMissionRange(time_t startTime, time_t endTime)
{
	myTimeFilter.TimeMin = startTime; 
	myTimeFilter.TimeMax = endTime; 
	myTimeFilter.Enabled = true;
	myDataSet->ApplyFilters();
	Update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
DataSet* VisualizationManager::GetDataSet() 
{ 
	return myDataSet; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int VisualizationManager::GetSelectedField() 
{
	return mySelectedField; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void VisualizationManager::SetSelectedField(int i) 
{
	mySelectedField = i; 

    mySondeMapper->SetScalarModeToUsePointFieldData();
    mySondeMapper->SelectColorArray(mySelectedField);
	mySondeMapper->SetLookupTable(myColorFunctionManager->GetColorFunction(mySelectedField));
    mySondeMapper->Update();

    mySelectionMapper->SetScalarModeToUsePointFieldData();
    mySelectionMapper->SelectColorArray(mySelectedField);
	mySelectionMapper->SetLookupTable(myColorFunctionManager->GetColorFunction(mySelectedField));
    mySelectionMapper->Update();

    myScalarBar->SetTitle(myDataSet->GetFieldName(mySelectedField));
	myScalarBar->SetLookupTable(myColorFunctionManager->GetColorFunction(mySelectedField, true));

	for(int i = 0; i < MAX_PLOT_VIEWS; i++)
	{
		if(myPlotView[i] != NULL && myPlotView[i]->IsEnabled())
		{
			myPlotView[i]->Update();
		}
	}

	Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/*void VisualizationManager::SetSelectedIsosurfaceField(int i) 
{
	const char* name = myDataSet->GetFieldName(i);
	myIsosurfacePolyData->SetPoints(myDataSet->GetVtkData()->GetPoints());
	myIsosurfacePolyData->GetPointData()->SetScalars(
		 myDataSet->GetVtkData()->GetPointData()->GetScalars(name));
	mySeparationTransform->Update();
	myIsosurfaceVolumeBuilder->SetModelBounds(mySeparationTransform->GetOutput()->GetBounds());
	myIsosurfaceVolumeBuilder->Update();
	myIsosurfaceMapper->SetLookupTable(myColorFunctionManager->GetColorFunction(mySelectedField));
}*/

///////////////////////////////////////////////////////////////////////////////////////////////////
//void VisualizationManager::SetSelectedIsosurfaceValue(double value)
//{
//	myIsosurfaceContour->SetValue(0, value);
//	myIsosurfaceContour->Update();
//	Render();
//}

///////////////////////////////////////////////////////////////////////////////////////////////////
void VisualizationManager::SetPointReductionFactor(int factor)
{
	myPointReductionFactor = factor;
	myPointFilter->SetOnRatio(myPointReductionFactor);
	myPointFilter->Update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void VisualizationManager::SetDepthScale(int value)
{
	myDepthScale = value;
	/*if(mySliceView != NULL && mySliceView->IsEnabled())
	{
		mySliceView->SetDepthScale(value);
	}*/
	myGeoDataView->SetDepthScale(value);
	mySondeActor->SetScale(1, value, 1);
	mySelectionActor->SetScale(1, value, 1);
	//myIsosurfaceActor->SetScale(1, (float)value / DataSet::SLICE_SEPARATION, 1);
	//myIsosurfaceActor->SetScale(1, myDepthScale, 1);
    //GetUI()->vtkView->GetRenderWindow()->Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double VisualizationManager::GetErrorMarkerMean()
{
	double result = 0;
	for(int i = 0; i < myNumErrorMarkers; i++)
	{
		switch(myErrorMarkerMode)
		{
		case ErrorMarkerMode::OldVsSonde:
			result += myOldVsSondeDepthError[i];
			break;
		case ErrorMarkerMode::OldVsSonar:
			result += myOldVsSonarDepthError[i];
			break;
		case ErrorMarkerMode::SondeVsSonar:
			result += mySondeVsSonarDepthError[i];
			break;
		}
	}
	return result / myNumErrorMarkers;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double VisualizationManager::GetErrorMarkerStdDev()
{
	double result = 0;
	double mean = GetErrorMarkerMean();
	for(int i = 0; i < myNumErrorMarkers; i++)
	{
		double tmp;
		switch(myErrorMarkerMode)
		{
		case ErrorMarkerMode::OldVsSonde:
			tmp = myOldVsSondeDepthError[i] - mean;
			result += tmp * tmp;
			break;
		case ErrorMarkerMode::OldVsSonar:
			tmp = myOldVsSonarDepthError[i] - mean;
			result += tmp * tmp;
			break;
		case ErrorMarkerMode::SondeVsSonar:
			tmp = mySondeVsSonarDepthError[i] - mean;
			result += tmp * tmp;
			break;
		}
	}
	result = sqrt(result / myNumErrorMarkers);
	return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void VisualizationManager::SetErrorMarkerMode(ErrorMarkerMode::Enum mode)
{
	myErrorMarkerMode = mode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//void VisualizationManager::InitIsosurfaces()
//{
//	myIsosurfaceVolumeBuilder = vtkShepardMethod::New();
//	myIsosurfaceContour = vtkMarchingContourFilter::New();
//	myIsosurfaceMapper = vtkDataSetMapper::New();
//	myIsosurfaceActor = vtkActor::New();
//	myIsosurfacePolyData = vtkPolyData::New();
//	myIsosurfaceThreshold = vtkThreshold::New();
//
//	vtkTransform* transform = vtkTransform::New();
//	transform->Scale(1, DataSet::SLICE_SEPARATION, 1);
//
//	mySeparationTransform = vtkTransformFilter::New();
//	mySeparationTransform->SetTransform(transform);
//	mySeparationTransform->SetInput(myIsosurfacePolyData);
//
//	myIsosurfaceVolumeBuilder->SetInput(mySeparationTransform->GetOutput());
//    myIsosurfaceVolumeBuilder->SetSampleDimensions(30, 60, 30);
//    myIsosurfaceVolumeBuilder->SetMaximumDistance(3.0f / DataSet::SLICE_SEPARATION);
//    myIsosurfaceVolumeBuilder->SetNullValue(999.0f);
//
//	myIsosurfaceThreshold->SetInput(myIsosurfaceVolumeBuilder->GetOutput());
//	myIsosurfaceThreshold->ThresholdBetween(-1000, 990);
//
//	myIsosurfaceContour->SetInput(myIsosurfaceThreshold->GetOutput());
//
//	myIsosurfaceMapper->SetInputConnection(myIsosurfaceContour->GetOutputPort());
//
//	myIsosurfaceActor->SetMapper(myIsosurfaceMapper);
//	//myIsosurfaceActor->GetProperty()->SetRepresentationToPoints();
//	myIsosurfaceActor->GetProperty()->FrontfaceCullingOff();
//	myIsosurfaceActor->GetProperty()->BackfaceCullingOff();
//	myIsosurfaceActor->GetProperty()->SetAmbientColor(1, 1, 1);
//	myIsosurfaceActor->VisibilityOff();
//
//	myRenderer->AddActor(myIsosurfaceActor);
//
//	//SetSelectedIsosurfaceField(0);
//}

///////////////////////////////////////////////////////////////////////////////////////////////////
/*void VisualizationManager::EnableIsosurfaces(bool enable)
{
	if(enable)
	{
		myIsosurfaceActor->VisibilityOn();
		Render();
	}
	else
	{
		myIsosurfaceActor->VisibilityOff();
		Render();
	}
}*/

///////////////////////////////////////////////////////////////////////////////////////////////////
void VisualizationManager::OnStartInteraction()
{
	int* pos = myRenderWindow->GetInteractor()->GetEventPosition();
	myMouseX = pos[0];
	myMouseY = pos[1];
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void VisualizationManager::OnEndInteraction()
{
	int* pos = myRenderWindow->GetInteractor()->GetEventPosition();
	if(myMouseX == pos[0] && myMouseY == pos[1])
	{
		myPicker->Pick(pos[0], pos[1], 0, myRenderer);

		if(myPicker->GetPointId() != -1)
		{
			vtkIdType ptId = myPicker->GetPointId();
			
			// A point has been selected. retrieve grid coords from its Id.
			vtkPointData* pts = myPointFilter->GetOutput()->GetPointData();

			// ENDURANCE HACK:
			// Field names here must be mapped to X, Y, Z field names from config info, not be hardcoded.
			vtkDoubleArray* arX = vtkDoubleArray::New();
			pts->GetArray("X")->GetData(ptId, ptId, 0, 0, arX);

			vtkDoubleArray* arY = vtkDoubleArray::New();
			pts->GetArray("Y")->GetData(ptId, ptId, 0, 0, arY);

			vtkDoubleArray* arZ = vtkDoubleArray::New();
			pts->GetArray("Depth")->GetData(ptId, ptId, 0, 0, arZ);

			double* X = arX->GetPointer(0);
			double* Y = arY->GetPointer(0);
			double* Z = arZ->GetPointer(0);

			DataItem* pdi = myDataSet->FindDataItem((float)X[0], (float)Y[0], (float)Z[0]);

			if(pdi == NULL)
			{
				GetUI()->statusbar->message(QString("VisualizationManager::OnEndInteraction Unable to find point in dataset at position (%1, %2, %3)").
					arg((float)X[0]).arg((float)Y[0]).arg((float)Z[0]));
			}
			else
			{
				bool addToSelection = (QApplication::keyboardModifiers() & Qt::ControlModifier);
				myDataSet->SelectByTag(
					pdi->Tag1, DataSetInfo::Tag1, 
					DataSet::FilteredData, 
					addToSelection ? DataSet::SelectionToggle : DataSet::SelectionNew);

				// Get sonde based depth at the specified point.
				// The value returned here is already water-level corrected.
				ptId = myDataSet->GetSondeBathyData()->FindPoint(
					Y[0], myPointFilter->GetOutput()->GetPoint(ptId)[1], X[0]);

				float oldDepth = myGeoDataView->GetDepthAt(Y[0], X[0], false);
				float sonarDepth = myGeoDataView->GetDepthAt(Y[0], X[0], true);
				float sondeDepth = myDataSet->GetSondeBathyData()->GetPoint(ptId)[1];
				//char str[256];
				//float levelCorrection = DataSet::GetWaterLevelCorrection((int)Day[0]);
				//sprintf(str, "Point UTM: E%d N%d | Depth: (Old)%.2fm (Sonar)%.2fm (Sonde)%.2fm (Corrected Sonde)%.2fm | Water Lvl Correction: %.2fm", 
				//	(int)X, (int)Y, 
				//	oldDepth,
				//	sonarDepth,
				//	sondeDepth + levelCorrection, // To get the original sonde-based depth we have to inver water level correction
				//	sondeDepth,
				//	levelCorrection);
				//printf("%s\n", str);
			}
			// Release allocated arrays.
			arX->Delete();
			arY->Delete();
		}
		else
		{
			// No valid point selected: clear selection.
			myDataSet->ClearSelection();
		}
		Update();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void VisualizationManager::OnQuitTrigger(bool)
{
	QApplication::quit();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void VisualizationManager::OnPreferencesTrigger(bool)
{
	myPreferencesWindow->Update();
	myPreferencesWindow->show();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void VisualizationManager::OnSaveSnapshotTrigger(bool)
{
	Utils::SaveScreenshot(myRenderWindow);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void VisualizationManager::SetStatusbarMessage(const QString& msg)
{
	GetUI()->statusbar->message(msg);
}
