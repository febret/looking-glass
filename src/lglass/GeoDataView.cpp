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
#include <vtkDelimitedTextReader.h>
#include <vtkTableToPolyData.h>
#include <vtkTable.h>
#include <vtkGlyph3D.h>
#include <vtkGlyph2D.h>
#include <vtkGlyphSource2D.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
GeoDataItem::GeoDataItem():
	myMeshUI(NULL),
	myImageUI(NULL),
	myPointsUI(NULL),
	myPanel(NULL),
	myActor(NULL),
	myContourActor(NULL),
	myElevationFilter(NULL),
	myGlyphFilter(NULL)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataItem::Initialize(GeoDataView* view, Setting& cfg)
{
	myView = view;

	QString type = cfg["Type"];
	if(type == "Mesh") myType = Mesh;
	else if(type == "Image") myType = Image;
	else if(type == "PointCloud") myType = PointCloud;
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
		else if(myType == PointCloud)
		{
			InitPoints(file, cfg);
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
	double* bounds = reader->GetOutput()->GetBounds();
	Console::Message(QString("Mesh %1 bounds: %2 %3  |  %4 %5  |  %6 %7")
		.arg(myLabel).arg(bounds[0]).arg(bounds[1]).arg(bounds[2]).arg(bounds[3]).arg(bounds[4]).arg(bounds[5]));

	if(cfg.exists("DrawMode"))
	{
		// Hack: for now always draw vertext glyphs when DrawMode is present.
		myGlyphFilter = vtkGlyph3D::New();
		myGlyphFilter->SetInput(reader->GetOutput());
		myMapper->SetInput(myGlyphFilter->GetOutput());

		vtkGlyphSource2D* gs = vtkGlyphSource2D::New();
		gs->SetGlyphTypeToVertex();
		myGlyphFilter->SetSource(gs->GetOutput());
	}
	else
	{
		// Initialize contour.
		myElevationFilter = vtkElevationFilter::New();
		myElevationFilter->SetInput(reader->GetOutput());
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
		myContourActor->SetPosition(0, 0.5f, 0);
		myContourActor->SetScale(1, VisualizationManager::DefaultDepthScale, 1);
	}

	InitActorTransform(cfg);

	vtkRenderer* renderer = myView->GetVisualizationManager()->GetMainRenderer();
	renderer->AddActor(myActor);
	if(myContourActor != NULL)
	{
		renderer->AddActor(myContourActor);
	}

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

	if(myContourActor != NULL)
	{
		myContourColorButton = new pqColorChooserButton(myPanel);
		myPanel->layout()->addWidget(myContourColorButton);
		myContourColorButton->setText("Contour Color");
		myContourColorButton->setChosenColor(QColor(0, 0, 0));
		connect(myContourColorButton, SIGNAL(chosenColorChanged(const QColor&)), SLOT(OnContourColorChanged(const QColor&)));
	}
	else
	{
		myMeshUI->contourCheck->setVisible(false);
		myMeshUI->contourStartBox->setVisible(false);
		myMeshUI->contourIntervalBox->setVisible(false);
		myMeshUI->label->setVisible(false);
		myMeshUI->label_2->setVisible(false);
		myMeshUI->refreshButton->setVisible(false);
	}

	connect(myMeshUI->visibleCheck, SIGNAL(toggled(bool)), SLOT(OnVisibleToggle(bool)));
	connect(myMainColorButton, SIGNAL(chosenColorChanged(const QColor&)), SLOT(OnMainColorChanged(const QColor&)));
	connect(myMeshUI->opacityBox, SIGNAL(valueChanged(double)), SLOT(OnOpacityChanged(double)));

	connect(myMeshUI->refreshButton, SIGNAL(clicked()), SLOT(OnRefreshClicked()));
	connect(myMeshUI->contourCheck, SIGNAL(toggled(bool)), SLOT(OnContourToggle(bool)));
	connect(myMeshUI->contourStartBox, SIGNAL(valueChanged(double)), SLOT(OnContourParamsChanged()));
	connect(myMeshUI->contourIntervalBox, SIGNAL(valueChanged(double)), SLOT(OnContourParamsChanged()));

	myMeshUI->refreshButton->setEnabled(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataItem::InitPoints(QFile* file, Setting& cfg)
{
	vtkDelimitedTextReader* reader = vtkDelimitedTextReader::New();
	reader->SetFileName(file->fileName().ascii());
	reader->SetFieldDelimiterCharacters(",");
	reader->SetDetectNumericColumns(1);
	reader->SetHaveHeaders(0);
	reader->Update();

	vtkTable* tb = reader->GetOutput();
	int nr = tb->GetNumberOfRows();

	vtkTableToPolyData* table2poly = vtkTableToPolyData::New();
	table2poly->SetInputConnection(0, reader->GetOutputPort(0));
	table2poly->SetXColumn(cfg["XColumn"]);
	table2poly->SetYColumn(cfg["YColumn"]);
	table2poly->SetZColumn(cfg["ZColumn"]);
	table2poly->Update();

	double* bounds = table2poly->GetOutput()->GetBounds();
	Console::Message(QString("Point cloud %1 size %2, bounds: %3 %4  |  %5 %6  |  %7 %8")
		.arg(myLabel).arg(nr).arg(bounds[0]).arg(bounds[1]).arg(bounds[2]).arg(bounds[3]).arg(bounds[4]).arg(bounds[5]));

	myMapper = vtkDataSetMapper::New();
	myMapper->SetInput(table2poly->GetOutput());
	//myMapper->SetLookupTable(lakeLut);
	myMapper->Update();

	myActor = vtkActor::New();
	myActor->SetMapper(myMapper);
	myActor->GetProperty()->BackfaceCullingOff();
	myActor->GetProperty()->FrontfaceCullingOff();
	myActor->GetProperty()->SetAmbientColor(1, 1, 1);
	myActor->GetProperty()->SetAmbient(0.2);
	myActor->SetScale(1, VisualizationManager::DefaultDepthScale, 1);
	myActor->GetProperty()->SetLighting(0);
	myActor->PickableOff();

	InitActorTransform(cfg);

	vtkRenderer* renderer = myView->GetVisualizationManager()->GetMainRenderer();
	renderer->AddActor(myActor);

	reader->Delete();
	table2poly->Delete();

	// Setup the UI
	myPointsUI = new Ui_GeoDataPointsPanel();
	myPointsUI->setupUi(myPanel);

	connect(myPointsUI->visibleCheck, SIGNAL(toggled(bool)), SLOT(OnVisibleToggle(bool)));
	connect(myPointsUI->opacityBox, SIGNAL(valueChanged(double)), SLOT(OnOpacityChanged(double)));
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
	myActor->PickableOff();
	myActor->GetProperty()->SetLighting(0);

	InitActorTransform(cfg);

	vtkRenderer* renderer = myView->GetVisualizationManager()->GetMainRenderer();
	renderer->AddActor(myActor);

	reader->Delete();

	// Setup the UI
	myImageUI = new Ui_GeoDataImagePanel();
	myImageUI->setupUi(myPanel);

	connect(myImageUI->visibleCheck, SIGNAL(toggled(bool)), SLOT(OnVisibleToggle(bool)));
	connect(myImageUI->opacityBox, SIGNAL(valueChanged(double)), SLOT(OnOpacityChanged(double)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataItem::InitActorTransform(Setting& cfg)
{
	if(cfg.exists("Scale"))
	{
		Setting& s = cfg["Scale"];
		myScale[0] = s[0];
		myScale[1] = s[1];
		myScale[2] = s[2];
	}
	else
	{
		myScale[0] = myScale[1] = myScale[2] = 1;
	}
	if(cfg.exists("Position"))
	{
		Setting& s = cfg["Position"];
		myPosition[0] = s[0];
		myPosition[1] = s[1];
		myPosition[2] = s[2];
	}
	else
	{
		myPosition[0] = myPosition[1] = myPosition[2] = 0;
	}
	if(cfg.exists("Orientation"))
	{
		Setting& s = cfg["Orientation"];
		myOrientation[0] = s[0];
		myOrientation[1] = s[1];
		myOrientation[2] = s[2];
	}
	else
	{
		myOrientation[0] = myOrientation[1] = myOrientation[2] = 0;
	}

	myActor->SetOrientation(myOrientation[0], myOrientation[1], myOrientation[2]);
	myActor->SetScale(myScale[0], myScale[1], myScale[2]);
	myActor->SetPosition(myPosition[0], myPosition[1], myPosition[2]);

	if(myContourActor != NULL)
	{
		myContourActor->SetOrientation(myOrientation[0], myOrientation[1], myOrientation[2]);
		myContourActor->SetScale(myScale[0], myScale[1], myScale[2]);
		myContourActor->SetPosition(myPosition[0], myPosition[1], myPosition[2]);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataItem::SetDepthScale(int value)
{
	myActor->SetScale(myScale[0], myScale[1] * value, myScale[2]);
	if(myContourActor != NULL)
	{
		myContourActor->SetScale(myScale[0], myScale[1] * value, myScale[2]);
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


