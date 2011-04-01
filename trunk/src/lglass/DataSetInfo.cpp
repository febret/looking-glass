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
#include "DataSetInfo.h"
#include "AppConfig.h"

using namespace libconfig;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FieldInfo::Load(libconfig::Setting& s)
{
	myName = s.getName();
	myLabel = (string)s["Label"];
	if(s.exists("FieldIndex"))
	{
		myType = FieldInfo::Data;
		myFieldIndex = s["FieldIndex"];
	}
	else if(s.exists("Expression"))
	{
		myType = FieldInfo::Expression;
		myExpression = (string)s["Expression"];
	}
	else
	{
		myType = FieldInfo::Script;
		myScript = (string)s["Script"];
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FieldInfo::Save(libconfig::Setting& s)
{
	// cleanup the setting
	while(s.getLength() != 0) s.remove((unsigned int)0);

	Setting& sLabel = s.add("Label", Setting::TypeString);
	sLabel = myLabel;
	switch(myType)
	{
	case FieldInfo::Data:
		{
			Setting& sFieldIndex = s.add("FieldIndex", Setting::TypeInt);
			sFieldIndex = myFieldIndex;
			break;
		}
	case FieldInfo::Expression:
		{
			Setting& sExpr = s.add("Expression", Setting::TypeString);
			sExpr = myExpression;
			break;
		}
	case FieldInfo::Script:
		{
			Setting& sScript = s.add("Script", Setting::TypeString);
			sScript = myScript;
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DataSetInfo::Load(AppConfig* cfg)
{
	myTag1Index = -1;
	myTag2Index = -1;
	myTag3Index = -1;
	myTag4Index = -1;

	myTag1Label = "Tag1";
	myTag2Label = "Tag2";
	myTag3Label = "Tag3";
	myTag4Label = "Tag4";

	try
	{
		Config* c = cfg->GetDataConfig();

		// Load file list.
		Setting& files = c->lookup("Application/DataSet/Files");
		for(int i = 0; i < files.getLength(); i++)
		{
			myFiles.push_back((string)files[i]);
		}

		myXFieldName = (string)c->lookup("Application/DataSet/XFieldName");
		myYFieldName = (string)c->lookup("Application/DataSet/YFieldName");
		myZFieldName = (string)c->lookup("Application/DataSet/ZFieldName");

		if(c->exists("Application/DataSet/Tag1Index"))
		{
			myTag1Index = (int)c->lookup("Application/DataSet/Tag1Index");
		}

		if(c->exists("Application/DataSet/Tag2Index"))
		{
			myTag2Index = (int)c->lookup("Application/DataSet/Tag2Index");
		}

		if(c->exists("Application/DataSet/Tag3Index"))
		{
			myTag3Index = (int)c->lookup("Application/DataSet/Tag3Index");
		}

		if(c->exists("Application/DataSet/Tag4Index"))
		{
			myTag4Index = (int)c->lookup("Application/DataSet/Tag4Index");
		}

		if(c->exists("Application/DataSet/Tag1Label"))
		{
			myTag1Label = (string)c->lookup("Application/DataSet/Tag1Label");
		}

		if(c->exists("Application/DataSet/Tag2Label"))
		{
			myTag2Label = (string)c->lookup("Application/DataSet/Tag2Label");
		}

		if(c->exists("Application/DataSet/Tag3Label"))
		{
			myTag3Label = (string)c->lookup("Application/DataSet/Tag3Label");
		}

		if(c->exists("Application/DataSet/Tag4Label"))
		{
			myTag4Label = (string)c->lookup("Application/DataSet/Tag4Label");
		}

		myTimestampDateIndex = c->lookup("Application/DataSet/TimestampDateIndex");
		myTimestampTimeIndex = c->lookup("Application/DataSet/TimestampTimeIndex");
		myTimestampStringFormat = (string)c->lookup("Application/DataSet/TimestampStringFormat");

		// Load fields.
		Setting& fields = c->lookup("Application/DataSet/Fields");
		for(int i = 0; i < fields.getLength(); i++)
		{
			Setting& sField = fields[i];
			FieldInfo* field = new FieldInfo();
			field->Load(sField);
			myFields.push_back(field);
		}
	}
	catch(SettingNotFoundException e)
	{
		printf("DataSetInfo::Load - Setting not found: %s\n", e.getPath());
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DataSetInfo::Save(AppConfig* cfg)
{
	try
	{
		Config* c = cfg->GetDataConfig();

		c->lookup("Application/DataSet/XFieldName") = myXFieldName;
		c->lookup("Application/DataSet/YFieldName") = myYFieldName;
		c->lookup("Application/DataSet/ZFieldName") = myZFieldName;

		for(vector<FieldInfo*>::iterator i = myFields.begin(); i != myFields.end(); i++)
		{
			QString fieldName = QString("Application/DataSet/Fields/%1").arg((*i)->GetName());
			if(c->exists(fieldName))
			{
				(*i)->Save(c->lookup(fieldName));
			}
			else
			{
				c->getRoot().add(fieldName, Setting::TypeGroup);
				(*i)->Save(c->lookup(fieldName));
			}
		}
	}
	catch(SettingNotFoundException e)
	{
		printf("DataSetInfo::Save - Setting not found: %s\n", e.getPath());
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FieldInfo* DataSetInfo::GetField(int index) 
{ 
	if((unsigned)index < myFields.size()) return myFields[index]; 
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FieldInfo* DataSetInfo::GetFieldByName(const QString& name) 
{ 
	int i = GetFieldIndex(name);
	if(i != -1) return myFields[i];
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString DataSetInfo::GetFile(int index)
{
	if((unsigned)index < myFiles.size()) return QString(myFiles[index].c_str()); 
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int DataSetInfo::GetFieldIndex(const QString& name)
{
	for(int i = 0; i < myFields.size(); i++)
	{
		if(myFields[i]->GetName() == name) return i;
	}
	return -1;
}

