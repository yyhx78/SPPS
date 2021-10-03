#pragma warning( push, 0 )
#include "ui_RltView.h"
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QFileDialog>
#include <QStandardItem>
#include <qdockwidget.h>
#include <qlistwidget.h>
#include <qbasictimer.h>
#include <qplaintextedit.h>
#include <QtXml/QtXml>
#include <QtXml/qdom.h>
#include <QComboBox.h>

#include "vtkPlaneCollection.h"
#include <vtkDataObjectToTable.h>
#include <vtkElevationFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkVectorText.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPolyData.h>
#include <vtkRendererCollection.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include "vtkSmartPointer.h"
#include "vtkPlane.h"
#include "vtkCutter.h"
#include "vtkCell.h"
#include "vtkProperty.h"
#include "vtkMapper.h"
#include "vtkFeatureEdges.h"
#include "vtkCleanPolyData.h"
#include "vtkScalarBarActor.h"
#include "vtkFloatArray.h"
#include "vtkPlane.h"
#include "vtkLookupTable.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkDataObjectTreeIterator.h"
#include "vtkAreaPicker.h"
#include "vtkProp3DCollection.h"
#include "vtkCallbackCommand.h"
#include "vtkSmartPointer.h"
#include "vtkObject.h"

#pragma warning(pop)

#include <string>
#include <map>
#include "assert.h"
#include <sstream>

#include "MainWnd.h"

#include "RadioButtonTreeView.h"
#include "sheetDisplayOptions.h"
#include "editQueryResults.h"
#include "pageCuttingPlane.h"
#include "dlgOpenCase.h"
#include "dataModel/Connect.h"

#include "TreeModel.h"
#include "VtkExt/sppVtk.h"
#include "VtkExt/sppPlaneWidget.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

static MainWnd* s_mainWnd = NULL;
MainWnd* mainWnd()
{
	return s_mainWnd;
}
QtVtkView* MainWnd::qvtkWidget()
{
	return m_ui->qvtkWidget;
}

// Constructor
MainWnd::MainWnd():	m_pTreeModel(NULL)
{
	s_mainWnd = this;

	this->m_ui = new Ui_RltView;
	this->m_ui->setupUi(this);

	createDockWindows();

    auto &cn = SPP::Connect::cn();
        
    QWidget *spacerWidget = new QWidget(this);
    spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacerWidget->setVisible(true);
    
    m_connectBox = new QComboBox;
    m_connectBox->addItem(cn.urlRoot());
    m_ui->toolBar_View->addWidget(spacerWidget);
    m_ui->toolBar_View->addWidget(m_connectBox);
    connect(m_connectBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotConnectItemChanged(int)));

	// Set up action signals and slots
    connect(this->m_ui->actionOpenFile, SIGNAL(triggered()), this, SLOT(slotOpenFile()));
    connect(this->m_ui->actionOpen_Folder, SIGNAL(triggered()), this, SLOT(slotOpen_Folder()));
	connect(this->m_ui->actionExport, SIGNAL(triggered()), this, SLOT(slotExport()));
	connect(this->m_ui->actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));

	connect(this->m_ui->actionCuts, SIGNAL(triggered()), this, SLOT(slotDoCutting()));
	connect(this->m_ui->actionViewCenter, SIGNAL(triggered()), this, SLOT(slotDoCenter()));
	connect(this->m_ui->actionViewQuery, SIGNAL(triggered()), this, SLOT(slotDoQuery()));
	connect(this->m_ui->actionViewRotation, SIGNAL(triggered()), this, SLOT(slotDoRotate()));
	connect(this->m_ui->actionViewReset, SIGNAL(triggered()), this, SLOT(slotDoReset()));

    connect(this->m_ui->actionViewRubberBandZoom, SIGNAL(triggered()), m_ui->qvtkWidget, SLOT(slotDoRubberBandZoom()));

   	//this->showMaximized();
};

MainWnd::~MainWnd()
{
    if (m_pTreeModel)
        delete m_pTreeModel;
}

TreeModel* MainWnd::dataModel()
{
	if (m_pTreeModel == NULL)
	{
		m_pTreeModel = new TreeModel();
		if (m_treeView)
			m_treeView->setModel(m_pTreeModel);
	}

	return m_pTreeModel;
}

void MainWnd::displayScalarBar()
{
	//hide all first
	int nBars = m_ui->qvtkWidget->GetNumberOfScalarBars();
	for (int iBar = 0; iBar < nBars; iBar++)
		this->m_ui->qvtkWidget->ShowScalarBar(iBar, false);

	auto& lSrvDoc = SPP::Doc::getDoc();
	auto& lCase = lSrvDoc.GetCase();

	int lActiveRlt = lCase.m_activeRlt;
	if (lActiveRlt < 0)
	{
		this->m_ui->qvtkWidget->ShowScalarBar(0, false);
	}
	else
	{
		vtkScalarBarActor* scalarBarActor = this->m_ui->qvtkWidget->scalarBarActor(0);
		if (scalarBarActor)
		{
			double scalarRg[2] = { lCase.m_scalarRange[0], lCase.m_scalarRange[1] };
			auto lut = scalarBarActor->GetLookupTable();
			if (lut)
			{
				lut->SetRange(scalarRg);
				auto& lParts = lCase.GetPartList();
				for (auto& part : lParts)
				{
					if (part.m_surfaceA)
					{
						part.m_surfaceA->GetMapper()->SetLookupTable(lut);
						part.m_surfaceA->GetMapper()->SetScalarRange(scalarRg);
					}
                    
                    for (auto &cut : part.m_cuts)
					{
						cut.m_cutA->GetMapper()->SetLookupTable(lut);
						cut.m_cutA->GetMapper()->SetScalarRange(scalarRg);
					}
				}
			}
			auto& lRlt = lCase.GetResultList()[lActiveRlt];

            std::stringstream ss;
            ss << lRlt.GetName() << "[" << lRlt.GetUnit() << "]";
			scalarBarActor->SetTitle(ss.str().c_str());

			this->m_ui->qvtkWidget->ShowScalarBar(0, true);
		}
	}
}

void MainWnd::rebuildTreeView()
{
	//rebuild the tree
	this->m_treeView->reset();
	dataModel()->setupTree();
	this->m_treeView->expandAll();
}

vtkRenderer* MainWnd::renderer()
{
	return m_ui->qvtkWidget->renderer();
}

void MainWnd::repaint()
{
	this->m_ui->qvtkWidget->Render();
	this->m_ui->qvtkWidget->repaint();
}

void MainWnd::resetDisplay()
{
	//a study is supposed to have been loaded

	auto& sppDoc = SPP::Doc::getDoc();
	auto& lCase = sppDoc.GetCase();

	auto& lParts = lCase.GetPartList();
	for (auto& part : lParts)
	{
		if (lCase.surfaceVisible())
		{
			if (!part.m_surfaceA)
			{
				if (part.m_surface)
				{//surface
					auto lMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
					lMapper->SetInputData(part.m_surface);

					auto lActor = vtkSmartPointer<vtkActor>::New();
					lActor->SetMapper(lMapper);
					part.m_surfaceA = lActor;

					this->renderer()->AddActor(lActor);
				}
			}
			else
			{
				part.m_surfaceA->SetVisibility(1);
			}
		}
		else
		{
			if (part.m_surfaceA)
				part.m_surfaceA->SetVisibility(0);
		}

		if (!part.m_meshEdgesA)
		{
			if (part.m_meshEdges)
			{//mesh edges
				auto lMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
				lMapper->SetInputData(part.m_meshEdges);

				auto lActor = vtkSmartPointer<vtkActor>::New();
				lActor->SetMapper(lMapper);
				lActor->GetProperty()->SetColor(0, 0, 0);
				part.m_meshEdgesA = lActor;

				this->renderer()->AddActor(lActor);
			}
		}

		if (!part.m_featureEdgesA)
		{
			if (part.m_featureEdges)
			{//feature edges
				auto lMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
				lMapper->SetInputData(part.m_featureEdges);

				auto lActor = vtkSmartPointer<vtkActor>::New();
				lActor->SetMapper(lMapper);
				lActor->GetProperty()->SetColor(0, 0, 0);
				part.m_featureEdgesA = lActor;

				this->renderer()->AddActor(lActor);
			}
		}
	}

	if (lCase.isoSurfaceVisible())
	{
		if (lCase.m_isoAs.empty())
		{
			auto& lIsoPDs = lCase.m_isoPDs;
			if (!lIsoPDs.empty())
			{
				int nPDs = (int)lCase.m_isoPDs.size();
				lCase.m_isoAs.resize(nPDs);
				for (auto i = 0; i < nPDs; ++i)
				{
					vtkScalarsToColors* lut = nullptr;
					vtkScalarBarActor* scalarBarActor = this->m_ui->qvtkWidget->scalarBarActor(0);
					if (scalarBarActor)
					{
						lut = scalarBarActor->GetLookupTable();
					}

					auto lMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
					lMapper->SetInputData(lCase.m_isoPDs[i]);
					lMapper->SetScalarRange(lCase.m_scalarRange[0], lCase.m_scalarRange[1]);
					lMapper->SetLookupTable(lut);

					auto lActor = vtkSmartPointer<vtkActor>::New();
					lActor->SetMapper(lMapper);
					lCase.m_isoAs[i] = lActor;

					this->renderer()->AddActor(lActor);
				}
			}
		}
		else
		{
			for (auto a : lCase.m_isoAs)
			{
				a->SetVisibility(1);
			}
		}
	}
	else
	{
		for (auto a : lCase.m_isoAs)
		{
			a->SetVisibility(0);
		}
	}

	if (!lCase.m_streamLinePDs.empty())
	{
		if (lCase.m_streamLineAs.empty())
		{
			auto& lStreamLinePDs = lCase.m_streamLinePDs;
			{
				vtkScalarsToColors* lut = nullptr;
				vtkScalarBarActor* scalarBarActor = this->m_ui->qvtkWidget->scalarBarActor(0);
				if (scalarBarActor)
				{
					lut = scalarBarActor->GetLookupTable();
				}

				int nPDs = (int)lCase.m_streamLinePDs.size();
				lCase.m_streamLineAs.resize(nPDs);
				for (auto i = 0; i < nPDs; ++i)
				{
					auto lMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
					lMapper->SetInputData(lCase.m_streamLinePDs[i]);
					lMapper->SetScalarRange(lCase.m_scalarRange[0], lCase.m_scalarRange[1]);
					lMapper->SetLookupTable(lut);

					auto lActor = vtkSmartPointer<vtkActor>::New();
					lActor->SetMapper(lMapper);
					lCase.m_streamLineAs[i] = lActor;

					this->renderer()->AddActor(lActor);
				}
			}
		}
		else
		{
			for (auto a : lCase.m_streamLineAs)
			{
				a->SetVisibility(1);
			}
		}

		//streamlines are displayed, set the scalars on surface to null
		for (auto& part : lParts)
		{
			if (part.m_surface)
			{//surface
				part.m_surface->GetPointData()->SetScalars(nullptr);
			}
		}
	}
	else
	{
		//streamlines are results, if the pds is empty, different result has been selected.
		//destroy all of the actors
		for (auto a : lCase.m_streamLineAs)
		{
			renderer()->RemoveActor(a);
		}
		lCase.m_streamLineAs.clear();
	}
}

void MainWnd::createDockWindows()
{
	viewMenu = menuBar()->addMenu(tr("&View"));
	//results view
	QDockWidget *dock = new QDockWidget(tr("Case Data"), this);
	dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

	m_treeView = new RadioButtonTreeView(dock);
	dock->setWidget(m_treeView);
	addDockWidget(Qt::LeftDockWidgetArea, dock);
	viewMenu->addAction(dock->toggleViewAction());
	//properties view
	dock = new QDockWidget(tr("Properties"), this);
	dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
	m_PropertiesView = new sheetDisplayOptions(dock);

	dock->setWidget(m_PropertiesView);
	addDockWidget(Qt::BottomDockWidgetArea, dock);
	viewMenu->addAction(dock->toggleViewAction());
	//connections
	connect(this->m_ui->qvtkWidget, SIGNAL(signal_OnScalarBarModified(vtkScalarBarActor*)),
            this, SLOT(slot_OnScalarBarModified(vtkScalarBarActor*)));
	//inform main window that a doCenter or doSelection have just been done
	connect(this->m_ui->qvtkWidget, SIGNAL(signal_OnCenter()), this, SLOT(slot_OnCenter()));
	connect(this->m_ui->qvtkWidget, SIGNAL(signal_OnSelect()), this, SLOT(slot_OnSelect()));
	connect(this->m_ui->qvtkWidget, SIGNAL(mouseEvent(QMouseEvent*)), this, SLOT(slotOnVtkViewMouseEvent(QMouseEvent*)));

	connect(dataModel(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex & )),
		this, SLOT(slotTreeViewItemDataChanged(const QModelIndex &, const QModelIndex & )));

	connect(dataModel(), SIGNAL(itemCheckStatusChanged(const QModelIndex &)),
		this, SLOT(slotTreeViewItemCheckStatusChanged(const QModelIndex & )));

	//inform the properties window, mesh or result selection has been changed
	connect(dataModel(), SIGNAL(itemCheckStatusChanged(const QModelIndex &)),
		m_PropertiesView, SLOT(slotTreeViewItemCheckStatusChanged(const QModelIndex & )));

	//register the model
	m_treeView->expandAll();

	resize(1400, 900);
}

void MainWnd::createCuttingPlane(double *aClickedPt, double *aNormal)
{
	if (!aClickedPt || !aNormal)
		return;

	double adjustedPt[3];//adjust the point to cut internal
	for (int i = 0; i < 3; i++)
		adjustedPt[i] = aClickedPt[i] + 1.0E-5 * aNormal[i];
	double lReversedN[3];
	for (int i = 0; i < 3; i++)
		lReversedN[i] = -aNormal[i];

    auto& lSrvDoc = SPP::Doc::getDoc();
    auto& lCase = lSrvDoc.GetCase();

    auto vP = vtkSmartPointer<vtkPlane>::New();

    lSrvDoc.GetCase().cuttingPlanes().push_back(SPP::CuttingPlane());
    auto &lNewCuttingPlane = lSrvDoc.GetCase().cuttingPlanes().back();
    lNewCuttingPlane.enabled(true); //enabled by default
    lNewCuttingPlane.m_plane = vP;

    vP->SetOrigin(adjustedPt);
	vP->SetNormal(lReversedN);

    updateCuttingPlane(vP);
        
    //apply the plane on all actors
    activateCuttingPlane(vP, true);
    
    //deactivate the old one if it exists
    activateCuttingPlaneWidget(lCase.activePlane(), false);
    //activate the new one
    activateCuttingPlaneWidget(vP, true);//display the plane and the widget
 
    auto cbk = vtkSmartPointer<cbkCuttingPlaneModified>::New();
    vP->AddObserver(vtkCommand::ModifiedEvent, cbk);
    vP->Modified();

	this->repaint();
}

void MainWnd::slotOnVtkViewMouseEvent(QMouseEvent* event)
{
	vtkRenderer *vRenderer = m_ui->qvtkWidget->renderer();
	if (vRenderer == NULL)
		return;

	switch (event->button())
	{
	case Qt::LeftButton:
		{
			switch (event->type())
			{
			case QEvent::MouseButtonPress:
				{
				}
				break;
			case QEvent::MouseButtonRelease:
				if (this->m_ui->qvtkWidget->m_pVtk->GetStatus() == eViewMode_Query)
				{
                } else if (this->m_ui->qvtkWidget->qtVtkViewMode() == eQtVtkViewMode_CuttingPlane)
                {
                    sppPlaneWidget *widget = m_ui->qvtkWidget->m_pVtk->planeWidget();
					if (!widget || !widget->GetEnabled())// || widget->GetState() == sppPlaneWidget::WidgetState::Outside)
					{
						//do selection
						int x = event->x();
						int y = m_ui->qvtkWidget->height() - event->y();
						vtkCellPicker* retPicker = m_ui->qvtkWidget->m_pVtk->SelectCell(x, y, 0.001);
						if (retPicker && retPicker->GetActor())
						{
                            auto *lPickedA = retPicker->GetActor();
                            double lXyz[3];
                            retPicker->GetMapperPosition(lXyz); //sorted!

                            //check whether a cut is picked
                            auto& lSrvDoc = SPP::Doc::getDoc();
                            auto& lCase = lSrvDoc.GetCase();
                            SPP::Cut *lPickedCut = nullptr;
                            for (auto& part : lCase.GetPartList())
                            {
                                for (auto& c : part.m_cuts)
                                {
                                    if (lPickedA == c.m_cutA.GetPointer())
                                    {
                                        lPickedCut = &c;
                                        break;
                                    }
                                }
                                if (!lPickedCut)
                                    break;
                            }

                            if (lPickedCut)
                            {//activate the picked plane
                                lPickedCut->m_plane->SetOrigin(lXyz);
                                this->activateCuttingPlane(lPickedCut->m_plane, true);
                                this->activateCuttingPlaneWidget(lPickedCut->m_plane, true);
                            } else
                            {
                                auto* lNormal = retPicker->GetMapperNormal();
                                createCuttingPlane(lXyz, lNormal);
                            }
						}
					}
                }
                break;
			default:
				break;
			}
		}
		break;
	case Qt::RightButton:
		{
		}
		break;
	default:
		break;
	}
}

void MainWnd::slotConnectItemChanged(int)
{
    
}

void MainWnd::slotTreeViewItemDataChanged(const QModelIndex & upLeft, const QModelIndex & bottomRight)
{
	if (upLeft == bottomRight){
	}
}

static bool static_blockSignal = false;
void MainWnd::slotTreeViewItemCheckStatusChanged(const QModelIndex & index)
{
	if (static_blockSignal)
		return;
	if (index.isValid()) 
	{
		EItemDataType itemDataType = eIDT_Unknown;
		TreeItem* item = dataModel()->item(index);
		if (item)
		{
			bool bChecked = item->isChecked();
			const QVariant dPtr = item->getDataPtr(itemDataType);
			switch (itemDataType) {
				case eIDT_MeshComponents:
				{
					auto rows = m_pTreeModel->rowCount(index);
					auto cols = m_pTreeModel->columnCount(index);

					int i = (int)(bChecked? Qt::Checked : Qt::Unchecked);

					const int col = index.column();
					for (int row = 0; row < rows; ++row) 
					{
						const auto &child = dataModel()->index(row, col, index);
						if (m_pTreeModel->item(child)->isChecked() == bChecked)
							continue;
						//uncheck all of the rest
						m_pTreeModel->setData(child, QVariant::fromValue(i), Qt::CheckStateRole);
					}
				}
				break;
				case eIDT_MeshComponent:
				{
					auto iPart = dPtr.toInt();
					if (iPart >= 0)
					{
						auto& lSrvDoc = SPP::Doc::getDoc();
						//reset the existing study to load a new one
						auto& lParts = lSrvDoc.GetCase().GetPartList();
						if (iPart < lParts.size())
						{
							auto& lPart = lParts[iPart];
							if (lPart.m_featureEdgesA)
								lPart.m_featureEdgesA->SetVisibility(bChecked);
							if (lPart.m_meshEdgesA)
								lPart.m_meshEdgesA->SetVisibility(bChecked);
							if (lPart.m_surfaceA)
								lPart.m_surfaceA->SetVisibility(bChecked);
						}
					}
				}
				break;
				case eIDT_Result:
				{
					//uncheck all first
					static_blockSignal = true;
					m_treeView->uncheckSibling(index);
					static_blockSignal = false;

					int iRlt = dPtr.toInt();
					if (iRlt >= 0)
					{
						auto& lSrvDoc = SPP::Doc::getDoc();
						auto& lCase = lSrvDoc.GetCase();

						//reset the existing study to load a new one
						auto& lRlts = lCase.GetResultList();
						if (iRlt < lRlts.size())
						{
							auto& lRlt = lRlts[iRlt];
							if (bChecked)
							{
								QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

								lCase.m_activeRlt = iRlt;
								//load the result if needed
								lCase.LoadResultData(iRlt, 0); //-1: default indp is the first one
								
								updateCuttingPlane(lCase.activePlane());

								QApplication::restoreOverrideCursor();
							}
							else
							{
								lCase.m_activeRlt = -1;
								//remove result from surface, cutting plane. if in iso, switch to surface display.
								if (lCase.isoSurfaceVisible())
								{
									lCase.surfaceVisible(true);
									lCase.isoSurfaceVisible(false);
								}

								auto& lParts = lCase.GetPartList();
								for (auto& part : lParts)
								{
									if (part.m_surface)
										part.m_surface->GetPointData()->SetScalars(nullptr);
									for (auto& cut : part.m_cuts)
									{
										if (cut.m_cut)
											cut.m_cut->GetPointData()->SetScalars(nullptr);
									}
								}
							}

							m_PropertiesView->onResultSelectionChanged();

							resetDisplay();

							displayScalarBar();
						}
					}
				}
				break;
                default:
                    break;
			}
		}
	}
	this->m_ui->qvtkWidget->Render();
	this->m_ui->qvtkWidget->repaint();
}

void MainWnd::openResultsFolder(const char* aFolder)
{
    if (!aFolder)
        return;
    
    if (m_pTreeModel == NULL)
    {
        m_pTreeModel = new TreeModel();
        if (m_treeView)
            m_treeView->setModel(m_pTreeModel);
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    auto& lSrvDoc = SPP::Doc::getDoc();
    lSrvDoc.Renderer(m_ui->qvtkWidget->renderer());

    int N = 10; //the data transfer may take time, need to wait. About N*2 seconds, should be enough
    while (N-- > 0)
    {
        //reset the existing study to load a new one
        auto& lCase = lSrvDoc.GetCase();
        lCase.Reset();

        lSrvDoc.LoadCase(aFolder);

        //load parts if needed
        auto& lCasePartList = lCase.GetPartList();
        auto nParts = lCasePartList.size();
        
        if (nParts <= 0)
        {
#ifdef _WIN32
            Sleep(2000);
#else
            sleep(2);//wait for 2s
#endif
            continue;
        }

        for (int i = 0; i < nParts; i++)
        {
            lCase.LoadPartData(i, 0);
        }

        this->rebuildTreeView();
        this->resetDisplay();
        this->m_ui->qvtkWidget->AutoCenter();
        
        break;
    }

    QApplication::restoreOverrideCursor();
}

static const QString DEFAULT_DIR_KEY("last_opened_file_dir");

extern void searchResultsFolder(const char* aFolder, std::vector<SPP::CaseAddress> &aCaseFolders);

void MainWnd::slotOpenFile()
{
    QSettings openFileSettings;

    QString defaultLoc = openFileSettings.value(DEFAULT_DIR_KEY).toString();
    if (defaultLoc.isEmpty())
        defaultLoc = QDir::homePath();
    
    QFileDialog dialog(this, ("Open File"));
    dialog.setDirectory(defaultLoc);
    dialog.setFileMode(QFileDialog::ExistingFile);
    //*.json is generated by foamToOFRV from OpenFoam
    dialog.setNameFilter(("Simulation Results File (*.srd *.vtk *.xml *.xrd *.sug *.upd *.ex2 *.foam *.json *.obj)"));
    QString fileName;
#ifdef SRLT_OS_MAC //the setDirectory funtion does not work on mac, use this to solve the problem
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
    if (dialog.exec()) {
        QStringList filenames = dialog.selectedFiles();
        fileName = filenames.at(0);
    
        if (fileName.isNull())
            return;
        
        QFileInfo fInfo(fileName);
        QDir CurrentDir = fInfo.absoluteDir();
        openFileSettings.setValue(DEFAULT_DIR_KEY, CurrentDir.absolutePath());

        auto lFileName = fInfo.fileName().toStdString();
        auto lParentFolder = CurrentDir.absolutePath().toStdString();
        
        //if a service exists, kill and remove it to build a new one with the new data folder
        auto& cn = SPP::Connect::cn();
        
        std::string lId, lVersion;
        std::string lIP("http://localhost");
        int iPort = -1;
        bool lState = false;
        if (cn.findDockerContainer(lId, lVersion, lIP, iPort, lState))
        {
            if (lState || iPort < 0 || lVersion.empty())
            {//it is running, kill it
                cn.killService(lId);
            }
            
            cn.removeService(lId);
        }
        
    //open the file
        iPort = 9597;
        std::string pt("9597");
        std::string dataFolder = lParentFolder;
        cn.createService(pt, dataFolder);

        std::string url = lIP + ":" + std::to_string(iPort);
        cn.urlRoot(url.c_str());
        cn.mountedFolder(dataFolder.c_str());
        
        std::string lFilePathOnServer = std::string("/home/yyhx78/cases/") + lFileName;

        this->openResultsFolder(lFilePathOnServer.c_str());
    }
}

void MainWnd::slotOpen_Folder()
{
    QSettings openFolderSettings;

    QString defaultLoc = openFolderSettings.value(DEFAULT_DIR_KEY).toString();
    if (defaultLoc.isEmpty())
        defaultLoc = QDir::homePath();

    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Results Folder"),
                                                defaultLoc,
                                                QFileDialog::ShowDirsOnly
                                                | QFileDialog::DontResolveSymlinks);
    
    std::string lFolder = dir.toStdString();
    if (lFolder.empty())
        return;
    
    openFolderSettings.setValue(DEFAULT_DIR_KEY, dir);

    std::vector<SPP::CaseAddress> lCaseFolders;
    searchResultsFolder(lFolder.c_str(), lCaseFolders);
    
    if (lCaseFolders.size() == 1)
        this->openResultsFolder(lCaseFolders[0].GetFile());
}

void MainWnd::slotExport()
{
	auto& cn = SPP::Connect::cn();
	cn.doExport();
}

class sppPlaneWidget;
class sppProbeDataCmd : public vtkCommand
{
public:
    static sppProbeDataCmd* New()
    {
        return new sppProbeDataCmd(nullptr, nullptr);
    }
    
	sppProbeDataCmd(MainWnd *wnd, sppPlaneWidget *widget) : m_wnd(wnd), m_widget(widget)
	{
	}

	virtual void Execute(vtkObject *caller, unsigned long uEvent, void*)
	{
		if (uEvent == vtkCommand::InteractionEvent)
		{
			vtkPlane *plane = vtkPlane::New();
			m_widget->GetPlane(plane);
			m_wnd->setCuttingPlane(plane);
			plane->Delete();
		}
	}

	MainWnd *m_wnd;
    sppPlaneWidget *m_widget;
};

void MainWnd::updateCuttingPlane(vtkPlane* aPlane)
{
	if (!aPlane)
		return;

    //share the same lut with the scalar bar
	vtkScalarsToColors* lut = nullptr;
	vtkScalarBarActor* scalarBarActor = this->m_ui->qvtkWidget->scalarBarActor(0);
	if (scalarBarActor)
	{
		lut = scalarBarActor->GetLookupTable();
	}

    auto& lSrvDoc = SPP::Doc::getDoc();
    auto& lCase = lSrvDoc.GetCase();

    //the plane may have been moved, need to update the cuts from server
	lCase.LoadSlicesOnParts(aPlane);
    
    //update the actors
	auto& lParts = lCase.GetPartList();
	for (auto& part : lParts)
	{
        for (auto& cut : part.m_cuts)
		{//surface
			if (!cut.m_cutA)
			{
				auto lMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
				lMapper->SetInputData(cut.m_cut);

				auto lActor = vtkSmartPointer<vtkActor>::New();
				lActor->SetMapper(lMapper);
				cut.m_cutA = lActor;

				m_ui->qvtkWidget->renderer()->AddActor(lActor);
                //a new cut is just generated, clip with other existing planes
                for (auto &cp : lCase.cuttingPlanes())
                {
                    if (cp.m_plane.GetPointer() != aPlane && cp.enabled())
                    {
                        lMapper->AddClippingPlane(cp.m_plane);
                    }
                }
			}
			else
			{
				auto lMapper = vtkPolyDataMapper::SafeDownCast(cut.m_cutA->GetMapper());
				if (lMapper)
					lMapper->SetInputData(cut.m_cut);
			}

			if (cut.m_cutA)
			{
				cut.m_cutA->GetMapper()->SetScalarRange(lut->GetRange());
				cut.m_cutA->GetMapper()->SetLookupTable(lut);
			}
		}
	}
}

void MainWnd::setCuttingPlane(vtkPlane *plane)
{
	if (plane == NULL)
		return;

	auto& lSrvDoc = SPP::Doc::getDoc();
	auto& lCase = lSrvDoc.GetCase();
	if (lCase.activePlane())
	{
        auto lActivePlane = lCase.activePlane();
		lActivePlane->SetOrigin(plane->GetOrigin());
		lActivePlane->SetNormal(plane->GetNormal());
		//

		updateCuttingPlane(lActivePlane);

		//update the diplay
		this->repaint();
	}
}

static void addClippingPlane(vtkMapper* m, vtkPlane* p)
{
    if (!m || !p)
        return;
    
    bool bExist = false;
    auto *lPlanes = m->GetClippingPlanes();
    if (lPlanes)
    {
        vtkPlane *lPlane;
        for (lPlanes->InitTraversal(); (lPlane = lPlanes->GetNextItem());)
        {
            if (lPlane == p)
            {
                bExist = true;
                break;
            }
        }
    }
    
    if (bExist)
        return;
    
    m->AddClippingPlane(p);
    m->Modified();
}

static void removeClippingPlane(vtkMapper* m, vtkPlane* p)
{
    if (!m || !p)
        return;
    
    m->RemoveClippingPlane(p);
    m->Modified();
}

void MainWnd::activateCuttingPlane(vtkPlane *aPlane, bool bActivate)
{
    if (!aPlane)
        return;
            
    auto& lSrvDoc = SPP::Doc::getDoc();
    auto& lCase = lSrvDoc.GetCase();
    //remove all cuts and their actors
    auto& lParts = lCase.GetPartList();
    
    auto lCP = lCase.findCP(aPlane);
    if (lCP)
        lCP->enabled(bActivate);

    for (auto& part : lParts)
    {
        auto cut = part.FindCut(aPlane);
        if (bActivate)
        {
            if (part.m_featureEdgesA)
                addClippingPlane(part.m_featureEdgesA->GetMapper(), aPlane);
            if (part.m_meshEdgesA)
                addClippingPlane(part.m_meshEdgesA->GetMapper(), aPlane);
            if (part.m_surfaceA)
                addClippingPlane(part.m_surfaceA->GetMapper(), aPlane);
            
            vtkActor *lNewCutActor = nullptr;
            for (auto &cd : part.m_cuts)
            {
                if (cd.m_plane.GetPointer() != aPlane)
                {
                    if (cd.m_cutA)
                    {
                        auto lCP = lCase.findCP(aPlane);
                        if (lCP && lCP->enabled())
                            addClippingPlane(cd.m_cutA->GetMapper(), aPlane);
                    }
                } else
                {
                    lNewCutActor = cd.m_cutA;
                }
            }
            
            if (lNewCutActor)
            {
                for (auto &cd : part.m_cuts)
                {
                    if (cd.m_plane.GetPointer() != aPlane)
                    {
                        auto lCP = lCase.findCP(cd.m_plane);
                        if (lCP && lCP->enabled())
                            addClippingPlane(lNewCutActor->GetMapper(), cd.m_plane);
                    }
                }
            }
        } else
        {
            if (part.m_featureEdgesA)
                removeClippingPlane(part.m_featureEdgesA->GetMapper(), aPlane);
            if (part.m_meshEdgesA)
                removeClippingPlane(part.m_meshEdgesA->GetMapper(), aPlane);
            if (part.m_surfaceA)
                removeClippingPlane(part.m_surfaceA->GetMapper(), aPlane);

            auto &lCuts = part.m_cuts;
            for (auto& c : lCuts)
            {
                if (c.m_plane.GetPointer() != aPlane)
                {
                    if (c.m_cutA)
                        removeClippingPlane(c.m_cutA->GetMapper(), aPlane);
                }
            }
            
            if (cut)
            {
                if (cut->m_cutA)
                {//surface
                    renderer()->RemoveActor(cut->m_cutA);
                    cut->m_cutA = nullptr;
                }
                
                cut->m_cut = nullptr;
                
                for (auto it = lCuts.begin(); it != lCuts.end(); it++)
                {
                    if (&(*it) == cut)
                    {
//                        lCuts.erase(it);
                    }
                }
            }
        }
    }
}

void MainWnd::activateCuttingPlaneWidget(vtkPlane *vPlane, bool bActivate)
{
	if (vPlane == NULL && bActivate)
		return; //vPlane is not needed for disable

	auto& lSrvDoc = SPP::Doc::getDoc();
	auto& lCase = lSrvDoc.GetCase();
	if (vPlane != lCase.activePlane())
	{
		lCase.activePlane(vPlane);
	}

	if (bActivate)
	{
		double bs[6];
		if (lCase.GetBounds(bs))
		{
			double size = bs[1] - bs[0];
			if (size < (bs[3] - bs[2]))
				size = bs[3] - bs[2];
			if (size < (bs[5] - bs[4]))
				size = bs[5] - bs[4];

			double widgetSize = 0.16 * size;
			m_ui->qvtkWidget->ShowPlaneWidget(bActivate, vPlane->GetOrigin(), vPlane->GetNormal(), widgetSize);
		}
	}

	sppPlaneWidget *widget = m_ui->qvtkWidget->m_pVtk->planeWidget();
	if (widget)
	{
		if (bActivate)
		{
            auto cmd = vtkSmartPointer<sppProbeDataCmd>::New();
            cmd->m_wnd = this;
            cmd->m_widget = widget;
            widget->AddObserver(vtkCommand::InteractionEvent, cmd);
		} else
		{
			widget->RemoveAllObservers();
			m_ui->qvtkWidget->ShowPlaneWidget(false);
		}
	}
}

void MainWnd::slotExit() {
	qApp->exit();
}

void MainWnd::onViewStatusChange(QAction* a)
{
	if (a == NULL)
		return;

	if (a == m_ui->actionCuts && m_ui->qvtkWidget->qtVtkViewMode() == eQtVtkViewMode_CuttingPlane)
	{//disable cutting mode (hide the plane widget)
		m_ui->qvtkWidget->qtVtkViewMode(eQtVtkViewMode_Unknown);
		this->activateCuttingPlaneWidget(NULL, false);
	}

	if (a->isChecked())
	{
		if (a == m_ui->actionCuts)
        {
            m_ui->qvtkWidget->qtVtkViewMode(eQtVtkViewMode_CuttingPlane);
            //uncheck all others
            //			m_ui->actionCuts->setChecked(false);
            m_ui->actionViewQuery->setChecked(false);
            m_ui->actionViewCenter->setChecked(false);
            m_ui->actionViewRotation->setChecked(false);
            m_ui->actionViewRubberBandZoom->setChecked(false);
        } else if (a == m_ui->actionViewQuery)
        {
            m_ui->qvtkWidget->m_pVtk->SetStatus(eViewMode_Query);
            //uncheck all others
            m_ui->actionCuts->setChecked(false);
            //			m_ui->actionViewQuery->setChecked(false);
            m_ui->actionViewCenter->setChecked(false);
            m_ui->actionViewRotation->setChecked(false);
            m_ui->actionViewRubberBandZoom->setChecked(false);
        } else	if (a == m_ui->actionViewCenter)
        {
            m_ui->qvtkWidget->m_pVtk->SetStatus(eViewMode_Center);
            //uncheck all others
            m_ui->actionCuts->setChecked(false);
            m_ui->actionViewQuery->setChecked(false);
            //	        m_ui->actionViewCenter->setChecked(false);
            m_ui->actionViewRotation->setChecked(false);
            m_ui->actionViewRubberBandZoom->setChecked(false);
        } else 	if (a == m_ui->actionViewRotation)
        {
            m_ui->qvtkWidget->m_pVtk->SetStatus(eViewMode_Rotate);
            //uncheck all others
            m_ui->actionCuts->setChecked(false);
            m_ui->actionViewQuery->setChecked(false);
            m_ui->actionViewCenter->setChecked(false);
            //			m_ui->actionViewRotation->setChecked(false);
            m_ui->actionViewRubberBandZoom->setChecked(false);
        } else if (a == m_ui->actionViewRubberBandZoom)
        {
            m_ui->qvtkWidget->m_pVtk->SetStatus(eViewMode_RubberBand_Zoom);
            //uncheck all others
            m_ui->actionCuts->setChecked(false);
            m_ui->actionViewQuery->setChecked(false);
            m_ui->actionViewCenter->setChecked(false);
            m_ui->actionViewRotation->setChecked(false);
            //			m_ui->actionViewZoom->setChecked(false);
        }
	}
	else
	{
		if (a == m_ui->actionCuts)
		{//temperary, remove all cutitng planes. the cutting button is clicked two times
			//later, need to find a place to manage all cutting planes (a dialog)
		}

		this->m_ui->qvtkWidget->m_pVtk->SetStatus(eViewMode_Rotate);
	}
}

void MainWnd::slot_OnScalarBarModified(vtkScalarBarActor* a)
{
}

void MainWnd::slot_OnCenter(){
	m_ui->actionViewRotation->setChecked(true);
	onViewStatusChange(m_ui->actionViewRotation);
}

void MainWnd::slotDoCutting(){
	onViewStatusChange(m_ui->actionCuts);
}

void MainWnd::slotDoCenter(){
	//    this->m_ui->qvtkWidget->m_pVtk->DoCenter(0, 0);
	onViewStatusChange(m_ui->actionViewCenter);
}

void MainWnd::slotDoQuery(){
	onViewStatusChange(m_ui->actionViewQuery);
}

void MainWnd::slotDoRotate(){
	onViewStatusChange(m_ui->actionViewRotation);
}

void MainWnd::slotDoReset(){
	this->m_ui->qvtkWidget->ResetView(0, 0, 1, 0, 1, 0);
}
