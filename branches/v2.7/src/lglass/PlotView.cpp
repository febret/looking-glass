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

#include <QmessageBox>

#include <vtkAttributeDataToFieldDataFilter.h>
#include <vtkColorTransferFunction.h>
#include <vtkFieldData.h>
#include <vtkFloatArray.h>
#include <vtkInteractorStyleRubberband2D.h>
#include <vtkPointData.h>
#include <vtkPointSet.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkTextProperty.h>

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
	
	myUI->xLegendPositionSlider->setValue(70);
	myUI->yLegendPositionSlider->setValue(0);
	myUI->plotXSlider->setValue(0);
	myUI->plotWidthSlider->setValue(100);

	//myUI->layoutBox->setVisible(false);
	// Setup some default plot to show something.
	myUI->xAxisBox->setCurrentIndex(0);
	myUI->yAxisBox->setCurrentIndex(1);

    connect(myUI->xAxisBox, SIGNAL(currentIndexChanged(int)), SLOT(OnAxisFieldChanged(int)));
    connect(myUI->yAxisBox, SIGNAL(currentIndexChanged(int)), SLOT(OnAxisFieldChanged(int)));
	connect(myUI->viewAllButton, SIGNAL(clicked()),	SLOT(OnViewAllButtonClicked()));
	connect(myUI->exportImageButton, SIGNAL(clicked()),	SLOT(OnExportImageButtonClicked()));

	connect(myUI->xRangeMinBox, SIGNAL(valueChanged(double)), SLOT(OnRangeChanged()));
	connect(myUI->yRangeMinBox, SIGNAL(valueChanged(double)), SLOT(OnRangeChanged()));
	connect(myUI->xRangeMaxBox, SIGNAL(valueChanged(double)), SLOT(OnRangeChanged()));
	connect(myUI->yRangeMaxBox, SIGNAL(valueChanged(double)), SLOT(OnRangeChanged()));

	connect(myUI->xMinorTicksBox, SIGNAL(valueChanged(int)), SLOT(SetPlotProperties()));
	connect(myUI->xMajorTicksBox, SIGNAL(valueChanged(int)), SLOT(SetPlotProperties()));
	connect(myUI->yMinorTicksBox, SIGNAL(valueChanged(int)), SLOT(SetPlotProperties()));
	connect(myUI->yMajorTicksBox, SIGNAL(valueChanged(int)), SLOT(SetPlotProperties()));

	connect(myUI->xReferenceEnabledBox, SIGNAL(toggled(bool)), SLOT(OnReferenceLinesChanged()));
	connect(myUI->yReferenceEnabledBox, SIGNAL(toggled(bool)), SLOT(OnReferenceLinesChanged()));
	connect(myUI->xReferenceBox, SIGNAL(valueChanged(double)), SLOT(OnReferenceLinesChanged()));
	connect(myUI->yReferenceBox, SIGNAL(valueChanged(double)), SLOT(OnReferenceLinesChanged()));

	connect(myUI->yLegendPositionSlider, SIGNAL(valueChanged(int)), SLOT(OnLegendPositionChanged()));
	connect(myUI->xLegendPositionSlider, SIGNAL(valueChanged(int)), SLOT(OnLegendPositionChanged()));
	connect(myUI->plotXSlider, SIGNAL(valueChanged(int)), SLOT(OnLegendPositionChanged()));
	connect(myUI->plotWidthSlider, SIGNAL(valueChanged(int)), SLOT(OnLegendPositionChanged()));

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

	float plotX = (float)myUI->plotXSlider->value() / 100;
	float plotWidth = (float)myUI->plotWidthSlider->value() / 100;
    myPlot->SetWidth(plotWidth);
	myPlot->SetPosition(plotX, 0);

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

		// Take into account enabled reference lines when sizing legend box.
		if(myUI->xReferenceEnabledBox->isChecked()) nonEmptyGroups++;
		if(myUI->yReferenceEnabledBox->isChecked()) nonEmptyGroups++;

		int* size = myPlotRenderWindow->GetSize();
		float yPos = (float)nonEmptyGroups * 20 / size[1];
		if(yPos > 1) yPos = 1;

		float xpp = (float)myUI->xLegendPositionSlider->value() / 100;
		float ypp = (float)myUI->yLegendPositionSlider->value() / 100;
		//xpp *= 1 / plotWidth;
		//float xpp = 1;
		//float ypp = 0;

		myPlot->SetLegendPosition(xpp, 1 - yPos - ypp);
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
	//myPlot->SetAdjustXLabels(prefs->GetAdjustPlotLabels());
	//myPlot->SetAdjustYLabels(prefs->GetAdjustPlotLabels());
	
	myPlot->SetAdjustXLabels(0);
	myPlot->SetAdjustYLabels(0);

	myPlot->SetNumberOfXLabels(myUI->xMajorTicksBox->value());
	myPlot->SetNumberOfXMinorTicks(myUI->xMinorTicksBox->value());

	myPlot->SetNumberOfYLabels(myUI->yMajorTicksBox->value());
	myPlot->SetNumberOfYMinorTicks(myUI->yMinorTicksBox->value());

	myPlotRenderWindow->Render();
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

		int grps = data->GetNumSortedGroups();

		//int nonEmptyGroups = grps;
		/*for(int i = 0; i < grps; i++)
		{
			DataGroup* grp = data->GetGroup(i);
			if(grp->Size > 0) nonEmptyGroups++;
		}*/

		// Check the plot limit.
		if(grps > 500)
		{
			Console::Error("PLOT LIMIT REACHED: Cannot plot more than 500 curves.");
			ShutdownApp(true, false);
		}
		if(grps > 100)
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
		if(myUI->xReferenceEnabledBox->isChecked())
		{
			float x1 = (float)myUI->xReferenceBox->value();
			float x2 = (float)myUI->xReferenceBox->value();
			float y1 = (float)myUI->yRangeMinBox->value();
			float y2 = (float)myUI->yRangeMaxBox->value();

			AddReferenceLine(c, x1, y1, x2, y2, myUI->xAxisBox->currentText(), myUI->xReferenceBox->value());
			c++;
		}
		if(myUI->yReferenceEnabledBox->isChecked())
		{
			float y1 = (float)myUI->yReferenceBox->value();
			float y2 = (float)myUI->yReferenceBox->value();
			float x1 = (float)myUI->xRangeMinBox->value();
			float x2 = (float)myUI->xRangeMaxBox->value();

			AddReferenceLine(c, x1, y1, x2, y2, myUI->yAxisBox->currentText(), myUI->yReferenceBox->value());
			c++;
		}
		for(int i = 0; i < grps; i++)
		{
			DataGroup* grp = data->GetSortedGroup(i);
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

				float colorWeight = (float)c / (grps - 1);
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
void PlotView::AddReferenceLine(int c, float x1, float y1, float x2, float y2, QString name, float value)
{
	Preferences* prefs = AppConfig::GetInstance()->GetPreferences();

	vtkFloatArray* xData = vtkFloatArray::New();
	vtkFloatArray* yData = vtkFloatArray::New();

	xData->InsertNextTuple1(x1);
	xData->InsertNextTuple1(x2);
	yData->InsertNextTuple1(y1);
	yData->InsertNextTuple1(y2);

	vtkFieldData* selectedFieldData = vtkFieldData::New();
	selectedFieldData->AddArray(xData);
	selectedFieldData->AddArray(yData);

	vtkDataObject* selectedDataObject = vtkDataObject::New();
	selectedDataObject->SetFieldData(selectedFieldData);

	myPlot->AddDataObjectInput(selectedDataObject);
	myPlot->SetPlotLines(c + 1, 1);

	xData->Delete();
	yData->Delete();
	selectedFieldData->Delete();
	selectedDataObject->Delete();

	myPlot->SetPlotColor(c + 1, QCOLOR_TO_VTK(prefs->GetPlotForegroundColor()));

	myPlot->SetDataObjectXComponent(c + 1, 0);
	myPlot->SetDataObjectYComponent(c + 1, 1);
	QString refLabel = QString("%1 = %2").arg(name).arg(value);
	myPlot->SetPlotLabel(c + 1, refLabel);
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
		myUI->xRangeMinBox->setValue(x1);
		myUI->xRangeMaxBox->setValue(x2);

		myUI->yRangeMinBox->setValue(y1);
		myUI->yRangeMaxBox->setValue(y2);

		//OnRangeChanged();
		//myPlotRenderWindow->Render();
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
	QString xAxis = myUI->xAxisBox->currentText();
	QString yAxis = myUI->yAxisBox->currentText();

	myUI->xRangeLabel->setText(xAxis + " range:");
	myUI->yRangeLabel->setText(yAxis + " range:");
	myUI->xTicksLabel->setText(xAxis + " ticks:");
	myUI->yTicksLabel->setText(yAxis + " ticks:");
	myUI->xReferenceEnabledBox->setText(xAxis + " reference line");
	myUI->yReferenceEnabledBox->setText(yAxis + " reference line");

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

	myUI->xRangeMinBox->setMinimum(xr[0]);
	myUI->xRangeMaxBox->setMinimum(xr[0]);
	myUI->xRangeMinBox->setMaximum(xr[1]);
	myUI->xRangeMaxBox->setMaximum(xr[1]);
	myUI->xReferenceBox->setMinimum(xr[0]);
	myUI->xReferenceBox->setMaximum(xr[1]);

	myUI->yRangeMinBox->setMinimum(yr[0]);
	myUI->yRangeMaxBox->setMinimum(yr[0]);
	myUI->yRangeMinBox->setMaximum(yr[1]);
	myUI->yRangeMaxBox->setMaximum(yr[1]);
	myUI->yReferenceBox->setMinimum(yr[0]);
	myUI->yReferenceBox->setMaximum(yr[1]);

	myUI->xRangeMinBox->setValue(xr[0]);
	myUI->xRangeMaxBox->setValue(xr[1]);

	myUI->yRangeMinBox->setValue(yr[0]);
	myUI->yRangeMaxBox->setValue(yr[1]);

	myPlotRenderWindow->Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlotView::OnRangeChanged()
{
	myPlot->SetXRange(myUI->xRangeMinBox->value(), myUI->xRangeMaxBox->value());
	myPlot->SetYRange(myUI->yRangeMinBox->value(), myUI->yRangeMaxBox->value());

	//myUI->zoomButton->setChecked(true);

	myPlotRenderWindow->Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlotView::OnExportImageButtonClicked()
{
	Utils::SaveScreenshot(myPlotRenderWindow);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlotView::OnReferenceLinesChanged()
{
	Update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlotView::OnLegendPositionChanged()
{
	SetPlotProperties();
}