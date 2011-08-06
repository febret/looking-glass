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
#ifndef DATAFIELDSETTINGS_H
#define DATAFIELDSETTINGS_H

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "LookingGlassSystem.h"
#include "DockedTool.h"
#include "ui_DataFieldSettings.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class DataFieldSettings: public DockedTool
{
    Q_OBJECT
public:
    /////////////////////////////////////////////////////// Ctor / Dtor.
    DataFieldSettings(VisualizationManager* mng);
    ~DataFieldSettings();

	void Initialize();
	void Update();

protected slots:
	void OnAddFieldOkButtonClicked();
	void OnAddFieldCancelButtonClicked();
	void OnAddFieldButtonClicked();
	void OnRemoveFieldButtonClicked();
	void OnQuickUpdateButtonClicked();
	void OnFullUpdateButtonClicked();
	void OnFieldModeChanged();
	void OnSelectedFieldChanged(int index);


private:
	void SetupUI();
	void UpdateSelectedFieldInfo();

private:
	// UI.
	Ui::DataFieldSettings* myUI;
	VisualizationManager* myVizMng;
};

#endif