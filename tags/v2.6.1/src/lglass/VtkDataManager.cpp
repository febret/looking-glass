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
#include "VtkDataManager.h"
#include "DataSet.h"
#include "DataSetInfo.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VtkDataManager* VtkDataManager::myInstance = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VtkDataManager::Initialize(DataSet* dataSet)
{
	myInstance = new VtkDataManager(dataSet);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VtkDataManager::VtkDataManager(DataSet* dataSet)
{
	myDataSet = dataSet;

	myVtkData = vtkUnstructuredGrid::New();
	mySelectedVtkData = vtkUnstructuredGrid::New();
	myFilteredVtkData = vtkUnstructuredGrid::New();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VtkDataManager::Update(DataSet::SubsetType subset)
{
	DataSetInfo* info = myDataSet->GetInfo();

	vtkPointSet* pset = GetPointSet(subset);
	pset->Modified();

	int l = myDataSet->GetDataLength(subset);

	vtkPoints* pts = vtkPoints::New();
	pts->SetNumberOfPoints(l);
	pset->SetPoints(pts);

	vtkFloatArray* fields[DataSetInfo::MAX_FIELDS]; 
	for(int i = 0; i < info->GetNumFields(); i++)
	{
		fields[i] = vtkFloatArray::New();
		fields[i]->Allocate(l);
		fields[i]->SetName(myDataSet->GetFieldName(i));
		pset->GetPointData()->AddArray(fields[i]);
	}

	for(int i = 0; i < l; i++)
	{
		DataItem* d = myDataSet->GetData(i, subset);
		pts->SetPoint(i, d->Y, d->Z, d->X);

		for(int j = 0; j < info->GetNumFields(); j++)
		{
			fields[j]->InsertValue(i, d->Field[j]);
		}
	}

	//pset->GetPointData()->SetActiveScalars(GetFieldName(0));

	// Delete temporary objects.
	for(int i = 0; i < info->GetNumFields(); i++) fields[i]->Delete();
	pts->Delete();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vtkPointSet* VtkDataManager::GetPointSet(DataSet::SubsetType subset)
{
	if(subset == DataSet::AllData) return myVtkData;
	else if(subset == DataSet::FilteredData) return myFilteredVtkData;
	return mySelectedVtkData;
}
