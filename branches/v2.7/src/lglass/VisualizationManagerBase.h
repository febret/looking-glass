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
#ifndef VISUALIZATIONMANAGERBASE_H
#define VISUALIZATIONMANAGERBASE_H

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "LookingGlassSystem.h"
#include "Ui_MainWindow.h"

class QMainWindow;

///////////////////////////////////////////////////////////////////////////////////////////////////
class VisualizationManagerBase: public QObject
{
	Q_OBJECT
public:
    /////////////////////////////////////////////////////// Ctor / Dtor.
    VisualizationManagerBase();
    ~VisualizationManagerBase();

    /////////////////////////////////////////////////////// Other Methods.
    void Initialize();
	// Redraws the content of the main render window.
	void Render();
	// Updates the rendering pipeline and refreshed all the visualizations.
	virtual void Update();
	// Adds a 3D widget to the main render window.
	void Add3DWidget(vtkInteractorObserver* widget);
	// Gets the renderer associated to the main render window.
	vtkRenderer* GetMainRenderer();
	// Gets the unique instance of the main application window.
	QMainWindow* GetMainWindow();
    // Sets the render window that will be used as the main visualization viewport.
	QAction* AddWindowMenuAction(QString name);
	Ui_MainWindow* GetUI() { return myUI; }
	vtkRenderWindow* GetRenderWindow() { return myRenderWindow; }

protected slots:
	void OnQuitTrigger(bool);

private:
	// Reference to UI.
	QMainWindow* myMainWindow;
	Ui_MainWindow* myUI;

    vtkRenderer* myRenderer;
    vtkRenderWindow* myRenderWindow;
};

#endif // MAINWINDOW_H
