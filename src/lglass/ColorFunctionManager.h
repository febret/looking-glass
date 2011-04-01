///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef COLORFUNCTIONMANAGER_H
#define COLORFUNCTIONMANAGER_H

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "LookingGlassSystem.h"
#include "DockedTool.h"
#include "ui_ColorFunctionManagerDock.h"
#include "DataSet.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
class ColorFunctionManager: public DockedTool
{
	Q_OBJECT
public:
    /////////////////////////////////////////////////////// Ctor / Dtor.
	ColorFunctionManager(VisualizationManager* mng);
    virtual ~ColorFunctionManager();

    void Initialize();
	vtkColorTransferFunction* GetColorFunction(int index, bool noInvalid = false);

signals:
	// Raised when the color transfer function identified by the specified
	// index has changed.
	//void ColorFunctionChanged(int index, vtkColorTransferFunction*);

protected slots:
	void OnColorTransferPointChange(int);
	void OnColorTransferPointMove(int);

private:
	void UpdateModel(int index);
	void UpdateColorFunction(int index);
    void SetupUI();

private:
	Ui::ColorFunctionManagerDock* myUI;
	VisualizationManager* myVizMng;
	QLabel* myFieldLabel[DataSetInfo::MAX_FIELDS];
	pqColorMapWidget* myWidget[DataSetInfo::MAX_FIELDS];
	pqColorMapModel* myModel[DataSetInfo::MAX_FIELDS];
	vtkColorTransferFunction* myFunc[DataSetInfo::MAX_FIELDS];
	vtkColorTransferFunction* myNoInvalidFunc[DataSetInfo::MAX_FIELDS];
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline vtkColorTransferFunction* ColorFunctionManager::GetColorFunction(int index, bool noInvalid)
{
	if(noInvalid)
	{
		return myNoInvalidFunc[index];
	}
	return myFunc[index];
}

#endif 
