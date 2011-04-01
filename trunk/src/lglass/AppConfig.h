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
#ifndef APPCONFIG_H
#define APPCONFIG_H
#include "LookingGlassSystem.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class AppConfig
{
public:
	static void Initialize(const char* cfgFilename);
	static AppConfig* GetInstance() { return myInstance; }

	QString GetProfileName();
	libconfig::Config* GetDataConfig() { return myConfig; }
	void Save(VisualizationManager* vizMng);

	bool IsConsoleMode() { return myConsoleMode; }
	void SetConsoleMode(bool value) { myConsoleMode = value; }

	QString GetRequiredAppVersion() { return myRequiredAppVersion; }

	Preferences* GetPreferences() { return myPreferences; }

private:
	AppConfig(const char* cfgFilename);

private:
	// Singleton instance.
	static AppConfig* myInstance;

	Preferences* myPreferences;
	QString myRequiredAppVersion;
	libconfig::Config* myConfig;
	QString myProfileName;
	bool myConsoleMode;
};

#endif