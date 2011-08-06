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
#ifndef NAVIGATIONVIEW_H
#define NAVIGATIONVIEW_H

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "LookingGlassSystem.h"
#include "DockedTool.h"
#include "ui_NavigationViewDock.h"

#include "vtkTextActor3D.h"
#include "vtkCaptionActor2D.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_REPLAY_ITEMS 500000

///////////////////////////////////////////////////////////////////////////////////////////////////
struct NavigationViewItem
{
	int Timestamp;
	float X;
	float Y;
	float Depth;
	char UPImageName[DEFAULT_STRING];
	char Log[DEFAULT_STRING];
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class ImageOverlay
{
public:
    /////////////////////////////////////////////////////// Ctor / Dtor.
    ImageOverlay(VisualizationManager* msg);
    ~ImageOverlay();

	void SetTexture(const char* imagePathFormat, const char* imgPath);
	void SetPosition(float X, float Y);
	void SetSize(float size);
	void SetVisible(bool visible);

	int Index;
	float X; 
	float Y;

private:
	VisualizationManager* myVizMng;
	vtkPNGReader* myOverlayReader;
	vtkTexture* myOverlayTexture;
	vtkPlaneSource* myOverlayPlane;
	vtkPolyDataMapper* myOverlayMapper;
	vtkActor* myOverlayActor;
	vtkTextActor3D* myLabelActor;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class NavigationView: public DockedTool
{
	Q_OBJECT
public:
	// Configuration fields.
	// Filenames of the mission replay files to be loaded.
	int PathDecimation;
	vector<string> MissionFiles;
	float PathDepthOffset;
	string ImagePathFormat;
	string ImageExportPathFormat;
	string MissionDataPathFormat;
	float IcePickingThreshold;

public:
    /////////////////////////////////////////////////////// Ctor / Dtor.
	NavigationView(VisualizationManager* mng);
    virtual ~NavigationView();

    void Initialize();

	void LoadMissionSet(QString missionName);

	QColor GetMissionPathColor();
	void SetMissionPathColor(QColor value);

protected slots:
	void OnDiveBoxChanged(int value);
	void OnTimeSliderValueChanged(int value);
	void OnShowMissionButtonToggle(bool value);
	void OnPlayButtonToggle(bool);
	void OnTimerTick();
	void OnTimeForwardButtonClick();
	void OnTimeBackButtonClick();
	void OnExportImageButtonClick();
	void OnImgUpdateButtonClick();
	void OnImgDistanceSliderValueChanged(int value);
	void OnImgEnabledButtonButtonToggle(bool);

private:
    void SetupUI();
    void LoadMission(int num);
	void Update();
	void UpdateImageOverlays();

private:
	// Max number of image overlays.
	static const int MAX_IMAGE_OVERLAYS = 1024;

	// UI stuff.
	Ui::NavigationViewDock* myUI;
	VisualizationManager* myVizMng;

	// Mission replay data.
	NavigationViewItem myNavigationViewData[MAX_REPLAY_ITEMS];
	int myNavigationViewDataLength;
	int myDepthScale;

	// Mission replay VTK objects.
	vtkPolyData* myMissionPathData;
	vtkPolyDataMapper* myMissionPathMapper;
	vtkActor* myMissionPathActor;

	// AUV object
	vtkSphereSource* myAUVSource;
	vtkPolyDataMapper* myAUVMapper;
	vtkActor* myAUVActor;

	// Timer and play counters.
	QTimer* myTimer;
	int myPlayIndex;
	int myLogIndex;

	// Camera images
	QPixmap myUpImage;

	// Image overlays
	ImageOverlay* myOverlays[MAX_IMAGE_OVERLAYS];
	int myActiveOverlays;
	int myOverlayStartIndex;
	float myOverlayDistance;


	// Colors
	QColor myMissionPathColor;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline QColor NavigationView::GetMissionPathColor()
{
	return myMissionPathColor;
}

#endif 
