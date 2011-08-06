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
#include "DataSet.h"
#include "NavigationView.h"
#include "VisualizationManager.h"
#include "RepositoryManager.h"

#include <QTextStream>
#include <QTimer>

#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkDataSetMapper.h>
#include <vtkImageData.h>
#include <vtkPlane.h>
#include <vtkPlaneSource.h>
#include <vtkPNGReader.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>
#include <vtkTexture.h>

using namespace libconfig;

///////////////////////////////////////////////////////////////////////////////////////////////////
ImageOverlay::ImageOverlay(VisualizationManager* mng):
	myVizMng(mng)
{
	myOverlayReader = vtkPNGReader::New();

	myOverlayTexture = vtkTexture::New();
	myOverlayTexture->SetInput(myOverlayReader->GetOutput());
	myOverlayTexture->InterpolateOn();

	myOverlayPlane = vtkPlaneSource::New();
	myOverlayPlane->SetOrigin(0, 0, 0);
	myOverlayPlane->SetPoint2(0, 0, -1);
	myOverlayPlane->SetPoint1(2, 0, 0);
	myOverlayPlane->SetCenter(0, 0, 0);

	myOverlayMapper = vtkPolyDataMapper::New();
	myOverlayMapper = vtkPolyDataMapper::New();
	myOverlayMapper->SetInput(myOverlayPlane->GetOutput());

	myOverlayActor = vtkActor::New();
	myOverlayActor->SetMapper(myOverlayMapper);
	myOverlayActor->SetTexture(myOverlayTexture);
	myOverlayActor->SetOrientation(0, 120, 0);
	myOverlayActor->PickableOff();
	myOverlayActor->GetProperty()->SetLighting(0);

	myLabelActor = vtkTextActor3D::New();
	myLabelActor->SetInput("Hello World!");
	myLabelActor->SetOrientation(-90, 60, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
ImageOverlay::~ImageOverlay()
{
	myOverlayReader->Delete();
	myOverlayTexture->Delete();
	myOverlayPlane->Delete();
	myOverlayMapper->Delete();
	myOverlayActor->Delete();
	myLabelActor->Delete();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ImageOverlay::SetTexture(const char* imagePathFormat, const char* imgPath)
{
	char path[DEFAULT_STRING];
	sprintf(path, imagePathFormat, imgPath);
	myOverlayReader->SetFileName(path);
	myOverlayReader->Update();
	myOverlayMapper->Update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ImageOverlay::SetPosition(float X, float Y)
{
	myOverlayActor->SetPosition(X, 4, Y);
	myLabelActor->SetPosition(X, 3, Y);
	this->X = X;
	this->Y = Y;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ImageOverlay::SetSize(float size)
{
	myOverlayActor->SetScale(-size, 1, -size);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ImageOverlay::SetVisible(bool visible)
{
	vtkRenderer* renderer = myVizMng->GetMainRenderer();
	if(visible)
	{
		renderer->AddActor(myOverlayActor);
		//renderer->AddActor(myLabelActor);
	}
	else
	{
		renderer->RemoveActor(myOverlayActor);
		//renderer->RemoveActor(myLabelActor);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
NavigationView::NavigationView(VisualizationManager* mng): 
	DockedTool(mng, "Mission Replay", Qt::BottomDockWidgetArea),
	myMissionPathColor(255, 106, 0),
	myDepthScale(7),
	PathDecimation(5),
	PathDepthOffset(4),
	ImagePathFormat("G:/bonney_2009/%s"),
	MissionDataPathFormat("/nav09/%s"),
	ImageExportPathFormat("./data/ExportedImages/%d_%d_%d_%f.png"),
	IcePickingThreshold(4.5f)
{
	myVizMng = mng;
	GetMenuAction()->setIcon(QIcon(":/icons/NavigationView.png"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
NavigationView::~NavigationView()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NavigationView::Initialize()
{
	SetupUI();

	// Create the VTK objects.
	myMissionPathData = vtkPolyData::New();
	myMissionPathMapper = vtkPolyDataMapper::New();
	myMissionPathActor = vtkActor::New();
	myMissionPathActor->SetVisibility(0);

	// Initalize the AUV object
	myAUVSource = vtkSphereSource::New();
	myAUVMapper = vtkPolyDataMapper::New();
	myAUVActor = vtkActor::New();

	myAUVSource->SetPhiResolution(5);
	myAUVSource->SetThetaResolution(5);
	myAUVMapper->SetInput(myAUVSource->GetOutput());
	myAUVActor->SetMapper(myAUVMapper);
	myAUVActor->SetVisibility(0);
	myVizMng->GetMainRenderer()->AddActor(myAUVActor);

	// Initialize image overlays
	for(int i = 0; i < MAX_IMAGE_OVERLAYS; i++)
	{
		myOverlays[i] = new ImageOverlay(myVizMng);
	}

	// TODO: Endurance hack.
	LoadMissionSet("Bonney2009");

	SetMissionPathColor(myMissionPathColor);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NavigationView::LoadMissionSet(QString setName)
{
	Config* c = AppConfig::GetInstance()->GetDataConfig();
	QString section = QString("Application/NavigationView/%1").arg(setName);
	if(!c->exists(section))
	{
		Console::Error(QString("Missing config section %1: wrong configuration file?").arg(setName));
		ShutdownApp(true);
	}

	Setting& missionSet = c->lookup(section);

	ImagePathFormat = (string)missionSet["ImagePathFormat"];
	ImageExportPathFormat = (string)missionSet["ImageExportPathFormat"];
	MissionDataPathFormat = (string)missionSet["MissionDataPathFormat"];

	MissionFiles.clear();

	Setting& files = missionSet["MissionFiles"];
	for(int i = 0; i < files.getLength(); i++)
	{
		MissionFiles.push_back((string)files[i]);
	}

	// Setup the mission box.
	myUI->diveBox->clear();
	for(int i = 0; i < MissionFiles.size(); i++)
	{
		myUI->diveBox->addItem(MissionFiles[i].c_str());
	}

	LoadMission(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NavigationView::LoadMission(int num)
{
	// Count the number of items.
	char path[DEFAULT_STRING];
	sprintf(path, MissionDataPathFormat.c_str(), MissionFiles[num].c_str());


	QFile* file = RepositoryManager::GetInstance()->TryOpen(path);//fopen(path, "r");
	if(!file)
	{
		Console::Error(QString("Opening navigation file %1 failed.").arg(path));
		return;
	}
	
	QTextStream dataFile(file);

	myNavigationViewDataLength = 0;
	while (!dataFile.atEnd())
    {
		QString line = dataFile.readLine();
		myNavigationViewDataLength++;
	}
	// Ignore first line.
	myNavigationViewDataLength--;
	
	file->seek(0);

	printf("Mission data length: %d\n", myNavigationViewDataLength);

    // process the first line which is a header
	QString line = dataFile.readLine();
	int i = 0;
    while (!dataFile.atEnd())
    {
		float x, y, depth;
		int timestamp;

		QString line =  dataFile.readLine();

		//%Timestamp X Y Depth ImageName
        int l = sscanf(line, "%d %f %f %f %s", &timestamp, &x, &y, &depth, myNavigationViewData[i].UPImageName);

		myNavigationViewData[i].X = x;
		myNavigationViewData[i].Y = y;
		myNavigationViewData[i].Depth = depth;
		myNavigationViewData[i].Timestamp = timestamp;
		//strcpy(myNavigationViewData[i].Log, ((char*)line.data()) + 48 + strlen(myNavigationViewData[i].UPImageName));

		i++;
    }

	// close and free up file.
	file->close();
	delete file;
	file = NULL;

	// Setup the polydata.
	vtkPoints* pts = vtkPoints::New();
	vtkCellArray* cells = vtkCellArray::New();
	pts->SetNumberOfPoints(myNavigationViewDataLength);
	int numPts = 0;
	for(int i = 0; i < myNavigationViewDataLength; i++)
	{
		if(i % PathDecimation == 0)
		{
			pts->SetPoint(numPts, myNavigationViewData[i].X, -myNavigationViewData[i].Depth, myNavigationViewData[i].Y);
			if(i < myNavigationViewDataLength - PathDecimation)
			{
				cells->InsertNextCell(2);
				cells->InsertCellPoint(numPts);
				cells->InsertCellPoint(numPts + 1);
			}
			numPts++;
		}
	}
	myMissionPathData->Allocate(myNavigationViewDataLength, myNavigationViewDataLength);
	myMissionPathData->SetPoints(pts);
	myMissionPathData->SetLines(cells);

	cells->Delete();
	pts->Delete();

	//double* range = outline->GetOutput()->GetBounds();
	//printf("Mission Range: %f-%f %f-%f %f-%f\n", range[0], range[1], range[2], range[3], range[4], range[5]);

	myMissionPathMapper->SetInput(myMissionPathData);
	myMissionPathMapper->Update();

	myMissionPathActor->SetMapper(myMissionPathMapper);
	myMissionPathActor->SetPosition(0, PathDepthOffset * myDepthScale, 0);
	myMissionPathActor->SetScale(1, myDepthScale, 1);
	myMissionPathActor->GetProperty()->SetLineWidth(1);
	myMissionPathActor->GetProperty()->SetRepresentationToPoints();

	myVizMng->GetMainRenderer()->AddActor(myMissionPathActor);
	myVizMng->Render();

	myLogIndex = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NavigationView::SetupUI()
{
	myUI = new Ui::NavigationViewDock();
	myUI->setupUi(GetDockWidget());

	myTimer = new QTimer();

	myUI->imgDistanceSlider->setValue(50);
	OnImgDistanceSliderValueChanged(50);

	// Wire events.
	connect(myUI->timeSlider, SIGNAL(sliderMoved(int)),
		SLOT(OnTimeSliderValueChanged(int)));

	connect(myUI->imgDistanceSlider, SIGNAL(sliderMoved(int)),
		SLOT(OnImgDistanceSliderValueChanged(int)));

	connect(myTimer, SIGNAL(timeout()),
		SLOT(OnTimerTick()));

	connect(myUI->diveBox, SIGNAL(currentIndexChanged(int)),
		SLOT(OnDiveBoxChanged(int)));

	connect(myUI->timeBackButton, SIGNAL(clicked()), 
		SLOT(OnTimeBackButtonClick()));

	connect(myUI->timeForwardButton, SIGNAL(clicked()), 
		SLOT(OnTimeForwardButtonClick()));

	connect(myUI->showMissionButton, SIGNAL(toggled(bool)), 
		SLOT(OnShowMissionButtonToggle(bool)));

	connect(myUI->imgEnabledButton, SIGNAL(toggled(bool)), 
		SLOT(OnImgEnabledButtonButtonToggle(bool)));

	connect(myUI->playButton, SIGNAL(toggled(bool)), 
		SLOT(OnPlayButtonToggle(bool)));

	connect(myUI->exportImageButton, SIGNAL(clicked()), 
		SLOT(OnExportImageButtonClick()));

	connect(myUI->imgUpdateButton, SIGNAL(clicked()), 
		SLOT(OnImgUpdateButtonClick()));

}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NavigationView::Update()
{
	NavigationViewItem* item = &myNavigationViewData[myPlayIndex];

	time_t time = item->Timestamp;
	tm* ptm = gmtime(&time);

	myUI->timeLabel->setText(asctime(ptm));

	myAUVActor->SetPosition(item->X, (-item->Depth + PathDepthOffset) * myDepthScale , item->Y);
	myAUVActor->SetScale(6, 6, 6);
	myAUVActor->GetProperty()->SetLighting(0);
	myAUVActor->GetProperty()->SetColor(0, 1, 1);
	myAUVActor->GetProperty()->BackfaceCullingOff();
	myAUVActor->GetProperty()->FrontfaceCullingOff();
	myVizMng->Render();

	// Print log message.
	if(myPlayIndex > myLogIndex)
	{
		for(int i = myLogIndex; i < myPlayIndex; i++)
		{
			if(strlen(myNavigationViewData[i].Log) > 2)
			{
				time_t ltime = myNavigationViewData[i].Timestamp;
				tm* lptm = gmtime(&ltime);
				char str[DEFAULT_STRING * 4];
				sprintf(str, "%d:%d:%d - %s", ptm->tm_hour, ptm->tm_min, ptm->tm_sec, myNavigationViewData[i].Log);
				str[strlen(str) - 1] = '\0';
				myUI->logView->addItem(str);
			}
			myVizMng->SetStatusbarMessage(QString("X: %1    Y: %2   Depth: %3")
				.arg(myNavigationViewData[i].X)
				.arg(myNavigationViewData[i].Y)
				.arg(myNavigationViewData[i].Depth));
		}
	}
	myUI->logView->scrollToBottom();
	myLogIndex = myPlayIndex;

	// Update camera images.
	if(item->UPImageName[0] != '#')
	{
		char path[DEFAULT_STRING];
		sprintf(path, ImagePathFormat.c_str(), item->UPImageName);
		myUpImage.load(path);
		myUI->upView->setPixmap(myUpImage);
	}
	else
	{
		// Hack - to avoid sparse empty images - this problem should actually
		// be solved inside the preprocessing tool.
		if(myNavigationViewData[myPlayIndex + 1].UPImageName[0] == '#')
		{
			myUI->upView->clear();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NavigationView::UpdateImageOverlays()
{
	myOverlayStartIndex = 0;
	
	for(int i = 0; i < MAX_IMAGE_OVERLAYS; i++)
	{
		vtkRenderer* renderer = myVizMng->GetMainRenderer();
		myOverlays[i]->SetVisible(false);
	}

	int lastIndex = 0;
	int overlayIndex = 0;
	// STEP 1: Always add images closer to the ice surface.
	//for(int i = myOverlayStartIndex; i < myNavigationViewDataLength; i++)
	//{
	//	// if this data point has no associated image, skip it.
	//	if(myNavigationViewData[i].UPImageName[0] != '#')
	//	{
	//		if(myNavigationViewData[i].Depth < IcePickingThreshold)
	//		{
	//			// Check to see if image is not too close to other already added overlays
	//			bool ok = true;
	//			for(int j = 0; j < overlayIndex; j++)
	//			{
	//				float dx2 = myNavigationViewData[i].X - myOverlays[j]->X;
	//				float dy2 = myNavigationViewData[i].Y - myOverlays[j]->Y;
	//				float d2 = sqrt(dx2 * dx2 + dy2 * dy2);
	//				if(d2 < 30)
	//				{
	//					ok = false;
	//					break;
	//				}
	//			}
	//			if(ok)
	//			{
	//				// If distance check is ok, create a new image overlay from this data point.
	//				myOverlays[overlayIndex]->Index = i;
	//				myOverlays[overlayIndex]->SetTexture(ImagePathFormat.c_str(), myNavigationViewData[i].UPImageName);
	//				myOverlays[overlayIndex]->SetPosition(myNavigationViewData[i].X, myNavigationViewData[i].Y);
	//				// Scale image overlay depending on overlay distance.
	//				myOverlays[overlayIndex]->SetSize(myOverlayDistance / 1.5f);
	//				myOverlays[overlayIndex]->SetVisible(true);
	//				overlayIndex++;
	//				lastIndex = i;
	//			}
	//		}
	//	}
	//}

	for(int i = myOverlayStartIndex; i < myNavigationViewDataLength; i++)
	{
		// if this data point has no associated image, skip it.
		if(myNavigationViewData[i].UPImageName[0] != '#')
		{
			// Check to see if image is not too close to other already added overlays
			bool ok = true;
			for(int j = 0; j < overlayIndex; j++)
			{
				float dx2 = myNavigationViewData[i].X - myOverlays[j]->X;
				float dy2 = myNavigationViewData[i].Y - myOverlays[j]->Y;
				float d2 = sqrt(dx2 * dx2 + dy2 * dy2);
				if(d2 < myOverlayDistance)
				{
					ok = false;
					break;
				}
			}
			if(ok)
			{
				// If distance check is ok, create a new image overlay from this data point.
				myOverlays[overlayIndex]->Index = i;
				myOverlays[overlayIndex]->SetTexture(ImagePathFormat.c_str(), myNavigationViewData[i].UPImageName);
				myOverlays[overlayIndex]->SetPosition(myNavigationViewData[i].X, myNavigationViewData[i].Y);
				// Scale image overlay depending on overlay distance.
				myOverlays[overlayIndex]->SetSize(myOverlayDistance / 1.5f);
				myOverlays[overlayIndex]->SetVisible(true);
				overlayIndex++;
				lastIndex = i;
			}
		}
	}
	myActiveOverlays = overlayIndex;
	myUI->imgEnabledButton->setChecked(true);
	myVizMng->Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NavigationView::SetMissionPathColor(QColor value)
{
	myMissionPathColor = value;
	myMissionPathActor->GetProperty()->SetColor(QCOLOR_TO_VTK(myMissionPathColor));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NavigationView::OnShowMissionButtonToggle(bool value)
{
	myAUVActor->SetVisibility(value);
	myMissionPathActor->SetVisibility(value);
	if(value)
	{
		OnTimeSliderValueChanged(myUI->timeSlider->value());
	}
	myVizMng->Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NavigationView::OnTimeSliderValueChanged(int value)
{
	myPlayIndex = myNavigationViewDataLength * value / myUI->timeSlider->maximum();
	Update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NavigationView::OnDiveBoxChanged(int value)
{
	LoadMission(value);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NavigationView::OnTimerTick()
{
	myPlayIndex++;
	if(myPlayIndex == myNavigationViewDataLength)
	{
		myTimer->stop();
		myUI->playButton->setChecked(false);
	}
	else
	{
		Update();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NavigationView::OnPlayButtonToggle(bool value)
{
	if(value)
	{
		myTimer->start(200, false);
	}
	else
	{
		myTimer->stop();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NavigationView::OnTimeForwardButtonClick()
{
	if(myPlayIndex < myNavigationViewDataLength - 1)
	{
		myPlayIndex++;
		Update();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NavigationView::OnTimeBackButtonClick()
{
	if(myPlayIndex > 0)
	{
		myPlayIndex--;
		Update();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NavigationView::OnExportImageButtonClick()
{
	for(int i = 0; i < myActiveOverlays; i++)
	{
		int idx = myOverlays[i]->Index;
		char path[DEFAULT_STRING];
		sprintf(path, ImagePathFormat.c_str(), myNavigationViewData[idx].UPImageName);
		QFile* source = new QFile(path);
		sprintf(path, ImageExportPathFormat.c_str(), 
			myNavigationViewData[idx].Timestamp,
			(int)myNavigationViewData[idx].X,
			(int)myNavigationViewData[idx].Y,
			myNavigationViewData[idx].Depth);
		source->copy(path);
		delete source;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NavigationView::OnImgUpdateButtonClick()
{
	UpdateImageOverlays();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NavigationView::OnImgDistanceSliderValueChanged(int value)
{
	myOverlayDistance = value;
	myUI->imgDistanceLabel->setText(QString("Image Distance: %1 meters").arg(value));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NavigationView::OnImgEnabledButtonButtonToggle(bool value)
{
	for(int i = 0; i < myActiveOverlays; i++)
	{
		myOverlays[i]->SetVisible(value);
	}
	myVizMng->Render();
}