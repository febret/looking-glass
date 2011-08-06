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
#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "LookingGlassSystem.h"
#include "DataSet.h"
#include "pqColorMapModel.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Preferences
{
public:
	Preferences();

	void Save(libconfig::Config* cfg);
	void Load(libconfig::Config* cfg);

	void Load(const char* filename);
	void Save(const char* filename);

	DataSetInfo::TagId GetGroupingTagId() { return myGroupingTagId; }
	void SetGroupingTagId(DataSetInfo::TagId value) { myGroupingTagId = value; }

	DataSet::SubsetType GetGroupingSubset() { return myGroupingSubset; }
	void SetGroupingSubset(DataSet::SubsetType value) { myGroupingSubset = value; }

	int GetPlotXLabels() { return myPlotXLabels; }
	void SetPlotXLabels(int value) { myPlotXLabels = value; }

	int GetPlotYLabels() { return myPlotYLabels; }
	void SetPlotYLabels(int value) { myPlotYLabels = value; }

	int GetPlotLabelFontSize() { return myPlotLabelFontSize; }
	void SetPlotLabelFontSize(int value) { myPlotLabelFontSize = value; }

	QColor GetPlotBackgroundColor() { return myPlotBackgroundColor; }
	void SetPlotBackgroundColor(QColor value) { myPlotBackgroundColor = value; }

	QColor GetPlotForegroundColor() { return myPlotForegroundColor; }
	void SetPlotForegroundColor(QColor value) { myPlotForegroundColor = value; }

	QColor GetPlotDefaultDataColor() { return myPlotDefaultDataColor; }
	void SetPlotDefaultDataColor(QColor value) { myPlotDefaultDataColor = value; }

	bool GetPlotPoints() { return myPlotPoints; }
	void SetPlotPoints(bool value) { myPlotPoints = value; }

	bool GetPlotLegend() { return myPlotLegend; }
	void SetPlotLegend(bool value) { myPlotLegend = value; }

	bool GetAdjustPlotLabels() { return myAdjustPlotLabels; }
	void SetAdjustPlotLabels(bool value) { myAdjustPlotLabels = value; }

	pqColorMapModel* GetPlotColorModel() { return myPlotColorModel; }
	void GetPlotColorModel(pqColorMapModel* value) { myPlotColorModel = value; }

	QByteArray GetWindowState() { return myWindowState; }
	void SetWindowState(const QByteArray& value) { myWindowState = value; }

	vtkColorTransferFunction* GetPlotColorTransferFunction() { return myPlotColorTransferFunction; }

	float GetDepthScale() { return myDepthScale; }
	void SetDepthScale(float value) { myDepthScale = value; }

	Setting& GetSection(const QString& name);

private:
	Config myCfg;

	// Grouping preferences.
	DataSetInfo::TagId myGroupingTagId;
	DataSet::SubsetType myGroupingSubset;

	// Plot preferences
	int myPlotXLabels;
	int myPlotYLabels;
	int myPlotLabelFontSize;

	bool myPlotPoints;
	bool myPlotLegend;
	bool myAdjustPlotLabels;

	pqColorMapModel* myPlotColorModel;
	vtkColorTransferFunction* myPlotColorTransferFunction;

	QColor myPlotBackgroundColor;
	QColor myPlotForegroundColor;
	QColor myPlotDefaultDataColor;

	QByteArray myWindowState;

	float myDepthScale;
};

#endif