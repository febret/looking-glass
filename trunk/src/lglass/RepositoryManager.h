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
#ifndef DATAFILEMANAGER_H
#define DATAFILEMANAGER_H

#include "LookingGlassSystem.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class RepositoryManager: public QObject
{
	Q_OBJECT
public:
	static void Initialize();
	static RepositoryManager* GetInstance() { return myInstance; }

	RepositoryManager();
	~RepositoryManager();

	void SyncToRepository(QApplication& app);
	void SyncAppVersion(QString& ver, QApplication& app);
	QFile* TryOpen(const QString& fileName);

signals:
	void FileReady(QFile& file);

protected slots:
	void OnLoaderDone(bool error);
	void OnLoaderProgress(int done, int total);

private:
	static RepositoryManager* myInstance;

	QString myLocalPath;
	QString myLocalFilePath;
	bool myDone;

	QQueue<QString> myQueue;
	QHttp* myLoader;
};

#endif