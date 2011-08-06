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
#include "AppConfig.h"
#include "RepositoryManager.h"
#include "ProgressWindow.h"

#include <QDir>

using namespace libconfig;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RepositoryManager* RepositoryManager::myInstance;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RepositoryManager::Initialize(QApplication* app)
{
	myInstance = new RepositoryManager(app);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RepositoryManager::RepositoryManager(QApplication* app):
	myApplication(app)
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
		Config* c = AppConfig::GetInstance()->GetDataConfig();

		string repoHost = (string)c->lookup("Application/RepositoryHost");
		string repoUri = (string)c->lookup("Application/RepositoryURI");
		string profileName = (string)c->lookup("Application/ProfileName");

		QString localFilePath = QString("./%1/%2").arg(profileName.c_str()).arg(fileName);
		myLocalFilePath = localFilePath;
		QString remoteFilePath = QString("%1/%2").arg(repoUri.c_str()).arg(fileName);

		myDone = false;
		ProgressWindow::GetInstance()->SetItemName(QString("Downloading %1").arg(remoteFilePath));
		myLoader->get(remoteFilePath);
		while(!myDone) myApplication->processOneEvent();

		return TryOpen(fileName);
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
void RepositoryManager::SyncAppVersion(QString& ver)
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

		while(!myDone) myApplication->processOneEvent();
	}

	ProgressWindow::GetInstance()->Done();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RepositoryManager::SyncToRepository()
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

			while(!myDone) myApplication->processOneEvent();
		}
	}

	ProgressWindow::GetInstance()->Done();
}

