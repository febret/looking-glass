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
#include "ColorFunctionManager.h"
#include "DataSet.h"
#include "VisualizationManager.h"

#include "pqColorMapWidget.h"
#include "pqColorMapModel.h"
#include "pqChartValue.h"

#include <QColorDIalog>

#include <vtkColorTransferFunction.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
ColorFunctionManager::ColorFunctionManager(VisualizationManager* mng): 
	DockedTool(mng, "Color Function Manager", Qt::RightDockWidgetArea)
{
	myVizMng = mng;
	DataSet* data = mng->GetDataSet();
	for(int i = 0; i < data->GetInfo()->GetNumFields(); i++)
	{
		myFunc[i] = vtkColorTransferFunction::New();
		myNoInvalidFunc[i] = vtkColorTransferFunction::New();
		myModel[i] = new pqColorMapModel();
		myModel[i]->setColorSpaceFromInt(0);
	}
	GetMenuAction()->setIcon(QIcon(":/icons/ColorFunctionManager.png"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
ColorFunctionManager::~ColorFunctionManager()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ColorFunctionManager::Initialize()
{
	DataSet* data = myVizMng->GetDataSet();
	for(int i = 0; i < data->GetInfo()->GetNumFields(); i++)
	{
		float* range = data->GetFieldRange(i);
		myModel[i]->addPoint(range[0], QColor(0, 0, 255));
		myModel[i]->addPoint(range[1], QColor(255, 0, 0));
		UpdateColorFunction(i);
	}

	SetupUI();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ColorFunctionManager::SetupUI()
{
	myUI = new Ui::ColorFunctionManagerDock();
	myUI->setupUi(GetDockWidget());
	myUI->verticalLayout->setMargin(0);

	for(int i = 0; i < myVizMng->GetDataSet()->GetInfo()->GetNumFields(); i++)
	{
		myFieldLabel[i] = new QLabel(myUI->scrollArea);
		myFieldLabel[i]->setText(myVizMng->GetDataSet()->GetFieldName(i));
		myUI->verticalLayout_2->addWidget(myFieldLabel[i]);
		
		myWidget[i] = new pqColorMapWidget(myUI->scrollArea);
		myWidget[i]->setModel(myModel[i]);
		myWidget[i]->setMinimumHeight(32);
		myWidget[i]->setMaximumHeight(32);
		myUI->verticalLayout_2->addWidget(myWidget[i]);

		QObject::connect(myWidget[i], SIGNAL(pointMoved(int)),
			SLOT(OnColorTransferPointMove(int)));
		QObject::connect(myWidget[i], SIGNAL(colorChangeRequested(int)),
			SLOT(OnColorTransferPointChange(int)));
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ColorFunctionManager::UpdateColorFunction(int index)
{
	pqColorMapModel* cmodel = myModel[index];
	
	vtkColorTransferFunction* colorTrans = myFunc[index];
	colorTrans->RemoveAllPoints(); 

	for(int i = 0; i < cmodel->getNumberOfPoints(); i++)
	{
		QColor color;
		pqChartValue val;
		cmodel->getPointValue(i, val);
		cmodel->getPointColor(i, color);
		double actualValue = val.getDoubleValue();
		colorTrans->AddRGBPoint(actualValue, color.redF(), color.greenF(), color.blueF());
	}
	
	myNoInvalidFunc[index]->DeepCopy(colorTrans);
	
	//colorTrans->AddRGBPoint(DataSet::InvalidValue, 0, 0, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ColorFunctionManager::OnColorTransferPointChange(int pt)
{
	for(int i = 0; i < DataSetInfo::MAX_FIELDS; i++)
	{
		if(myWidget[i] == sender())
		{
			QColor color = QColorDialog::getColor();
			myModel[i]->setPointColor(pt, color);
			UpdateColorFunction(i);
			break;
		}
	}
	GetVisualizationManager()->Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ColorFunctionManager::OnColorTransferPointMove(int pt)
{
	for(int i = 0; i < DataSetInfo::MAX_FIELDS; i++)
	{
		if(myWidget[i] == sender())
		{
			UpdateColorFunction(i);
			break;
		}
	}
	GetVisualizationManager()->Render();
}
