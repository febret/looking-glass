/**************************************************************************************************
 * THE LOOKING GLASS VISUALIZATION TOOLSET
 *-------------------------------------------------------------------------------------------------
 * Author: 
 *	Alessandro Febretti		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Contact & Web:
 *  febret@gmail.com		http://febretpository.hopto.org
 *-------------------------------------------------------------------------------------------------
 * Looking Glass has been built as part of the ENDURANCE Project (http://www.evl.uic.edu/endurance/).
 * ENDURANCE is supported by the NASA ASTEP program under Grant NNX07AM88G and by the NSF USAP.
 *-------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2011, Electronic Visualization Laboratory, University of Illinois at Chicago
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, are permitted 
 * provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this list of conditions 
 * and the following disclaimer. Redistributions in binary form must reproduce the above copyright 
 * notice, this list of conditions and the following disclaimer in the documentation and/or other 
 * materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF 
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *************************************************************************************************/ 
#ifndef TOOLSET_SETUP_WINDOW
#define TOOLSET_SETUP_WINDOW

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "LookingGlassSystem.h"
#include "ui_SetupToolset.h"

#include <Qdir>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ENUM(ToolsetMode)
	None,
	SondeAndBathymetry,
	Glacier
END_ENUM;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ToolsetSetupWindow: public QObject
{
	Q_OBJECT
public:
	ToolsetSetupWindow() 
	{
		myToolsetMode = ToolsetMode::None;
	}
	~ToolsetSetupWindow() {}
	ToolsetMode::Enum GetToolsetMode() { return myToolsetMode; }
	void Show(QApplication& app)
	{
		Ui_SetupToolset* setupToolsetWin = new Ui_SetupToolset();
		myToolsetWindow = new QWidget();
		setupToolsetWin->setupUi(myToolsetWindow);

		setupToolsetWin->groupBox_2->setVisible(false);

		QDir dir("./");
		QStringList filters; 
		filters << "*.cfg";

		QStringList files = dir.entryList(filters);
		setupToolsetWin->profileBox->addItems(files);

		//setupToolsetWin->profileBox->addItem(QString("Endurance.cfg"));

		// Wire events
		QObject::connect(setupToolsetWin->runGlacierToolsetButton, SIGNAL(clicked()),
			this, SLOT(OnRunGlacierToolsetButtonClick()));
		QObject::connect(setupToolsetWin->runSondeToolsetButton, SIGNAL(clicked()),
			this, SLOT(OnRunSondeToolsetButtonClick()));

		myToolsetWindow->show();
		
		app.exec();

		mySelectedProfile = setupToolsetWin->profileBox->currentText();

		delete myToolsetWindow;
		myToolsetWindow = NULL;
	}

	bool GetRecomputeSondeBathy() { return myRecomputeSondeBathy; }
	const char* GetSelectedProfile() { return mySelectedProfile; }

public slots:
	void OnRunGlacierToolsetButtonClick()
	{
		myToolsetMode = ToolsetMode::Glacier;
		myToolsetWindow->close();
	}

	void OnRunSondeToolsetButtonClick()
	{
		myToolsetMode = ToolsetMode::SondeAndBathymetry;
		myToolsetWindow->close();
	}

private:
	ToolsetMode::Enum myToolsetMode;
	QWidget* myToolsetWindow;
	QString mySelectedProfile;
	bool myRecomputeSondeBathy;
	float mySondeBathyOffset;
};

#endif
