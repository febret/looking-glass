///////////////////////////////////////////////////////////////////////////////////////////////////
#include "GlacierVisualizationManager.h"
#include "GlacierViewOptions.h"
#include "ui_MainWindow.h"
#include "PreferencesWindow.h"
#include "VisualizationManager.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
#define GLACIER_BATHYMETRY_FILE "./data/glacier.vtk"
#define GLACIER_SURFACE_FILE "./data/glacier_surface2.vtk"

///////////////////////////////////////////////////////////////////////////////////////////////////
GlacierVisualizationManager::GlacierVisualizationManager():
	myRenderWindow(NULL)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
GlacierVisualizationManager::~GlacierVisualizationManager()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GlacierVisualizationManager::SetupUI()
{
	// Disable preferences window in glacier mode.
	GetUI()->actionPreferences->setEnabled(false);

	connect(GetUI()->actionQuit, SIGNAL(triggered(bool)),
		SLOT(OnQuitTrigger(bool)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GlacierVisualizationManager::Initialize()
{
	VisualizationManagerBase::Initialize();
	GetMainWindow()->setCaption("Glacier Toolset - Looking Glass " LOOKING_GLASS_VERSION);

	LoadSonarData();
	//GenerateSurfaceData();
	LoadSurfaceData();

	myViewOptions = new GlacierViewOptions(this);
	myViewOptions->Initialize();
	myViewOptions->Enable();

	vtkInteractorStyleTerrain* terrainInteractor = vtkInteractorStyleTerrain::New();
    GetUI()->vtkView->GetInteractor()->SetInteractorStyle(terrainInteractor);

	SetupUI();
	Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GlacierVisualizationManager::GenerateSurfaceData()
{
	// This does not work right now.
	// Loaded surface is generated through paraview.
   /*SetInitMessage("Generate Glacier Surface Data...");
	
    vtkPolyDataWriter* writer;
    vtkDelaunay2D* lakeDelaunay;
    vtkPolyDataNormals* lakeNormals;

	// I have to rotate the points to perform a correct delaunay, then rotate them
	// back again.
	// TODO: Refactor & beautify this code a little bit..
	vtkTransform* xfd1 = vtkTransform::New();
	xfd1->RotateX(-90);
	vtkTransformFilter* xform1 = vtkTransformFilter::New();
	xform1->SetInput(myGlacierPointsReader->GetOutput());
	xform1->SetTransform(xfd1);

	lakeDelaunay = vtkDelaunay2D::New();
	lakeDelaunay->SetInput(xform1->GetOutput());
    lakeDelaunay->BoundingTriangulationOff();
    lakeDelaunay->SetTolerance(0.0005f);

	vtkTransform* xfd2 = vtkTransform::New();
	xfd2->RotateX(90);
	vtkTransformFilter* xform2 = vtkTransformFilter::New();
	xform2->SetInput(lakeDelaunay->GetOutputDataObject(0));
	xform2->SetTransform(xfd2);

    lakeNormals = vtkPolyDataNormals::New();
    lakeNormals->SetInput(xform2->GetOutputDataObject(0));
    lakeNormals->SetFeatureAngle(45);
	lakeNormals->Update();

	writer = vtkPolyDataWriter::New();
	writer->SetInput(lakeNormals->GetOutput());
	writer->SetFileName(GLACIER_SURFACE_FILE);
	writer->SetFileTypeToBinary();
	writer->Write();

	writer->Delete();
	lakeNormals->Delete();
	lakeDelaunay->Delete();
	xform1->Delete();
	xform2->Delete();
	xfd1->Delete();
	xfd2->Delete();*/
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GlacierVisualizationManager::LoadSonarData()
{
    SetInitMessage("Loading Glacier Surface Data...");

	// Sonar based bathymetry.
	myGlacierPointsReader = vtkUnstructuredGridReader::New();
    myGlacierPointsReader->SetFileName(GLACIER_BATHYMETRY_FILE);
    myGlacierPointsReader->Update();

	myTransformDef = vtkTransform::New();
	// Invert depth.
	myTransformDef->Scale(1, -VisualizationManager::DefaultDepthScale, -1);
	myTransformDef->RotateX(-90);

	mySonarTransform = vtkTransformFilter::New();
	mySonarTransform->SetInput(myGlacierPointsReader->GetOutput());
	mySonarTransform->SetTransform(myTransformDef);
	mySonarTransform->Update();

	myGlacierPointsMapper = vtkDataSetMapper::New();
	myGlacierPointsMapper->SetInput(mySonarTransform->GetOutput());
	myGlacierPointsMapper->Update();
	myGlacierPointsMapper->ScalarVisibilityOff();

	myGlacierPointsActor = vtkActor::New();
	myGlacierPointsActor->SetVisibility(1);
    myGlacierPointsActor->SetMapper(myGlacierPointsMapper);
	myGlacierPointsActor->GetProperty()->SetRepresentationToPoints();
	myGlacierPointsActor->GetProperty()->SetColor(1, 1, 1);

	GetMainRenderer()->AddActor(myGlacierPointsActor);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GlacierVisualizationManager::LoadSurfaceData()
{
    SetInitMessage("Loading Glacier Sonar Data...");

	// Sonar based bathymetry.
	myGlacierSurfaceReader = vtkPolyDataReader::New();
    myGlacierSurfaceReader->SetFileName(GLACIER_SURFACE_FILE);
    myGlacierSurfaceReader->Update();

	mySurfaceTransformDef = vtkTransform::New();
	mySurfaceTransformDef->Scale(1, -VisualizationManager::DefaultDepthScale, 1);
	mySurfaceTransformDef->Translate(0, 4, 0);

	mySurfaceTransform = vtkTransformFilter::New();
	mySurfaceTransform->SetInput(myGlacierSurfaceReader->GetOutput());
	mySurfaceTransform->SetTransform(mySurfaceTransformDef);
	mySurfaceTransform->Update();

	myGlacierSurfaceMapper = vtkPolyDataMapper::New();
	myGlacierSurfaceMapper->SetInputConnection(mySurfaceTransform->GetOutputPort());
	myGlacierSurfaceMapper->Update();
	myGlacierSurfaceMapper->ScalarVisibilityOff();

	myGlacierSurfaceActor = vtkActor::New();
	myGlacierSurfaceActor->SetVisibility(1);
    myGlacierSurfaceActor->SetMapper(myGlacierSurfaceMapper);
	myGlacierSurfaceActor->GetProperty()->SetOpacity(0.0f);
    myGlacierSurfaceActor->GetProperty()->BackfaceCullingOff();
    myGlacierSurfaceActor->GetProperty()->FrontfaceCullingOff();
    myGlacierSurfaceActor->GetProperty()->SetAmbientColor(0.3f, 0.3f, 0.3f);
    myGlacierSurfaceActor->GetProperty()->SetAmbient(0.2f);
	myGlacierSurfaceActor->GetProperty()->SetDiffuse(1);
	myGlacierSurfaceActor->GetProperty()->SetDiffuseColor(0.6f, 0.6f, 0.8f);
	myGlacierSurfaceActor->GetProperty()->SetSpecular(0);
	//myGlacierSurfaceActor->GetProperty()->SetRepresentationToPoints();
	//myGlacierSurfaceActor->GetProperty()->SetColor(1, 1, 1);

	GetMainRenderer()->AddActor(myGlacierSurfaceActor);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GlacierVisualizationManager::Update()
{
	Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GlacierVisualizationManager::SetDepthScale(int value)
{
	myTransformDef->Identity();
	myTransformDef->Scale(1, -value, -1);
	myTransformDef->RotateX(-90);
	mySurfaceTransformDef->Identity();
	mySurfaceTransformDef->Scale(1, -value, 1);
	mySurfaceTransformDef->Translate(0, 4, 0);
	Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GlacierVisualizationManager::SetSurfaceOpacity(int value)
{
	myGlacierSurfaceActor->GetProperty()->SetOpacity((float)value / 90);
	Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GlacierVisualizationManager::SetSonarDataVisibility(bool visible)
{
	myGlacierPointsActor->SetVisibility(visible);
	Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GlacierVisualizationManager::OnQuitTrigger(bool)
{
	QApplication::quit();
}
