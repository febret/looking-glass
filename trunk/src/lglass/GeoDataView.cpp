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
#include "GeoDataView.h"
#include "DataSet.h"
#include "VisualizationManager.h"
#include "ui_MainWindow.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
GeoDataView::GeoDataView(VisualizationManager* mng): 
	DockedTool(mng, "Geographic Data"),
	myOldBathyColor(128, 128, 128),
	mySondeBathyColor(180, 180, 0),
	myOldBathyContourColor(0, 0, 0),
	mySondeBathyContourColor(255, 255, 255)
{
	myVizMng = mng;
	GetMenuAction()->setIcon(QIcon(":/icons/GeoDataView.png"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
GeoDataView::~GeoDataView()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataView::Initialize()
{
    SetInitMessage("Initializing Bathymetry Model...");

	QString oldBathymetryFile = AppConfig::GetInstance()->GetProfileName() + "/bonney-09.vtk";
	QString sonarBathymetryFile = AppConfig::GetInstance()->GetProfileName() + "/bonney-09.vtk";
	//QString sondeBathymetryFile = AppConfig::GetInstance()->GetProfileName() + "/sonde_bathymetry.vtk";
	
	QString sondeBathymetryFile = AppConfig::GetInstance()->GetProfileName() + "/bonney-09.vtk";

	//QString sondeBathymetryFile = AppConfig::GetInstance()->GetProfileName() + "/bathymetry.vtk";

	VisualizationManager* mng = myVizMng;
	vtkRenderer* renderer = mng->GetMainRenderer();

    myReader = vtkPolyDataReader::New();
    myReader->SetFileName(oldBathymetryFile);
    myReader->Update();

	double* depthRange;
	depthRange = myReader->GetOutput()->GetScalarRange();
	vtkColorTransferFunction* lakeLut = vtkColorTransferFunction::New();

    myOldBathyMapper = vtkPolyDataMapper::New();
	myOldBathyMapper->SetInput(myReader->GetOutput());
    myOldBathyMapper->SetLookupTable(lakeLut);
    myOldBathyMapper->SetScalarRange(depthRange);
	myReader->GetOutput()->GetPointData()->SetActiveScalars("Scalar");

	SetOldBathyColor(myOldBathyColor);
    myOldBathyMapper->Update();

    myLakeActor = vtkActor::New();
    myLakeActor->SetMapper(myOldBathyMapper);
    myLakeActor->GetProperty()->BackfaceCullingOff();
    myLakeActor->GetProperty()->FrontfaceCullingOff();
    myLakeActor->GetProperty()->SetAmbientColor(1, 1, 1);
    myLakeActor->GetProperty()->SetAmbient(0.2);
	myLakeActor->SetPosition(0, 0, 0);
	myLakeActor->SetScale(1, 7, 1);
	myLakeActor->PickableOff();

	// SONDE_BASED BATHYMETRY
    mySondeBathyReader = vtkPolyDataReader::New();
    mySondeBathyReader->SetFileName(sondeBathymetryFile);
    mySondeBathyReader->Update();
	mySondeBathyReader->GetOutput()->GetPointData()->SetActiveScalars("Z");

	double* range = mySondeBathyReader->GetOutput()->GetBounds();
	Console::Message(QString("Sonde Bathy Range: %1-%2 %3-%4 %5-%6").arg(range[0]).arg(range[1]).arg(range[2]).arg(range[3]).arg(range[4]).arg(range[5]));

	range = myReader->GetOutput()->GetBounds();
	Console::Message(QString("Old Bathy Range: %1-%2 %3-%4 %5-%6").arg(range[0]).arg(range[1]).arg(range[2]).arg(range[3]).arg(range[4]).arg(range[5]));

	float* fr = myVizMng->GetDataSet()->GetZRange(); 
	depthRange[0] = fr[0];
	depthRange[1] = fr[1];
	lakeLut = vtkColorTransferFunction::New();

    mySondeBathyMapper = vtkPolyDataMapper::New();
	mySondeBathyMapper->SetInput(mySondeBathyReader->GetOutput());
    mySondeBathyMapper->SetLookupTable(lakeLut);
    mySondeBathyMapper->SetScalarRange(depthRange);
	mySondeBathyMapper->SetColorModeToMapScalars();

	SetSondeBathyColor(mySondeBathyColor);
    mySondeBathyMapper->Update();

    mySondeBathyActor = vtkActor::New();
    mySondeBathyActor->SetMapper(mySondeBathyMapper);
    mySondeBathyActor->GetProperty()->BackfaceCullingOff();
    mySondeBathyActor->GetProperty()->FrontfaceCullingOff();
    mySondeBathyActor->GetProperty()->SetAmbientColor(0.5, 0.5, 0.5);
    mySondeBathyActor->GetProperty()->SetAmbient(0.2);
	mySondeBathyActor->SetPosition(0, 0, 0);
	mySondeBathyActor->GetProperty()->SetInterpolationToGouraud();
	mySondeBathyActor->GetProperty()->SetOpacity(0);
	mySondeBathyActor->PickableOff();


	// Sonar based bathymetry.
	mySonarBathyReader = vtkPolyDataReader::New();
    mySonarBathyReader->SetFileName(sonarBathymetryFile);
    mySonarBathyReader->Update();

	//double* range = mySonarBathyReader->GetOutput()->GetBounds();
	//printf("Sonar Bathy Range: %f-%f %f-%f %f-%f\n", range[0], range[1], range[2], range[3], range[4], range[5]);

	mySonarBathyMapper = vtkDataSetMapper::New();
	mySonarBathyMapper->SetInput(mySonarBathyReader->GetOutput());
	mySonarBathyMapper->Update();
	mySonarBathyMapper->ScalarVisibilityOff();

	mySonarBathyActor = vtkActor::New();
	mySonarBathyActor->SetVisibility(0);
    mySonarBathyActor->SetMapper(mySonarBathyMapper);
	mySonarBathyActor->GetProperty()->SetRepresentationToPoints();
	mySonarBathyActor->GetProperty()->SetColor(1, 1, 1);

    // a renderer and render window
    renderer->AddActor(myLakeActor);
    renderer->AddActor(mySondeBathyActor);
    renderer->AddActor(mySonarBathyActor);

	// Initialize the map overlay.
	myOverlayReader = vtkJPEGReader::New();
	QString overlayFile = AppConfig::GetInstance()->GetProfileName() + "/overlay.jpg";
	myOverlayReader->SetFileName(overlayFile);
	myOverlayTexture = vtkTexture::New();
	myOverlayTexture->SetInput(myOverlayReader->GetOutput());
	myOverlayTexture->InterpolateOn();
	myOverlayPlane = vtkPlaneSource::New();
	myOverlayPlane->SetOrigin(0, 0, 0);
	myOverlayPlane->SetPoint2(0, 0, -1);
	myOverlayPlane->SetPoint1(1, 0, 0);
	myOverlayMapper = vtkPolyDataMapper::New();
	myOverlayMapper->SetInput(myOverlayPlane->GetOutput());
	myOverlayActor = vtkActor::New();
	myOverlayActor->SetMapper(myOverlayMapper);
	myOverlayActor->SetTexture(myOverlayTexture);

	// TODO: Argh, hardcoded values.
	myOverlayActor->SetOrientation(0, 90, 0);
	myOverlayActor->SetScale(-2936, 1, -2284);
	myOverlayActor->SetPosition(1370239, 0, 434526);
	myOverlayActor->GetProperty()->SetOpacity(0);
	myOverlayActor->PickableOff();
	myOverlayActor->GetProperty()->SetLighting(0);

	renderer->AddActor(myOverlayActor);
	GetVisualizationManager()->Render();

	DataSet* data = myVizMng->GetDataSet();

	SetupContours();
	SetupUI();

	// Enable contours on old bathymetry by default.
	myUI->oldContoursButton->setChecked(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataView::SetupContours()
{
	VisualizationManager* mng = myVizMng;
	vtkRenderer* renderer = mng->GetMainRenderer();

	myOldBathyContour = vtkContourFilter::New();
	myOldBathyContourMapper = vtkPolyDataMapper::New();
	myOldBathyContourActor = vtkActor::New();

	// Needed to force update of scalar range used for contour generation.
	myReader->Modified();

	myOldBathyContour->SetInput(myReader->GetOutput());
	myOldBathyContour->GenerateValues(10, 
		myReader->GetOutput()->GetScalarRange());
	myOldBathyContourMapper->SetInput(myOldBathyContour->GetOutput());
	myOldBathyContourMapper->SetScalarVisibility(0);
	myOldBathyContourMapper->Update();
	
	myOldBathyContourActor->SetMapper(myOldBathyContourMapper);
	myOldBathyContourActor->GetProperty()->SetLighting(0);
	myOldBathyContourActor->GetProperty()->SetLineWidth(1);
	myOldBathyContourActor->SetPosition(0, 2.0f, 0);
	myOldBathyContourActor->GetProperty()->SetColor(QCOLOR_TO_VTK(myOldBathyContourColor));
	myOldBathyContourActor->GetProperty()->SetOpacity(1);
	myOldBathyContourActor->VisibilityOff();
	myOldBathyContourActor->SetScale(1, VisualizationManager::DefaultDepthScale, 1);

	mySondeBathyContour = vtkContourFilter::New();
	mySondeBathyContourMapper = vtkPolyDataMapper::New();
	mySondeBathyContourActor = vtkActor::New();

	// Needed to force update of scalar range used for contour generation.
	mySondeBathyReader->Modified();

	mySondeBathyContour->SetInput(mySondeBathyReader->GetOutput());
	mySondeBathyContour->GenerateValues(10, 
		mySondeBathyReader->GetOutput()->GetScalarRange());
	mySondeBathyContourMapper->SetInput(mySondeBathyContour->GetOutput());
	mySondeBathyContourMapper->SetScalarVisibility(0);
	mySondeBathyContourMapper->Update();
	mySondeBathyContourActor->SetMapper(mySondeBathyContourMapper);
	mySondeBathyContourActor->GetProperty()->SetLighting(0);
	mySondeBathyContourActor->GetProperty()->SetLineWidth(1);
	mySondeBathyContourActor->SetPosition(0, 2.0f, 0);
	mySondeBathyContourActor->VisibilityOff();
	mySondeBathyContourActor->GetProperty()->SetColor(QCOLOR_TO_VTK(mySondeBathyContourColor));

	renderer->AddActor(myOldBathyContourActor);
	renderer->AddActor(mySondeBathyContourActor);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataView::SetupUI()
{
	myUI = new Ui::GeoDataViewDock();
	myUI->setupUi(GetDockWidget());
	myUI->axesButton->hide();

	// Set dafault values to controls.
    myUI->opacitySlider->setValue(100);
    myUI->sondeBathyOpacitySlider->setValue(0);
    myUI->sondeContoursSlider->setValue(10);
    myUI->oldContoursSlider->setValue(10);
	myUI->disabledMarkersButton->setChecked(true);

	myUI->errorMeanLabel->setVisible(false);
	myUI->errorStdDevLabel->setVisible(false);

	// Wire events.
    connect(myUI->opacitySlider, SIGNAL(valueChanged(int)), 
		SLOT(OpacitySliderChanged(int)));

	connect(myUI->overlayOpacitySlider, SIGNAL(valueChanged(int)), 
		SLOT(OnOverlayOpacitySliderChanged(int)));

	connect(myUI->sondeBathyOpacitySlider, SIGNAL(valueChanged(int)), 
		SLOT(SondeBathyOpacitySliderChanged(int)));

	connect(myUI->axesButton, SIGNAL(toggled(bool)), 
		SLOT(OnAxesButtonToggle(bool)));

	connect(myUI->oldContoursSlider, SIGNAL(sliderReleased()), 
		SLOT(OnOldContoursSliderReleased()));

	connect(myUI->sondeContoursSlider, SIGNAL(sliderReleased()), 
		SLOT(OnSondeContoursSliderReleased()));

	connect(myUI->sondeContoursButton, SIGNAL(toggled(bool)), 
		SLOT(OnSondeContoursButtonToggle(bool)));

	connect(myUI->oldContoursButton, SIGNAL(toggled(bool)), 
		SLOT(OnOldContoursButtonToggle(bool)));

	connect(myUI->disabledMarkersButton, SIGNAL(toggled(bool)), 
		SLOT(OnShowErrorMarkersButtonToggle(bool)));

	connect(myUI->oldVsSondeMarkersButton, SIGNAL(toggled(bool)), 
		SLOT(OnShowErrorMarkersButtonToggle(bool)));

	connect(myUI->oldVsSonarMarkersButton, SIGNAL(toggled(bool)), 
		SLOT(OnShowErrorMarkersButtonToggle(bool)));

	connect(myUI->sondeVsSonarMarkersButton, SIGNAL(toggled(bool)), 
		SLOT(OnShowErrorMarkersButtonToggle(bool)));

	connect(myUI->showSonarDataButton, SIGNAL(toggled(bool)), 
		SLOT(OnShowSonarDataButtonToggle(bool)));

	myUI->errorMarkerBox->setVisible(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double GeoDataView::GetDepthAt(float x, float y, bool sonar)
{
	int pointNum = 0;
	double totD = 0;
	int count = 0;
	double tresholdD = 0;
	double* range;
	if(!sonar)
	{
		count = myReader->GetOutput()->GetNumberOfPoints();
		range = myReader->GetOutput()->GetPoints()->GetBounds();
	}
	else
	{
		count = mySonarBathyReader->GetOutput()->GetNumberOfPoints();
		range = mySonarBathyReader->GetOutput()->GetPoints()->GetBounds();
	}
	tresholdD = sqrt(
		(range[0] - range[1])*(range[0] - range[1]) + 
		(range[4] - range[5])*(range[4] - range[5]));
	// Distance threshold to include point in calculation: 1 / 32 of total range.
	tresholdD = tresholdD / 32;
	// STEP 1: compute total distance reciprocal of selected points.
	for(int i = 0; i < count; i++)
	{
		double* pt = NULL;
		if(!sonar)
		{
			pt = myReader->GetOutput()->GetPoint(i);
		}
		else
		{
			pt = mySonarBathyReader->GetOutput()->GetPoint(i);
		}
		double dx = x - pt[0];
		double dy = y - pt[2];
		double d = sqrt(dx * dx + dy * dy);
		// If the point distance is less than threshold include it in depth calculation.
		if(d < tresholdD)
		{
			double rd = 1.0f / d;
			totD += rd;
			pointNum++;
		}
	}
	// STEP 2: Compute depth as weighted average of selected points.
	double depth = 0;
	for(int i = 0; i < count; i++)
	{
		double* pt = NULL;
		if(!sonar)
		{
			pt = myReader->GetOutput()->GetPoint(i);
		}
		else
		{
			pt = mySonarBathyReader->GetOutput()->GetPoint(i);
		}
		double dx = x - pt[0];
		double dy = y - pt[2];
		double d = sqrt(dx * dx + dy * dy);
		// If the point distance is less than threshold include it in depth calculation.
		if(d < tresholdD)
		{
			double weight = (1.0f / d) / totD;
			depth += weight * pt[1];
		}
	}
	return depth;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataView::SetDepthScale(int value)
{
	myLakeActor->SetScale(1, value, 1);
	myOldBathyContourActor->SetScale(1, value, 1);
	mySonarBathyActor->SetScale(1, value, 1);
	mySondeBathyActor->SetScale(1, value, 1);
	mySondeBathyContourActor->SetScale(1, value, 1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataView::SetOldBathyColor(QColor value)
{
	myOldBathyColor = value;
	vtkColorTransferFunction* lakeLut = (vtkColorTransferFunction*)myOldBathyMapper->GetLookupTable();
	double* depthRange = myReader->GetOutput()->GetScalarRange(); 
	lakeLut->RemoveAllPoints();
	lakeLut->AddRGBPoint(depthRange[1], (double)value.red() / 500, 
                             (double)value.green() / 500, 
                             (double)value.blue() / 500);
	lakeLut->AddRGBPoint(depthRange[0], QCOLOR_TO_VTK(myOldBathyColor));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataView::SetSondeBathyColor(QColor value)
{
	mySondeBathyColor = value;
	vtkColorTransferFunction* lakeLut = (vtkColorTransferFunction*)mySondeBathyMapper->GetLookupTable();
	float* depthRange = myVizMng->GetDataSet()->GetZRange(); 
	lakeLut->RemoveAllPoints();
	lakeLut->AddRGBPoint(-depthRange[0], (double)value.red() / 500, 
                             (double)value.green() / 500, 
                             (double)value.blue() / 500);
	lakeLut->AddRGBPoint(-depthRange[1], QCOLOR_TO_VTK(mySondeBathyColor));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataView::SetOldBathyContourColor(QColor value)
{
	myOldBathyContourColor = value;
	myOldBathyContourActor->GetProperty()->SetColor(QCOLOR_TO_VTK(myOldBathyContourColor));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataView::SetSondeBathyContourColor(QColor value)
{
	mySondeBathyContourColor = value;
	mySondeBathyContourActor->GetProperty()->SetColor(QCOLOR_TO_VTK(mySondeBathyContourColor));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataView::OpacitySliderChanged(int value)
{
    myLakeActor->GetProperty()->SetOpacity(((double)value) / 10);
	GetVisualizationManager()->Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataView::SondeBathyOpacitySliderChanged(int value)
{
    mySondeBathyActor->GetProperty()->SetOpacity(((double)value) / 10);
	GetVisualizationManager()->Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataView::SetupBathyErrorMap()
{
	myBathyErrorColorFunction = vtkColorTransferFunction::New();
	//myBathyErrorColorFunction->SetColorSpaceToDiverging();

	myBathyErrorData = vtkPolyData::New();
	myBathyErrorData->CopyStructure(mySondeBathyReader->GetOutput());

	vtkFloatArray* oldVsSonar = vtkFloatArray::New();
	vtkFloatArray* oldVsSonde = vtkFloatArray::New();
	vtkFloatArray* sondeVsSonar = vtkFloatArray::New();
	oldVsSonar->SetName("OldVsSonar");
	sondeVsSonar->SetName("SondeVsSonar");
	oldVsSonde->SetName("OldVsSonde");

	int numMarkers = myVizMng->GetNumErrorMarkers();
	oldVsSonar->Allocate(numMarkers);
	oldVsSonde->Allocate(numMarkers);
	sondeVsSonar->Allocate(numMarkers);

	double range[2];
	range[0] = 1000;
	range[1] = 0;
	for(int i = 0; i < numMarkers; i++)
	{
		myVizMng->SetErrorMarkerMode(ErrorMarkerMode::OldVsSonar);
		double* data = myVizMng->GetErrorMarkerData();
		if(data[i] > range[1]) range[1] = data[i];
		if(data[i] < range[0]) range[0] = data[i];
		oldVsSonar->SetValue(i, data[i]);

		myVizMng->SetErrorMarkerMode(ErrorMarkerMode::SondeVsSonar);
		data = myVizMng->GetErrorMarkerData();
		if(data[i] > range[1]) range[1] = data[i];
		if(data[i] < range[0]) range[0] = data[i];
		sondeVsSonar->SetValue(i, data[i]);

		myVizMng->SetErrorMarkerMode(ErrorMarkerMode::OldVsSonde);
		data = myVizMng->GetErrorMarkerData();
		if(data[i] > range[1]) range[1] = data[i];
		if(data[i] < range[0]) range[0] = data[i];
		oldVsSonde->SetValue(i, data[i]);
	}

	range[0] /= 2;
	range[1] /= 2;

	myBathyErrorColorFunction->AddRGBPoint(range[0], 0, 0, 1);
	myBathyErrorColorFunction->AddRGBPoint(0, 1, 1, 1);
	myBathyErrorColorFunction->AddRGBPoint(range[1], 1, 0, 0);

	myBathyErrorData->GetPointData()->AddArray(oldVsSonar);
	myBathyErrorData->GetPointData()->AddArray(sondeVsSonar);
	myBathyErrorData->GetPointData()->AddArray(oldVsSonde);

	// TODO: normals and subd should be class objects.
    vtkPolyDataNormals* normals = vtkPolyDataNormals::New();
    normals->SetInput(myBathyErrorData);
    normals->SetFeatureAngle(45);
	normals->Update();

    vtkLoopSubdivisionFilter* subd = vtkLoopSubdivisionFilter::New();
	subd->SetInput(normals->GetOutput());
	subd->Update();

	myBathyErrorMapper = vtkPolyDataMapper::New();
	myBathyErrorMapper->SetInput(subd->GetOutput());
	myBathyErrorMapper->SetLookupTable(myBathyErrorColorFunction);

	myBathyErrorActor = vtkActor::New();
	myBathyErrorActor->SetMapper(myBathyErrorMapper);
	myBathyErrorActor->GetProperty()->SetLighting(0);
	myBathyErrorActor->SetScale(1, 0.01f, 1);
	myBathyErrorActor->SetPosition(0, 5, 0);
	myBathyErrorActor->SetVisibility(0);

	myVizMng->GetMainRenderer()->AddActor(myBathyErrorActor);

	myBathyErrorContour = vtkContourFilter::New();
	myBathyErrorContour->SetInput(subd->GetOutput());
	// 1 isoline every 2 meters.
	myBathyErrorContour->GenerateValues((int)(range[1] - range[0]) / 2, range);
	
	myBathyErrorContourMapper = vtkPolyDataMapper::New();
	myBathyErrorContourMapper->SetInput(myBathyErrorContour->GetOutput());
	myBathyErrorContourMapper->SetScalarVisibility(0);
	myBathyErrorContourMapper->Update();

	myBathyErrorContourActor = vtkActor::New();
	myBathyErrorContourActor->SetMapper(myBathyErrorContourMapper);
	myBathyErrorContourActor->GetProperty()->SetLighting(0);
	myBathyErrorContourActor->GetProperty()->SetLineWidth(2);
	myBathyErrorContourActor->GetProperty()->SetColor(0, 0, 0);
	myBathyErrorContourActor->VisibilityOff();
	myBathyErrorContourActor->SetPosition(0, 5.2f, 0);
	myBathyErrorContourActor->SetScale(1, 0.01f, 1);
	myVizMng->GetMainRenderer()->AddActor(myBathyErrorContourActor);

	// Setup the bathymewtry error map actor.
	myBathyErrorScaleActor = vtkScalarBarActor::New();
    myBathyErrorScaleActor->GetPositionCoordinate()->SetCoordinateSystemToDisplay();
    myBathyErrorScaleActor->GetPositionCoordinate()->SetValue(30, 55);
	myBathyErrorScaleActor->GetPosition2Coordinate()->SetCoordinateSystemToDisplay();
    myBathyErrorScaleActor->GetPosition2Coordinate()->SetValue(280, 50);
	myBathyErrorScaleActor->SetOrientationToHorizontal();
	myBathyErrorScaleActor->SetTitle("Depth difference (m)");
    myBathyErrorScaleActor->GetTitleTextProperty()->ShadowOff();
	myBathyErrorScaleActor->SetLookupTable(myBathyErrorColorFunction);
	myBathyErrorScaleActor->SetVisibility(0);
	myVizMng->GetMainRenderer()->AddActor2D(myBathyErrorScaleActor);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataView::OnOverlayOpacitySliderChanged(int value)
{
	myOverlayActor->GetProperty()->SetOpacity(((double)value) / 10);
	GetVisualizationManager()->Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataView::OnAxesButtonToggle(bool checked)
{
	VisualizationManager* mng = myVizMng;
	if(checked)
	{
		mng->GetMainRenderer()->AddActor(myAxesActor);
	}
	else
	{
		mng->GetMainRenderer()->RemoveActor(myAxesActor);
	}
	mng->Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataView::OnSondeContoursSliderReleased()
{
	mySondeBathyContour->GenerateValues(myUI->sondeContoursSlider->value(), 
		mySondeBathyReader->GetOutput()->GetScalarRange());
	GetVisualizationManager()->Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataView::OnOldContoursSliderReleased()
{
	myOldBathyContour->GenerateValues(myUI->oldContoursSlider->value(), 
		myReader->GetOutput()->GetScalarRange());
	GetVisualizationManager()->Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataView::OnOldContoursButtonToggle(bool checked)
{
	myOldBathyContourActor->SetVisibility(checked);
	GetVisualizationManager()->Render();
	OnOldContoursSliderReleased();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataView::OnSondeContoursButtonToggle(bool checked)
{
	mySondeBathyContourActor->SetVisibility(checked);
	GetVisualizationManager()->Render();
	OnSondeContoursSliderReleased();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataView::OnShowErrorMarkersButtonToggle(bool checked)
{
	ErrorMarkerMode::Enum mode = ErrorMarkerMode::Disabled;
	if(myUI->disabledMarkersButton->isChecked())
	{
		mode = ErrorMarkerMode::Disabled;
	}
	else if(myUI->oldVsSondeMarkersButton->isChecked())
	{
		mode = ErrorMarkerMode::OldVsSonde;
		myBathyErrorData->GetPointData()->SetActiveScalars("OldVsSonde");
	}
	else if(myUI->oldVsSonarMarkersButton->isChecked())
	{
		mode = ErrorMarkerMode::OldVsSonar;
		myBathyErrorData->GetPointData()->SetActiveScalars("OldVsSonar");
	}
	else if(myUI->sondeVsSonarMarkersButton->isChecked())
	{
		mode = ErrorMarkerMode::SondeVsSonar;
		myBathyErrorData->GetPointData()->SetActiveScalars("SondeVsSonar");
	}
	myVizMng->SetErrorMarkerMode(mode);
	if(mode != ErrorMarkerMode::Disabled)
	{
		myBathyErrorActor->SetVisibility(1);
		myBathyErrorScaleActor->SetVisibility(1);
		myBathyErrorContourActor->SetVisibility(1);
		myUI->errorMeanLabel->setVisible(true);
		myUI->errorStdDevLabel->setVisible(true);
		myUI->errorMeanLabel->setText(QString("%1").arg(
			myVizMng->GetErrorMarkerMean(), 5));
		myUI->errorStdDevLabel->setText(QString("%1").arg(
			myVizMng->GetErrorMarkerStdDev(), 5));
	}
	else
	{
		myBathyErrorActor->SetVisibility(0);
		myBathyErrorScaleActor->SetVisibility(0);
		myBathyErrorContourActor->SetVisibility(0);
		myUI->errorMeanLabel->setVisible(false);
		myUI->errorStdDevLabel->setVisible(false);
	}
	myVizMng->Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataView::OnShowSonarDataButtonToggle(bool checked)
{
	mySonarBathyActor->SetVisibility(checked);
	GetVisualizationManager()->Render();
}