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
void GeoDataView::SavePreferences(Setting& s)
{
	for(int i = 0; i < myNumItems; i++)
	{
		const QString& itemName = myItems[i]->GetName();
		if(!s.exists(itemName)) s.add(itemName, Setting::TypeGroup);
		myItems[i]->SavePreferences(s[itemName.ascii()]);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataView::LoadPreferences(Setting& s)
{
	for(int i = 0; i < myNumItems; i++)
	{
		const QString& itemName = myItems[i]->GetName();
		if(s.exists(itemName))
		{
			myItems[i]->LoadPreferences(s[itemName.ascii()]);
		}
	}
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
		item->SetVisible(false);
		myItems[i] = item;
		myNumItems++;

		myUI->geoItemBox->addItem(item->GetLabel());
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

	connect(myUI->geoItemBox, SIGNAL(currentIndexChanged(int)), SLOT(OnGeoItemBoxChanged(int)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataView::SetDepthScale(int value)
{
	for(int i = 0; i < myNumItems; i++)
	{
		myItems[i]->SetDepthScale(value);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GeoDataView::OnGeoItemBoxChanged(int index)
{
	for(int i = 0; i < myNumItems; i++)
	{
		if(i == index) myItems[i]->SetVisible(true);
		else myItems[i]->SetVisible(false);
	}
}