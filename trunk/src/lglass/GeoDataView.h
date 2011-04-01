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
#ifndef GEODATAVIEW_H
#define GEODATAVIEW_H

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "LookingGlassSystem.h"
#include "DockedTool.h"
#include "ui_GeoDataViewDock.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
class DataSet;
class Ui_MainWindow;
class VisualizationManager;
class vtkActor;

///////////////////////////////////////////////////////////////////////////////////////////////////
class GeoDataView: public DockedTool
{
    Q_OBJECT

public:
	static void BuildSondeBathymetry(DataSet* data);

    /////////////////////////////////////////////////////// Ctor / Dtor.
    GeoDataView(VisualizationManager* msg);
    ~GeoDataView();

	void Initialize();
    // Sets the bathymetry depth scale.
    void SetDepthScale(int value);
	// Color Accessors
	QColor GetOldBathyColor();
	QColor GetSondeBathyColor();
	QColor GetOldBathyContourColor();
	QColor GetSondeBathyContourColor();
	void SetOldBathyColor(QColor value);
	void SetSondeBathyColor(QColor value);
	void SetOldBathyContourColor(QColor value);
	void SetSondeBathyContourColor(QColor value);
	// Gets the depth value from old bathymetry or sonar data at the specified point.
	double GetDepthAt(float x, float y, bool sonde);
	void SetupBathyErrorMap();
	vtkPolyDataReader* GetOldBathyReader() { return myReader; }

public:
	// Sonar bathymetry offset values. These values are applied as offsets
	// over sonde data translations (specified in DataSet).
	static const int SonarBathymetryOffsetX = 800;
	static const int SonarBathymetryOffsetY = 200;
	
protected slots:
    void OpacitySliderChanged(int value);
	void OnOverlayOpacitySliderChanged(int);
	void OnAxesButtonToggle(bool checked);
	void SondeBathyOpacitySliderChanged(int value);
	void OnOldContoursButtonToggle(bool checked);
	void OnSondeContoursButtonToggle(bool checked);
	void OnOldContoursSliderReleased();
	void OnSondeContoursSliderReleased();
	void OnShowErrorMarkersButtonToggle(bool checked);
	void OnShowSonarDataButtonToggle(bool checked);

private:
	void SetupUI();
	void SetupContours();
	void BuildBathymetry();

private:
	// UI.
	Ui_GeoDataViewDock* myUI;
	VisualizationManager* myVizMng;

    // VTK stuff
	vtkJPEGReader* myOverlayReader;
	vtkTexture* myOverlayTexture;
	vtkPlaneSource* myOverlayPlane;
	vtkPolyDataMapper* myOverlayMapper;
	vtkActor* myOverlayActor;
	vtkAxesActor* myAxesActor;

	// Old bathymetry data.
	vtkPolyDataReader* myReader;
    vtkActor* myLakeActor;
	vtkPolyDataMapper* myOldBathyMapper;

	// Sonde-based batymetry data.
	vtkPolyDataReader* mySondeBathyReader;
    vtkPolyDataMapper* mySondeBathyMapper;
    vtkActor* mySondeBathyActor;

	// Sonar bathymetry data.
	vtkPolyDataReader* mySonarBathyReader;
    vtkDataSetMapper* mySonarBathyMapper;
    vtkActor* mySonarBathyActor;

	// Contours
	vtkContourFilter* myOldBathyContour;
    vtkPolyDataMapper* myOldBathyContourMapper;
    vtkActor* myOldBathyContourActor;
	vtkContourFilter* mySondeBathyContour;
    vtkPolyDataMapper* mySondeBathyContourMapper;
    vtkActor* mySondeBathyContourActor;
	
	// Bathymetry error map.
    vtkColorTransferFunction* myBathyErrorColorFunction;
    vtkPolyData* myBathyErrorData;
    vtkPolyDataMapper* myBathyErrorMapper;
    vtkActor* myBathyErrorActor;
	vtkContourFilter* myBathyErrorContour;
    vtkPolyDataMapper* myBathyErrorContourMapper;
    vtkActor* myBathyErrorContourActor;
    vtkScalarBarActor* myBathyErrorScaleActor;

	// Colors
	QColor myOldBathyColor;
	QColor mySondeBathyColor;
	QColor myOldBathyContourColor;
	QColor mySondeBathyContourColor;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline QColor GeoDataView::GetOldBathyColor()
{
	return myOldBathyColor;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline QColor GeoDataView::GetSondeBathyColor()
{
	return mySondeBathyColor;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline QColor GeoDataView::GetOldBathyContourColor()
{
	return myOldBathyContourColor;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline QColor GeoDataView::GetSondeBathyContourColor()
{
	return mySondeBathyContourColor;
}

#endif 
