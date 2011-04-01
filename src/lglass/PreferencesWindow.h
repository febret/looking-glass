///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef PREFERENCESWINDOW_H
#define PREFERENCESWINDOW_H

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "LookingGlassSystem.h"
#include "DockedTool.h"
#include "ui_PreferencesWindow.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
class PreferencesWindow: public QDialog
{
    Q_OBJECT
public:
    /////////////////////////////////////////////////////// Ctor / Dtor.
    PreferencesWindow(VisualizationManager* mng);
    ~PreferencesWindow();

	void Update();

protected slots:
	void OnCancelButtonClick();
	void OnOkButtonClick();
	void OnReductionSliderValueChanged(int value);
	void OnDepthScaleSliderValueChanged(int value);
	void OnPlotColorPointChange(int pt);
	void OnPlotColorPointMove(int pt);

private:
	// UI.
	Ui::PreferencesWindow* myUI;
	VisualizationManager* myVizMng;
	pqColorChooserButton* myOldBathyColorButton;
	pqColorChooserButton* mySondeBathyColorButton;
	pqColorChooserButton* myOldBathyContourButton;
	pqColorChooserButton* mySondeBathyContourButton;

	pqColorChooserButton*myPlotDataDefaultButton;
	pqColorChooserButton*myPlotForegroundButton;
	pqColorChooserButton*myPlotBackgroundButton;

	pqColorChooserButton* myMissionPathButton;

	pqColorMapWidget* myPlotColorWidget;

	bool myIsUpdating;
};
#endif 
