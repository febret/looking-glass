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
#ifndef PLOTVIEW_H
#define PLOTVIEW_H

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "LookingGlassSystem.h"
#include "DataSet.h"
#include "DockedTool.h"
#include "ui_PlotViewDock.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
class PlotView: public DockedTool
{
	Q_OBJECT
public:
    /////////////////////////////////////////////////////// Ctor / Dtor.
    PlotView(VisualizationManager* msg, int index);
    ~PlotView();

    /////////////////////////////////////////////////////// Other Methods.
    void Initialize();
    void Update();
	void Render();
    
	void OnSelectionChanged(int* sel);

protected slots:
    void OnAxisFieldChanged(int i);
	void OnViewAllButtonClicked();
	void OnExportImageButtonClicked();

private:
	void SetupUI();
	void SetPlotProperties();

private:
	// UI.
	Ui_PlotViewDock* myUI;
	VisualizationManager* myVizMng;

    // Plot stuff.
    Plot* myPlot;
    vtkRenderWindow* myPlotRenderWindow;
    vtkRenderer* myPlotRenderer;
    vtkAttributeDataToFieldDataFilter* myPlotDataFilter;
	vtkInteractorStyleRubberBand2D* myInteractorStyle;

	// Filters
	static DynamicFilter* myXFilter;
	static DynamicFilter* myYFilter;
};

#endif 
