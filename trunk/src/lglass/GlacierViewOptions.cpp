///////////////////////////////////////////////////////////////////////////////////////////////////
#include "GlacierViewOptions.h"
#include "GlacierVisualizationManager.h"
#include "VisualizationManager.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
GlacierViewOptions::GlacierViewOptions(GlacierVisualizationManager* mng): 
	DockedTool(mng, "View Options", Qt::LeftDockWidgetArea)
{
	myVizMng = mng;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
GlacierViewOptions::~GlacierViewOptions()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GlacierViewOptions::SetupUI()
{
	myUI = new Ui_GlacierViewOptionsDock();
	myUI->setupUi(GetDockWidget());

	myUI->depthScaleSlider->setValue(VisualizationManager::DefaultDepthScale);
	myUI->opacitySlider->setValue(0);
	myUI->showSonarDataButton->setChecked(true);

	// Wire events.
	connect(myUI->depthScaleSlider, SIGNAL(sliderReleased()),
		SLOT(OnDepthScaleSliderChanged()));

    connect(myUI->opacitySlider, SIGNAL(valueChanged(int)), 
		SLOT(OpacitySliderChanged(int)));

	connect(myUI->showSonarDataButton, SIGNAL(toggled(bool)), 
		SLOT(OnShowSonarDataButtonToggle(bool)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GlacierViewOptions::Initialize()
{
	SetupUI();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GlacierViewOptions::OpacitySliderChanged(int value)
{
	myVizMng->SetSurfaceOpacity(value);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GlacierViewOptions::OnDepthScaleSliderChanged()
{
	int value = myUI->depthScaleSlider->value();
	myVizMng->SetDepthScale(value);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GlacierViewOptions::OnShowSonarDataButtonToggle(bool value)
{
	myVizMng->SetSonarDataVisibility(value);
}