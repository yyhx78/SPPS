#pragma warning( push, 0 )
#include <QtGui>
#include <QtWidgets>

#include "vtkObject.h"
#include "vtkSmartPointer.h"
#include "vtkUnstructuredGrid.h"
#include "vtkPolyData.h"
#pragma warning(pop)

#include "TreeModel.h"
#include "dataModel/Doc.h"

#include <set>
extern std::set<TreeItem*> curItems;

TreeModel::TreeModel(QObject *parent)
: QAbstractItemModel(parent), m_cuttingPlaneGroupItem(0)
{
	QStringList headers;
    headers << "Post Processing Data";
    
    QVector<QVariant> rootData;
    foreach (QString header, headers)
        rootData << header;

    m_rootItem = new TreeItem(rootData);
}

TreeModel::~TreeModel()
{
    delete m_rootItem;
}

int TreeModel::columnCount(const QModelIndex & /* parent */) const
{
    return m_rootItem->columnCount();
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    TreeItem *item = getItem(index);

    if ( role == Qt::CheckStateRole && index.column() == 0 )
        return static_cast< int >( item->isChecked() ? Qt::Checked : Qt::Unchecked );

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    return item->data(index.column());
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    Qt::ItemFlags flags = Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if ( index.column() == 0 )
        flags |= Qt::ItemIsUserCheckable;

    return flags;
}

TreeItem *TreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item) return item;
    }
    return m_rootItem;
}

TreeItem* TreeModel::item(const QModelIndex &index) const
{
    return getItem(index);
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return m_rootItem->data(section);

    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    TreeItem *parentItem = getItem(parent);

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

bool TreeModel::insertColumns(int position, int columns, const QModelIndex &parent)
{
    bool success;

    beginInsertColumns(parent, position, position + columns - 1);
    success = m_rootItem->insertColumns(position, columns);
    endInsertColumns();

    return success;
}

bool TreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    bool success;

    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->insertChildren(position, rows, m_rootItem->columnCount());
    endInsertRows();

    return success;
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = getItem(index);
    TreeItem *parentItem = childItem->parent();

	TreeItem *validItem = dynamic_cast<TreeItem*>(parentItem);
	if (validItem == NULL)
		return QModelIndex();

    if (parentItem == m_rootItem)
        return QModelIndex();

	if (curItems.find(parentItem) == curItems.end())
		return QModelIndex();

	int nChildren = 0;
	try {
		nChildren = parentItem->childNumber();
	}
	catch(...)
	{
		nChildren = 0;
	}

    return createIndex(nChildren, 0, parentItem);
}

bool TreeModel::removeColumns(int position, int columns, const QModelIndex &parent)
{
    bool success;

    beginRemoveColumns(parent, position, position + columns - 1);
    success = m_rootItem->removeColumns(position, columns);
    endRemoveColumns();

    if (m_rootItem->columnCount() == 0)
        removeRows(0, rowCount());

    return success;
}

bool TreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    bool success = true;

    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem = getItem(parent);

    return parentItem->childCount();
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value,
                        int role)
{
    TreeItem *item = getItem(index);

	bool result = false;
    if ( role == Qt::CheckStateRole && index.column() == 0 )
    {
        Qt::CheckState eChecked = static_cast< Qt::CheckState >( value.toInt() );
        bool bNewValue = eChecked == Qt::Checked;
        item->setChecked(bNewValue);

        emit itemCheckStatusChanged(index);

        result = true;
    } else if (role == Qt::EditRole)
		result = item->setData(index.column(), value);

    if (result)
        emit dataChanged(index, index);

    return result;
}

bool TreeModel::setHeaderData(int section, Qt::Orientation orientation,
                              const QVariant &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    bool result = m_rootItem->setData(section, value);

    if (result)
        emit headerDataChanged(orientation, section, section);

    return result;
}

void TreeModel::setupTree()
{
    if (m_rootItem != NULL) //root has to be recreated to prevent the crash
        delete m_rootItem;

    QStringList headers;
    headers << "Post Processing Data";

    QVector<QVariant> rootData;
    foreach(QString header, headers)
        rootData << header;

    m_rootItem = new TreeItem(rootData);
    TreeItem* parent = m_rootItem;

    parent->insertChildren(0, 2, 1);//column count is determined by the root column count

    auto &lCase = SPP::Doc::getDoc().GetCase();
    auto &lCasePartList = lCase.GetPartList();
    auto &lCaseRltInfoList = lCase.GetResultList();

    {//mesh components
        TreeItem* lMshFragmentsItem = parent->child(0);
        lMshFragmentsItem->setData(0, "Mesh Components");
        lMshFragmentsItem->setDataPtr(eIDT_MeshComponents, QVariant(0));
        lMshFragmentsItem->setChecked(true); //default
        auto nMeshCmpts = lCasePartList.size();
        if (nMeshCmpts > 0) {
            lMshFragmentsItem->insertChildren(0, (int)nMeshCmpts, 1);
            QString qs;
            for (int i = 0; i < nMeshCmpts; i++) 
            { //for each new child
                auto lCasePart = lCasePartList[i];
                qs = lCasePart.GetName();

                TreeItem* lMshFragmentItem = lMshFragmentsItem->child(i);
                lMshFragmentItem->setData(0, qs);
                lMshFragmentItem->setChecked(true); //default
                QVariant qv = (int)i;
                lMshFragmentItem->setDataPtr(eIDT_MeshComponent, qv);
            }
        }
    }

    {//Results
        TreeItem *lRltsItem = parent->child(1);
        lRltsItem->setData(0, "Results");
        auto nRlts = lCaseRltInfoList.size();
        if (nRlts > 0) {
            lRltsItem->insertChildren(0, (int)nRlts, 1);
            QString qs;
            for (int i = 0; i < nRlts; i++) { //for each new child
                TreeItem *lRltItem = lRltsItem->child(i);
                
                auto rltInfo = lCaseRltInfoList[i];
                std::string ws = rltInfo.GetName();
                qs.sprintf(ws.c_str(), i);
                lRltItem->setData(0, qs);
				lRltItem->setChecked(false); //default
                QVariant qv = (int)i;
                lRltItem->setDataPtr(eIDT_Result, qv);
            }
        }
    }
}
