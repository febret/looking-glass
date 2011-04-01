/********************************************************************************************************************** 
 * The LookingGlass Project
 *
 * Author: 
 *	Alessandro Febretti
 *********************************************************************************************************************/ 
#ifndef UTILS_H
#define UTILS_H
#include "LookingGlassSystem.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Utils
{
public:
	static int CountFileLines(const QString& filename);
	static void SetEvalVariables(const DataItem& data, DataSetInfo* info);
	static double Eval(const QString& expr);
	static void SaveScreenshot(vtkRenderWindow* win);
	static void  UpdateColorTransferFunction(vtkColorTransferFunction* colorTrans, pqColorMapModel* cmodel);

private:
	Utils() {}
};

#endif