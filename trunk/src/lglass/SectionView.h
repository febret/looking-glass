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
#ifndef SECTION_VIEW_H
#define SECTION_VIEW_H

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "LookingGlassSystem.h"
#include "DataSet.h"
#include "DockedTool.h"
#include "ui_SectionViewDock.h"

class vtkGenericCutter;

///////////////////////////////////////////////////////////////////////////////////////////////////
class SectionView: public DockedTool
{
	Q_OBJECT
public:
    SectionView(VisualizationManager* msg, int index);
    ~SectionView();

    void Initialize();
    void Update();
	void Render();
    
protected slots:
    void OnFieldChanged(int i);
	void OnExportImageButtonClicked();
	void OnApplyButtonClicked();
	void OnStartPointClicked();
	void OnEndPointClicked();

private:
	void SetupUI();

private:
	// UI.
	Ui_SectionViewDock* myUI;
	VisualizationManager* myVizMng;

	int myVolumeSamples[3];
	int mySelectedField;

	vtkPlane* myPlane;
	vtkGenericCutter* myCutter;

	vtkTransformFilter* mySeparationTransform;
    vtkShepardMethod* myVolumeBuilder;
	vtkThreshold* myVolumeCleanupFilter;

    vtkScalarBarActor* myScalarBar;

    vtkDataSetMapper* mySectionMapper;
    vtkActor* mySectionActor;

    vtkRenderWindow* myRenderWindow;
    vtkRenderer* myRenderer;
};

#endif 
