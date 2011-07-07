///////////////////////////////////////////////////////////////////////////////////////////////////
#include "DataViewOptions.h"
#include "VisualizationManager.h"
#include "DataSet.h"
#include "ui_DataViewOptionsDock.h"

#define DAY_SIZE (3600 * 24)

///////////////////////////////////////////////////////////////////////////////////////////////////
DataViewOptions::DataViewOptions(VisualizationManager* mng): 
	DockedTool(mng, "View Options", Qt::LeftDockWidgetArea)
{
	myVizMng = mng;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
DataViewOptions::~DataViewOptions()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DataViewOptions::SetupUI()
{
	myUI = new Ui_DataViewOptionsDock();
	myUI->setupUi(GetDockWidget());

	DataSet* data = myVizMng->GetDataSet();

    // Setup the components box.
	for(int i = 0; i < data->GetInfo()->GetNumFields(); i++)
    {
        myUI->componentBox->addItem(data->GetFieldName(i), i);
    }

	// Setup UI
	myUI->componentBox->setCurrentIndex(0);
	myUI->showSondeDataButton->setChecked(true);
	
	time_t* range = data->GetTimestampRange();
    myUI->startMissionSlider->setRange(range[0] / DAY_SIZE, range[1] / DAY_SIZE);
    myUI->endMissionSlider->setRange(range[0] / DAY_SIZE, range[1] / DAY_SIZE);
	//myUI->showAllMissionButton->setChecked(true);

	myUI->startDateBox->addItem("All Missions");
	myUI->startDateBox->addItem("2008");
	myUI->startDateBox->addItem("2009");

	OnSelectedDateChanged(0);

	DataSetInfo* dataInfo = data->GetInfo();

	myUI->selTypeBox->addItem(QString(dataInfo->GetTag1Label().c_str()));
	myUI->selTypeBox->addItem(QString(dataInfo->GetTag2Label().c_str()));
	myUI->selTypeBox->addItem(QString(dataInfo->GetTag3Label().c_str()));
	myUI->selTypeBox->addItem(QString(dataInfo->GetTag4Label().c_str()));
	

	// Wire events.
	connect(myUI->componentBox, SIGNAL(currentIndexChanged(int)), SLOT(OnSelectedFieldChanged(int)));
	connect(myUI->startDateBox, SIGNAL(currentIndexChanged(int)), SLOT(OnSelectedDateChanged(int)));
	connect(myUI->showSingleMissionButton, SIGNAL(toggled(bool)), SLOT(OnShowMissionButtonToggle(bool)));
	connect(myUI->showIntervalMissionButton, SIGNAL(toggled(bool)),	SLOT(OnShowMissionButtonToggle(bool)));
	connect(myUI->startMissionSlider, SIGNAL(valueChanged(int)), SLOT(OnStartMissionSliderChange(int)));
	connect(myUI->endMissionSlider, SIGNAL(valueChanged(int)), 	SLOT(OnEndMissionSliderChange(int)));
	connect(myUI->showSondeDataButton, SIGNAL(toggled(bool)), SLOT(OnShowSondeDataButton(bool)));
	connect(myUI->selApplyButton, SIGNAL(clicked()), SLOT(OnSelectionApplyClicked()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DataViewOptions::Initialize()
{
	SetupUI();

	// Set the initially visualized field.
	OnSelectedFieldChanged(0);
	//OnSelectedIsoFieldChanged(0);

	QString zFieldName = myVizMng->GetDataSet()->GetInfo()->GetZFieldName();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DataViewOptions::OnSelectionApplyClicked()
{
	DataSet* data = myVizMng->GetDataSet();
	QString query = myUI->selExpressionBox->text();
	bool selStart = true;
	if(query != "")
	{
		QStringList qlist = query.split(",");
		for(int i = 0; i < qlist.size(); i++)
		{
			QString item = qlist.at(i);
			item = item.trimmed();
			data->SelectByTag(
				item, (DataSetInfo::TagId)myUI->selTypeBox->currentIndex(), 
				DataSet::FilteredData, 
				selStart ? DataSet::SelectionNew : DataSet::SelectionToggle);
			selStart = false;
		}
		myVizMng->Update();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DataViewOptions::OnShowMissionButtonToggle(bool)
{
	if(myUI->showSingleMissionButton->isEnabled())
    {
		if(myUI->showSingleMissionButton->isChecked())
		{
			myUI->startMissionSlider->setEnabled(true);
			myUI->endMissionSlider->setEnabled(false);
		}
		else
		{
			myUI->startMissionSlider->setEnabled(true);
			myUI->endMissionSlider->setEnabled(true);
		}
	}
	else
	{
		myUI->startMissionSlider->setEnabled(false);
		myUI->endMissionSlider->setEnabled(false);
	}
    UpdateMissionRange();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DataViewOptions::UpdateMissionRange()
{
	if(!myUI->showSingleMissionButton->isEnabled())
    {
		time_t* range = myVizMng->GetDataSet()->GetTimestampRange();
        myVizMng->SetMissionRange(range[0], range[1]);
    }
    else if(myUI->showSingleMissionButton->isChecked())
    {
        myVizMng->SetMissionRange(
                myUI->startMissionSlider->value() * DAY_SIZE,
                (myUI->startMissionSlider->value() + 1)  * DAY_SIZE);
    }
    else
    {
        myVizMng->SetMissionRange(
                myUI->startMissionSlider->value() * DAY_SIZE,
                myUI->endMissionSlider->value() * DAY_SIZE);
    }
    GetVisualizationManager()->Update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DataViewOptions::OnStartMissionSliderChange(int value)
{
	time_t t = (time_t)value * DAY_SIZE;
	struct tm* time = localtime(&t);
	myUI->startMissionLabel->setText(QString("%1/%2/%3").arg(time->tm_mon + 1).arg(time->tm_mday + 1).arg(time->tm_year + 1900));
    UpdateMissionRange();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DataViewOptions::OnEndMissionSliderChange(int value)
{
	time_t t = (time_t)value * DAY_SIZE;
	struct tm* time = localtime(&t);
	myUI->endMissionLabel->setText(QString("%1/%2/%3").arg(time->tm_mon + 1).arg(time->tm_mday + 1).arg(time->tm_year + 1900));
    UpdateMissionRange();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DataViewOptions::OnSelectedFieldChanged(int i)
{
	myVizMng->SetSelectedField(i);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/*void DataViewOptions::OnSelectedIsoFieldChanged(int i)
{
	myVizMng->SetSelectedIsosurfaceField(i);
	float* range = myVizMng->GetDataSet()->GetFieldRange(i);
	myUI->isoValueBox->setMinimum(range[0]);
	myUI->isoValueBox->setMaximum(range[1]);
	OnIsoValueSliderChanged(myUI->isoValueSlider->value());
}*/

///////////////////////////////////////////////////////////////////////////////////////////////////
void DataViewOptions::OnShowSondeDataButton(bool value)
{
	myVizMng->SetSondeDataVisibility(value);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//void DataViewOptions::OnIsoValueSliderChanged(int value)
//{
//	float* range = myVizMng->GetDataSet()->GetZRange();
//	double v = range[0] + (range[1] - range[0]) * 
//		(double)value / myUI->isoValueSlider->maximum();
//	myUI->isoValueBox->setValue(v);
//	myVizMng->SetSelectedIsosurfaceValue(v);
//}

///////////////////////////////////////////////////////////////////////////////////////////////////
//void DataViewOptions::OnShowIsoButtonToggle(bool value)
//{
//	myVizMng->EnableIsosurfaces(value);
//}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DataViewOptions::OnSelectedDateChanged(int index)
{
	time_t range[2];
	range[0] = myVizMng->GetDataSet()->GetTimestampRange()[0];
	range[1] = myVizMng->GetDataSet()->GetTimestampRange()[1];
	if(index == 0)
	{
		// All missions
        myUI->startMissionSlider->setEnabled(false);
        myUI->endMissionSlider->setEnabled(false);
		myUI->showIntervalMissionButton->setEnabled(false);
		myUI->showSingleMissionButton->setEnabled(false);
	}
	else if(index == 1)
	{
		// 2008 data
		range[1] = QDateTime(QDate(2008, 12, 23), QTime()).toTime_t();
		myUI->showIntervalMissionButton->setEnabled(true);
		myUI->showSingleMissionButton->setEnabled(true);
        myUI->startMissionSlider->setEnabled(true);
        myUI->endMissionSlider->setEnabled(true);
	}
	else
	{
		// 2009 data
		range[0] = QDateTime(QDate(2009, 11, 7), QTime()).toTime_t();
		myUI->showIntervalMissionButton->setEnabled(true);
		myUI->showSingleMissionButton->setEnabled(true);
        myUI->startMissionSlider->setEnabled(true);
        myUI->endMissionSlider->setEnabled(true);
	}
	myUI->startMissionSlider->setRange(range[0] / DAY_SIZE, range[1] / DAY_SIZE);
	myUI->endMissionSlider->setRange(range[0] / DAY_SIZE, range[1] / DAY_SIZE);

	myUI->startMissionSlider->setValue(range[0] / DAY_SIZE);
	myUI->endMissionSlider->setValue(range[1] / DAY_SIZE);

	myUI->showIntervalMissionButton->setChecked(true);

    UpdateMissionRange();
}

