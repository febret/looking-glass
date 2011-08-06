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
// This is used to disable the console under win32.
#ifndef _DEBUG
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// System includes 
#include <vtkXMLFileOutputWindow.h>

// LookingGlass includes
#include "RepositoryManager.h"
#include "DataSet.h"
#include "GeoDataView.h"
#include "VisualizationManager.h"
#include "ToolsetSetupWindow.h"
#include "AppConfig.h"
#include "ProgressWindow.h"
#include "VtkDataManager.h"

// UI
#include "ui_MainWindow.h"
#include "ui_SetupToolset.h"

#include <QsplashScreen>
#include <QProcess>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QSplashScreen* gSplashScreen;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SetInitMessage(const char* msg)
{
	if(gSplashScreen != NULL)
	{
		char str[DEFAULT_STRING];
		sprintf(str, "%s :: LookingGlass %s", msg, LOOKING_GLASS_VERSION);
	    gSplashScreen->showMessage(str, Qt::AlignRight | Qt::AlignBottom, Qt::white);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ShutdownApp(bool whoops, bool saveConfig)
{
	if(whoops) Console::Message("LookingGlass Whoops! - to avoid breaking more stuff, the application will now close.");
	exit(1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
	// Send vtk error output to an XML log file.
	// TODO: save vtk log to the main application log.
    vtkXMLFileOutputWindow* log = vtkXMLFileOutputWindow::New();
    vtkOutputWindow::SetInstance(log);

    QApplication a(argc, argv);

	ProgressWindow::Initialize();

	QString selectedProfile;

	// If no profile has been specified as input, open the toolset selection window.
	if(argc == 1)
	{
		// Just start the program in sondebathy mode for now.
		ToolsetSetupWindow* toolsetSetupWindow = new ToolsetSetupWindow();
		
		// Default sonde bathymetry offset: 1.5 meters. (is this still used?)
		//toolsetSetupWindow->SetSondeBathyOffset(1.5f);

		// Show and manage Toolset setup Window. 
		toolsetSetupWindow->Show(a);

		selectedProfile = toolsetSetupWindow->GetSelectedProfile();
	}
	else
	{
		selectedProfile = argv[1];
	}

	// Initialize app config
	AppConfig::Initialize(selectedProfile);

	// Initialize the data file manager and sync the local data cache to the remote repository.
	RepositoryManager::Initialize(&a);

	// Check application version.
	QString rver = AppConfig::GetInstance()->GetRequiredAppVersion();
	if(rver != LOOKING_GLASS_VERSION)
	{
		// Damn, wrong version. Look if the right executable is present on the local cache. Otherwise download it.
		Console::Message(QString("Currect application version: %1 required version is %2").arg(LOOKING_GLASS_VERSION ).arg(rver));

		RepositoryManager::GetInstance()->SyncAppVersion(rver);

		// Run the external version of the app.
		QString cmdLine = "./app/lglass-" + rver + ".exe " + selectedProfile;

		QProcess proc;
		proc.setWorkingDirectory("./");
		proc.start(cmdLine);
		exit(0);
	}

	RepositoryManager::GetInstance()->SyncToRepository();

	// Setup splash screen.
    QSplashScreen splashScreen(QPixmap(":/images/Splash.png"));
	gSplashScreen = &splashScreen;
    splashScreen.show();
    a.processEvents();

	// Initialize and load the ENDURANCE dataset.
	DataSet* dataSet = new DataSet();
	VtkDataManager::Initialize(dataSet);
	dataSet->Initialize();

	// Create and initialize the visualization manager.
	VisualizationManager* vizManager = new VisualizationManager();
	vizManager->Initialize(dataSet);
	splashScreen.finish(vizManager->GetMainWindow());
	vizManager->GetMainWindow()->show();

    a.exec();

	AppConfig::GetInstance()->Save(vizManager);

	return 0;
}

