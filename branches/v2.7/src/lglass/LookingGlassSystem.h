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

///////////////////////////////////////////////////////////////////////////////////////////////////
// Program configuration
#define MAX_PLOT_VIEWS 4
#define ENABLE_NAVIGATION_VIEW
//#define ENABLE_SLICE_VIEWER
//#define ENABLE_DATA_FIELD_SETTINGS
//#define ENABLE_SECTION_VIEW
#define MAX_SECTION_VIEWS 2
// The data decimation amout when the program is running in debug mode. Used to speed up loading.
#define DEBUG_DATA_DECIMATION 10
// KLUDGE_CONTOUR_OFFSET is used to avoid z fighting when displaying contours.
#define KLUDGE_CONTOUR_OFFSET 1.0


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
#include <vector>
#include <fstream>
#include<limits>
#include<time.h>

// QT Includes
#include <QObject>
#include <QString>
#include <QFile>
//#include <QtGui/QtGui>
//#include <QtNetwork/QtNetwork>
//#include <QQueue>
//#include <QTextStream>
//#include <QTimer>

// Paraview QT widgets
//#include "pq/pqChartValue.h"
//#include "pq/pqColorMapModel.h"
//#include "pq/pqColorMapWidget.h"

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
// Forward declaration of VTK classes
class vtkActor;
class vtkAttributeDataToFieldDataFilter;
class vtkAxesActor;
class vtkCubeSource;
class vtkContourFilter;
class vtkDoubleArray;
class vtkInteractorStyleTerrain;
class vtkInteractorObserver;
class vtkRenderWindow;
class vtkUnstructuredGridReader;
class vtkAlgorithmOutput;
class vtkCamera;
class vtkCellArray;
class vtkCylinderSource;
class vtkCutter;
class vtkColorTransferFunction;
class vtkCommand;
class vtkClipVolume;
class vtkFloatArray;
class vtkImageData;
class vtkImplicitPlaneRepresentation;
class vtkImplicitPlaneWidget2;
class vtkInteractorStyleTerrain;
class vtkLineWidget;
class vtkAxes;
class vtkDataObject;
class vtkDataSetMapper;
class vtkDelaunay2D;
class vtkFieldData;
class vtkJPEGReader;
class vtkGlyph3D;
class vtkImageShiftScale;
class vtkLookupTable;
class vtkMaskPoints;
class vtkMarchingContourFilter;
class vtkOutlineFilter;
class vtkParticleReader;
class vtkPieceWiseFunction;
class vtkPlane;
class vtkPNGReader;
class vtkPNGWriter;
class vtkPointData;
class vtkPointPicker;
class vtkPointSet;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkPolyDataNormals;
class vtkPolyDataReader;
class vtkPolyDataWriter;
class vtkProbeFilter;
class vtkProperty2D;
class vtkThreshold;
class vtkThresholdPoints;
class vtkPlaneSource;
class vtkProp;
class vtkProperty;
class vtkRenderer;
class vtkRenderWindow;
class vtkScalarBarActor;
class vtkSphereSource;
class vtkShepardMethod;
class vtkLoopSubdivisionFilter;
class vtkTextActor;
class vtkTexture;
class vtkTextProperty;
class vtkTransform;
class vtkTransformFilter;
class vtkUnstructuredGrid;
class vtkVolume;
class vtkVolumeProperty;
class vtkVolumeRayCastMapper;
class vtkColorTransferFunction;
class vtkVolumeRayCastCompositeFunction;
class vtkWindowToImageFilter;
class vtkInteractorStyleRubberBand2D;

///////////////////////////////////////////////////////////////////////////////////////////////////
class pqColorChooserButton;
class pqColorMapWidget;

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
