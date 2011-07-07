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
#include "PointSourceWindow.h"
#include "pq/pqColorChooserButton.h"

#include <vtkPLYReader.h>
#include <vtkElevationFilter.h>
#include <vtkDelimitedTextReader.h>
#include <vtkTableToPolyData.h>
#include <vtkTable.h>
#include <vtkGlyph3D.h>
#include <vtkGlyph2D.h>
#include <vtkGlyphSource2D.h>
#include <vtkBoxClipDataSet.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
GeoDataItem::GeoDataItem():
	myMeshUI(NULL),
	myImageUI(NULL),
	myPointsUI(NULL),
	myPanel(NULL),
	myActor(NULL),
	myContourActor(NULL),
	myElevationFilter(NULL),
	myGlyphFilter(NULL),
	myClipFilter(NULL)
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

	// Needed to force update of scalar range used for contour generation.
	reader->Update();
	double* bounds = reader->GetOutput()->GetBounds();
	Console::Message(QString("Mesh %1 bounds: %2 %3  |  %4 %5  |  %6 %7")
		.arg(myLabel).arg(bounds[0]).arg(bounds[1]).arg(bounds[2]).arg(bounds[3]).arg(bounds[4]).arg(bounds[5]));

	myClipFilter = vtkBoxClipDataSet::New();
	myClipFilter->SetInput(reader->GetOutput());
	myClipFilter->SetBoxClip(bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]);

	myMapper = vtkDataSetMapper::New();
	myMapper->SetInput(myClipFilter->GetOutput());
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

	if(cfg.exists("DrawMode"))
	{
		// Hack: for now always draw vertext glyphs when DrawMode is present.
		myGlyphFilter = vtkGlyph3D::New();
		myGlyphFilter->SetInput(reader->GetOutput());

		myClipFilter->SetInput(myGlyphFilter->GetOutput());
		myMapper->SetInput(myClipFilter->GetOutput());

		vtkGlyphSource2D* gs = vtkGlyphSource2D::New();
		gs->SetGlyphTypeToVertex();
		myGlyphFilter->SetSource(gs->GetOutput());
	}
	else
	{
		// Initialize contour.
		myElevationFilter = vtkElevationFilter::New();
		myElevationFilter->SetInput(myClipFilter->GetOutput());
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

	myMeshUI->startXBox->setMinimum(bounds[0]);
	myMeshUI->startXBox->setMaximum(bounds[1]);
	myMeshUI->endXBox->setMinimum(bounds[0]);
	myMeshUI->endXBox->setMaximum(bounds[1]);
	myMeshUI->startXBox->setValue(bounds[0]);
	myMeshUI->endXBox->setValue(bounds[1]);

	myMeshUI->startYBox->setMinimum(bounds[2]);
	myMeshUI->startYBox->setMaximum(bounds[3]);
	myMeshUI->endYBox->setMinimum(bounds[2]);
	myMeshUI->endYBox->setMaximum(bounds[3]);
	myMeshUI->startYBox->setValue(bounds[2]);
	myMeshUI->endYBox->setValue(bounds[3]);

	myMeshUI->startZBox->setMinimum(bounds[4]);
	myMeshUI->startZBox->setMaximum(bounds[5]);
	myMeshUI->endZBox->setMinimum(bounds[4]);
	myMeshUI->endZBox->setMaximum(bounds[5]);
	myMeshUI->startZBox->setValue(bounds[4]);
	myMeshUI->endZBox->setValue(bounds[5]);

	connect(myMeshUI->startPointButton, SIGNAL(clicked()), SLOT(OnStartPointClicked()));
	connect(myMeshUI->endPointButton, SIGNAL(clicked()), SLOT(OnEndPointClicked()));
	connect(myMeshUI->selApplyButton, SIGNAL(clicked()), SLOT(OnSelApplyClicked()));
	connect(myMeshUI->selResetButton, SIGNAL(clicked()), SLOT(OnSelResetClicked()));
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

	myClipFilter = vtkBoxClipDataSet::New();
	myClipFilter->SetInput(table2poly->GetOutput());
	myClipFilter->SetBoxClip(bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]);
	myClipFilter->Update();

	myMapper = vtkDataSetMapper::New();
	myMapper->SetInput(myClipFilter->GetOutput());
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

	myPointsUI->startXBox->setMinimum(bounds[0]);
	myPointsUI->startXBox->setMaximum(bounds[1]);
	myPointsUI->endXBox->setMinimum(bounds[0]);
	myPointsUI->endXBox->setMaximum(bounds[1]);
	myPointsUI->startXBox->setValue(bounds[0]);
	myPointsUI->endXBox->setValue(bounds[1]);

	myPointsUI->startYBox->setMinimum(bounds[2]);
	myPointsUI->startYBox->setMaximum(bounds[3]);
	myPointsUI->endYBox->setMinimum(bounds[2]);
	myPointsUI->endYBox->setMaximum(bounds[3]);
	myPointsUI->startYBox->setValue(bounds[2]);
	myPointsUI->endYBox->setValue(bounds[3]);

	myPointsUI->startZBox->setMinimum(bounds[4]);
	myPointsUI->startZBox->setMaximum(bounds[5]);
	myPointsUI->endZBox->setMinimum(bounds[4]);
	myPointsUI->endZBox->setMaximum(bounds[5]);
	myPointsUI->startZBox->setValue(bounds[4]);
	myPointsUI->endZBox->setValue(bounds[5]);

	connect(myPointsUI->startPointButton, SIGNAL(clicked()), SLOT(OnStartPointClicked()));
	connect(myPointsUI->endPointButton, SIGNAL(clicked()), SLOT(OnEndPointClicked()));
	connect(myPointsUI->selApplyButton, SIGNAL(clicked()), SLOT(OnSelApplyClicked()));
	connect(myPointsUI->selResetButton, SIGNAL(clicked()), SLOT(OnSelResetClicked()));
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
void GeoDataItem::OnSelApplyClicked()
{
	double bounds[6];
	if(myPointsUI != NULL)
	{
		bounds[0] = myPointsUI->startXBox->value();
		bounds[1] = myPointsUI->endXBox->value();
		bounds[2] = myPointsUI->startYBox->value();
		bounds[3] = myPointsUI->endYBox->value();
		bounds[4] = myPointsUI->startZBox->value();
		bounds[5] = myPointsUI->endZBox->value();
	}
	else if(myMeshUI != NULL)
	{
		bounds[0] = myMeshUI->startXBox->value();
		bounds[1] = myMeshUI->endXBox->value();
		bounds[2] = myMeshUI->startYBox->value();
		bounds[3] = myMeshUI->endYBox->value();
		bounds[4] = myMeshUI->startZBox->value();
		bounds[5] = myMeshUI->endZBox->value();
	}
	if(myClipFilter != NULL)
	{
		myClipFilter->SetBoxClip(bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]);
		myView->GetVisualizationManager()->Render();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataItem::OnSelResetClicked()
{
	if(myPointsUI != NULL)
	{
		double bounds[6];
		bounds[0] = myPointsUI->startXBox->minimum();
		bounds[1] = myPointsUI->startXBox->maximum();
		bounds[2] = myPointsUI->startYBox->minimum();
		bounds[3] = myPointsUI->startYBox->maximum();
		bounds[4] = myPointsUI->startZBox->minimum();
		bounds[5] = myPointsUI->startZBox->maximum();

		myPointsUI->startXBox->setValue(bounds[0]);
		myPointsUI->endXBox->setValue(bounds[1]);
		myPointsUI->startYBox->setValue(bounds[2]);
		myPointsUI->endYBox->setValue(bounds[3]);
		myPointsUI->startZBox->setValue(bounds[4]);
		myPointsUI->endZBox->setValue(bounds[5]);
	}
	else
	{
		double bounds[6];
		bounds[0] = myMeshUI->startXBox->minimum();
		bounds[1] = myMeshUI->startXBox->maximum();
		bounds[2] = myMeshUI->startYBox->minimum();
		bounds[3] = myMeshUI->startYBox->maximum();
		bounds[4] = myMeshUI->startZBox->minimum();
		bounds[5] = myMeshUI->startZBox->maximum();

		myMeshUI->startXBox->setValue(bounds[0]);
		myMeshUI->endXBox->setValue(bounds[1]);
		myMeshUI->startYBox->setValue(bounds[2]);
		myMeshUI->endYBox->setValue(bounds[3]);
		myMeshUI->startZBox->setValue(bounds[4]);
		myMeshUI->endZBox->setValue(bounds[5]);
	}
	OnSelApplyClicked();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataItem::OnStartPointClicked()
{
	PointSourceWindow::GetInstance()->exec();
	double point[3];
	if(PointSourceWindow::GetInstance()->GetPoint(point))
	{
		if(myMeshUI != NULL)
		{
			myMeshUI->startXBox->setValue(point[0]);
			myMeshUI->startYBox->setValue(point[1]);
			myMeshUI->startZBox->setValue(point[2]);
		}
		else if(myPointsUI != NULL)
		{
			myPointsUI->startXBox->setValue(point[0]);
			myPointsUI->startYBox->setValue(point[1]);
			myPointsUI->startZBox->setValue(point[2]);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataItem::OnEndPointClicked()
{
	PointSourceWindow::GetInstance()->exec();
	double point[3];
	if(PointSourceWindow::GetInstance()->GetPoint(point))
	{
		if(myMeshUI != NULL)
		{
			myMeshUI->endXBox->setValue(point[0]);
			myMeshUI->endYBox->setValue(point[1]);
			myMeshUI->endZBox->setValue(point[2]);
		}
		else if(myPointsUI != NULL)
		{
			myPointsUI->endXBox->setValue(point[0]);
			myPointsUI->endYBox->setValue(point[1]);
			myPointsUI->endZBox->setValue(point[2]);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataItem::GetClipStartPoint(double* point)
{
	if(myMeshUI != NULL)
	{
		point[0] = myMeshUI->startXBox->value();
		point[1] = myMeshUI->startYBox->value();
		point[2] = myMeshUI->startZBox->value();
	}
	else if(myPointsUI != NULL)
	{
		point[0] = myPointsUI->startXBox->value();
		point[1] = myPointsUI->startYBox->value();
		point[2] = myPointsUI->startZBox->value();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataItem::GetClipEndPoint(double* point)
{
	if(myMeshUI != NULL)
	{
		point[0] = myMeshUI->endXBox->value();
		point[1] = myMeshUI->endYBox->value();
		point[2] = myMeshUI->endZBox->value();
	}
	else if(myPointsUI != NULL)
	{
		point[0] = myPointsUI->endXBox->value();
		point[1] = myPointsUI->endYBox->value();
		point[2] = myPointsUI->endZBox->value();
	}
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

	QWidget* dock = GetDockWidget()->widget();
	QVBoxLayout* vbox = (QVBoxLayout*)dock->layout();
	vbox->addStretch();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataView::SetupUI()
{
	myUI = new Ui::GeoDataViewDock();
	myUI->setupUi(GetDockWidget());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataView::SetDepthScale(int value)
{
	for(int i = 0; i < myNumItems; i++)
	{
		myItems[i]->SetDepthScale(value);
	}
}


