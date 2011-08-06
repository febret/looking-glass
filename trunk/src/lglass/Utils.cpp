/**************************************************************************************************
 * THE LOOKING GLASS VISUALIZATION TOOLSET
 *-------------------------------------------------------------------------------------------------
 * Author: 
 *	Alessandro Febretti		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Contact & Web:
 *  febret@gmail.com		http://febretpository.hopto.org
 *-------------------------------------------------------------------------------------------------
 * Looking Glass has been built as part of the ENDURANCE Project (http://www.evl.uic.edu/endurance/).
 * ENDURANCE is supported by the NASA ASTEP program under Grant NNX07AM88G and by the NSF USAP.
 *-------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2011, Electronic Visualization Laboratory, University of Illinois at Chicago
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, are permitted 
 * provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this list of conditions 
 * and the following disclaimer. Redistributions in binary form must reproduce the above copyright 
 * notice, this list of conditions and the following disclaimer in the documentation and/or other 
 * materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF 
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *************************************************************************************************/ 
#include "Utils.h"
#include "RepositoryManager.h"
#include "DataSet.h"

#include "pqChartValue.h"

#include <QTextStream>
#include <QFileDialog>

#include <vtkColorTransferFunction.h>
#include <vtkRenderWindow.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>

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

///////////////////////////////////////////////////////////////////////////////////////////////////
void Utils::SerializeColor(const QColor& color, Setting& s, const QString& name)
{
	if(s.exists(name)) s.remove(name);
	Setting& sc = s.add(name, Setting::TypeArray);
	sc.add(Setting::TypeFloat) = (float)color.red() / 255;
	sc.add(Setting::TypeFloat) = (float)color.green() / 255;
	sc.add(Setting::TypeFloat) = (float)color.blue() / 255;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QColor Utils::DeserializeColor(Setting& s, const QString& name)
{
	if(s.exists(name))
	{
		Setting& sc = s[name.ascii()];
		int r = (int)((float)sc[0] * 255);
		int g = (int)((float)sc[1] * 255);
		int b = (int)((float)sc[2] * 255);
		return QColor(r, g, b);
	}
	return QColor(0, 0, 0);
}