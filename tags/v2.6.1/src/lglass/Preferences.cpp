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
#include "Preferences.h"
#include "AppConfig.h"
#include "Utils.h"

using namespace libconfig;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Preferences::Preferences():
	myGroupingTagId(DataSetInfo::Tag3),
		myGroupingSubset(DataSet::SelectedData),
	myPlotXLabels(5),
	myPlotYLabels(5),
	myPlotBackgroundColor(255, 255, 255),
	myPlotForegroundColor(0, 0, 0),
	myPlotDefaultDataColor(120, 120, 120),
	myPlotLabelFontSize(8),
	myPlotLegend(true),
	myPlotPoints(false),
	myAdjustPlotLabels(true)
{
	myPlotColorModel = new pqColorMapModel();
	myPlotColorModel->setColorSpaceFromInt(0);
	myPlotColorModel->addPoint(0.0f, QColor(0, 0, 255));
	myPlotColorModel->addPoint(1.0f, QColor(255, 0, 0));
	myPlotColorModel->addPoint(0.5f, QColor(0, 255, 0));
	myPlotColorModel->setValueRange(0.0f, 1.0f);

	myPlotColorTransferFunction = vtkColorTransferFunction::New();

	Utils::UpdateColorTransferFunction(myPlotColorTransferFunction, myPlotColorModel);
}

#define WRITE_SETTING(name, settingType, type, var) \
	if(!pref.exists(name)) pref.add(name, settingType); \
	pref[name] = (type)var;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Preferences::Save(Config* cfg)
{
	try
	{
		// create the preference section if it does not exist
		if(!cfg->exists("Preferences"))
		{
			Setting& app = cfg->getRoot();
			app.add("Preferences", Setting::TypeGroup);
		}

		Setting& pref = cfg->lookup("Preferences");

		WRITE_SETTING("GroupingSubset", Setting::TypeInt, int, myGroupingSubset);
		WRITE_SETTING("GroupingTagId", Setting::TypeInt, int, myGroupingTagId);
		WRITE_SETTING("PlotLabelFontSize", Setting::TypeInt, int, myPlotLabelFontSize);

		WRITE_SETTING("PlotLegend", Setting::TypeBoolean, bool, myPlotLegend);
		WRITE_SETTING("PlotPoints", Setting::TypeBoolean, bool, myPlotPoints);
		WRITE_SETTING("AdjustPlotLabels", Setting::TypeBoolean, bool, myAdjustPlotLabels);

		WRITE_SETTING("PlotXLabels", Setting::TypeInt, int, myPlotXLabels);
		WRITE_SETTING("PlotYLabels", Setting::TypeInt, int, myPlotYLabels);

		WRITE_SETTING("PlotDefaultDataColor", Setting::TypeString, const char*, myPlotDefaultDataColor.name());
		WRITE_SETTING("PlotBackgroundColor", Setting::TypeString, const char*, myPlotBackgroundColor.name());
		WRITE_SETTING("PlotForegroundColor", Setting::TypeString, const char*, myPlotForegroundColor.name());

		// Save window state.
		WRITE_SETTING("WindowState", Setting::TypeString, const char*, myWindowState.toBase64());

		if(!pref.exists("PlotColors"))
		{
			Setting& stPlotColors = pref.add("PlotColors", Setting::TypeGroup);
		}
		Setting& stPlotColors = pref["PlotColors"];
		for(int i = 0; i < stPlotColors.getLength(); i++) stPlotColors.remove(i);

		int pts = myPlotColorModel->getNumberOfPoints();
		for(int i = 0; i < pts; i++)
		{
			QColor color;
			pqChartValue value;
			myPlotColorModel->getPointColor(i, color);
			myPlotColorModel->getPointValue(i, value);
			Setting& stPtData = stPlotColors.add(QString("Value%1").arg(i), Setting::TypeArray);

			stPtData.add(Setting::TypeFloat) = (float)value;
			stPtData.add(Setting::TypeFloat) = (float)color.red() / 255;
			stPtData.add(Setting::TypeFloat) = (float)color.green() / 255;
			stPtData.add(Setting::TypeFloat) = (float)color.blue() / 255;
		}
	}
	catch(SettingNotFoundException e)
	{
		printf("DataSetInfo::Load - Setting not found: %s\n", e.getPath());
	}
}
#undef WRITE_SETTING

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Preferences::Load(Config* cfg)
{
	try
	{
		if(cfg->exists("Preferences"))
		{
			Setting& pref = cfg->lookup("Preferences");

			if(pref.exists("GroupingSubset"))
				myGroupingSubset = (DataSet::SubsetType)((int)pref["GroupingSubset"]);

			if(pref.exists("GroupingTagId"))
				myGroupingTagId = (DataSetInfo::TagId)((int)pref["GroupingTagId"]);

			if(pref.exists("PlotLabelFontSize"))
				myPlotLabelFontSize = pref["PlotLabelFontSize"];

			if(pref.exists("PlotLegend"))
				myPlotLegend = pref["PlotLegend"];

			if(pref.exists("PlotPoints"))
				myPlotPoints = pref["PlotPoints"];

			if(pref.exists("PlotXLabels"))
				myPlotXLabels = pref["PlotXLabels"];

			if(pref.exists("PlotYLabels"))
				myPlotYLabels = pref["PlotYLabels"];

			if(pref.exists("AdjustPlotLabels"))
				myAdjustPlotLabels = pref["AdjustPlotLabels"];

			if(pref.exists("PlotDefaultDataColor"))
				myPlotDefaultDataColor.setNamedColor((const char*)pref["PlotDefaultDataColor"]);

			if(pref.exists("PlotBackgroundColor"))
				myPlotBackgroundColor.setNamedColor((const char*)pref["PlotBackgroundColor"]);

			if(pref.exists("PlotForegroundColor"))
				myPlotForegroundColor.setNamedColor((const char*)pref["PlotForegroundColor"]);

			if(pref.exists("WindowState"))
				myWindowState = QByteArray::fromBase64((const char*)pref["WindowState"]);

			if(pref.exists("PlotColors"))
			{
				myPlotColorModel->removeAllPoints();
				Setting& stPlotColors = pref["PlotColors"];
				int pts = stPlotColors.getLength();
				for(int i = 0; i < pts; i++)
				{
					Setting& stPtData = stPlotColors[i];
					int r = (int)((float)stPtData[1] * 255);
					int g = (int)((float)stPtData[2] * 255);
					int b = (int)((float)stPtData[3] * 255);
					myPlotColorModel->addPoint((float)stPtData[0], QColor(r, g, b));
				}
				Utils::UpdateColorTransferFunction(myPlotColorTransferFunction, myPlotColorModel);
			}
		}
	}
	catch(SettingNotFoundException e)
	{
		printf("DataSetInfo::Load - Setting not found: %s\n", e.getPath());
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Preferences::Load(const char* filename)
{
	if(!QFile::exists(filename))
	{
		Console::Message(QString("Preferences file %1 not found: using default settings.").arg(filename));
		return;
	}

	FILE* stream = fopen(filename, "r");
	Config cfg;
	try
	{
		cfg.read(stream);

		// Load preferences.
		Load(&cfg);
	}
	catch(ParseException e)
	{
		Console::Error(QString("Preference loading: %1 at line %2").arg(e.getError()).arg(e.getLine()));
		ShutdownApp(true, false);
	}
	fclose(stream);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Preferences::Save(const char* filename)
{
	Config cfg;
	Save(&cfg);
	cfg.writeFile(filename);
}
