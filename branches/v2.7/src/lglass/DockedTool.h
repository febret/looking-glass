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
#ifndef DOCKEDTOOL_H
#define DOCKEDTOOL_H

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "LookingGlassSystem.h"
#include "ui_PlotViewDock.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class DockedTool: public QObject
{
	Q_OBJECT
public:
    // Ctor / Dtor.
	DockedTool(VisualizationManagerBase* msg, QString dockName, Qt::DockWidgetArea area = Qt::LeftDockWidgetArea);
    virtual ~DockedTool();

    // Other Methods.
	void Enable();
	void Disable();
	bool IsEnabled();
	QDockWidget* GetDockWidget();
	QAction* GetMenuAction();
	VisualizationManagerBase* GetVisualizationManager();

	// Virtual Methods.
	virtual void Update() {}
	virtual void Render();
	virtual void Initialize() {}

public slots:
	void SetEnabled(bool enabled);

protected:
     bool eventFilter(QObject *obj, QEvent *event);

private:
	VisualizationManagerBase* myMng;
	QDockWidget* myDock;
	QAction* myMenuAction;
	bool myEnabled;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline QDockWidget* DockedTool::GetDockWidget() 
{ 
	return myDock; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline VisualizationManagerBase* DockedTool::GetVisualizationManager() 
{
	return myMng; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline QAction* DockedTool::GetMenuAction() 
{
	return myMenuAction;
}

#endif 
