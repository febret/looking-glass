/********************************************************************************************************************** 
 * THE LOOKING GLASS VISUALIZATION TOOLSET
 *---------------------------------------------------------------------------------------------------------------------
 * Author: 
 *	Alessandro Febretti							Electronic Visualization Laboratory, University of Illinois at Chicago
 * Contact & Web:
 *  febret@gmail.com							http://febretpository.hopto.org
 *---------------------------------------------------------------------------------------------------------------------
 * Looking Glass has been built as part of the ENDURANCE Project (http://www.evl.uic.edu/endurance/).
 * ENDURANCE is supported by the NASA ASTEP program under Grant NNX07AM88G and by the NSF USAP.
 *********************************************************************************************************************/ 
#include "DataFieldSettings.h"
#include "VisualizationManager.h"
#include "DataSet.h"
#include "DataSetInfo.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DataFieldSettings::DataFieldSettings(VisualizationManager* mng):
DockedTool(mng, QString("Data Field Settings"), Qt::BottomDockWidgetArea)
{
	myVizMng = mng;
	GetMenuAction()->setIcon(QIcon(":/icons/PlotView.png"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DataFieldSettings::~DataFieldSettings()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DataFieldSettings::Initialize()
{
	SetupUI();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DataFieldSettings::SetupUI()
{
	myUI = new Ui::DataFieldSettings();
	myUI->setupUi(GetDockWidget());

    QObject::connect(myUI->addFieldButton, SIGNAL(clicked()), this, SLOT(OnAddFieldButtonClicked()));
    QObject::connect(myUI->addFieldOkButton, SIGNAL(clicked()), this, SLOT(OnAddFieldOkButtonClicked()));
    QObject::connect(myUI->addFieldCancelButton, SIGNAL(clicked()), this, SLOT(OnAddFieldCancelButtonClicked()));
    QObject::connect(myUI->removeFieldButton, SIGNAL(clicked()), this, SLOT(OnRemoveFieldButtonClicked()));
    QObject::connect(myUI->quickUpdateButton, SIGNAL(clicked()), this, SLOT(OnQuickUpdateButtonClicked()));
    QObject::connect(myUI->fullUpdateButton, SIGNAL(clicked()), this, SLOT(OnFullUpdateButtonClicked()));
    QObject::connect(myUI->indexCheck, SIGNAL(clicked()), this, SLOT(OnFieldModeChanged()));
    QObject::connect(myUI->exprCheck, SIGNAL(clicked()), this, SLOT(OnFieldModeChanged()));
    QObject::connect(myUI->scriptCheck, SIGNAL(clicked()), this, SLOT(OnFieldModeChanged()));
    QObject::connect(myUI->fieldList, SIGNAL(currentRowChanged(int)), this, SLOT(OnSelectedFieldChanged(int)));

	myUI->fieldSettingsGroup->setEnabled(false);

	myUI->addFieldOkButton->hide();
	myUI->fieldNameBox->hide();

	Update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DataFieldSettings::Update()
{
	DataSet* data = myVizMng->GetDataSet();
	DataSetInfo* info = data->GetInfo();
	for(int i = 0; i < info->GetNumFields(); i++)
	{
		FieldInfo* fi = info->GetField(i);
		myUI->fieldList->addItem(fi->GetName());
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DataFieldSettings::OnAddFieldButtonClicked()
{
	myUI->addFieldButton->hide();
	myUI->removeFieldButton->hide();

	myUI->addFieldOkButton->show();
	myUI->addFieldCancelButton->show();
	myUI->fieldNameBox->show();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DataFieldSettings::OnAddFieldOkButtonClicked()
{
	myUI->addFieldButton->show();
	myUI->removeFieldButton->show();

	myUI->addFieldOkButton->hide();
	myUI->addFieldCancelButton->hide();
	myUI->fieldNameBox->hide();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DataFieldSettings::OnAddFieldCancelButtonClicked()
{
	myUI->addFieldButton->show();
	myUI->removeFieldButton->show();

	myUI->addFieldOkButton->hide();
	myUI->addFieldCancelButton->hide();
	myUI->fieldNameBox->hide();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DataFieldSettings::OnRemoveFieldButtonClicked()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DataFieldSettings::UpdateSelectedFieldInfo()
{
	int id = myUI->fieldList->currentRow();
	FieldInfo* fi = myVizMng->GetDataSet()->GetInfo()->GetField(id);
	
	fi->SetLabel(myUI->labelBox->text());

	if(myUI->indexCheck->isChecked()) fi->SetType(FieldInfo::Data);
	else if(myUI->exprCheck->isChecked()) fi->SetType(FieldInfo::Expression);
	else if(myUI->scriptCheck->isChecked()) fi->SetType(FieldInfo::Script);

	fi->SetExpression(myUI->exprBox->text());
	fi->SetFieldIndex(myUI->indexBox->value());
	fi->SetScript(myUI->scriptBox->text());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DataFieldSettings::OnQuickUpdateButtonClicked()
{
	int id = myUI->fieldList->currentRow();
	UpdateSelectedFieldInfo();
	myVizMng->GetDataSet()->UpdateField(id);
	myVizMng->Update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DataFieldSettings::OnFullUpdateButtonClicked()
{
	int id = myUI->fieldList->currentRow();
	UpdateSelectedFieldInfo();
	myVizMng->GetDataSet()->UpdateField(id);
	myVizMng->Update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DataFieldSettings::OnFieldModeChanged()
{
	int id = myUI->fieldList->currentRow();
	FieldInfo* fi = myVizMng->GetDataSet()->GetInfo()->GetField(id);

	// Move away from here.
	if(myUI->indexCheck->isChecked()) fi->SetType(FieldInfo::Data);
	else if(myUI->exprCheck->isChecked()) fi->SetType(FieldInfo::Expression);
	else if(myUI->scriptCheck->isChecked()) fi->SetType(FieldInfo::Script);

	OnSelectedFieldChanged(id);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DataFieldSettings::OnSelectedFieldChanged(int id)
{
	myUI->fieldSettingsGroup->setEnabled(true);
	FieldInfo* fi = myVizMng->GetDataSet()->GetInfo()->GetField(id);

	myUI->labelBox->setText(fi->GetLabel());

	myUI->indexBox->setEnabled(false);
	myUI->indexBox->clear();

	myUI->exprBox->setEnabled(false);
	myUI->exprBox->clear();

	myUI->scriptBox->setEnabled(false);
	myUI->scriptBox->clear();

	switch(fi->GetType())
	{
	case FieldInfo::Data:
		myUI->indexCheck->setChecked(true);
		myUI->indexBox->setEnabled(true);
		myUI->indexBox->setValue(fi->GetFieldIndex());
		break;
	case FieldInfo::Expression:
		myUI->exprCheck->setChecked(true);
		myUI->exprBox->setEnabled(true);
		myUI->exprBox->setText(fi->GetExpression());
		break;
	case FieldInfo::Script:
		myUI->scriptCheck->setChecked(true);
		myUI->scriptBox->setEnabled(true);
		myUI->scriptBox->setText(fi->GetScript());
		break;
	}
}
