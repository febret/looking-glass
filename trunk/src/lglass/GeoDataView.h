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
#ifndef GEODATAVIEW_H
#define GEODATAVIEW_H

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "LookingGlassSystem.h"
#include "DockedTool.h"
#include "ui_GeoDataViewDock.h"
#include "ui_GeoDataMeshPanel.h"
#include "ui_GeoDataImagePanel.h"
#include "ui_GeoDataPointsPanel.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
class DataSet;
class Ui_MainWindow;
class VisualizationManager;
class GeoDataView;
class vtkActor;
class vtkElevationFilter;
class vtkGlyph3D;

///////////////////////////////////////////////////////////////////////////////////////////////////
class GeoDataItem: public QObject
{
    Q_OBJECT
public:
	enum Type { Mesh, PointCloud, Image };

public:
	GeoDataItem();

	void Initialize(GeoDataView* view, Setting& cfg);
	Type GetType() { return myType; }

    void SetDepthScale(int value);


	const QString& GetName() { return myName; }
	const QString& GetLabel() { return myLabel; }

	QColor GetMainColor() { myMainColor; }
	QColor GetContourColor() { myContourColor; }

	void SetMainColor(QColor color);
	void SetContourColor(QColor color);

protected slots:
	void OnVisibleToggle(bool checked);
	void OnContourToggle(bool checked);
	void OnMainColorChanged(const QColor& color);
	void OnContourColorChanged(const QColor& color);
	void OnOpacityChanged(double value);
	void OnRefreshClicked();
	void OnContourParamsChanged();

private:
	void InitMesh(QFile* file, Setting& cfg);
	void InitPoints(QFile* file, Setting& cfg);
	void InitImage(QFile* file, Setting& cfg);
	void InitActorTransform(Setting& cfg);

private:
	Type myType;

	GeoDataView* myView;

	// Vtk stuff.
	vtkTexture* myImageTexture;
	vtkPlaneSource* myImagePlane;
	vtkDataSetMapper* myMapper;
	vtkActor* myActor;
	vtkElevationFilter* myElevationFilter;
	vtkGlyph3D* myGlyphFilter;
	vtkContourFilter* myContourFilter;
    vtkPolyDataMapper* myContourMapper;
    vtkActor* myContourActor;

	QString myName;
	QString myLabel;
	QString myFilename;

	// Transform
	float myScale[3];
	float myOrientation[3];
	float myPosition[3];

	// Colors
	QColor myMainColor;
	QColor myContourColor;

	// UI
	Ui_GeoDataMeshPanel* myMeshUI;
	Ui_GeoDataImagePanel* myImageUI;
	Ui_GeoDataPointsPanel* myPointsUI;
	pqColorChooserButton* myMainColorButton;
	pqColorChooserButton* myContourColorButton;
	pqCollapsedGroup* myPanel;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class GeoDataView: public DockedTool
{
    Q_OBJECT
public:
	static const int MaxGeoDataItems = 64;

public:
	static void BuildSondeBathymetry(DataSet* data);

    /////////////////////////////////////////////////////// Ctor / Dtor.
    GeoDataView(VisualizationManager* msg);
    ~GeoDataView();

	void Initialize();
    // Sets the bathymetry depth scale.
    void SetDepthScale(int value);

	Ui_GeoDataViewDock* GetUI() { return myUI; }

	// Gets the depth value from old bathymetry or sonar data at the specified point.
	//double GetDepthAt(float x, float y, bool sonde);
	//void SetupBathyErrorMap();
	//vtkPolyDataReader* GetOldBathyReader() { return myReader; }

//protected slots:
//    void OpacitySliderChanged(int value);
//	void OnOverlayOpacitySliderChanged(int);
//	void OnAxesButtonToggle(bool checked);
//	void SondeBathyOpacitySliderChanged(int value);
//	void OnOldContoursButtonToggle(bool checked);
//	void OnSondeContoursButtonToggle(bool checked);
//	void OnOldContoursSliderReleased();
//	void OnSondeContoursSliderReleased();
//	void OnShowErrorMarkersButtonToggle(bool checked);
//	void OnShowSonarDataButtonToggle(bool checked);

private:
	void SetupUI();
	//void SetupContours();
	//void BuildBathymetry();

private:
	// UI.
	Ui_GeoDataViewDock* myUI;
	VisualizationManager* myVizMng;

	GeoDataItem* myItems[MaxGeoDataItems];
	int myNumItems;

	// Bathymetry error map.
	 //   vtkColorTransferFunction* myBathyErrorColorFunction;
	 //   vtkPolyData* myBathyErrorData;
	 //   vtkPolyDataMapper* myBathyErrorMapper;
	 //   vtkActor* myBathyErrorActor;
	//vtkContourFilter* myBathyErrorContour;
	 //   vtkPolyDataMapper* myBathyErrorContourMapper;
	 //   vtkActor* myBathyErrorContourActor;
	 //   vtkScalarBarActor* myBathyErrorScaleActor;
};

#endif 
