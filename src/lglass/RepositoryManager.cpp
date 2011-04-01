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
#include "RepositoryManager.h"
#include "ProgressWindow.h"

using namespace libconfig;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RepositoryManager* RepositoryManager::myInstance;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RepositoryManager::Initialize()
{
	myInstance = new RepositoryManager();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RepositoryManager::RepositoryManager()
{
	myLoader = new QHttp();

	connect(myLoader, SIGNAL(done(bool)), this, SLOT(OnLoaderDone(bool)));
	connect(myLoader, SIGNAL(dataReadProgress(int, int)), this, SLOT(OnLoaderProgress(int, int)));

	myDone = true;

	myLocalPath = AppConfig::GetInstance()->GetProfileName() + "/";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RepositoryManager::~RepositoryManager()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QFile* RepositoryManager::TryOpen(const QString& fileName)
{
	if(QFile::exists(myLocalPath + fileName))
	{
		QFile* file = new QFile(myLocalPath + fileName);
		file->open(QFile::OpenModeFlag::ReadOnly);
		return file;
	}
	else
	{
		// Request HTTP;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RepositoryManager::OnLoaderDone(bool error)
{
	if(error)
	{
		Config* c = AppConfig::GetInstance()->GetDataConfig();
		string repoHost = (string)c->lookup("Application/RepositoryHost");
		string repoUri = (string)c->lookup("Application/RepositoryURI");
		Console::Error(QString("Repository manager error while downloading %1 host: %2 repo root URI: %3").arg(myLocalFilePath).arg(repoHost.c_str()).arg(repoUri.c_str()));
		Console::Error(myLoader->errorString());
		ShutdownApp(true);
	}
	if(!myDone)
	{
		myDone = true;
		QByteArray data = myLoader->readAll();
		QFile output(myLocalFilePath);
		output.open(QFile::OpenModeFlag::WriteOnly);
		output.write(data);
		output.close();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RepositoryManager::OnLoaderProgress(int done, int total)
{
	ProgressWindow::GetInstance()->SetItemProgress(done * 100 / total);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RepositoryManager::SyncAppVersion(QString& ver, QApplication& app)
{
	Config* c = AppConfig::GetInstance()->GetDataConfig();
	string repoHost = (string)c->lookup("Application/RepositoryHost");
	string repoUri = (string)c->lookup("Application/RepositoryURI");
	QDir dir("./");

	// Create the local app directory if not present.
	if(!dir.exists("app"))
	{
		dir.mkdir("app");
	}
	myLoader->setHost(repoHost.c_str());

	Setting& fields = c->lookup("Application/RepositoryFiles");
	QString fileName = "lglass-" + ver + ".exe";

	QString localFilePath = "./app/" + fileName; 
	myLocalFilePath = localFilePath;
	QString remoteFilePath = QString("%1/app/%2").arg(repoUri.c_str()).arg(fileName);

	if(!QFile::exists(localFilePath))
	{
		myDone = false;

		ProgressWindow::GetInstance()->SetItemName(QString("Downloading Application %1 Binary").arg(ver));
		myLoader->get(remoteFilePath);

		while(!myDone) app.processOneEvent();
	}

	ProgressWindow::GetInstance()->Done();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RepositoryManager::SyncToRepository(QApplication& app)
{
	Config* c = AppConfig::GetInstance()->GetDataConfig();

	string repoHost = (string)c->lookup("Application/RepositoryHost");
	string repoUri = (string)c->lookup("Application/RepositoryURI");
	string profileName = (string)c->lookup("Application/ProfileName");

	QDir dir("./");

	// Create the profile local directory if not present.
	if(!dir.exists(profileName.c_str()))
	{
		dir.mkdir(profileName.c_str());
	}

	myLoader->setHost(repoHost.c_str());

	Setting& fields = c->lookup("Application/RepositoryFiles");
	for(int i = 0; i < fields.getLength(); i++)
	{
		Setting& sField = fields[i];
		string fileName = (string)sField;

		QString localFilePath = QString("./%1/%2").arg(profileName.c_str()).arg(fileName.c_str());
		myLocalFilePath = localFilePath;
		QString remoteFilePath = QString("%1/%2").arg(repoUri.c_str()).arg(fileName.c_str());

		if(!QFile::exists(localFilePath))
		{
			myDone = false;

			ProgressWindow::GetInstance()->SetItemName(QString("Downloading %1").arg(remoteFilePath));
			myLoader->get(remoteFilePath);

			while(!myDone) app.processOneEvent();
		}
	}

	ProgressWindow::GetInstance()->Done();
}

