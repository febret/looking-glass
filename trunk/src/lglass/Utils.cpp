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
#include "Utils.h"
#include "RepositoryManager.h"
#include "DataSet.h"

extern "C"
{
#include "eval/evaldefs.h"
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int Utils::CountFileLines(const QString& filename)
{
	// Count the number of items.
	QFile* file = RepositoryManager::GetInstance()->TryOpen(filename);
	QTextStream stream(file);

	QString text = stream.readAll();
	int l = text.count('\n');

	file->close();
	delete file;
	file = NULL;

	return l;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Utils::SetEvalVariables(const DataItem& data, DataSetInfo* info)
{
	int i = 0;
	FieldInfo* fi;
	while((fi = info->GetField(i)))
	{
		double* vp = locateVariableByName((char*)fi->GetName().ascii());
		*vp = data.Field[i];
		//if(optimized)
		//{
		//	if(!fi->GetEvalVariable()) 
		//	{ 
		//		double* vp = locateVariableByName((char*)fi->GetName().ascii());
		//		fi->SetEvalVariable(vp); 
		//	}
		//	*fi->GetEvalVariable() = data.Field[i];
		//}
		//else
		//{
		//	QString expr = QString("%1 = %2;").arg(fi->GetName()).arg(data.Field[i]);
		//	evaluateExpression((char*)expr.ascii());
		//}
		i++;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double Utils::Eval(const QString& expr)
{
	evaluateExpression((char*)expr.ascii());

	pushChar('_');
	pushChar('r');

	double* v = locateVariable(2);

	return *v;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Utils::SaveScreenshot(vtkRenderWindow* win)
{
	QString fileName = QFileDialog::getSaveFileName("./", "*.png");
	if(!fileName.isNull())
	{
		vtkWindowToImageFilter* exporter = vtkWindowToImageFilter::New();
		vtkPNGWriter* writer = vtkPNGWriter::New();

		exporter->SetInput(win);
		writer->SetInputConnection(exporter->GetOutputPort());
		writer->SetFileName(fileName);
		writer->Write();

		exporter->Delete();
		writer->Delete();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Utils::UpdateColorTransferFunction(vtkColorTransferFunction* colorTrans, pqColorMapModel* cmodel)
{
	colorTrans->RemoveAllPoints(); 

	for(int i = 0; i < cmodel->getNumberOfPoints(); i++)
	{
		QColor color;
		pqChartValue val;
		cmodel->getPointValue(i, val);
		cmodel->getPointColor(i, color);
		double actualValue = val.getDoubleValue();
		colorTrans->AddRGBPoint(actualValue, color.redF(), color.greenF(), color.blueF());
	}
}