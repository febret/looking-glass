///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef DataViewOptions_H
#define DataViewOptions_H

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "LookingGlassSystem.h"
#include "ui_DataViewOptionsDock.h"
#include "DataSet.h"
#include "DockedTool.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
class DataViewOptions: public DockedTool
{
	Q_OBJECT
public:
    /////////////////////////////////////////////////////// Ctor / Dtor.
    DataViewOptions(VisualizationManager* msg);
    ~DataViewOptions();

    /////////////////////////////////////////////////////// Other Methods.
    void Initialize();
    //void Update();

protected slots:
	void OnSelectedFieldChanged(int i);
	//void OnSelectedIsoFieldChanged(int i);
    void OnShowMissionButtonToggle(bool enabled);
    void OnEndMissionSliderChange(int value);
    void OnStartMissionSliderChange(int value);
	void OnShowSondeDataButton(bool value);
	//void OnIsoValueSliderChanged(int value);
	//void OnShowIsoButtonToggle(bool value);
	void OnSelectedDateChanged(int);

private:
	void SetupUI();
    void UpdateMissionRange();

private:
	// Reference to UI.
	Ui_DataViewOptionsDock* myUI;
	VisualizationManager* myVizMng;
};

#endif // MAINWINDOW_H
