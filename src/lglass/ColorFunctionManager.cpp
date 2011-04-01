///////////////////////////////////////////////////////////////////////////////////////////////////
#include "ColorFunctionManager.h"
#include "DataSet.h"
#include "VisualizationManager.h"

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
