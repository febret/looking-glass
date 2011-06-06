///////////////////////////////////////////////////////////////////////////////////////////////////
#include "AppConfig.h"
#include "Preferences.h"
#include "PreferencesWindow.h"
#include "VisualizationManager.h"
#include "GeoDataView.h"
#include "NavigationView.h"
#include "DataSet.h"
#include "pq/pqColorChooserButton.h"
#include "Utils.h"
#include "VtkDataManager.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
PreferencesWindow::PreferencesWindow(VisualizationManager* mng):
	myVizMng(mng)
{
	myUI = new Ui::PreferencesWindow();
	myUI->setupUi(this);

	setModal(true);

	// Add color buttons to Geo Data View preferences.
	myOldBathyColorButton = new pqColorChooserButton(myUI->geoDataBox);
	myUI->geoDataBox->layout()->addWidget(myOldBathyColorButton);
	myOldBathyColorButton->setText("Bathy Color");

	mySondeBathyColorButton = new pqColorChooserButton(myUI->geoDataBox);
	mySondeBathyColorButton->setText("Sonde Bathy Color");
	myUI->geoDataBox->layout()->addWidget(mySondeBathyColorButton);

	myOldBathyContourButton = new pqColorChooserButton(myUI->geoDataBox);
	myUI->geoDataBox->layout()->addWidget(myOldBathyContourButton);
	myOldBathyContourButton->setText("Bathy Contour");

	mySondeBathyContourButton = new pqColorChooserButton(myUI->geoDataBox);
	mySondeBathyContourButton->setText("Sonde Bathy Contour");
	myUI->geoDataBox->layout()->addWidget(mySondeBathyContourButton);

	myMissionPathButton = new pqColorChooserButton(myUI->geoDataBox);
	myMissionPathButton->setText("Mission Path");
	myUI->NavigationViewBox->layout()->addWidget(myMissionPathButton);

	myPlotBackgroundButton = new pqColorChooserButton(myUI->plotBox);
	myPlotBackgroundButton->setText("Plot Background");
	myUI->plotColorsBox->layout()->addWidget(myPlotBackgroundButton);

	myPlotForegroundButton = new pqColorChooserButton(myUI->plotBox);
	myPlotForegroundButton->setText("Plot Foreground");
	myUI->plotColorsBox->layout()->addWidget(myPlotForegroundButton);

	myPlotDataDefaultButton = new pqColorChooserButton(myUI->plotBox);
	myPlotDataDefaultButton->setText("Plot Data Default Color");
	myUI->plotColorsBox->layout()->addWidget(myPlotDataDefaultButton);

	myPlotColorWidget = new pqColorMapWidget(myUI->plotBox);
	myPlotColorWidget->setMinimumHeight(32);
	myPlotColorWidget->setMaximumHeight(32);
	myUI->plotColorLayout->addWidget(myPlotColorWidget);

	// Set tag grouping labels.
	DataSetInfo* dataInfo = myVizMng->GetDataSet()->GetInfo();
	myUI->plotGroupingBox->addItem(QString(dataInfo->GetTag1Label().c_str()));
	myUI->plotGroupingBox->addItem(QString(dataInfo->GetTag2Label().c_str()));
	myUI->plotGroupingBox->addItem(QString(dataInfo->GetTag3Label().c_str()));
	myUI->plotGroupingBox->addItem(QString(dataInfo->GetTag4Label().c_str()));

	// Wire signals
	connect(myUI->okButton, SIGNAL(clicked()), SLOT(OnOkButtonClick()));
	connect(myUI->cancelButton, SIGNAL(clicked()), SLOT(OnCancelButtonClick()));
	connect(myUI->reductionSlider, SIGNAL(valueChanged(int)), SLOT(OnReductionSliderValueChanged(int)));
	connect(myUI->scaleSlider, SIGNAL(valueChanged(int)), SLOT(OnDepthScaleSliderValueChanged(int)));
	connect(myPlotColorWidget, SIGNAL(pointMoved(int)), SLOT(OnPlotColorPointMove(int)));
	connect(myPlotColorWidget, SIGNAL(colorChangeRequested(int)), SLOT(OnPlotColorPointChange(int)));

	// Set initial value for scale slider (sort of hack, 6 is set as default elsewhere...)
	myUI->scaleSlider->setValue(6);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
PreferencesWindow::~PreferencesWindow()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PreferencesWindow::Update()
{
	myIsUpdating = true;

	Preferences* pref = AppConfig::GetInstance()->GetPreferences();

	GeoDataView* gdv = myVizMng->GetGeoDataVew();
	myUI->reductionSlider->setValue(myVizMng->GetPointReductionFactor());
	//myOldBathyColorButton->setChosenColor(gdv->GetOldBathyColor());
	//mySondeBathyColorButton->setChosenColor(gdv->GetSondeBathyColor());
	//myOldBathyContourButton->setChosenColor(gdv->GetOldBathyContourColor());
	//mySondeBathyContourButton->setChosenColor(gdv->GetSondeBathyContourColor());

	//myUI->xLabelsBox->setValue(pref->GetPlotXLabels());
	//myUI->yLabelsBox->setValue(pref->GetPlotYLabels());
	myUI->labelFontSizeBox->setValue(pref->GetPlotLabelFontSize());

	myPlotBackgroundButton->setChosenColor(pref->GetPlotBackgroundColor());
	myPlotForegroundButton->setChosenColor(pref->GetPlotForegroundColor());
	myPlotDataDefaultButton->setChosenColor(pref->GetPlotDefaultDataColor());

	//myUI->plotAdjustLabelsChoice->setChecked(pref->GetAdjustPlotLabels());
	myUI->plotLegendChoice->setChecked(pref->GetPlotLegend());
	myUI->plotPointsChoice->setChecked(pref->GetPlotPoints());

	myPlotColorWidget->setModel(pref->GetPlotColorModel());

	myUI->plotGroupingBox->setCurrentIndex(pref->GetGroupingTagId());
	myUI->plotSourceBox->setCurrentIndex(pref->GetGroupingSubset());

	myIsUpdating = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PreferencesWindow::OnReductionSliderValueChanged(int value)
{
	int val = 100 / value;
	myUI->reductionLabel->setText(QString("Displaying %1% of original points").arg(val));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PreferencesWindow::OnDepthScaleSliderValueChanged(int value)
{
	myUI->scaleLabel->setText(QString("Depth Scale: %1x").arg(value));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PreferencesWindow::OnOkButtonClick()
{
	Preferences* pref = AppConfig::GetInstance()->GetPreferences();

	// Commit changes.
	GeoDataView* gdv = myVizMng->GetGeoDataVew();
	NavigationView* mr = myVizMng->GetNavigationView();
	myVizMng->SetPointReductionFactor(myUI->reductionSlider->value());
	//gdv->SetOldBathyColor(myOldBathyColorButton->chosenColor());
	//gdv->SetOldBathyContourColor(myOldBathyContourButton->chosenColor());
	//gdv->SetSondeBathyColor(mySondeBathyColorButton->chosenColor());
	//gdv->SetSondeBathyContourColor(mySondeBathyContourButton->chosenColor());
	//mr->SetMissionPathColor(myMissionPathButton->chosenColor());

	//pref->SetPlotXLabels(myUI->xLabelsBox->value());
	//pref->SetPlotYLabels(myUI->yLabelsBox->value());
	pref->SetPlotLabelFontSize(myUI->labelFontSizeBox->value());

	pref->SetPlotForegroundColor(myPlotForegroundButton->chosenColor());
	pref->SetPlotBackgroundColor(myPlotBackgroundButton->chosenColor());
	pref->SetPlotDefaultDataColor(myPlotDataDefaultButton->chosenColor());

	myVizMng->SetDepthScale(myUI->scaleSlider->value());

	pref->SetPlotLegend(myUI->plotLegendChoice->isChecked());
	//pref->SetAdjustPlotLabels(myUI->plotAdjustLabelsChoice->isChecked());
	pref->SetPlotPoints(myUI->plotPointsChoice->isChecked());

	DataSet::SubsetType subset = (DataSet::SubsetType)myUI->plotSourceBox->currentIndex();
	DataSetInfo::TagId tagId = (DataSetInfo::TagId)myUI->plotGroupingBox->currentIndex();

	pref->SetGroupingSubset(subset);
	pref->SetGroupingTagId(tagId);

	Utils::UpdateColorTransferFunction(pref->GetPlotColorTransferFunction(), pref->GetPlotColorModel());

	myVizMng->GetDataSet()->UpdateGroups(tagId, subset);

	VtkDataManager::GetInstance()->Update(DataSet::FilteredData);

	myVizMng->Update();
	hide();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PreferencesWindow::OnCancelButtonClick()
{
	hide();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PreferencesWindow::OnPlotColorPointChange(int pt)
{
	QColor color = QColorDialog::getColor();
	myPlotColorWidget->getModel()->setPointColor(pt, color);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PreferencesWindow::OnPlotColorPointMove(int pt)
{
}

