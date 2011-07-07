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
#include "PointSourceWindow.h"
#include "GeoDataView.h"
#include "LineTool.h"

PointSourceWindow* PointSourceWindow::myInstance = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////
void PointSourceWindow::Initialize(VisualizationManager* mng)
{
	myInstance = new PointSourceWindow(mng);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
PointSourceWindow* PointSourceWindow::GetInstance()
{
	return myInstance;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
PointSourceWindow::PointSourceWindow(VisualizationManager* mng):
	myVizMng(mng)
{
	myUI = new Ui::PointSourceDialog();
	myUI->setupUi(this);

	setModal(true);

	connect(myUI->buttonBox, SIGNAL(accepted()), SLOT(OnAccepted()));
	connect(myUI->buttonBox, SIGNAL(rejected()), SLOT(OnAccepted()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
PointSourceWindow::~PointSourceWindow()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PointSourceWindow::showEvent ( QShowEvent * event )
{
	QDialog::showEvent(event);

	myUI->pointList->clear();

	LineTool* lineTool = myVizMng->GetLineTool();
	for(int i = 0; i < LineTool::MaxLines; i++)
	{
		LineInfo& li = lineTool->GetLine(i);
		QListWidgetItem* startItem = new QListWidgetItem(QString("Line %1 start point").arg(i), myUI->pointList);
		QListWidgetItem* endItem = new QListWidgetItem(QString("Line %1 end point").arg(i), myUI->pointList);

		startItem->setData(Qt::UserRole, QVariant(QLineF(li.Min[0], li.Min[1], li.Min[2], 0)));
		endItem->setData(Qt::UserRole, QVariant(QLineF(li.Max[0], li.Max[1], li.Max[2], 0)));
	}

	GeoDataView* geoData = myVizMng->GetGeoDataVew();
	for(int i = 0; i < geoData->GetNumItems(); i++)
	{
		GeoDataItem* item = geoData->GetItem(i);
		QListWidgetItem* startItem = new QListWidgetItem(QString("%1 start point").arg(item->GetLabel()), myUI->pointList);
		QListWidgetItem* endItem = new QListWidgetItem(QString("%1 end point").arg(item->GetLabel()), myUI->pointList);

		double point[3];
		item->GetClipStartPoint(point);
		startItem->setData(Qt::UserRole, QVariant(QLineF(point[0], point[1], point[2], 0)));
		item->GetClipEndPoint(point);
		endItem->setData(Qt::UserRole, QVariant(QLineF(point[0], point[1], point[2], 0)));
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool PointSourceWindow::GetPoint(double* point)
{
	point[0] = myPoint[0];
	point[1] = myPoint[1];
	point[2] = myPoint[2];
	return myValid;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PointSourceWindow::OnAccepted()
{
	QListWidgetItem* item = myUI->pointList->currentItem();
	if(item != NULL)
	{
		myValid = true;
		QLineF line = item->data(Qt::UserRole).value<QLineF>();
		myPoint[0] = line.x1();
		myPoint[1] = line.y1();
		myPoint[2] = line.x2();
	}
	else
	{
		myValid = false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PointSourceWindow::OnRejected()
{
	myValid = false;
}



