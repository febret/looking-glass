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
#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "LookingGlassSystem.h"
#include "DataSet.h"

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

private:
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
};

#endif