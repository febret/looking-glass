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
#include "AppConfig.h"
#include "Console.h"
#include "DataSet.h"
#include "DataSetInfo.h"
#include "Preferences.h"
#include "VisualizationManager.h"

using namespace libconfig;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AppConfig* AppConfig::myInstance = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AppConfig::Initialize(const char* cfgFilename)
{
	myInstance = new AppConfig(cfgFilename);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AppConfig::AppConfig(const char* cfgFilename)
{
	myConsoleMode = false;
	myProfileName = cfgFilename;
	myConfig = new libconfig::Config();
	myPreferences = new Preferences();

	myPreferences->Load("./settings.lgcfg");

	FILE* stream = fopen(cfgFilename, "r");
	try
	{
		myConfig->read(stream);
	}
	catch(ParseException e)
	{
		Console::Error(QString("Profile loading: %1 at line %2").arg(e.getError()).arg(e.getLine()));
		ShutdownApp(true, false);
	}
	fclose(stream);

	// Read the required application version
	myRequiredAppVersion = QString((const char*)myConfig->lookup("Application/Version"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString AppConfig::GetProfileName()
{
	return (const char*)myConfig->lookup("Application/ProfileName");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Don't really like the passing of a VisualizationManager pointer here. Possible to make it more transparent?
void AppConfig::Save(VisualizationManager* vizMng)
{
	// Serialize dataset info.
	vizMng->GetDataSet()->GetInfo()->Save(this);

	// Serialize window state.
	myPreferences->SetWindowState(vizMng->GetMainWindow()->saveState());

	myPreferences->Save("./settings.lgcfg");

	// Save config.
	myConfig->writeFile(myProfileName.ascii());
}
