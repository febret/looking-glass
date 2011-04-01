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
#ifndef TABLEVIEW_H
#define TABLEVIEW_H

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "LookingGlassSystem.h"
#include "DataSet.h"
#include "DockedTool.h"
#include "ui_TableView.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class TableModel: public QAbstractTableModel
{
	Q_OBJECT
public:
	TableModel(DataSet* data);

	int rowCount( const QModelIndex & parent = QModelIndex()) const;
	int columnCount( const QModelIndex & parent = QModelIndex()) const;
	QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const ;
	Qt::ItemFlags flags ( const QModelIndex& index ) const; 
	QVariant headerData(int section, Qt::Orientation orientation, int role) const;
 
private:
	DataSet* myData;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class TableView: public DockedTool
{
    Q_OBJECT

public:
    TableView(VisualizationManager* mng);
    ~TableView();

	void Initialize();
	void Update();

protected slots:
	void OnSaveAsCSVButtonClick();
	void OnColumnCheckedChanged();
	void OnSelectAllButtonClicked();
	void OnClearAllButtonClicked();

private:
	void SetupUI();

private:
	// UI.
	Ui::TableView* myUI;
	VisualizationManager* myVizMng;

	QList<QCheckBox*> myColumnCheckBoxes;

	TableModel* myModel;

	DynamicFilter myFilter;
};

#endif