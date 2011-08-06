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
#include "SectionView.h"
#include "AppConfig.h"
#include "Preferences.h"
#include "DataSet.h"
#include "GeoDataView.h"
#include "Utils.h"
#include "VisualizationManager.h"
#include "VtkDataManager.h"
#include "ColorFunctionManager.h"
#include "PointSourceWindow.h"


#include <vtkActor.h>
#include <vtkColorTransferFunction.h>
#include <vtkDataSetMapper.h>
#include <vtkImageData.h>
#include <vtkGenericCutter.h>
#include <vtkMath.h>
#include <vtkPlane.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkScalarBarActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkShepardMethod.h>
#include <vtkTextProperty.h>
#include <vtkThreshold.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkVector.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
SectionView::SectionView(VisualizationManager* mng, int index): 
	DockedTool(mng, QString("Section View %1").arg(index), Qt::RightDockWidgetArea)
{
	myVizMng = mng;
	GetMenuAction()->setIcon(QIcon(":/icons/section.png"));

	myVolumeSamples[0] = 60;
	myVolumeSamples[1] = 50;
	myVolumeSamples[2] = 60;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
SectionView::~SectionView()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SectionView::Initialize()
{
	SetupUI();

	vtkPointSet* grid = VtkDataManager::GetInstance()->GetPointSet(DataSet::AllData);

	vtkTransform* transform = vtkTransform::New();
	transform->Scale(1, 1, 1);

	mySeparationTransform = vtkTransformFilter::New();
	mySeparationTransform->SetTransform(transform);
	mySeparationTransform->SetInput(grid);
	mySeparationTransform->Update();

    myVolumeBuilder = vtkShepardMethod::New();
	myVolumeBuilder->SetInput(mySeparationTransform->GetOutput());
	myVolumeBuilder->SetModelBounds(mySeparationTransform->GetOutput()->GetBounds());
    myVolumeBuilder->SetSampleDimensions(myVolumeSamples[0], myVolumeSamples[1], myVolumeSamples[2]);
	myVolumeBuilder->SetMaximumDistance(3.0f);
    myVolumeBuilder->SetNullValue(999.0f);
	myVolumeBuilder->Update();

    myVolumeCleanupFilter = vtkThreshold::New();
	myVolumeCleanupFilter->SetInput(myVolumeBuilder->GetOutput());
    myVolumeCleanupFilter->ThresholdByLower(990);
	myVolumeCleanupFilter->Update();

	// Setup the scalar color bar.
    myScalarBar = vtkScalarBarActor::New();
    myScalarBar->GetPositionCoordinate()->SetValue(0, 0.15);
	myScalarBar->SetOrientationToVertical();
    myScalarBar->GetPosition2Coordinate()->SetValue(0.1, 0.9);
	myScalarBar->SetOrientationToVertical();
    myScalarBar->GetTitleTextProperty()->ShadowOff();

	myPlane = vtkPlane::New();
	myCutter = vtkGenericCutter::New();
	myCutter->SetCutFunction(myPlane);
	//myCutter->SetValue(0, 0);
	myCutter->SetInput(myVolumeBuilder->GetOutput());

	// Setup the probe mapper and actor.
    mySectionMapper = vtkDataSetMapper::New();
    mySectionMapper->SetInput(myCutter->GetOutput());
	mySectionMapper->ScalarVisibilityOn();

    mySectionActor = vtkActor::New();
    mySectionActor->SetMapper(mySectionMapper);
    mySectionActor->GetProperty()->BackfaceCullingOff();
    mySectionActor->GetProperty()->FrontfaceCullingOff();
	//mySectionActor->RotateX(90);
	//mySectionActor->SetScale(1, 1.0f / DataSet::SLICE_SEPARATION, 1);
	mySectionActor->GetProperty()->SetLighting(0);

    myRenderer = vtkRenderer::New();
	myRenderWindow = myUI->vtkView->GetRenderWindow();
	myRenderWindow->AddRenderer(myRenderer);
	myRenderer->AddActor(mySectionActor);
	myRenderer->AddActor2D(myScalarBar);

	OnFieldChanged(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SectionView::SetupUI()
{
	myUI = new Ui_SectionViewDock();
	myUI->setupUi(GetDockWidget());

	myUI->startXBox->setMaximum(FLT_MAX);
	myUI->startYBox->setMaximum(FLT_MAX);
	myUI->startZBox->setMaximum(FLT_MAX);
	myUI->endXBox->setMaximum(FLT_MAX);
	myUI->endYBox->setMaximum(FLT_MAX);
	myUI->endZBox->setMaximum(FLT_MAX);
	myUI->startXBox->setMinimum(-FLT_MAX);
	myUI->startYBox->setMinimum(-FLT_MAX);
	myUI->startZBox->setMinimum(-FLT_MAX);
	myUI->endXBox->setMinimum(-FLT_MAX);
	myUI->endYBox->setMinimum(-FLT_MAX);
	myUI->endZBox->setMinimum(-FLT_MAX);

    // Setup the components box.
	int numFields = myVizMng->GetDataSet()->GetInfo()->GetNumFields();
	for(int i = 0; i < numFields; i++)
    {
        myUI->fieldBox->addItem(myVizMng->GetDataSet()->GetFieldName(i), i);
    }

	connect(myUI->fieldBox, SIGNAL(currentIndexChanged(int)), SLOT(OnFieldChanged(int)));
	connect(myUI->applyButton, SIGNAL(clicked()), SLOT(OnApplyButtonClicked()));
	connect(myUI->startPointButton, SIGNAL(clicked()), SLOT(OnStartPointClicked()));
	connect(myUI->endPointButton, SIGNAL(clicked()), SLOT(OnEndPointClicked()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SectionView::Update()
{
	VisualizationManager* mng = myVizMng;
	DataSet* data = mng->GetDataSet();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SectionView::Render()
{
	myRenderWindow->Render();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
void SectionView::OnFieldChanged(int index)
{
	mySelectedField = index;
	// Force an update of sonde transform BEFORE setting the active scalars on the output object.
	// If I set the active scalars before forcing the update, the active scalars will be reset by the
	// update itself.
	// NOTE: I have to force an update here because changing the active scalars on point data does not
	// trigger the main dataset modified flag, and the subsequent volume update will do nothing.
	DataSet* data = myVizMng->GetDataSet();
	ColorFunctionManager* colorMng = myVizMng->GetColorFunctionManager();

	VtkDataManager::GetInstance()->GetPointSet(DataSet::AllData)->GetPointData()->SetActiveScalars(data->GetFieldName(index));

	myVolumeCleanupFilter->Update();

	// Update color legend.
    //myScalarBar->SetTitle(data->GetFieldName(mySelectedField));
	myScalarBar->SetLookupTable(colorMng->GetColorFunction(mySelectedField, true));

	myUI->vtkView->GetRenderWindow()->Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SectionView::OnExportImageButtonClicked()
{
	Utils::SaveScreenshot(myRenderWindow);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SectionView::OnApplyButtonClicked()
{
	double p1[3];
	double p2[3];
	double p3[3];

	p1[0] = myUI->startXBox->value();
	p1[1] = myUI->startYBox->value();
	p1[2] = myUI->startZBox->value();

	p2[0] = myUI->endXBox->value();
	p2[1] = myUI->endYBox->value();
	p2[2] = myUI->endZBox->value();

	p3[0] = myUI->startXBox->value();
	p3[1] = myUI->startYBox->value();
	p3[2] = myUI->endZBox->value();

	double v1[3];
	double v2[3];
	for(int i = 0; i < 3; i++)
	{
		v1[i] = p1[i] - p2[i];
		v2[i] = p1[i] - p3[i];
	}

	double normal[3];
	vtkMath::Cross(v1, v2, normal);
	vtkMath::Normalize(normal);

	myPlane->SetOrigin(p1);
	myPlane->SetNormal(normal);

	myCutter->Update();
	mySectionMapper->Update();
	myRenderWindow->Render();

	double* bounds = mySectionActor->GetBounds();
	Console::Message(QString("Section bounds: %1 %2  |  %3 %4  |  %5 %6")
		.arg(bounds[0]).arg(bounds[1]).arg(bounds[2]).arg(bounds[3]).arg(bounds[4]).arg(bounds[5]));

	bounds = myVolumeBuilder->GetOutput()->GetBounds();
	Console::Message(QString("Volume output bounds: %1 %2  |  %3 %4  |  %5 %6")
		.arg(bounds[0]).arg(bounds[1]).arg(bounds[2]).arg(bounds[3]).arg(bounds[4]).arg(bounds[5]));

	bounds = myCutter->GetOutput()->GetBounds();
	Console::Message(QString("Cutter output bounds: %1 %2  |  %3 %4  |  %5 %6")
		.arg(bounds[0]).arg(bounds[1]).arg(bounds[2]).arg(bounds[3]).arg(bounds[4]).arg(bounds[5]));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SectionView::OnStartPointClicked()
{
	PointSourceWindow::GetInstance()->exec();
	double point[3];
	if(PointSourceWindow::GetInstance()->GetPoint(point))
	{
		myUI->startXBox->setValue(point[0]);
		myUI->startYBox->setValue(point[1]);
		myUI->startZBox->setValue(point[2]);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SectionView::OnEndPointClicked()
{
	PointSourceWindow::GetInstance()->exec();
	double point[3];
	if(PointSourceWindow::GetInstance()->GetPoint(point))
	{
		myUI->endXBox->setValue(point[0]);
		myUI->endYBox->setValue(point[1]);
		myUI->endZBox->setValue(point[2]);
	}
}
