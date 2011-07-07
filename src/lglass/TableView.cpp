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
#include "DataSet.h"
#include "TableView.h"
#include "VisualizationManager.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TableModel::TableModel(DataSet* data)
{
	myData = data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int TableModel::rowCount(const QModelIndex& parent) const
{
	// If there is some selected data, table view will visualize selected data, otherwise filtered data.
	if(myData->GetDataLength(DataSet::SelectedData) > 0)
	{
		return myData->GetDataLength(DataSet::SelectedData);
	}
	return myData->GetDataLength(DataSet::FilteredData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int TableModel::columnCount(const QModelIndex& parent) const
{
	int c = 0;
	DataSetInfo* di = myData->GetInfo();
	for(int i = 0; i < di->GetNumFields(); i++)
	{
		if(di->GetField(i)->IsEnabled()) c++;
	}
	return c + 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QVariant TableModel::data( const QModelIndex & index, int role) const 
{
	if(role == Qt::DisplayRole)
	{
		// If there is some selected data, table view will visualize selected data, otherwise filtered data.
		if(myData->GetDataLength(DataSet::SelectedData) > 0)
		{
			int i = index.column();
			// Print tag1
			if(i == 0 && myData->GetInfo()->GetTag1Index() != -1)
			{
				return QVariant(myData->GetData(index.row(), DataSet::SelectedData)->Tag1);
			}
			int c = -1;
			DataSetInfo* di = myData->GetInfo();
			for(i = 0; i < di->GetNumFields(); i++)
			{
				if(di->GetField(i)->IsEnabled()) c++;
				if(c == index.column() - 1) break;
			}			 
			if(i < di->GetNumFields())
			{
				return QVariant((float)myData->GetData(index.row(), DataSet::SelectedData)->Field[i]);
			}
			else
			{
				return QVariant();
			}
		}
		else
		{
			int i = index.column();
			// Print tag1
			if(i == 0 && myData->GetInfo()->GetTag1Index() != -1)
			{
				return QVariant(myData->GetData(index.row(), DataSet::FilteredData)->Tag1);
			}
			int c = -1;
			DataSetInfo* di = myData->GetInfo();
			for(i = 0; i < di->GetNumFields(); i++)
			{
				if(di->GetField(i)->IsEnabled()) c++;
				if(c == index.column() - 1) break;
			}			 
			if(i < di->GetNumFields())
			{
				return QVariant((float)myData->GetData(index.row(), DataSet::FilteredData)->Field[i]);
			}
			else
			{
				return QVariant();
			}
		}
	}	
	else
	{
	    return QVariant();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
 {
     if (role != Qt::DisplayRole)
         return QVariant();

	 if (orientation == Qt::Vertical || section >= myData->GetInfo()->GetNumFields())
	 {
         return section;
	 }
     else
	 {
		 // Print tag1
		 if(section == 0 && myData->GetInfo()->GetTag1Index() != -1)
		 {
			 return QString::fromStdString(myData->GetInfo()->GetTag1Label());
		 }
		 else
		 {
			DataSetInfo* di = myData->GetInfo();
			int c = -1;
			int i;
			for(i = 0; i < di->GetNumFields(); i++)
			{
				if(di->GetField(i)->IsEnabled()) c++;
				if(c == section - 1) break;
			}			
			if(i < di->GetNumFields())
			{
				return myData->GetFieldName(i);
			}
			else
			{
				return section;
			}
		 }
	 }
 }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Qt::ItemFlags TableModel::flags( const QModelIndex& index ) const
{
	return Qt::ItemIsEnabled;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TableView::TableView(VisualizationManager* mng):
	DockedTool(mng, QString("Table View"), Qt::BottomDockWidgetArea)
{
	myVizMng = mng;
	GetMenuAction()->setIcon(QIcon(":/icons/TableView.png"));

	myFilter.Enabled = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TableView::~TableView()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TableView::Initialize()
{
	SetupUI();
	myVizMng->GetDataSet()->AddFilter(&myFilter);
	myModel = new TableModel(myVizMng->GetDataSet());
	Update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TableView::SetupUI()
{
	myUI = new Ui::TableView();
	myUI->setupUi(GetDockWidget());

	myUI->table->verticalHeader()->setVisible(false);

	connect(myUI->saveCSVButton, SIGNAL(clicked()), this, SLOT(OnSaveAsCSVButtonClick()));
	connect(myUI->selectAllButton, SIGNAL(clicked()), this, SLOT(OnSelectAllButtonClicked()));
	connect(myUI->clearAllButton, SIGNAL(clicked()), this, SLOT(OnClearAllButtonClicked()));
	connect(myUI->chooseColumnsButton, SIGNAL(toggled(bool)), myUI->columnsBox, SLOT(setVisible(bool)));

	DataSet* data = myVizMng->GetDataSet();
	DataSetInfo* di = data->GetInfo();
	for(int i = 0; i < di->GetNumFields(); i++)
	{
		QCheckBox* cb = new QCheckBox(myUI->columnsBox);
		myUI->columnsBox->layout()->addWidget(cb);

		cb->setText(di->GetField(i)->GetLabel());
		cb->setChecked(true);

		connect(cb, SIGNAL(toggled(bool)), this, SLOT(OnColumnCheckedChanged()));
		myColumnCheckBoxes.push_back(cb);
	}

	myUI->columnsBox->setVisible(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TableView::Update()
{
	DataSet* data = myVizMng->GetDataSet();
	QStringList headers;

	myUI->table->setSelectionBehavior(QAbstractItemView::SelectRows);
	myUI->table->setModel(NULL);
	myUI->table->setModel(myModel);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TableView::OnSaveAsCSVButtonClick()
{
	QString fileName = QFileDialog::getSaveFileName("./", "*.csv");
	if(!fileName.isNull())
	{
		myVizMng->GetDataSet()->SaveAsCSV(fileName);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TableView::OnColumnCheckedChanged()
{
	DataSet* data = myVizMng->GetDataSet();
	DataSetInfo* di = data->GetInfo();
	for(int i = 0; i < myColumnCheckBoxes.length(); i++)
	{
		QCheckBox* cb = myColumnCheckBoxes[i];
		di->GetField(i)->SetEnabled(cb->isChecked());
	}
	Update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TableView::OnSelectAllButtonClicked()
{
	DataSet* data = myVizMng->GetDataSet();
	DataSetInfo* di = data->GetInfo();
	for(int i = 0; i < myColumnCheckBoxes.length(); i++)
	{
		QCheckBox* cb = myColumnCheckBoxes[i];
		cb->setChecked(true);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TableView::OnClearAllButtonClicked()
{
	DataSet* data = myVizMng->GetDataSet();
	DataSetInfo* di = data->GetInfo();
	for(int i = 0; i < myColumnCheckBoxes.length(); i++)
	{
		QCheckBox* cb = myColumnCheckBoxes[i];
		cb->setChecked(false);
	}
}
