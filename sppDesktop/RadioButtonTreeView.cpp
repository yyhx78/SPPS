
#pragma warning( push, 0 )
#include <QMouseEvent>
#include <QKeyEvent>
#include <QApplication>
#include <QMenu>
#include "vtkSmartPointer.h"
#include <vtkUnstructuredGrid.h>
#include "vtkPolyData.h"
#include "vtkPlane.h"
#pragma warning( pop)

#include <assert.h>

#include "dlgOpenCase.h"
#include "RadioButtonTreeView.h"
#include "TreeModel.h"

class MainWnd;
extern MainWnd* mainWnd();

RadioButtonTreeView::RadioButtonTreeView(QWidget *parent)
	: QTreeView(parent)
{
}

void RadioButtonTreeView::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::RightButton) {
        TreeModel *treeModel = dynamic_cast<TreeModel*>(this->model());
        if (treeModel != NULL)
        {
            QModelIndex index = indexAt(event->pos());
            TreeItem* item = treeModel->item(index);
            EItemDataType itemDataType = eIDT_Unknown;
            const QVariant dPtr = item->getDataPtr(itemDataType);
            switch(itemDataType) {
                case eIDT_MeshComponent:
                {
                }
                    break;
                default:
                    break;
            }
        }

        event->accept();
	} else
		QTreeView::mousePressEvent(event);
}

void RadioButtonTreeView::slotSelectionChanged()
{
    QModelIndexList selItems = selectedIndexes();
    if (selItems.size() > 0)
    {
        QModelIndex index = selItems[0];
        if (index.isValid())
        {
        }
    }
}
#if 0
void RadioButtonTreeView::mouseReleaseEvent(QMouseEvent *event)
{
	QModelIndex index = indexAt(event->pos());
	if (index.data(Qt::CheckStateRole).value<bool>()) {
		/// clicking on an alternative's item in tree view should select alternative
        //		switchRadioFlag(index);
		event->accept();
	} else
		QTreeView::mouseReleaseEvent(event);
}

void RadioButtonTreeView::keyReleaseEvent(QKeyEvent *event)
{
	QModelIndex index = currentIndex();
    if (event->key() == Qt::Key_Space) {
 		event->accept();
	} else
		QTreeView::keyReleaseEvent(event);
}
void RadioButtonTreeView::switchRadioFlag(QModelIndex &index)
{
    
	const int maxRow = 1024;
	const int col = index.column();
    int iUncheck = (int)Qt::Unchecked;
	for (int row = 0; row < maxRow; ++row) {
		const QModelIndex &sib = index.sibling(row, col);
		if (sib != index) {
            //do nothing on the clicked item
         //uncheck all of the rest
            model()->setData(sib, QVariant::fromValue(iUncheck), Qt::CheckStateRole);
        }
	}
}
#endif

void RadioButtonTreeView::uncheckSibling(const QModelIndex &index)
{
    
	const int maxRow = 1024;
	const int col = index.column();
	for (int row = 0; row < maxRow; ++row) {
		const QModelIndex &sib = index.sibling(row, col);
		if (sib != index) {
            //do nothing on the clicked item
            //uncheck all of the rest
            int i = (int)Qt::Unchecked;
            model()->setData(sib, QVariant::fromValue(i), Qt::CheckStateRole);
        }
	}
}
