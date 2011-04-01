///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef TOOLSET_SETUP_WINDOW
#define TOOLSET_SETUP_WINDOW

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "LookingGlassSystem.h"
#include "ui_SetupToolset.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ENUM(ToolsetMode)
	None,
	SondeAndBathymetry,
	Glacier
END_ENUM;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ToolsetSetupWindow: public QObject
{
	Q_OBJECT
public:
	ToolsetSetupWindow() 
	{
		myToolsetMode = ToolsetMode::None;
	}
	~ToolsetSetupWindow() {}
	ToolsetMode::Enum GetToolsetMode() { return myToolsetMode; }
	void Show(QApplication& app)
	{
		Ui_SetupToolset* setupToolsetWin = new Ui_SetupToolset();
		myToolsetWindow = new QWidget();
		setupToolsetWin->setupUi(myToolsetWindow);

		setupToolsetWin->groupBox_2->setVisible(false);

		QDir dir("./");
		QStringList filters; 
		filters << "*.cfg";

		QStringList files = dir.entryList(filters);
		setupToolsetWin->profileBox->addItems(files);

		//setupToolsetWin->profileBox->addItem(QString("Endurance.cfg"));

		// Wire events
		QObject::connect(setupToolsetWin->runGlacierToolsetButton, SIGNAL(clicked()),
			this, SLOT(OnRunGlacierToolsetButtonClick()));
		QObject::connect(setupToolsetWin->runSondeToolsetButton, SIGNAL(clicked()),
			this, SLOT(OnRunSondeToolsetButtonClick()));

		myToolsetWindow->show();
		
		app.exec();

		mySelectedProfile = setupToolsetWin->profileBox->currentText();

		delete myToolsetWindow;
		myToolsetWindow = NULL;
	}

	bool GetRecomputeSondeBathy() { return myRecomputeSondeBathy; }
	const char* GetSelectedProfile() { return mySelectedProfile; }

public slots:
	void OnRunGlacierToolsetButtonClick()
	{
		myToolsetMode = ToolsetMode::Glacier;
		myToolsetWindow->close();
	}

	void OnRunSondeToolsetButtonClick()
	{
		myToolsetMode = ToolsetMode::SondeAndBathymetry;
		myToolsetWindow->close();
	}

private:
	ToolsetMode::Enum myToolsetMode;
	QWidget* myToolsetWindow;
	QString mySelectedProfile;
	bool myRecomputeSondeBathy;
	float mySondeBathyOffset;
};

#endif
