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
// Application Includes
#include "AppConfig.h"
#include "RepositoryManager.h"
#include "DataSet.h"
#include "DataSetInfo.h"
#include "ProgressWindow.h"
#include "Utils.h"
#include  "VtkDataManager.h"
#include  "Preferences.h"

extern "C"
{
#include "eval/evaldefs.h"
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DataSet::DataSet():
	mySondeBathyDepthCorrection(1.5f),
	myData(NULL),
	myDataLength(0)
{
	// create info object.
	myInfo = new DataSetInfo();

	// initialize ranges array.
	for (int i = 0; i < DataSetInfo::MAX_FIELDS; i++)
    {
		myFieldRange[i][0] =  FLT_MAX;
        myFieldRange[i][1] =  FLT_MIN;
    }

	myTimestampRange[0] =  INT_MAX;
	myTimestampRange[1] =  0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DataSet::~DataSet()
{
	delete myInfo;

	if(myData != NULL)
	{
		delete myData;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString DataSet::GetFieldName(int index)
{
	return myInfo->GetField(index)->GetLabel();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float* DataSet::GetFieldRange(int index)
{
	return myFieldRange[index];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DataSet::Initialize()
{
	Load();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
DataItem* DataSet::GetData(int index, DataSet::SubsetType subset)
{
	if(index >= GetDataLength(subset)) return NULL;

	if(subset == DataSet::AllData) return &myData[index];
	else if(subset == DataSet::FilteredData) return myFilteredData[index];
	return mySelectedData[index];
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int DataSet::GetDataLength(DataSet::SubsetType subset)
{
	if(subset == DataSet::AllData) return myDataLength;
	else if(subset == DataSet::FilteredData) return myFilteredDataLength;
	return mySelectedDataLength;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DataSet::Load()
{
    SetInitMessage("Initializing Sonde Data...");

	// Setup dataset info.
	myInfo->Load(AppConfig::GetInstance());

	// Load data files.
	// NOTE: Right now this supports just a single data file, beacuse of a bunch of bugs still lying around.
	if(myInfo->GetNumFiles() > 1)
	{
		Console::Error("This version of Looking Glass does not support multiple CSV file loading. Please specify a single cvs file in the profile configuration.");
		ShutdownApp(true);
	}

	int i = 0;
	while(!myInfo->GetFile(i).isNull())
	{
		LoadFile(myInfo->GetFile(i));
		i++;
	}

	// Initialize data grouping structures.
	InitGroups();

	Console::Message(QString("Tag1 Count: %1").arg(myTag1List.count()));
	Console::Message(QString("Tag2 Count: %1").arg(myTag2List.count()));
	Console::Message(QString("Tag3 Count: %1").arg(myTag3List.count()));
	Console::Message(QString("Tag4 Count: %1").arg(myTag4List.count()));

	/*Console::Message("Tag1 Values: " + myTag1List.join(", "));
	Console::Message("Tag2 Values: " + myTag2List.join(", "));
	Console::Message("Tag3 Values: " + myTag3List.join(", "));*/

	// Compute field expressions
	int j = 0;
	while(myInfo->GetField(j))
	{
		UpdateField(j);
		j++;
	}
    SetInitMessage("Done.");

	int zFieldId = myInfo->GetFieldIndex(myInfo->GetZFieldName());
	int yFieldId = myInfo->GetFieldIndex(myInfo->GetYFieldName());
	int xFieldId = myInfo->GetFieldIndex(myInfo->GetXFieldName());

	// Initialize X, Y and Z fields.
	for(int i = 0; i < myDataLength; i++)
	{
		myData[i].X = myData[i].Field[xFieldId];
		myData[i].Y = myData[i].Field[yFieldId];
		myData[i].Z = myData[i].Field[zFieldId];
	}

	VtkDataManager::GetInstance()->Update(DataSet::AllData);

	InitSondeBathyData();
	ApplyFilters();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DataSet::UpdateField(int index)
{
	FieldInfo* fi = myInfo->GetField(index);

	ProgressWindow* pw = ProgressWindow::GetInstance();

	pw->SetItemName(QString("Computing field: %1").arg(fi->GetLabel()));
	pw->SetItemProgress(0);

	// Reset ranges.
	myFieldRange[index][0] =  FLT_MAX;
    myFieldRange[index][1] =  FLT_MIN;

	if(fi->GetType() == FieldInfo::Expression)
	{
		for(int i = 0; i < myDataLength; i++)
		{
			Utils::SetEvalVariables(myData[i], myInfo);
			QString expr = QString("_r = %1").arg(fi->GetExpression());
			float value = Utils::Eval(expr);
			myData[i].Field[index] = value;

			// Update field ranges.
			if(value < myFieldRange[index][0]) myFieldRange[index][0] = value;
			if(value > myFieldRange[index][1]) myFieldRange[index][1] = value;

			if(i % 100 == 0) pw->SetItemProgress(i * 100 / myDataLength);
		}
	}
	else if(fi->GetType() == FieldInfo::Script)
	{
		QTextStream* scriptStream = new QTextStream(&fi->GetScript(), QIODevice::ReadOnly);
		QString script = scriptStream->readAll();

		script = script.replace("#out", "_r");
		QStringList scriptLines = script.split('\n');

		for(int i = 0; i < myDataLength; i++)
		{
			Utils::SetEvalVariables(myData[i], myInfo);
			float value = 0;
			
			QListIterator<QString> li(scriptLines);
			while(li.hasNext())
			{
				value = Utils::Eval(li.next());
			}

			myData[i].Field[index] = value;

			// Update field ranges.
			if(value < myFieldRange[index][0]) myFieldRange[index][0] = value;
			if(value > myFieldRange[index][1]) myFieldRange[index][1] = value;

			if(i % 100 == 0) pw->SetItemProgress(i * 100 / myDataLength);
		}
		delete scriptStream;
	}
	else
	{
		for(int i = 0; i < myDataLength; i++)
		{
			float value = myData[i].Field[index];
			// Update field ranges.
			if(value < myFieldRange[index][0]) myFieldRange[index][0] = value;
			if(value > myFieldRange[index][1]) myFieldRange[index][1] = value;
		}
	}

	ProgressWindow::GetInstance()->Done();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DataSet::LoadFile(const QString& name)
{
	//QString name = AppConfig::GetInstance()->GetProfileName() + "/" + filename;
	Console::Message("Loading data file: " + name);

	ProgressWindow* pw = ProgressWindow::GetInstance();

	pw->SetItemName(QString("Loading: %1").arg(name));
	pw->SetItemProgress(0);

	myDataLength = Utils::CountFileLines(name) - 1;

	// Allocate data.
	myData = new DataItem[myDataLength];

	myFilteredData = new DataItem*[myDataLength];
	myFilteredDataLength = 0;

	mySelectedData = new DataItem*[myDataLength];
	mySelectedDataLength = 0;

	QFile* file = RepositoryManager::GetInstance()->TryOpen(name);
	QTextStream dataFile(file);

	QString line = dataFile.readLine();
	line = dataFile.readLine();
	int curIdx = 0;
	int k = 0;
	while(!line.isNull())
	{
		QStringList tokens = line.split(',');

		// Parse date, time.
		int tid = myInfo->GetTimestampTimeIndex();
		int did = myInfo->GetTimestampDateIndex();

		CheckTokenIndex(tokens, tid, k, "TimestampTimeIndex");
		CheckTokenIndex(tokens, did, k, "TimestampDateIndex");

		QString dtString = tokens[did] + " " + tokens[tid];
		QDateTime dtm = QDateTime::fromString(dtString, myInfo->GetTimestampStringFormat());

		int m = dtm.date().month();
		int d = dtm.date().day();
		int y = dtm.date().year();

		if(!dtm.isValid())
		{
			Console::Warning(QString("Invalid date and time at line %1: %2").arg(k + 2).arg(dtString));
		}
		else
		{
			myData[curIdx].Timestamp = dtm.toTime_t();
			if(myData[curIdx].Timestamp < myTimestampRange[0]) myTimestampRange[0] = myData[curIdx].Timestamp;
			if(myData[curIdx].Timestamp > myTimestampRange[1]) myTimestampRange[1] = myData[curIdx].Timestamp;
		}

		// Read tags if present in specification
		if(myInfo->GetTag1Index() != -1)
		{
			CheckTokenIndex(tokens, myInfo->GetTag1Index(), k, "Tag1Index");
			strcpy(myData[curIdx].Tag1, tokens[myInfo->GetTag1Index()].ascii());

			if(!myTag1List.contains(myData[curIdx].Tag1)) myTag1List.insert(myData[curIdx].Tag1, 1);
			//else myTag1List[myData[curIdx].Tag1] = myTag1List[myData[curIdx].Tag1] = 1;
		}
		if(myInfo->GetTag2Index() != -1)
		{
			CheckTokenIndex(tokens, myInfo->GetTag2Index(), k, "Tag2Index");
			strcpy(myData[curIdx].Tag2, tokens[myInfo->GetTag2Index()].ascii());

			if(!myTag2List.contains(myData[curIdx].Tag2)) myTag2List.insert(myData[curIdx].Tag2, 1);
			//else myTag2List[myData[curIdx].Tag2] = myTag1List[myData[curIdx].Tag2] = 1;
		}
		if(myInfo->GetTag3Index() != -1)
		{
			CheckTokenIndex(tokens, myInfo->GetTag3Index(), k, "Tag3Index");
			strcpy(myData[curIdx].Tag3, tokens[myInfo->GetTag3Index()].ascii());

			if(!myTag3List.contains(myData[curIdx].Tag3)) myTag3List.insert(myData[curIdx].Tag3, 1);
			//else myTag3List[myData[curIdx].Tag3] = myTag3List[myData[curIdx].Tag3] = 1;
		}
		if(myInfo->GetTag4Index() != -1)
		{
			CheckTokenIndex(tokens, myInfo->GetTag4Index(), k, "Tag4Index");
			strcpy(myData[curIdx].Tag4, tokens[myInfo->GetTag4Index()].ascii());

			if(!myTag4List.contains(myData[curIdx].Tag4)) myTag4List.insert(myData[curIdx].Tag4, 1);
			//else myTag3List[myData[curIdx].Tag3] = myTag3List[myData[curIdx].Tag3] = 1;
		}

		for(int i = 0; myInfo->GetField(i) != NULL; i++)
		{
			FieldInfo* fi = myInfo->GetField(i);
			if(fi->GetType() == FieldInfo::Data)
			{
				// Parse and store the field value.
				int fid = fi->GetFieldIndex();
				CheckTokenIndex(tokens, fid, k, fi->GetName());
				float value = tokens[fid].toFloat();
				myData[curIdx].Field[i] = value;
			}
		}
		curIdx++;
		line = dataFile.readLine();
		k++;
		if(k % 100 == 0) pw->SetItemProgress(k * 100 / myDataLength);
	}

	myDataLength = curIdx;

	pw->Done();

	file->close();
	delete file;
	file = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DataSet::InitSondeBathyData()
{
	mySondeBathyVtkData = vtkUnstructuredGrid::New();

	vtkPoints* pts = vtkPoints::New();
	
	int curPt = 0;
	int curX = myData[0].X;
	int curY = myData[0].Y;
	for(int i = 0; i < myDataLength; i++)
	{
		if(myData[i].Y != curY || myData[i].X != curX)
		{
			curX =  myData[i].X;
			curY =  myData[i].Y;
			curPt++;
		}
	}
	pts->SetNumberOfPoints(curPt);

	mySondeBathyVtkData->SetPoints(pts);

	vtkFloatArray* fields; 
	fields = vtkFloatArray::New();
	fields->Allocate(curPt);

	fields->SetName("Z");
	mySondeBathyVtkData->GetPointData()->AddArray(fields);

	curX = myData[0].X;
	curY = myData[0].Y;
	float maxZ = 0;
	curPt = 0;
	for(int i = 0; i < myDataLength; i++)
	{
		if(myData[i].Y == curY && myData[i].X == curX)
		{
			if(maxZ < myData[i].Z)
			{
				maxZ = myData[i].Z;
			}
		}
		else
		{
			// Insert a new point (x and y coordinates have to be switched...
			// TODO: fix transforms to avoid this and keep things more consistent).
			pts->SetPoint(curPt, curY, maxZ - mySondeBathyDepthCorrection, curX);
			// Add field values.
			fields->InsertValue(curPt, maxZ - mySondeBathyDepthCorrection);
			//printf("Added point: %f\n", maxZ - mySondeBathyDepthCorrection);
			maxZ = 0;
			curX =  myData[i].X;
			curY =  myData[i].Y;
			curPt++;
		}
	}

	fields->Delete();
	pts->Delete();

	//double* range = myVtkData->GetBounds();
	//printf("Sonde Range: %f-%f %f-%f %f-%f\n", range[0], range[1], range[2], range[3], range[4], range[5]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float* DataSet::GetZRange()
{
	int zFieldId = myInfo->GetFieldIndex(myInfo->GetZFieldName());
	return myFieldRange[zFieldId];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float* DataSet::GetYRange()
{
	int yFieldId = myInfo->GetFieldIndex(myInfo->GetYFieldName());
	return myFieldRange[yFieldId];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float* DataSet::GetXRange()
{
	int xFieldId = myInfo->GetFieldIndex(myInfo->GetXFieldName());
	return myFieldRange[xFieldId];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DataSet::SaveAsCSV(const QString& fileName)
{
	FILE* file = fopen(fileName, "w");
	QTextStream out(file);

	QString data;

	// Write headers
	data += QString::fromStdString(myInfo->GetTag1Label());
	data += ",";

	for(int j = 0; j < myInfo->GetNumFields(); j++)
	{
		if(myInfo->GetField(j)->IsEnabled())
		{
			data += myInfo->GetField(j)->GetLabel() + ",";
		}
	}

	data += "\n";
	out << data;

	// Write data.
	SubsetType sst = DataSet::FilteredData;
	if(GetDataLength(DataSet::SelectedData) > 0) sst = DataSet::SelectedData;

	for(int i = 0; i < GetDataLength(sst); i++)
	{
		data = "";
		DataItem* item = GetData(i, sst);

		data += item->Tag1;
		data += ",";

		for(int j = 0; j < myInfo->GetNumFields(); j++)
		{
			if(myInfo->GetField(j)->IsEnabled())
			{
				data += QString("%1").arg(item->Field[j]) + ",";
			}
		}
		data += "\n";
		out << data;
	}

	out.flush();
	fclose(file);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DataSet::AddFilter(DynamicFilter* filter)
{
	myFilters.push_back(filter);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DataSet::RemoveFilter(DynamicFilter* filter)
{
	myFilters.remove(filter);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DataSet::ApplyFilters()
{
	// Do we need this?
	//UpdateFilteredDataLength();

	myFilteredDataLength = 0;

	// Clean up the filtered data array
	memset(myFilteredData, 0, sizeof(DataItem*) * myDataLength);

	int c = 0;
	for(int i = 0; i < myDataLength; i++)
	{
		if(ItemFilterPass(i))
		{
			myFilteredData[c] = &myData[i];
			c++;
		}
	}

	myFilteredDataLength = c;

	Preferences* pref = AppConfig::GetInstance()->GetPreferences();
	UpdateGroups(pref->GetGroupingTagId(), pref->GetGroupingSubset());

	VtkDataManager::GetInstance()->Update(DataSet::FilteredData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool DataSet::ItemFilterPass(int index)
{
	for(int i = 0; i < myFilters.length(); i++)
	{
		DynamicFilter* f = myFilters[i];
		if(f->Enabled)
		{
			if(f->Type == DynamicFilter::FieldFilter)
			{
				float value = myData[index].Field[f->FieldId];
				if(value < f->Min || value > f->Max) return false;
			}
			else if(f->Type == DynamicFilter::TimeFilter)
			{
				time_t value = myData[index].Timestamp;
				if(value < f->TimeMin || value > f->TimeMax) return false;
			}
			else
			{
				Utils::SetEvalVariables(myData[index], myInfo);
				QString expr = QString("_r = %1").arg(f->Expression);
				float result = Utils::Eval(expr);
				if (result == 0) return false;
			}
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DataSet::CheckTokenIndex(const QStringList& tokens, int index, int line, const QString& fieldName)
{
	if(index < 0 || index >= tokens.length())
	{
		Console::Error(QString("Invalid field index while loading dataset - Field name: %1 index: %2 data line: %3").arg(fieldName).arg(index).arg(line));
		ShutdownApp(true, false);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DataItem* DataSet::FindDataItem(float x, float y, float z)
{
	for(int i = 0; i < myFilteredDataLength; i++)
	{
		if(myFilteredData[i]->X == x &&
			myFilteredData[i]->Y == y &&
			myFilteredData[i]->Z == z) return myFilteredData[i];
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QPair<float, float> DataSet::ComputeGroupRange(int fieldId, DynamicFilter::FilterGrouping grouping, const QString& tag)
{
	if(grouping == DynamicFilter::Global) 
	{
		return QPair<float, float>(myFieldRange[fieldId][0], myFieldRange[fieldId][1]);
	}
	float min = FLT_MAX;
	float max = FLT_MIN;
	for(int i = 0; i < myDataLength; i++)
	{
		if(myData[i].Tag1 == tag)
		{
			float value = myData[i].Field[fieldId];
			if(value < min) min = value;
			if(value > max) max = value;
		}
	}
	return QPair<float, float>(min, max);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int DataSet::CountTagGroups(DataSetInfo::TagId tagId)
{
	if(tagId == DataSetInfo::Tag1) return myTag1List.count();
	if(tagId == DataSetInfo::Tag2) return myTag2List.count();
	if(tagId == DataSetInfo::Tag3) return myTag3List.count();
	else return myTag4List.count();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString DataSet::GetTag(DataSetInfo::TagId tagId, int index)
{
	if(tagId == DataSetInfo::Tag1) return myTag1List.keys()[index];
	else if(tagId == DataSetInfo::Tag2) return myTag2List.keys()[index];
	else if(tagId == DataSetInfo::Tag3) return myTag3List.keys()[index];
	else return myTag4List.keys()[index];
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DataSet::SelectByTag(QString tag, DataSetInfo::TagId tagId, SubsetType subset, SelectMode mode)
{
	int i = 0;
	DataItem* item = NULL;
	while(true)
	{
		// Get next item.
		item = GetData(i, subset);
		if(!item) break;

		// Get tag
		QString itag = item->GetTag(tagId);

		if(itag == tag)
		{
			if(mode == DataSet::SelectionAdd || mode == DataSet::SelectionNew) 
			{
				item->Flags |= DataItem::Selected;
			}
			else if(mode == DataSet::SelectionToggle)
			{
				if((item->Flags & DataItem::Selected) == DataItem::Selected)
				{
					item->Flags &= ~DataItem::Selected;
				}
				else
				{
					item->Flags |= DataItem::Selected;
				}
			}
		}
		else if(mode == DataSet::SelectionNew)
		{
			item->Flags &= ~DataItem::Selected;
		}

		i++;
	}

	mySelectedDataLength = UpdateSubset(mySelectedData, DataItem::Selected);

	VtkDataManager::GetInstance()->Update(DataSet::SelectedData);

	Preferences* pref = AppConfig::GetInstance()->GetPreferences();
	UpdateGroups(pref->GetGroupingTagId(), pref->GetGroupingSubset());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DataSet::ClearSelection()
{
	DataItem* item = NULL;
	int i = 0;
	while(true)
	{
		// Get next item.
		item = GetData(i, DataSet::AllData);
		if(!item) break;
		item->Flags &= ~DataItem::Selected;
		i++;
	}

	mySelectedDataLength = UpdateSubset(mySelectedData, DataItem::Selected);

	VtkDataManager::GetInstance()->Update(DataSet::SelectedData);

	Preferences* pref = AppConfig::GetInstance()->GetPreferences();
	UpdateGroups(pref->GetGroupingTagId(), pref->GetGroupingSubset());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DataSet::InitGroups()
{
	for(int i = 0; i < MAX_GROUPS; i++)
	{
		myGroups[i].Items = new DataItem*[myDataLength];
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
DataGroup* DataSet::FindGroup(const QString& tag)
{
	for(int i = 0; i < myNumGroups; i++)
	{
		if(myGroups[i].Tag == tag) return &myGroups[i];
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DataSet::UpdateGroups(DataSetInfo::TagId tagId, SubsetType subset)
{
	myNumGroups = CountTagGroups(tagId);
	if(myNumGroups > MAX_GROUPS)
	{
		Console::Error(QString("DataSet::UpdateGroups Too may groups! required: %1, supported: %2").arg(myNumGroups).arg(MAX_GROUPS));
		ShutdownApp(true, true);
	}

	// Create groups
	for(int i = 0; i < myNumGroups; i++)
	{
		myGroups[i].Size = 0;
		myGroups[i].Tag = GetTag(tagId, i);
	}

    int i = 0;
	bool done = false;
	DataItem* item = NULL;
	while(true)
	{
		// Get next item.
		item = GetData(i, subset);
		if(!item) break;

		
		// Get tag
		QString tag = item->GetTag(tagId);

		// Get data group
		DataGroup* grp = FindGroup(tag);
		if(!grp)
		{
			Console::Error("DataSet::UpdateGroups: Invalid group tag: " + tag);
			ShutdownApp(true);
		}

		// Insert element.
		grp->Items[grp->Size] = item;
		grp->Size++;

		i++;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int DataSet::UpdateSubset(DataItem** subset, DataItem::ItemFlags flag)
{
	int size = 0;
	int i = 0;
	DataItem* item = NULL;
	while(true)
	{
		// Get next item.
		item = GetData(i, DataSet::AllData);
		if(!item) break;

		if((item->Flags & flag) == flag)
		{
			subset[size] = item;
			size++;
		}
		i++;
	}
	return size;
}

