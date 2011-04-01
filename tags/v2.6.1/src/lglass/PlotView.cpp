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
#include "AppConfig.h"
#include "Preferences.h"
#include "PlotView.h"
#include "DataSet.h"
#include "GeoDataView.h"
#include "Plot.h"
#include "Utils.h"
#include "VisualizationManager.h"
#include "ui_MainWindow.h"
#include "VtkDataManager.h"

#include "vtkAxisActor2D.h"
#include "vtkLegendBoxActor.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
VTK_CALLBACK(SelectionChangedCallback, PlotView, OnSelectionChanged(reinterpret_cast<int*>(data)));

///////////////////////////////////////////////////////////////////////////////////////////////////
DynamicFilter* PlotView::myXFilter = NULL;
DynamicFilter* PlotView::myYFilter = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////
PlotView::PlotView(VisualizationManager* mng, int index): 
	DockedTool(mng, QString("Plot Window %1").arg(index), Qt::RightDockWidgetArea)
{
	myVizMng = mng;
	GetMenuAction()->setIcon(QIcon(":/icons/PlotView.png"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
PlotView::~PlotView()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlotView::Initialize()
{
	VisualizationManager* mng = myVizMng;

	if(!myXFilter)
	{
		myXFilter = new DynamicFilter();
		myYFilter = new DynamicFilter();

		myXFilter->Type = DynamicFilter::FieldFilter;
		myYFilter->Type = DynamicFilter::FieldFilter;
		
		myXFilter->Enabled = false;
		myYFilter->Enabled = false;

		myVizMng->GetDataSet()->AddFilter(myXFilter);
		myVizMng->GetDataSet()->AddFilter(myYFilter);
	}

	SetupUI();

	// Setup VTK stuff.
    myPlot = Plot::New();
    myPlot->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    myPlot->GetPositionCoordinate()->SetValue(0.0f, 0.0f);
    myPlot->SetPlotPoints(0);
    myPlot->SetWidth(1.0f);
    myPlot->SetHeight(1.0f);

    myPlotRenderer = vtkRenderer::New();
    myPlotRenderer->AddActor2D(myPlot);
	
	myPlotRenderWindow = myUI->vtkView->GetRenderWindow();
	myInteractorStyle = vtkInteractorStyleRubberBand2D::New();
	myInteractorStyle->AddObserver("SelectionChangedEvent", new SelectionChangedCallback(this));
	myPlotRenderWindow->GetInteractor()->SetInteractorStyle(myInteractorStyle);

    myPlotDataFilter = vtkAttributeDataToFieldDataFilter::New();

	myUI->vtkView->GetRenderWindow()->AddRenderer(myPlotRenderer);

	myVizMng->GetDataSet()->ApplyFilters();

	SetPlotProperties();

	//Update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlotView::SetupUI()
{
	DataSet* data = myVizMng->GetDataSet();

	myUI = new Ui_PlotViewDock();
	myUI->setupUi(GetDockWidget());
	for(int i = 0; i < data->GetInfo()->GetNumFields(); i++)
    {
        myUI->xAxisBox->addItem(data->GetFieldName(i), i);
        myUI->yAxisBox->addItem(data->GetFieldName(i), i);
    }
	
	// Setup some default plot to show something.
	myUI->xAxisBox->setCurrentIndex(0);
	myUI->yAxisBox->setCurrentIndex(1);

    connect(myUI->xAxisBox, SIGNAL(currentIndexChanged(int)), SLOT(OnAxisFieldChanged(int)));
    connect(myUI->yAxisBox, SIGNAL(currentIndexChanged(int)), SLOT(OnAxisFieldChanged(int)));
	connect(myUI->viewAllButton, SIGNAL(clicked()),	SLOT(OnViewAllButtonClicked()));
	connect(myUI->exportImageButton, SIGNAL(clicked()),	SLOT(OnExportImageButtonClicked()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlotView::SetPlotProperties()
{
	Preferences* prefs = AppConfig::GetInstance()->GetPreferences();

	myPlotRenderer->SetBackground(QCOLOR_TO_VTK(prefs->GetPlotBackgroundColor()));

	int labelFontSize = prefs->GetPlotLabelFontSize();

	vtkTextProperty* textProp = NULL;

	// Set plot parameters
	myPlot->SetPlotColor(0, QCOLOR_TO_VTK(prefs->GetPlotDefaultDataColor()));
	myPlot->GetXAxisActor2D()->GetProperty()->SetLineWidth(1);
	myPlot->GetXAxisActor2D()->GetProperty()->SetColor(QCOLOR_TO_VTK(prefs->GetPlotForegroundColor()));
	myPlot->GetYAxisActor2D()->GetProperty()->SetLineWidth(1);
	myPlot->GetYAxisActor2D()->GetProperty()->SetColor(QCOLOR_TO_VTK(prefs->GetPlotForegroundColor()));

	myPlot->GetXAxisActor2D()->SetLabelFactor((float)labelFontSize / 10.0f);
	myPlot->GetYAxisActor2D()->SetLabelFactor((float)labelFontSize / 10.0f);
	myPlot->GetTitleTextProperty()->SetFontSize(labelFontSize);
	myPlot->GetTitleTextProperty()->SetFontFamilyToCourier();

	myPlot->GetLegendActor()->SetDragable(0);
	myPlot->SetLegend(prefs->GetPlotLegend() ? 1 : 0);
	if(prefs->GetPlotLegend())
	{
		DataSet* data = myVizMng->GetDataSet();
		// Set legend size.
		int nonEmptyGroups = 1;
		int grps = data->GetNumGroups();
		for(int i = 0; i < grps; i++)
		{
			DataGroup* grp = data->GetGroup(i);
			if(grp->Size > 0) nonEmptyGroups++;
		}

		int* size = myPlotRenderWindow->GetSize();
		float yPos = (float)nonEmptyGroups * 20 / size[1];
		if(yPos > 1) yPos = 1;

		myPlot->SetLegendPosition(0.7f, 1 - yPos);
		myPlot->SetLegendPosition2(0.3f, yPos);
	}

	// X axis labels
	textProp = myPlot->GetXAxisActor2D()->GetLabelTextProperty();
	textProp->ShadowOff();
	textProp->SetColor(QCOLOR_TO_VTK(prefs->GetPlotForegroundColor()));
	textProp->SetFontFamilyToCourier();
	//textProp->SetFontSize(labelFontSize);

	// X axis title
	textProp = myPlot->GetXAxisActor2D()->GetTitleTextProperty();
	textProp->ShadowOff();
	textProp->SetColor(QCOLOR_TO_VTK(prefs->GetPlotForegroundColor()));
	textProp->SetFontFamilyToCourier();

	// Y axis labels
	textProp = myPlot->GetYAxisActor2D()->GetLabelTextProperty();
	textProp->ShadowOff();
	textProp->SetColor(QCOLOR_TO_VTK(prefs->GetPlotForegroundColor()));
	textProp->SetFontFamilyToCourier();

	// Y axis title
	textProp = myPlot->GetYAxisActor2D()->GetTitleTextProperty();
	textProp->ShadowOff();
	textProp->SetColor(QCOLOR_TO_VTK(prefs->GetPlotForegroundColor()));
	textProp->SetFontFamilyToCourier();

	myPlot->GetProperty()->SetLineWidth(2);

	// Set axis labels properties.
	myPlot->SetAdjustXLabels(prefs->GetAdjustPlotLabels());
	myPlot->SetAdjustYLabels(prefs->GetAdjustPlotLabels());
	myPlot->SetNumberOfXLabels(prefs->GetPlotXLabels());
	myPlot->SetNumberOfXMinorTicks(5);
	myPlot->SetNumberOfYLabels(prefs->GetPlotYLabels());
	myPlot->SetNumberOfYMinorTicks(5);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
void PlotView::Update()
{
	VisualizationManager* mng = myVizMng;
	DataSet* data = mng->GetDataSet();

	Preferences* prefs = AppConfig::GetInstance()->GetPreferences();

	int xField = myUI->xAxisBox->currentIndex();
	int yField = myUI->yAxisBox->currentIndex();

	SetPlotProperties();

	myPlot->GetLegendActor()->GetEntryTextProperty()->SetFontSize(5);
	myPlot->GetLegendActor()->GetEntryTextProperty()->SetFontFamilyToCourier();
	myPlot->GetLegendActor()->GetEntryTextProperty()->Modified();

	// If I use the same field on both coordinates, the XYPlotActor apparently gets stuck...
	if(xField != yField)
	{
		vtkPointData* dataPoints = VtkDataManager::GetInstance()->GetPointSet(DataSet::FilteredData)->GetPointData();

		vtkDataArray* xData = dataPoints->GetArray(data->GetFieldName(xField).ascii());
		vtkDataArray* yData = dataPoints->GetArray(data->GetFieldName(yField).ascii());

		vtkFieldData* fieldData = vtkFieldData::New();
		fieldData->AddArray(xData);
		fieldData->AddArray(yData);

		vtkDataObject *dataObject = vtkDataObject::New();
		dataObject->SetFieldData(fieldData);

		myPlot->RemoveAllInputs();

		myPlot->AddDataObjectInput(dataObject);

		myPlot->PlotCurveLinesOn();
		myPlot->SetPlotLines(0, 0);
		myPlot->PlotPointsOn();
		myPlot->SetXValuesToValue();
		myPlot->SetDataObjectXComponent(0, 0);
		myPlot->SetDataObjectYComponent(0, 1);

		//myPlot->GetLegendActor()->GetEntryTextProperty()->SetFontSize(10);

		char title[256];
		sprintf(title, "%s vs %s", data->GetFieldName(yField).ascii(), data->GetFieldName(xField).ascii());
		myPlot->SetXTitle(title);
		myPlot->SetYTitle("");

		vtkPointData* selectedDataPoints = NULL;
		vtkFieldData* selectedFieldData = NULL;
		vtkDataObject *selectedDataObject = NULL;

		int grps = data->GetNumGroups();

		int nonEmptyGroups = 0;
		for(int i = 0; i < grps; i++)
		{
			DataGroup* grp = data->GetGroup(i);
			if(grp->Size > 0) nonEmptyGroups++;
		}

		// Check the plot limit.
		if(nonEmptyGroups > 500)
		{
			Console::Error("PLOT LIMIT REACHED: Cannot plot more than 500 curves.");
			ShutdownApp(true, false);
		}
		if(nonEmptyGroups > 100)
		{
			QMessageBox::StandardButton btn = QMessageBox::question(
				NULL, 
				"Plot View Warning", 
				"The current plot parameters will generate more than 100 unique plot components. The operation may take a while. Do you want to proceed?", 
				QMessageBox::Abort | QMessageBox::Ok);
			if(btn == QMessageBox::Abort) return;
		}

		vtkColorTransferFunction* colorMap = prefs->GetPlotColorTransferFunction();		

		//printf("total Groups: %d, nonEmpty: %d\n", grps, nonEmptyGroups);

		//myPlot->GetLegendActor()->SetNumberOfEntries(nonEmptyGroups);

		int c = 0;
		for(int i = 0; i < grps; i++)
		{
			DataGroup* grp = data->GetGroup(i);
			if(grp->Size > 0)
			{
				vtkFloatArray* xData = vtkFloatArray::New();
				vtkFloatArray* yData = vtkFloatArray::New();

				for(int j = 0; j < grp->Size; j++)
				{
					xData->InsertNextTuple1(grp->Items[j]->Field[xField]);
					yData->InsertNextTuple1(grp->Items[j]->Field[yField]);
				}
				selectedFieldData = vtkFieldData::New();
				selectedFieldData->AddArray(xData);
				selectedFieldData->AddArray(yData);

				selectedDataObject = vtkDataObject::New();
				selectedDataObject->SetFieldData(selectedFieldData);

				
				myPlot->AddDataObjectInput(selectedDataObject);
				myPlot->SetPlotLines(c + 1, prefs->GetPlotPoints() ? 0 : 1);

				xData->Delete();
				yData->Delete();
				selectedFieldData->Delete();
				selectedDataObject->Delete();

				float colorWeight = (float)c / (nonEmptyGroups - 1);
				double* color = colorMap->GetColor(colorWeight);
				myPlot->SetPlotColor(c + 1, color);

				//printf("added Group ID: %d, plot component: %d\n", i, c + 1);

				myPlot->SetDataObjectXComponent(c + 1, 0);
				myPlot->SetDataObjectYComponent(c + 1, 1);
				myPlot->SetPlotLabel(c + 1, grp->Tag);
				c++;
			}
		}

		//myPlot->Modified();
		//myPlotRenderer->Render();
		//myPlot->GetLegendActor()->GetEntryTextProperty()->SetFontFamilyToCourier();
		myPlotRenderWindow->Render();

		myPlot->SetPlotLabel(0, "Other datapoints");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlotView::Render()
{
	myPlotRenderWindow->Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlotView::OnSelectionChanged(int* sel)
{
	double x1 = sel[0];
	double y1 = sel[1];
	double x2 = sel[2];
	double y2 = sel[3];
	int xField = myUI->xAxisBox->currentIndex();
	int yField = myUI->yAxisBox->currentIndex();
	VisualizationManager* mng = myVizMng;

	myPlot->ViewportToPlotCoordinate(myPlotRenderer, x1, y1);
	myPlot->ViewportToPlotCoordinate(myPlotRenderer, x2, y2);
	// Swap coordinates if necessary.
	if(x1 > x2)
	{
		double tmp = x1;
		x1 = x2;
		x2 = tmp;
	}
	if(y1 > y2)
	{
		double tmp = y1;
		y1 = y2;
		y2 = tmp;
	}	

	if(myUI->zoomButton->isChecked())
	{
		myPlot->SetXRange(x1, x2);
		myPlot->SetYRange(y1, y2);
		myPlotRenderWindow->Render();
	}
	else
	{
		if(x1 == x2 && y1 == y2)
		{
			// If action was just a click, clear selected area.
			myXFilter->Enabled = false;
			myYFilter->Enabled = false;
			myVizMng->GetDataSet()->ApplyFilters();
		}
		else
		{
			myXFilter->Enabled = true;
			myXFilter->FieldId = xField;
			myXFilter->Min = x1;
			myXFilter->Max = x2;
			myYFilter->Enabled = true;
			myYFilter->FieldId = yField;
			myYFilter->Min = y1;
			myYFilter->Max = y2;

			myVizMng->GetDataSet()->ApplyFilters();
		}
		mng->Update();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlotView::OnAxisFieldChanged(int i)
{
	Update();
	OnViewAllButtonClicked();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlotView::OnViewAllButtonClicked()
{
	int xField = myUI->xAxisBox->currentIndex();
	int yField = myUI->yAxisBox->currentIndex();
	float* xr = myVizMng->GetDataSet()->GetFieldRange(xField);
	float* yr = myVizMng->GetDataSet()->GetFieldRange(yField);
	myPlot->SetXRange(xr[0], xr[1]);
	myPlot->SetYRange(yr[0], yr[1]);
	myPlotRenderWindow->Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlotView::OnExportImageButtonClicked()
{
	Utils::SaveScreenshot(myPlotRenderWindow);
}