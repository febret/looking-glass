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
#include "ProgressWindow.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ProgressWindow* ProgressWindow::myInstance = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ProgressWindow::Initialize()
{
	myInstance = new ProgressWindow();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ProgressWindow::ProgressWindow()
{
	myUI = new Ui::ProgressWindow();
	myUI->setupUi(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ProgressWindow::~ProgressWindow()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ProgressWindow::SetItemName(const QString& name)
{
	if(!this->isVisible()) show();
	myUI->label->setText(name);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ProgressWindow::SetItemProgress(int value)
{
	if(!this->isVisible()) show();
	myUI->progressBar->setValue(value);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ProgressWindow::Done()
{
	this->hide();
}
