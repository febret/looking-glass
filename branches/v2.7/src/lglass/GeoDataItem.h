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
#ifndef GEODATAITEM_H
#define GEODATAITEM_H

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "LookingGlassSystem.h"
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
class vtkBoxClipDataSet;

///////////////////////////////////////////////////////////////////////////////////////////////////
class GeoDataItem: public QObject
{
    Q_OBJECT
public:
	enum Type { Mesh, PointCloud, Image };

public:
	GeoDataItem();

	void Initialize(GeoDataView* view, Setting& cfg);

	void LoadPreferences(Setting& s);
	void SavePreferences(Setting& s);

	Type GetType() { return myType; }

	void SetVisible(bool value);

    void SetDepthScale(int value);

	void GetClipStartPoint(double* point);
	void GetClipEndPoint(double* point);


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
	void OnSelApplyClicked();
	void OnSelResetClicked();
	void OnStartPointClicked();
	void OnEndPointClicked();

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
	vtkBoxClipDataSet* myClipFilter;
    vtkPolyDataMapper* myContourMapper;
    vtkActor* myContourActor;

	QString myName;
	QString myLabel;
	QString myFilename;

	// Transform
	float myScale[3];
	float myOrientation[3];
	float myPosition[3];

	bool myVisible;
	bool myContoursVisible;
	float myOpacity;

	// Colors
	QColor myMainColor;
	QColor myContourColor;

	// UI
	Ui_GeoDataMeshPanel* myMeshUI;
	Ui_GeoDataImagePanel* myImageUI;
	Ui_GeoDataPointsPanel* myPointsUI;
	pqColorChooserButton* myMainColorButton;
	pqColorChooserButton* myContourColorButton;
	QGroupBox* myPanel;
};

#endif 
