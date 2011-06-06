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
#include "AppConfig.h"
#include "GeoDataView.h"
#include "DataSet.h"
#include "VisualizationManager.h"
#include "RepositoryManager.h"
#include "ui_MainWindow.h"
#include "pq/pqColorChooserButton.h"

#include <vtkPLYReader.h>
#include <vtkElevationFilter.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
GeoDataItem::GeoDataItem():
	myMeshUI(NULL),
	myImageUI(NULL),
	myPointsUI(NULL),
	myPanel(NULL)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataItem::Initialize(GeoDataView* view, Setting& cfg)
{
	myView = view;

	QString type = cfg["Type"];
	if(type == "Mesh") myType = Mesh;
	else if(type == "Image") myType = Image;
	else if(type == "PointSet") myType = PointSet;
	else Console::Message(QString("GeoDataItem::Initialize - unknown item type: %1").arg(type));

	myFilename = cfg["File"];
	myLabel = cfg["Label"];

	QFile* file = RepositoryManager::GetInstance()->TryOpen(myFilename);
	if(file != NULL)
	{
		// Create the panel containing this geo item ui.
		QWidget* dock = myView->GetDockWidget()->widget();
		myPanel = new pqCollapsedGroup(dock);
		myPanel->setTitle(myLabel);
		dock->layout()->addWidget(myPanel);


		if(myType == Mesh)
		{
			InitMesh(file, cfg);
		}
		else if(myType == Image)
		{
			InitImage(file, cfg);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataItem::InitMesh(QFile* file, Setting& cfg)
{
	vtkPLYReader* reader = vtkPLYReader::New();
	reader->SetFileName(file->fileName().ascii());
	myMapper = vtkDataSetMapper::New();
	myMapper->SetInput(reader->GetOutput());
	//myMapper->SetLookupTable(lakeLut);
	myMapper->Update();

	myActor = vtkActor::New();
	myActor->SetMapper(myMapper);
	myActor->GetProperty()->BackfaceCullingOff();
	myActor->GetProperty()->FrontfaceCullingOff();
	myActor->GetProperty()->SetAmbientColor(1, 1, 1);
	myActor->GetProperty()->SetAmbient(0.2);
	//myActor->SetPosition(0, 0, 0);
	myActor->SetScale(1, VisualizationManager::DefaultDepthScale, 1);
	myActor->PickableOff();

	// Needed to force update of scalar range used for contour generation.
	reader->Update();

	// Initialize bathy contour.
	myElevationFilter = vtkElevationFilter::New();
	myElevationFilter->SetInput(reader->GetOutput());
	double* bounds = reader->GetOutput()->GetBounds();
	myElevationFilter->SetLowPoint(0, bounds[3], 0);
	myElevationFilter->SetHighPoint(0, bounds[2], 0);
	myElevationFilter->SetScalarRange(bounds[3], bounds[2]);
	myElevationFilter->Update();

	myContourFilter = vtkContourFilter::New();
	myContourFilter->SetInput(myElevationFilter->GetOutput());
	double rstart = myElevationFilter->GetScalarRange()[0];
	double rend = myElevationFilter->GetScalarRange()[1];
	//Console::Message(QString("Rstart %1 Rend %2").arg(rstart).arg(rend));
	myContourFilter->GenerateValues(20, rstart, rend);
	myContourMapper = vtkPolyDataMapper::New();
	myContourMapper->SetInput(myContourFilter->GetOutput());
	myContourMapper->SetScalarVisibility(0);
	myContourMapper->Update();

	myContourActor = vtkActor::New();
	myContourActor->SetMapper(myContourMapper);
	myContourActor->GetProperty()->SetLighting(0);
	myContourActor->GetProperty()->SetLineWidth(1);
	myContourActor->GetProperty()->SetOpacity(1);
	myContourActor->VisibilityOff();
	myContourActor->SetScale(1, VisualizationManager::DefaultDepthScale, 1);

	vtkRenderer* renderer = myView->GetVisualizationManager()->GetMainRenderer();
	renderer->AddActor(myActor);
	renderer->AddActor(myContourActor);

	reader->Delete();

	// Setup the UI
	myMeshUI = new Ui_GeoDataMeshPanel();
	myMeshUI->setupUi(myPanel);

	myMeshUI->contourStartBox->setMinimum(bounds[2]);
	myMeshUI->contourStartBox->setMaximum(bounds[3]);
	myMeshUI->contourIntervalBox->setValue(1);

	myMainColorButton = new pqColorChooserButton(myPanel);
	myPanel->layout()->addWidget(myMainColorButton);
	myMainColorButton->setText("Color");
	myMainColorButton->setChosenColor(QColor(255, 255, 255));

	myContourColorButton = new pqColorChooserButton(myPanel);
	myPanel->layout()->addWidget(myContourColorButton);
	myContourColorButton->setText("Contour Color");
	myContourColorButton->setChosenColor(QColor(0, 0, 0));

	connect(myMeshUI->visibleCheck, SIGNAL(toggled(bool)), SLOT(OnVisibleToggle(bool)));
	connect(myMainColorButton, SIGNAL(chosenColorChanged(const QColor&)), SLOT(OnMainColorChanged(const QColor&)));
	connect(myContourColorButton, SIGNAL(chosenColorChanged(const QColor&)), SLOT(OnContourColorChanged(const QColor&)));
	connect(myMeshUI->opacityBox, SIGNAL(valueChanged(double)), SLOT(OnOpacityChanged(double)));

	connect(myMeshUI->refreshButton, SIGNAL(clicked()), SLOT(OnRefreshClicked()));
	connect(myMeshUI->contourCheck, SIGNAL(toggled(bool)), SLOT(OnContourToggle(bool)));
	connect(myMeshUI->contourStartBox, SIGNAL(valueChanged(double)), SLOT(OnContourParamsChanged()));
	connect(myMeshUI->contourIntervalBox, SIGNAL(valueChanged(double)), SLOT(OnContourParamsChanged()));

	myMeshUI->refreshButton->setEnabled(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataItem::InitImage(QFile* file, Setting& cfg)
{
	Setting& sOrientation = cfg["Orientation"];
	Setting& sScale = cfg["Scale"];
	Setting& sPosition = cfg["Position"];

	// Initialize the map overlay.
	vtkJPEGReader* reader = vtkJPEGReader::New();
	reader->SetFileName(file->fileName().ascii());
	reader->Update();

	vtkTexture* texture = vtkTexture::New();
	texture->SetInput(reader->GetOutput());
	texture->InterpolateOn();

	vtkPlaneSource* plane = vtkPlaneSource::New();
	plane->SetOrigin(0, 0, 0);
	plane->SetPoint2(0, 0, -1);
	plane->SetPoint1(1, 0, 0);
	
	myMapper = vtkDataSetMapper::New();
	myMapper->SetInput(plane->GetOutput());
	myMapper->Update();

	myActor = vtkActor::New();
	myActor->SetMapper(myMapper);
	myActor->SetTexture(texture);
	myActor->GetProperty()->BackfaceCullingOff();
	myActor->GetProperty()->FrontfaceCullingOff();
	myActor->SetTexture(texture);
	myActor->SetOrientation(sOrientation[0], sOrientation[1], sOrientation[2]);
	myActor->SetScale(sScale[0], sScale[1], sScale[2]);
	myActor->SetPosition(sPosition[0], sPosition[1], sPosition[2]);
	myActor->GetProperty()->SetOpacity(1);
	myActor->PickableOff();
	myActor->GetProperty()->SetLighting(0);

	vtkRenderer* renderer = myView->GetVisualizationManager()->GetMainRenderer();
	renderer->AddActor(myActor);

	reader->Delete();

	// Setup the UI
	myImageUI = new Ui_GeoDataImagePanel();
	myImageUI->setupUi(myPanel);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataItem::SetDepthScale(int value)
{
	if(myType != Image)
	{
		myActor->SetScale(1, value, 1);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataItem::OnVisibleToggle(bool checked)
{
	myActor->SetVisibility(checked);
	myView->GetVisualizationManager()->Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataItem::OnContourToggle(bool checked)
{
	myContourActor->SetVisibility(checked);
	myView->GetVisualizationManager()->Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataItem::OnMainColorChanged(const QColor& color)
{
	myActor->GetProperty()->SetColor(QCOLOR_TO_VTK(color));
	myView->GetVisualizationManager()->Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataItem::OnOpacityChanged(double value)
{
    myActor->GetProperty()->SetOpacity(value);
	myView->GetVisualizationManager()->Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataItem::OnContourColorChanged(const QColor& color)
{
	myContourActor->GetProperty()->SetColor(QCOLOR_TO_VTK(color));
	myView->GetVisualizationManager()->Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataItem::OnContourParamsChanged()
{
	myMeshUI->refreshButton->setEnabled(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataItem::OnRefreshClicked()
{
	double rangeStart = myMeshUI->contourStartBox->value();
	double rangeInterval = myMeshUI->contourIntervalBox->value();

	double rangeEnd = myElevationFilter->GetOutput()->GetBounds()[2];
	double range = rangeEnd - rangeStart;
	if(range < 0) range = -range;
	int numContours = range / rangeInterval;
	//Console::Message(QString("Rstart %1 Rend %2 Contours %3").arg(rangeStart).arg(rangeEnd).arg(numContours));
	myContourFilter->GenerateValues(numContours, rangeStart, rangeEnd); 
	myContourFilter->Update();
	myView->GetVisualizationManager()->Render();

	myMeshUI->refreshButton->setEnabled(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
GeoDataView::GeoDataView(VisualizationManager* mng): 
	DockedTool(mng, "Geographic Data"),
	myNumItems(0),
	myVizMng(mng)
{
	memset(myItems, 0, MaxGeoDataItems * sizeof(void*));

	GetMenuAction()->setIcon(QIcon(":/icons/GeoDataView.png"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
GeoDataView::~GeoDataView()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataView::Initialize()
{
	SetupUI();

	Config* cfg = AppConfig::GetInstance()->GetDataConfig();
	Setting& sGeoData = cfg->lookup("Application/GeoData");

	for(int i = 0; i < sGeoData.getLength(); i++)
	{
		Setting& sItem = sGeoData[i];
		GeoDataItem* item = new GeoDataItem();
		item->Initialize(this, sItem);
		myItems[i] = item;
		myNumItems++;
	}

 //   SetInitMessage("Initializing Bathymetry Model...");

	//QString oldBathymetryFile = AppConfig::GetInstance()->GetProfileName() + "/bonney-09.vtk";
	//QString sonarBathymetryFile = AppConfig::GetInstance()->GetProfileName() + "/bonney-09.vtk";
	////QString sondeBathymetryFile = AppConfig::GetInstance()->GetProfileName() + "/sonde_bathymetry.vtk";
	//
	//QString sondeBathymetryFile = AppConfig::GetInstance()->GetProfileName() + "/bonney-09.vtk";

	////QString sondeBathymetryFile = AppConfig::GetInstance()->GetProfileName() + "/bathymetry.vtk";

	//VisualizationManager* mng = myVizMng;
	//vtkRenderer* renderer = mng->GetMainRenderer();

 //   myReader = vtkPolyDataReader::New();
 //   myReader->SetFileName(oldBathymetryFile);
 //   myReader->Update();

	//double* depthRange;
	//depthRange = myReader->GetOutput()->GetScalarRange();
	//vtkColorTransferFunction* lakeLut = vtkColorTransferFunction::New();

 //   myOldBathyMapper = vtkPolyDataMapper::New();
	//myOldBathyMapper->SetInput(myReader->GetOutput());
 //   myOldBathyMapper->SetLookupTable(lakeLut);
 //   myOldBathyMapper->SetScalarRange(depthRange);
	//myReader->GetOutput()->GetPointData()->SetActiveScalars("Scalar");

	//SetOldBathyColor(myOldBathyColor);
 //   myOldBathyMapper->Update();

 //   myLakeActor = vtkActor::New();
 //   myLakeActor->SetMapper(myOldBathyMapper);
 //   myLakeActor->GetProperty()->BackfaceCullingOff();
 //   myLakeActor->GetProperty()->FrontfaceCullingOff();
 //   myLakeActor->GetProperty()->SetAmbientColor(1, 1, 1);
 //   myLakeActor->GetProperty()->SetAmbient(0.2);
	//myLakeActor->SetPosition(0, 0, 0);
	//myLakeActor->SetScale(1, 7, 1);
	//myLakeActor->PickableOff();

	//// SONDE_BASED BATHYMETRY
 //   mySondeBathyReader = vtkPolyDataReader::New();
 //   mySondeBathyReader->SetFileName(sondeBathymetryFile);
 //   mySondeBathyReader->Update();
	//mySondeBathyReader->GetOutput()->GetPointData()->SetActiveScalars("Z");

	//double* range = mySondeBathyReader->GetOutput()->GetBounds();
	//Console::Message(QString("Sonde Bathy Range: %1-%2 %3-%4 %5-%6").arg(range[0]).arg(range[1]).arg(range[2]).arg(range[3]).arg(range[4]).arg(range[5]));

	//range = myReader->GetOutput()->GetBounds();
	//Console::Message(QString("Old Bathy Range: %1-%2 %3-%4 %5-%6").arg(range[0]).arg(range[1]).arg(range[2]).arg(range[3]).arg(range[4]).arg(range[5]));

	//float* fr = myVizMng->GetDataSet()->GetZRange(); 
	//depthRange[0] = fr[0];
	//depthRange[1] = fr[1];
	//lakeLut = vtkColorTransferFunction::New();

 //   mySondeBathyMapper = vtkPolyDataMapper::New();
	//mySondeBathyMapper->SetInput(mySondeBathyReader->GetOutput());
 //   mySondeBathyMapper->SetLookupTable(lakeLut);
 //   mySondeBathyMapper->SetScalarRange(depthRange);
	//mySondeBathyMapper->SetColorModeToMapScalars();

	//SetSondeBathyColor(mySondeBathyColor);
 //   mySondeBathyMapper->Update();

 //   mySondeBathyActor = vtkActor::New();
 //   mySondeBathyActor->SetMapper(mySondeBathyMapper);
 //   mySondeBathyActor->GetProperty()->BackfaceCullingOff();
 //   mySondeBathyActor->GetProperty()->FrontfaceCullingOff();
 //   mySondeBathyActor->GetProperty()->SetAmbientColor(0.5, 0.5, 0.5);
 //   mySondeBathyActor->GetProperty()->SetAmbient(0.2);
	//mySondeBathyActor->SetPosition(0, 0, 0);
	//mySondeBathyActor->GetProperty()->SetInterpolationToGouraud();
	//mySondeBathyActor->GetProperty()->SetOpacity(0);
	//mySondeBathyActor->PickableOff();


	//// Sonar based bathymetry.
	//mySonarBathyReader = vtkPolyDataReader::New();
 //   mySonarBathyReader->SetFileName(sonarBathymetryFile);
 //   mySonarBathyReader->Update();

	////double* range = mySonarBathyReader->GetOutput()->GetBounds();
	////printf("Sonar Bathy Range: %f-%f %f-%f %f-%f\n", range[0], range[1], range[2], range[3], range[4], range[5]);

	//mySonarBathyMapper = vtkDataSetMapper::New();
	//mySonarBathyMapper->SetInput(mySonarBathyReader->GetOutput());
	//mySonarBathyMapper->Update();
	//mySonarBathyMapper->ScalarVisibilityOff();

	//mySonarBathyActor = vtkActor::New();
	//mySonarBathyActor->SetVisibility(0);
 //   mySonarBathyActor->SetMapper(mySonarBathyMapper);
	//mySonarBathyActor->GetProperty()->SetRepresentationToPoints();
	//mySonarBathyActor->GetProperty()->SetColor(1, 1, 1);

 //   // a renderer and render window
 //   renderer->AddActor(myLakeActor);
 //   renderer->AddActor(mySondeBathyActor);
 //   renderer->AddActor(mySonarBathyActor);

	//// Initialize the map overlay.
	//myOverlayReader = vtkJPEGReader::New();
	//QString overlayFile = AppConfig::GetInstance()->GetProfileName() + "/overlay.jpg";
	//myOverlayReader->SetFileName(overlayFile);
	//myOverlayTexture = vtkTexture::New();
	//myOverlayTexture->SetInput(myOverlayReader->GetOutput());
	//myOverlayTexture->InterpolateOn();
	//myOverlayPlane = vtkPlaneSource::New();
	//myOverlayPlane->SetOrigin(0, 0, 0);
	//myOverlayPlane->SetPoint2(0, 0, -1);
	//myOverlayPlane->SetPoint1(1, 0, 0);
	//myOverlayMapper = vtkPolyDataMapper::New();
	//myOverlayMapper->SetInput(myOverlayPlane->GetOutput());
	//myOverlayActor = vtkActor::New();
	//myOverlayActor->SetMapper(myOverlayMapper);
	//myOverlayActor->SetTexture(myOverlayTexture);

	//// TODO: Argh, hardcoded values.
	//myOverlayActor->SetOrientation(0, 90, 0);
	//myOverlayActor->SetScale(-2936, 1, -2284);
	//myOverlayActor->SetPosition(1370239, 0, 434526);
	//myOverlayActor->GetProperty()->SetOpacity(0);
	//myOverlayActor->PickableOff();
	//myOverlayActor->GetProperty()->SetLighting(0);

	//renderer->AddActor(myOverlayActor);
	//GetVisualizationManager()->Render();

	//DataSet* data = myVizMng->GetDataSet();

	//SetupContours();
	//SetupUI();

	//// Enable contours on old bathymetry by default.
	//myUI->oldContoursButton->setChecked(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataView::SetupUI()
{
	myUI = new Ui::GeoDataViewDock();
	myUI->setupUi(GetDockWidget());
	//myUI->axesButton->hide();

	// Set dafault values to controls.
 //   myUI->opacitySlider->setValue(100);
 //   myUI->sondeBathyOpacitySlider->setValue(0);
 //   myUI->sondeContoursSlider->setValue(10);
 //   myUI->oldContoursSlider->setValue(10);
	//myUI->disabledMarkersButton->setChecked(true);

	//myUI->errorMeanLabel->setVisible(false);
	//myUI->errorStdDevLabel->setVisible(false);

	// Wire events.
 //   connect(myUI->opacitySlider, SIGNAL(valueChanged(int)), 
	//	SLOT(OpacitySliderChanged(int)));

	//connect(myUI->overlayOpacitySlider, SIGNAL(valueChanged(int)), 
	//	SLOT(OnOverlayOpacitySliderChanged(int)));

	//connect(myUI->sondeBathyOpacitySlider, SIGNAL(valueChanged(int)), 
	//	SLOT(SondeBathyOpacitySliderChanged(int)));

	//connect(myUI->axesButton, SIGNAL(toggled(bool)), 
	//	SLOT(OnAxesButtonToggle(bool)));

	//connect(myUI->oldContoursSlider, SIGNAL(sliderReleased()), 
	//	SLOT(OnOldContoursSliderReleased()));

	//connect(myUI->sondeContoursSlider, SIGNAL(sliderReleased()), 
	//	SLOT(OnSondeContoursSliderReleased()));

	//connect(myUI->sondeContoursButton, SIGNAL(toggled(bool)), 
	//	SLOT(OnSondeContoursButtonToggle(bool)));

	//connect(myUI->oldContoursButton, SIGNAL(toggled(bool)), 
	//	SLOT(OnOldContoursButtonToggle(bool)));

	//connect(myUI->disabledMarkersButton, SIGNAL(toggled(bool)), 
	//	SLOT(OnShowErrorMarkersButtonToggle(bool)));

	//connect(myUI->oldVsSondeMarkersButton, SIGNAL(toggled(bool)), 
	//	SLOT(OnShowErrorMarkersButtonToggle(bool)));

	//connect(myUI->oldVsSonarMarkersButton, SIGNAL(toggled(bool)), 
	//	SLOT(OnShowErrorMarkersButtonToggle(bool)));

	//connect(myUI->sondeVsSonarMarkersButton, SIGNAL(toggled(bool)), 
	//	SLOT(OnShowErrorMarkersButtonToggle(bool)));

	//connect(myUI->showSonarDataButton, SIGNAL(toggled(bool)), 
	//	SLOT(OnShowSonarDataButtonToggle(bool)));

	//myUI->errorMarkerBox->setVisible(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataView::SetDepthScale(int value)
{
	for(int i = 0; i < myNumItems; i++)
	{
		myItems[i]->SetDepthScale(value);
	}
}


