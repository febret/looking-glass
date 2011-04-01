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
#ifndef DATASET_H
#define DATASET_H

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "LookingGlassSystem.h"
#include "DataSetInfo.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Length of tag text.
#define TAG_LEN 32

// Maximum number of data groups.
#define MAX_GROUPS 1024

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Represents a single dataset item. 
struct DataItem
{
	enum ItemFlags 
	{
		None = 0,
		Filtered = 1,
		Selected = 1 << 2
	};

	// Constructor: set default values for some of the fields.
	DataItem()
	{
		Flags = None;
		Timestamp = 0;
	}

	char* GetTag(DataSetInfo::TagId id)
	{
		if(id == DataSetInfo::Tag1) return Tag1;
		else if(id == DataSetInfo::Tag2) return Tag2;
		else if(id == DataSetInfo::Tag3) return Tag3;
		return Tag4;
	}

	float X;
	float Y;
	float Z;
	time_t Timestamp;
	float Field[DataSetInfo::MAX_FIELDS];
	char Tag1[TAG_LEN];
	char Tag2[TAG_LEN];
	char Tag3[TAG_LEN];
	char Tag4[TAG_LEN];
	unsigned int Flags; 
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Represents a data group. 
struct DataGroup
{
	int Size;
	QString Tag;
	DataItem** Items;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct DynamicFilter
{
	enum FilterType 
	{ 
		ExpressionFilter = 1 << 1, 
		RangeCutoffFilter = 1 << 2,
		TimeFilter = 1 << 3,
		FieldFilter = 1 << 4 
	};

	// Grouping is used for filters that compute aggregate values (i.e. min, max values) to decide wether the
	// aggregation is global on the dataset or on subsets of the data identified by a tag.
	enum FilterGrouping
	{
		Global,
		Tag1
	};

	bool Enabled;
	FilterType Type;
	FilterGrouping Grouping;
	QString Expression;
	int FieldId;
	float Min;
	float Max;
	time_t TimeMin;
	time_t TimeMax;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class DataSet
{
public:
	// Data subset.
	// TODO: use this and unify methods to access data length and data.
	enum SubsetType { AllData, FilteredData, SelectedData };
	enum SelectMode { SelectionNew, SelectionAdd, SelectionToggle };

public:
	static const int SLICE_SEPARATION = 128; 

public:
	// Ctor / Dtor.
	DataSet();
	~DataSet();

	// Other methods.
	void Initialize();

	// Tag data access.
	int CountTagGroups(DataSetInfo::TagId tagId);
	QString GetTag(DataSetInfo::TagId tagId, int index);

	// Field information
	QString GetFieldName(int index);
	float* GetFieldRange(int index);

	// Filter management
	void AddFilter(DynamicFilter* filter);
	void RemoveFilter(DynamicFilter* filter);
	void ApplyFilters();

	// Data selection
	void SelectByTag(QString tag, DataSetInfo::TagId tagId, SubsetType subset, SelectMode mode);
	void ClearSelection();

	// Convenience methods for retrieving the range of X, Y, and Z fields.
	float* GetZRange();
	float* GetYRange();
	float* GetXRange();
	time_t* GetTimestampRange() { return myTimestampRange; }
	
	// Access grouped ranges.
	QPair<float, float> ComputeGroupRange(int fieldId, DynamicFilter::FilterGrouping grouping, const QString& tag = QString());

	// Sonde bathymetry (This should not be part of the dataset...)
	vtkPointSet* GetSondeBathyData();

	// Field update
	void UpdateField(int index);
	bool FilterPass(int index, int dataIdx);

	// Data item access.
	DataItem* FindDataItem(float x, float y, float z);

	// Access data.
	DataItem* GetData(int index, DataSet::SubsetType subset);
	int GetDataLength(DataSet::SubsetType subset);

	// Gets or Sets the depth correction used for sonde-based bathymetry model generation.
	void SetSondeBathyDepthCorrection(float value);
	float GetSondeBathyDepthCorrection();

	// Groups
	int GetNumGroups() { return myNumGroups; }
	DataGroup* GetGroup(int index) { return &myGroups[index]; } 
	DataGroup* FindGroup(const QString& tag);
	void UpdateGroups(DataSetInfo::TagId tagId, SubsetType subset);

	// Dataset info.
	DataSetInfo* GetInfo() { return myInfo; }

	// Save dataset.
	void SaveAsCSV(const QString& fileName);

public:
	//static const int InvalidValue = -999;

private:
	//enum LoadStep { Invalid, SondeData };

private:
	void Load();
	void LoadFile(const QString& name);
	void InitSondeBathyData();
	bool ItemFilterPass(int index);
	void InitGroups();
	void CheckTokenIndex(const QStringList& tokens, int index, int line, const QString& fieldName);

	int UpdateSubset(DataItem** subset, DataItem::ItemFlags flag);

private:
	// DatatSet info object.
	DataSetInfo* myInfo;

	QList<DynamicFilter*> myFilters;

	// Data ranges.
	time_t myTimestampRange[2];
	float myFieldRange[DataSetInfo::MAX_FIELDS][2];

	// Data.
	DataItem* myData;
	int myDataLength;

	// Filtered Data.
	DataItem** myFilteredData;
	int myFilteredDataLength;

	// Selected Data.
	DataItem** mySelectedData;
	int mySelectedDataLength;

	// Sonde Batymetry.
	float mySondeBathyDepthCorrection;
	vtkUnstructuredGrid* mySondeBathyVtkData;

	// Tag lists.
	QHash<QString, int> myTag1List;
	QHash<QString, int> myTag2List;
	QHash<QString, int> myTag3List;
	QHash<QString, int> myTag4List;

	// Data Groups
	int myNumGroups;
	DataGroup myGroups[MAX_GROUPS];
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline vtkPointSet* DataSet::GetSondeBathyData()
{
	return mySondeBathyVtkData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DataSet::SetSondeBathyDepthCorrection(float value)
{
	mySondeBathyDepthCorrection = value;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float DataSet::GetSondeBathyDepthCorrection()
{
	return mySondeBathyDepthCorrection;
}
#endif 
