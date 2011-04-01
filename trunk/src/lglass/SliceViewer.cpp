///////////////////////////////////////////////////////////////////////////////////////////////////
#include "SliceViewer.h"
#include "DataSet.h"
#include "GeoDataView.h"
#include "ColorFunctionManager.h"
#include "VisualizationManager.h"
#include "VtkDataManager.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
Slice::Slice(SliceViewer* owner)
{
	myOwner = owner;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Slice::~Slice()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Slice::Initialize()
{
	float c = 0.05;
	myRenderer = vtkRenderer::New();
	myRenderer->SetBackground(c, c, c);

    myPlane = vtkPlane::New();

	myPlaneProbe = vtkCutter::New();
	myPlaneProbe->SetCutFunction(myPlane);

	// Setup the probe mapper and actor.
    myProbeMapper = vtkDataSetMapper::New();
    myProbeMapper->SetInput(myPlaneProbe->GetOutput());
	myProbeMapper->ScalarVisibilityOn();

    myProbeActor = vtkActor::New();
    myProbeActor->SetMapper(myProbeMapper);
    myProbeActor->GetProperty()->BackfaceCullingOff();
    myProbeActor->GetProperty()->FrontfaceCullingOff();
	myProbeActor->RotateX(90);
	myProbeActor->SetScale(1, 1.0f / DataSet::SLICE_SEPARATION, 1);
	myProbeActor->GetProperty()->SetLighting(0);

	myBathyActor = vtkActor::New();
	myBathyActor->SetMapper(myOwner->GetBathyContourMapper());
    myBathyActor->GetProperty()->BackfaceCullingOff();
    myBathyActor->GetProperty()->FrontfaceCullingOff();
    myBathyActor->GetProperty()->SetAmbientColor(0.01, 0.01, 0.01);
	myBathyActor->GetProperty()->SetAmbient(1);
	myBathyActor->RotateX(90);
	myBathyActor->GetProperty()->SetLighting(0);

	myCaption = vtkTextActor::New();
	myCaption->SetTextScaleMode(vtkTextActor::TEXT_SCALE_MODE_VIEWPORT);
	myCaption->SetPosition(10, 10);
	myCaption->GetTextProperty()->SetFontSize(14);

	myContours = vtkContourFilter::New();
	myContours->SetInput(myPlaneProbe->GetOutput());

	myContourMapper = vtkPolyDataMapper::New();
	myContourMapper->SetInput(myContours->GetOutput());
	myContourMapper->SetScalarVisibility(0);

	myContourActor = vtkActor::New();
	myContourActor->SetMapper(myContourMapper);
    myContourActor->GetProperty()->BackfaceCullingOff();
    myContourActor->GetProperty()->FrontfaceCullingOff();
	myContourActor->SetPosition(0, 0, 1);
	myContourActor->RotateX(90);
	myContourActor->SetScale(1, 1.0f / DataSet::SLICE_SEPARATION, 1);
	myContourActor->GetProperty()->SetLighting(0);

	myRenderer->AddActor(myProbeActor);
	myRenderer->AddActor(myBathyActor);
	myRenderer->AddActor(myContourActor);

	myRenderer->AddActor2D(myCaption);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Slice::SetInput(vtkDataObject* input)
{
	myPlaneProbe->SetInput(input);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Slice::SetDepth(float depth)
{
	DataSet* data = myOwner->GetVisualizationManager()->GetDataSet();
	float* rng = data->GetZRange();
	sprintf(myCaptionText, "Depth: %f meters", depth);
	myCaption->SetInput(myCaptionText);
	myPlane->SetOrigin(0, -depth * DataSet::SLICE_SEPARATION, 0);
	myPlane->SetNormal(0, 1, 0);
	myPlaneProbe->Modified();
	myProbeMapper->Update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Slice::Update()
{
	int field = myOwner->GetSelectedField();
	VisualizationManager* mng = myOwner->GetVisualizationManager();

	float* range = mng->GetDataSet()->GetFieldRange(field);
	myContours->GenerateValues(myOwner->GetNumContours(), range[0], range[1]); 
	myContours->Update();
	if(myOwner->GetContoursEnabled())
	{
		myContourActor->VisibilityOn();
	}
	else
	{
		myContourActor->VisibilityOff();
	}

	myProbeMapper->SetLookupTable(mng->GetColorFunctionManager()->GetColorFunction(field));
	myProbeMapper->Update();

	if(myRenderer->GetRenderWindow() != NULL)
	{
		myRenderer->Render();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
SliceViewer::SliceViewer(VisualizationManager* mng)
{
	myContoursEnabled = true;
	myNumContours = 1;
	mySelectedField = 0;

	myVizMng = mng;
	myMenuAction = myVizMng->AddWindowMenuAction("Slice Viewer");
	myMenuAction->setCheckable(false);
	myMenuAction->setIcon(QIcon(":/icons/SliceView.png"));
	QObject::connect(myMenuAction, SIGNAL(triggered()), this, SLOT(Show()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
SliceViewer::~SliceViewer()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SliceViewer::SetupUI()
{
	myUI = new Ui::SliceViewerWindow();
	myUI->setupUi(this);

    // Setup the components box.
	int numFields = myVizMng->GetDataSet()->GetInfo()->GetNumFields();
	for(int i = 0; i < numFields; i++)
    {
        myUI->componentBox->addItem(myVizMng->GetDataSet()->GetFieldName(i), i);
    }

	connect(myUI->startSlider, SIGNAL(valueChanged(int)), SLOT(SetStartDepth(int)));
	connect(myUI->endSlider, SIGNAL(valueChanged(int)), SLOT(SetEndDepth(int)));
	connect(myUI->contoursSlider, SIGNAL(valueChanged(int)), SLOT(SetNumContours(int)));
	connect(myUI->contoursButton, SIGNAL(toggled(bool)), SLOT(OnContoursButtonToggle(bool)));
	connect(myUI->componentBox, SIGNAL(currentIndexChanged(int)), SLOT(OnSelectedFieldChanged(int)));
	connect(myUI->slice2, SIGNAL(toggled(bool)), SLOT(OnSliceButtonToggle(bool)));
	connect(myUI->slice4, SIGNAL(toggled(bool)), SLOT(OnSliceButtonToggle(bool)));
	connect(myUI->slice6, SIGNAL(toggled(bool)), SLOT(OnSliceButtonToggle(bool)));
	connect(myUI->slice9, SIGNAL(toggled(bool)), SLOT(OnSliceButtonToggle(bool)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SliceViewer::Show()
{
	this->show();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SliceViewer::Initialize()
{
	vtkPointSet* grid = VtkDataManager::GetInstance()->GetPointSet(DataSet::AllData);

	vtkTransform* transform = vtkTransform::New();
	transform->Scale(1, DataSet::SLICE_SEPARATION, 1);

	mySeparationTransform = vtkTransformFilter::New();
	mySeparationTransform->SetTransform(transform);
	mySeparationTransform->SetInput(grid);
	mySeparationTransform->Update();

    mySondeVolumeBuilder = vtkShepardMethod::New();
	mySondeVolumeBuilder->SetInput(mySeparationTransform->GetOutput());
	mySondeVolumeBuilder->SetModelBounds(mySeparationTransform->GetOutput()->GetBounds());
    mySondeVolumeBuilder->SetSampleDimensions(60, 50, 60);
	mySondeVolumeBuilder->SetMaximumDistance(3.0f / DataSet::SLICE_SEPARATION);
    mySondeVolumeBuilder->SetNullValue(999.0f);
	mySondeVolumeBuilder->Update();

    vtkThreshold* invalidPointThreshold = vtkThreshold::New();
	invalidPointThreshold->SetInput(mySondeVolumeBuilder->GetOutput());
    invalidPointThreshold->ThresholdByLower(990);
	invalidPointThreshold->Update();

	myBathyContour = vtkContourFilter::New();
	myBathyContourMapper = vtkPolyDataMapper::New();

	vtkPolyData* bathyData = myVizMng->GetGeoDataVew()->GetOldBathyReader()->GetOutput();

	myBathyContour->SetInput(bathyData);
	myBathyContour->GenerateValues(5, bathyData->GetScalarRange());
	myBathyContourMapper->SetInput(myBathyContour->GetOutput());
	myBathyContourMapper->SetScalarVisibility(0);
	myBathyContourMapper->Update();

	for(int i = 0; i < MAX_SLICES; i++)
	{
		mySlices[i] = new Slice(this);
		mySlices[i]->Initialize();
		mySlices[i]->SetDepth(10);
		mySlices[i]->SetInput(invalidPointThreshold->GetOutput());
	}

	// Setup the scalar color bar.
    myScalarBar = vtkScalarBarActor::New();
    myScalarBar->GetPositionCoordinate()->SetValue(0, 0.15);
	myScalarBar->SetOrientationToVertical();
    myScalarBar->GetPosition2Coordinate()->SetValue(0.1, 0.9);
	myScalarBar->SetOrientationToVertical();
    myScalarBar->GetTitleTextProperty()->ShadowOff();

	mySlices[0]->GetRenderer()->AddActor2D(myScalarBar);

	SetupUI();

	//myUI->sliceSlider->setValue(6);
	SetNumSlices(4);
	UpdateDepth();

	OnSelectedFieldChanged(0);
	// Setup cameras.
	vtkCamera* cam = NULL;
	for(int i = 0; i < MAX_SLICES; i++)
	{
		mySlices[i]->Update();
		if(cam == NULL) cam = mySlices[i]->GetRenderer()->GetActiveCamera();
		else mySlices[i]->GetRenderer()->SetActiveCamera(cam);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SliceViewer::SetNumSlices(int slices)
{
	myNumSlices = slices;
	for(int i = 0; i < MAX_SLICES; i++)
	{
		myUI->qvtkWidget->GetRenderWindow()->RemoveRenderer(mySlices[i]->GetRenderer());
	}

	float x = 0;
	float y = 1;
	float h = 1;
	float w = 1;
	// Compute viewport height, width.
	if(slices > 1)
	{
		if(slices > 6)
		{
			h = 0.33f;
			slices = slices / 3;
		}
		else if(slices > 2)
		{
			h = 0.5f;
			slices = slices / 2;
		}
		w = 1.0f / slices;
	}

	for(int i = 0; i < myNumSlices; i++)
	{
		myUI->qvtkWidget->GetRenderWindow()->AddRenderer(mySlices[i]->GetRenderer());
		mySlices[i]->GetRenderer()->SetViewport(x, (y - h), x + w, y);
		//printf("Added slice at: %f %f %f %f\n", x, y - h, x + w, y);
		x += w;
		if(x >= 1)
		{
			x = 0;
			y -= h;
		}
	}

	UpdateDepth();
	/*for(int i = 0; i < myNumSlices; i++)
	{
		mySlices[i]->Update();
	}*/

	myUI->qvtkWidget->GetRenderWindow()->Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SliceViewer::SetStartDepth(int depth)
{
	if(myUI->endSlider->value() < depth)
	{
		myUI->endSlider->setValue(depth);
	}
	UpdateDepth();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SliceViewer::SetEndDepth(int depth)
{
	if(myUI->startSlider->value() > depth)
	{
		myUI->startSlider->setValue(depth);
	}
	UpdateDepth();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SliceViewer::UpdateDepth()
{
	float* rng = myVizMng->GetDataSet()->GetZRange();
	float maxRange = (float)(rng[0] - rng[1]);
	float startDepth = -(float)(myUI->startSlider->value() * maxRange / 100);
	float endDepth = -(float)(myUI->endSlider->value() * maxRange / 100);
	float incr = (endDepth - startDepth) / myNumSlices;
	//printf("depth: %f %f %f\n", startDepth, endDepth, incr);
	for(int i = 0; i < myNumSlices; i++)
	{
		mySlices[i]->SetDepth(startDepth);
		startDepth += incr;
	}
	myUI->qvtkWidget->GetRenderWindow()->Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SliceViewer::SetNumContours(int contours)
{
	myNumContours = contours;
	for(int i = 0; i < myNumSlices; i++)
	{
		mySlices[i]->Update();
	}
	myUI->contoursButton->setText(QString("Contours: %1").arg(contours));
	myUI->qvtkWidget->GetRenderWindow()->Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SliceViewer::OnContoursButtonToggle(bool enabled)
{
	myContoursEnabled = enabled;
	for(int i = 0; i < myNumSlices; i++)
	{
		mySlices[i]->Update();
	}
	myUI->qvtkWidget->GetRenderWindow()->Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SliceViewer::OnSliceButtonToggle(bool checked)
{
	if(checked)
	{
		if(myUI->slice2->isChecked()) SetNumSlices(2);
		else if(myUI->slice4->isChecked()) SetNumSlices(4);
		else if(myUI->slice6->isChecked()) SetNumSlices(6);
		else if(myUI->slice9->isChecked()) SetNumSlices(9);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SliceViewer::OnSelectedFieldChanged(int index)
{
	mySelectedField = index;
	// Force an update of sonde transform BEFORE setting the active scalars on the output object.
	// If I set the active scalars before forcing the update, the active scalars will be reset by the
	// update itself.
	// NOTE: I have to force an update here because changing the active scalars on point data does not
	// trigger the main dataset modified flag, and the subsequent volume update will do nothing.
	DataSet* data = myVizMng->GetDataSet();
	ColorFunctionManager* colorMng = myVizMng->GetColorFunctionManager();

	VtkDataManager::GetInstance()->GetPointSet(DataSet::AllData)->GetPointData()->SetActiveScalars(data->GetFieldName(index));

	mySondeVolumeBuilder->Update();

	// Update color legend.
    //myScalarBar->SetTitle(data->GetFieldName(mySelectedField));
	myScalarBar->SetLookupTable(colorMng->GetColorFunction(mySelectedField, true));

	for(int i = 0; i < myNumSlices; i++)
	{
		mySlices[i]->Update();
	}

	myUI->qvtkWidget->GetRenderWindow()->Render();
}
