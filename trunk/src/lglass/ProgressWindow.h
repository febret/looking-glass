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
#ifndef PROGRESSWINDOW_H
#define PROGRESSWINDOW_H

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "LookingGlassSystem.h"
#include "ui_ProgressWindow.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ProgressWindow: public QDialog
{
    Q_OBJECT
public:
	// Singleton methods.
	static void Initialize();
	static ProgressWindow* GetInstance() { return myInstance; }

    // Ctor / Dtor.
	ProgressWindow();
	~ProgressWindow();

public slots:
	void SetItemName(const QString& name);
	void SetItemProgress(int value);
	void Done();

private:
	// Singleton instance.
	static ProgressWindow* myInstance;

	// UI.
	Ui::ProgressWindow* myUI;
};


#endif
