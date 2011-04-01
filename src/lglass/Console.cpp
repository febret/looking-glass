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
#include "LookingGlassSystem.h"
#include "Console.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static FILE* logFile = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void logMessage(const QString& msg)
{
	if(!logFile) logFile = fopen("lglass-log.txt", "w");
	fprintf(logFile, "%s\n", msg.ascii());
	fflush(logFile);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Console::Message(const QString& msg)
{
	printf("%s\n", msg.ascii());
	logMessage(msg);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Console::Warning(const QString& msg)
{
	printf("WARNING: %s\n", msg.ascii());
	logMessage(QString("WARNING: %1").arg(msg));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Console::Error(const QString& msg)
{
	printf("ERROR: %s\n", msg.ascii());
	logMessage(QString("ERROR: %1").arg(msg));
}
