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
#include "LineTool.h"
#include "DataSet.h"
#include "VisualizationManager.h"
#include "RepositoryManager.h"
#include "PointSourceWindow.h"
#include "ui_MainWindow.h"
#include "pq/pqColorChooserButton.h"
#include "vtkLineRepresentation.h"
#include "vtkLineWidget2.h"

#include <vtkProperty.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
VTK_CALLBACK(LineWidgetChangedCallback, LineTool, OnLineWidgetChanged());

///////////////////////////////////////////////////////////////////////////////////////////////////
LineTool::LineTool(VisualizationManager* mng): 
	DockedTool(mng, "Line Tool"),
	myVizMng(mng)
{
	GetMenuAction()->setIcon(QIcon(":/icons/vector.png"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
LineTool::~LineTool()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void LineTool::Initialize()
{
	float* xrange = myVizMng->GetDataSet()->GetYRange();
	float* yrange = myVizMng->GetDataSet()->GetZRange();
	float* zrange = myVizMng->GetDataSet()->GetXRange();

	for( int i = 0; i < MaxLines; i++)
	{
		myLines[i].Visible = false;
		myLines[i].Color = QColor(255, 255, 255);
		myLines[i].Max[0] = xrange[1];
		myLines[i].Max[1] = yrange[1];
		myLines[i].Max[2] = zrange[1];
		myLines[i].Min[0] = xrange[0];
		myLines[i].Min[1] = yrange[0];
		myLines[i].Min[2] = zrange[0];
	}

	// The line widget is used probe the dataset.
	myLineRep = vtkLineRepresentation::New();
	double bounds[6];
	bounds[0] = xrange[0];
	bounds[1] = xrange[1];
	bounds[2] = yrange[0];
	bounds[3] = yrange[1];
	bounds[4] = zrange[0];
	bounds[5] = zrange[1];
	myLineRep->PlaceWidget(bounds);
	//myLineRep->DistanceAnnotationVisibilityOn();
	//myLineRep->SetRenderer(myVizMng->GetMainRenderer());
	//myLineRep->BuildRepresentation();
	

	myLineWidget = vtkLineWidget2::New();
	myLineWidget->SetRepresentation(myLineRep);
	myLineWidget->AddObserver(vtkCommand::InteractionEvent, new LineWidgetChangedCallback(this));
	myVizMng->Add3DWidget(myLineWidget);
	myLineWidget->EnabledOn();

	SetupUI();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void LineTool::SetupUI()
{
	myUI = new Ui::LineToolDock();
	myUI->setupUi(GetDockWidget());

	QWidget* dock = GetDockWidget()->widget();
	myLineColorButton = new pqColorChooserButton(dock);
	dock->layout()->addWidget(myLineColorButton);
	
	myLineColorButton->setText("Line Color");
	myLineColorButton->setChosenColor(QColor(255, 255, 255));

	QVBoxLayout* vbox = (QVBoxLayout*)dock->layout();
	vbox->addStretch();

	for( int i = 0; i < MaxLines; i++)
	{
		myUI->lineBox->addItem(QString("Line %1").arg(i));
	}

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

	connect(myUI->lineBox, SIGNAL(currentIndexChanged(int)), SLOT(OnLineCurrentIndexChanged(int)));
	connect(myUI->showLineButton, SIGNAL(toggled(bool)), SLOT(OnShowLineToggled(bool)));
	connect(myLineColorButton, SIGNAL(chosenColorChanged(const QColor&)), SLOT(OnLineColorChanged(const QColor&)));
	connect(myUI->startXBox, SIGNAL(valueChanged(double)), SLOT(OnLineBoundsChanged()));
	connect(myUI->startYBox, SIGNAL(valueChanged(double)), SLOT(OnLineBoundsChanged()));
	connect(myUI->startZBox, SIGNAL(valueChanged(double)), SLOT(OnLineBoundsChanged()));
	connect(myUI->endXBox, SIGNAL(valueChanged(double)), SLOT(OnLineBoundsChanged()));
	connect(myUI->endYBox, SIGNAL(valueChanged(double)), SLOT(OnLineBoundsChanged()));
	connect(myUI->endZBox, SIGNAL(valueChanged(double)), SLOT(OnLineBoundsChanged()));
	connect(myUI->startPointButton, SIGNAL(clicked()), SLOT(OnStartPointClicked()));
	connect(myUI->endPointButton, SIGNAL(clicked()), SLOT(OnEndPointClicked()));

	OnLineCurrentIndexChanged(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void LineTool::OnLineCurrentIndexChanged(int)
{
	myIndexChanging = true;
	int i = myUI->lineBox->currentIndex();
	myUI->endXBox->setValue(myLines[i].Max[0]);
	myUI->endYBox->setValue(myLines[i].Max[1]);
	myUI->endZBox->setValue(myLines[i].Max[2]);
	myUI->startXBox->setValue(myLines[i].Min[0]);
	myUI->startYBox->setValue(myLines[i].Min[1]);
	myUI->startZBox->setValue(myLines[i].Min[2]);
	myLineColorButton->setChosenColor(myLines[i].Color);

	myLineRep->SetPoint1WorldPosition(myLines[i].Min);
	myLineRep->SetPoint2WorldPosition(myLines[i].Max);
	myLineRep->GetLineProperty()->SetColor(QCOLOR_TO_VTK(myLines[i].Color));

	myLineWidget->SetEnabled(myLines[i].Visible ? 1 : 0);

	myUI->showLineButton->setChecked(myLines[i].Visible);

	myIndexChanging = false;

	myVizMng->Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void LineTool::OnShowLineToggled(bool)
{
	int i = myUI->lineBox->currentIndex();
	myLines[i].Visible = myUI->showLineButton->isChecked();
	myLineWidget->SetEnabled(myLines[i].Visible ? 1 : 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void LineTool::OnLineColorChanged(const QColor& value)
{
	int i = myUI->lineBox->currentIndex();
	myLines[i].Color = value;
	myLineRep->GetLineProperty()->SetColor(QCOLOR_TO_VTK(value));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void LineTool::OnLineBoundsChanged()
{
	if(!myIndexChanging)
	{
		int i = myUI->lineBox->currentIndex();
		myLines[i].Max[0] = myUI->endXBox->value();
		myLines[i].Max[1] = myUI->endYBox->value();
		myLines[i].Max[2] = myUI->endZBox->value();
		myLines[i].Min[0] = myUI->startXBox->value();
		myLines[i].Min[1] = myUI->startYBox->value();
		myLines[i].Min[2] = myUI->startZBox->value();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void LineTool::OnStartPointClicked()
{
	int i = myUI->lineBox->currentIndex();
	PointSourceWindow::GetInstance()->exec();
	double point[3];
	if(PointSourceWindow::GetInstance()->GetPoint(point))
	{
		myLines[i].Min[0] = point[0];
		myLines[i].Min[1] = point[1];
		myLines[i].Min[2] = point[2];
		// Refresh UI boxes.
		OnLineCurrentIndexChanged(i);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void LineTool::OnEndPointClicked()
{
	int i = myUI->lineBox->currentIndex();
	PointSourceWindow::GetInstance()->exec();
	double point[3];
	if(PointSourceWindow::GetInstance()->GetPoint(point))
	{
		myLines[i].Max[0] = point[0];
		myLines[i].Max[1] = point[1];
		myLines[i].Max[2] = point[2];
		// Refresh UI boxes.
		OnLineCurrentIndexChanged(i);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void LineTool::OnLineWidgetChanged()
{
	int i = myUI->lineBox->currentIndex();
	double pt[3];
	myLineRep->GetPoint1WorldPosition(pt);
	myLines[i].Min[0] = pt[0];
	myLines[i].Min[1] = pt[1];
	myLines[i].Min[2] = pt[2];
	myLineRep->GetPoint2WorldPosition(pt);
	myLines[i].Max[0] = pt[0];
	myLines[i].Max[1] = pt[1];
	myLines[i].Max[2] = pt[2];

	myIndexChanging = true;
	myUI->endXBox->setValue(myLines[i].Max[0]);
	myUI->endYBox->setValue(myLines[i].Max[1]);
	myUI->endZBox->setValue(myLines[i].Max[2]);
	myUI->startXBox->setValue(myLines[i].Min[0]);
	myUI->startYBox->setValue(myLines[i].Min[1]);
	myUI->startZBox->setValue(myLines[i].Min[2]);
	myIndexChanging = false;
}
