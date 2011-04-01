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
#ifndef VTKDATAMANAGER_H
#define VTKDATAMANAGER_H

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "LookingGlassSystem.h"
#include "DataSet.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class VtkDataManager
{
public:
	// Singleton stuff.
	static void Initialize(DataSet* dataSet);
	static VtkDataManager* GetInstance() { return myInstance; }

	void Update(DataSet::SubsetType subset);
	vtkPointSet* GetPointSet(DataSet::SubsetType subset);

private:
	VtkDataManager(DataSet* dataSet);

	// Singleton instance.
	static VtkDataManager* myInstance;

	DataSet* myDataSet;

	// Data.
	vtkUnstructuredGrid* myVtkData;
	vtkUnstructuredGrid* myFilteredVtkData;
	vtkUnstructuredGrid* mySelectedVtkData;
};

#endif