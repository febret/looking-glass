///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef GLACIERVIEWOPTIONS_H
#define GLACIERVIEWOPTIONS_H

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "LookingGlassSystem.h"
#include "ui_GlacierViewOptionsDock.h"
#include "DockedTool.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
class GlacierViewOptions: public DockedTool
{
	Q_OBJECT
public:
    /////////////////////////////////////////////////////// Ctor / Dtor.
    GlacierViewOptions(GlacierVisualizationManager* msg);
    ~GlacierViewOptions();

    /////////////////////////////////////////////////////// Other Methods.
    void Initialize();

protected slots:
	void OnDepthScaleSliderChanged();
	void OpacitySliderChanged(int value);
	void OnShowSonarDataButtonToggle(bool);

private:
	void SetupUI();

private:
	// Reference to UI.
	Ui_GlacierViewOptionsDock* myUI;
	GlacierVisualizationManager* myVizMng;
};

#endif // MAINWINDOW_H
