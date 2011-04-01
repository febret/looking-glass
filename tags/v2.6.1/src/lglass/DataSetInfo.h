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
#ifndef DATASETINFO_H
#define DATASETINFO_H

#include "LookingGlassSystem.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class FieldInfo
{
public:
	enum Type {Data, Expression, Script};

public:
	FieldInfo() { myEvalVariable = NULL; myEnabled = true; }

	void Load(libconfig::Setting& s);
	void Save(libconfig::Setting& s);

	// Properties.
	QString GetExpression() { return myExpression.c_str(); }
	void SetExpression(const QString& value ) { myExpression = value; }

	QString GetName() { return myName.c_str(); }
	void SetName(const QString& value) { myName = value; }

	QString GetLabel() { return myLabel.c_str(); }
	void  SetLabel(const QString& value) { myLabel = value; }

	QString GetScript() { return myScript.c_str(); }
	void SetScript(const QString& value) { myScript = value; }

	int GetFieldIndex() { return myFieldIndex; }
	void SetFieldIndex(int value) { myFieldIndex = value; }

	Type GetType() { return myType; }
	void SetType(Type value) { myType = value; }

	bool IsHidden() { return myHidden; }
	void SetHidden(bool value) { myHidden = value; }

	bool IsEnabled() { return myEnabled; }
	void SetEnabled(bool value) { myEnabled = value; }

	void SetEvalVariable(double* value) { myEvalVariable = value; }
	double* GetEvalVariable() { return myEvalVariable; }

private:
	Type myType;
	int myFieldIndex;
	bool myEnabled;
	bool myHidden;
	string myScript;
	string myExpression;
	string myName;
	string myLabel;
	double* myEvalVariable;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class DataSetInfo
{
public:
	enum TagId { Tag1, Tag2, Tag3, Tag4 };
	enum TimestampType { None, TimestampField, DateTimeStringFields };
	// This may be removed in the future.
	static const int MAX_FIELDS = 32; 

public:
	DataSetInfo()
	{
		myTagEnabled[0] = true;
		myTagEnabled[1] = true;
		myTagEnabled[2] = true;
		myTagEnabled[3] = true;
	}

	void Load(AppConfig* cfg);
	void Save(AppConfig* cfg);

	// Field search;
	int GetFieldIndex(const QString& name);
	FieldInfo* GetFieldByName(const QString& name); 

	// Properties.
	int GetNumFiles() { return myFiles.size(); }
	int GetNumFields() { return myFields.size(); }
	int GetTimestampDateIndex() { return myTimestampDateIndex; }
	int GetTimestampTimeIndex() { return myTimestampTimeIndex; }
	QString GetTimestampStringFormat() { return QString(myTimestampStringFormat.c_str()); }
	void SetTimestampStringFormat(const QString& value) { myTimestampStringFormat = value; }
	QString GetXFieldName() { return QString(myXFieldName.c_str()); }
	QString GetYFieldName() { return QString(myYFieldName.c_str()); }
	QString GetZFieldName() { return QString(myZFieldName.c_str()); }
	QString GetFile(int index);
	FieldInfo* GetField(int index);

	int IsTagEnabled(int tagId) { return myTagEnabled[tagId]; }
	void SetTagEnabled(int tagId, bool enabled) { myTagEnabled[tagId] = enabled; }

	int GetTag1Index() { return myTag1Index; }
	void SetTag1Index(int value) { myTag1Index = value; }

	int GetTag2Index() { return myTag2Index; }
	void SetTag2Index(int value) { myTag2Index = value; }

	int GetTag3Index() { return myTag3Index; }
	void SetTag3Index(int value) { myTag3Index = value; }

	int GetTag4Index() { return myTag4Index; }
	void SetTag4Index(int value) { myTag4Index = value; }

	string GetTag1Label() { return myTag1Label; }
	void SetTag1Label(string value) { myTag1Label = value; }

	string GetTag2Label() { return myTag2Label; }
	void SetTag2Label(string value) { myTag2Label = value; }

	string GetTag3Label() { return myTag3Label; }
	void SetTag3Label(string value) { myTag3Label = value; }

	string GetTag4Label() { return myTag4Label; }
	void SetTag4Label(string value) { myTag4Label = value; }

private:
	string myXFieldName;
	string myYFieldName;
	string myZFieldName;

	int myTag1Index;
	int myTag2Index;
	int myTag3Index;
	int myTag4Index;

	string myTag1Label;
	string myTag2Label;
	string myTag3Label;
	string myTag4Label;

	bool myTagEnabled[4];

	int myNumFields;

	// Timestamp format
	TimestampType myTimestampType;
	int myTimestampDateIndex;
	int myTimestampTimeIndex;
	int myTimestampIndex;
	string myTimestampStringFormat;

	vector<string> myFiles;
	vector<FieldInfo*> myFields;
};

#endif