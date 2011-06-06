###################################################################################################
# THE LOOKING GLASS VISUALIZATION TOOLSET
#-------------------------------------------------------------------------------------------------
# Author: 
#	Alessandro Febretti		Electronic Visualization Laboratory, University of Illinois at Chicago
# Contact & Web:
#  febret@gmail.com		http://febretpository.hopto.org
#-------------------------------------------------------------------------------------------------
# Looking Glass has been built as part of the ENDURANCE Project (http://www.evl.uic.edu/endurance/).
# ENDURANCE is supported by the NASA ASTEP program under Grant NNX07AM88G and by the NSF USAP.
#-------------------------------------------------------------------------------------------------
# Copyright (c) 2010-2011, Electronic Visualization Laboratory, University of Illinois at Chicago
# All rights reserved.
# Redistribution and use in source and binary forms, with or without modification, are permitted 
# provided that the following conditions are met:
# 
# Redistributions of source code must retain the above copyright notice, this list of conditions 
# and the following disclaimer. Redistributions in binary form must reproduce the above copyright 
# notice, this list of conditions and the following disclaimer in the documentation and/or other 
# materials provided with the distribution. 
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
# FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF 
# USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
###################################################################################################

###################################################################################################
# Find VTK
FIND_PACKAGE(VTK)
IF(NOT VTK_DIR)
  MESSAGE(FATAL_ERROR "Please set VTK_DIR.")
ENDIF(NOT VTK_DIR)
INCLUDE(${VTK_USE_FILE})

###################################################################################################
# Setup QT VTK Widget
SET(QT_MOC_EXECUTABLE ${VTK_QT_MOC_EXECUTABLE} CACHE FILEPATH "")
SET(QT_UIC_EXECUTABLE ${VTK_QT_UIC_EXECUTABLE} CACHE FILEPATH "")
SET(QT_QMAKE_EXECUTABLE ${VTK_QT_QMAKE_EXECUTABLE} CACHE FILEPATH "")
SET(DESIRED_QT_VERSION ${VTK_DESIRED_QT_VERSION} CACHE FILEPATH "")

###################################################################################################
# Find and setup Qt
FIND_PACKAGE(Qt4)
SET(QT_USE_QTNETWORK true)
IF(QT_USE_FILE)
  INCLUDE(${QT_USE_FILE})
ELSE(QT_USE_FILE)
  SET(QT_LIBRARIES  ${QT_QT_LIBRARY})
ENDIF(QT_USE_FILE)


# Use the include path and library for Qt that is used by VTK.
INCLUDE_DIRECTORIES(
  ${QT_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${LookingGlass_SOURCE_DIR}/src/3rdparty/libconfig
  ${LookingGlass_SOURCE_DIR}/src/lglass/pq
)

LINK_DIRECTORIES(
  ${LookingGlass_BINARY_DIR}/src/3rdparty/libconfig
)

###################################################################################################
# Source files
SET( Srcs 
        AppConfig.cpp
        ColorFunctionManager.cpp
        Console.cpp
        DataFieldSettings.cpp
        DataSet.cpp
        DataSetInfo.cpp
        DockedTool.cpp
        GeoDataView.cpp
        Main.cpp
        NavigationView.cpp
        Plot.cpp
        PlotView.cpp
        Preferences.cpp
        PreferencesWindow.cpp
        ProgressWindow.cpp
        RepositoryManager.cpp
        SliceViewer.cpp
        DataViewOptions.cpp
        TableView.cpp
        Utils.cpp
        VisualizationManager.cpp
        VisualizationManagerBase.cpp
        VtkDataManager.cpp
        eval/evalwrap.c
        eval/evalkern.c)
    
###################################################################################################
# Headers
SET( Headers 
        AppConfig.h
        ColorFunctionManager.h
        Console.h
        DataFieldSettings.h
        DataSet.h
        DataSetInfo.h
        DockedTool.h
        GeoDataView.h
        LookingGlassSystem.h
        NavigationView.h
        Plot.h
        PlotView.h
        Preferences.h
        PreferencesWindow.h
        ProgressWindow.h
        RepositoryManager.h
        SliceViewer.h
        DataViewOptions.h
        ToolsetSetupWindow.h
        TableView.h
        Utils.h
        VisualizationManager.h
        VisualizationManagerBase.h
        VtkDataManager.h
        eval/evaldefs.h
        eval/evalkern.h
        eval/evalfunctions.h) 
        
###################################################################################################
SET( UIS 
		ui/ColorFunctionManagerDock.ui
        ui/DataFieldSettings.ui
		ui/GeoDataViewDock.ui
		ui/GeoDataMeshPanel.ui
		ui/GeoDataImagePanel.ui
		ui/GeoDataPointsPanel.ui
        ui/MainWindow.ui
        ui/NavigationViewDock.ui
		ui/PlotViewDock.ui
        ui/ProgressWindow.ui
		ui/PreferencesWindow.ui
		ui/SetupToolset.ui
		ui/SliceViewerWindow.ui
		ui/DataViewOptionsDock.ui
		ui/TableView.ui)
        
###################################################################################################
SET( pqSrc
        pq/pqColorChooserButton.cxx
        pq/pqColorMapColorChanger.cxx
        pq/pqColorMapModel.cxx
        pq/pqChartValue.cxx
        pq/pqPointMarker.cxx
        pq/pqChartPixelScale.cxx
        pq/pqColorMapWidget.cxx
        pq/pqCollapsedGroup.cxx
        )
        
###################################################################################################
SET( pqHeaders
        pq/pqColorChooserButton.h
        pq/pqColorMapColorChanger.h
        pq/pqColorMapModel.h
        pq/pqChartValue.h
        pq/pqPointMarker.h
        pq/pqChartPixelScale.h
        pq/pqColorMapWidget.h
        pq/pqCollapsedGroup.h)

###################################################################################################
# Qt wrapping and resources.
QT4_WRAP_UI( UIHeaders ${UIS})
QT4_WRAP_CPP( MOCSrcs ${Headers} ${pqHeaders})
QT4_ADD_RESOURCES( MOCSrcs resources/resources.qrc)

ADD_DEFINITIONS( -DQT_GUI_LIBS -DQT_CORE_LIB -DQT3_SUPPORT -DLIBCONFIG_STATIC -DLIBCONFIGXX_STATIC)

SET_SOURCE_FILES_PROPERTIES( ${Srcs} PROPERTIES OBJECT_DEPENDS "${UIHeaders}")
                            
###################################################################################################
# Organize sources and auto-generated files in folders (this is mainly for visual studio solutions)
SOURCE_GROUP( eval REGULAR_EXPRESSION eval*)
SOURCE_GROUP( UI REGULAR_EXPRESSION .*ui)
SOURCE_GROUP( Resources REGULAR_EXPRESSION qrc_*)
SOURCE_GROUP( pq REGULAR_EXPRESSION pq.*)
SOURCE_GROUP( Moc REGULAR_EXPRESSION moc_.*)

###################################################################################################
# Setup compile info
ADD_EXECUTABLE( lglass MACOSX_BUNDLE ${Srcs} ${ResourcesSrcs} ${Headers} ${UISrcs} ${MOCSrcs} ${pqSrc})

###################################################################################################
# Setup link info.
TARGET_LINK_LIBRARIES( lglass
    libconfig
    ${QT_LIBRARIES}
    QVTK
    vtkRendering
    vtkGraphics
    vtkIO
    vtkCommon
    vtkHybrid
    vtkVolumeRendering
)


