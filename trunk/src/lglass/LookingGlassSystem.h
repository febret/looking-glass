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
#ifndef LOOKINGGLASSSYSTEM_H
#define LOOKINGGLASSSYSTEM_H


/////////////////////////////////////////////////////////////////////////////////////////////////////
// USe secure std functions when compiling with visual studio, and disable additional secure warnings.
#ifdef _MSC_VER
#ifndef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1 
#endif
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// Global External Library includes. 

// System
#include <stdio.h>
#include <string>
#include <cmath>
#include <fstream>
#include<limits>
#include<time.h>

// QT
#include <QFile>
#include <QtGui/QtGui>
#include <QtNetwork/QtNetwork>
#include <QObject>
#include <QQueue>
#include <QTextStream>
#include <QTimer>

// VTK
#include <vtkActor.h>
#include <vtkAttributeDataToFieldDataFilter.h>
#include <vtkAxesActor.h>
#include <vtkCubeSource.h>
#include <vtkContourFilter.h>
#include <vtkDoubleArray.h>
#include <vtkInteractorStyleTerrain.h>
#include <vtkRenderWindow.h>
#include <vtkUnstructuredGridReader.h>
#include <vtkAlgorithmOutput.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCylinderSource.h>
#include <vtkCutter.h>
#include <vtkColorTransferFunction.h>
#include <vtkCommand.h>
#include <vtkClipVolume.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkImplicitPlaneRepresentation.h>
#include <vtkImplicitPlaneWidget2.h>
#include <vtkInteractorStyleTerrain.h>
#include <vtkLineWidget.h>
#include <vtkAxes.h>
#include <vtkDataSetMapper.h>
#include <vtkDelaunay2D.h>
#include <vtkFieldData.h>
#include <vtkJPEGReader.h>
#include <vtkGlyph3D.h>
#include <vtkImageShiftScale.h>
#include <vtkLookupTable.h>
#include <vtkMaskPoints.h>
#include <vtkMarchingContourFilter.h>
#include <vtkOutlineFilter.h>
#include <vtkParticleReader.h>
#include <vtkPieceWiseFunction.h>
#include <vtkPlane.h>
#include <vtkPNGReader.h>
#include <vtkPNGWriter.h>
#include <vtkPointData.h>
#include <vtkPointPicker.h>
#include <vtkPointSet.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkProbeFilter.h>
#include <vtkProperty2D.h>
#include <vtkThreshold.h>
#include <vtkThresholdPoints.h>
#include <vtkPlaneSource.h>
#include <vtkProp.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkScalarBarActor.h>
#include <vtkSphereSource.h>
#include <vtkShepardMethod.h>
#include <vtkLoopSubdivisionFilter.h>
#include <vtkTextActor.h>
#include <vtkTexture.h>
#include <vtkTextProperty.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkVolumeRayCastMapper.h>
#include <vtkColorTransferFunction.h>
#include <vtkVolumeRayCastCompositeFunction.h>
#include <vtkWindowToImageFilter.h>
#include <vtkInteractorStyleRubberBand2D.h>

// Paraview QT widgets
#include "pq/pqChartValue.h"
#include "pq/pqColorMapModel.h"
#include "pq/pqColorMapWidget.h"

// Libconfig
#include "libconfig.hh"

using namespace std;
using namespace libconfig;

// Including the console class here for convenience purposes.
#include "Console.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declaration of application classes
class AppConfig;
struct DataItem;
class DataFieldSettings;
class DataSet;
class DataSetInfo;
class FieldInfo;
class FilterInfo;
class NavigationView;
class Plot;
class PlotView;
class GeoDataView;
class GlacierViewOptions;
class GlacierVisualizationManager;
class SliceViewer;
class VolumeView;
class ColorFunctionManager;
class Preferences;
class PreferencesWindow;
class DataViewOptions;
class TableView;
class Ui_MainWindow;
class VisualizationManager;
class VisualizationManagerBase;
class VtkDataManager;
class pqColorChooserButton;
class LineTool;
class SectionView;

///////////////////////////////////////////////////////////////////////////////////////////////////
#define LOOKING_GLASS_VERSION "2.7"
#define DEFAULT_STRING 256

///////////////////////////////////////////////////////////////////////////////////////////////////
#define ENUM(name) \
	class name { \
	private: \
		name() {}; \
	public: \
	enum Enum { 

#define END_ENUM }; };

#define VTK_CALLBACK(name, obj, method) \
	class name: public vtkCommand { \
	private: \
		obj* mySelf; \
	public: \
		name(obj* self) { mySelf = self; } \
		virtual void Execute(vtkObject* caller, unsigned long, void* data) { mySelf->method; } };

#define QCOLOR_TO_VTK(color) (double)color.red() / 255, \
                             (double)color.green() / 255, \
                             (double)color.blue() / 255

///////////////////////////////////////////////////////////////////////////////////////////////////
void SetInitMessage(const char* msg);
void ShutdownApp(bool whoops = false, bool saveConfig = false);

#endif 
