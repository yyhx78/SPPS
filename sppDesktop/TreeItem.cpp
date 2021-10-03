
/*
    treeitem.cpp

    A container for items of data supplied by the simple tree model.
*/
#pragma warning( push, 0 )
#include <QStringList>
#pragma warning(pop)

#include "TreeItem.h"

#include <set>
#include "assert.h"
std::set<TreeItem*> curItems;
TreeItem::TreeItem(const QVector<QVariant> &data, TreeItem *parent)
{
    m_parentItem = parent;
    itemData = data;
    itemDataPtr = VPtr<void>::asQVariant(NULL);
	checked = false;

	if (curItems.find(this) == curItems.end())
		curItems.insert(this);
	else
	{
		assert(0);
	}
}

TreeItem::~TreeItem()
{
	assert(curItems.find(this) != curItems.end());
	curItems.erase(this);

    qDeleteAll(childItems);
}

TreeItem *TreeItem::child(int number)
{
    return childItems.value(number);
}

int TreeItem::childCount() const
{
    return childItems.count();
}

int TreeItem::childNumber() const
{
    if (m_parentItem)
	{
		TreeItem *validItem = dynamic_cast<TreeItem*>(m_parentItem);
		if (validItem != NULL)
	        return validItem->childItems.indexOf(const_cast<TreeItem*>(this));
	}

    return 0;
}

int TreeItem::columnCount() const
{
    return itemData.count();
}

QVariant TreeItem::data(int column) const
{
    return itemData.value(column);
}

bool TreeItem::insertChildren(int position, int count, int columns)
{
    if (position < 0 || position > childItems.size())
        return false;
    
    for (int row = 0; row < count; ++row) {
        QVector<QVariant> data(columns);
        TreeItem *item = new TreeItem(data, this);
        childItems.insert(position, item);
    }
    
    return true;
}

TreeItem* TreeItem::addChild(const char *name)
{
    if (TreeItem::insertChildren(childCount(), 1, 1))
    {
        TreeItem *item = child(childCount() - 1); //the last one
        if (name) {
            if (item)
                item->setData(0, name);
        }
        
        return item;
    } else
        return NULL;
}

bool TreeItem::insertColumns(int position, int columns)
{
    if (position < 0 || position > itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
        itemData.insert(position, QVariant());

    foreach (TreeItem *child, childItems)
        child->insertColumns(position, columns);

    return true;
}

TreeItem *TreeItem::parent()
{
    return m_parentItem;
}

bool TreeItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > childItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete childItems.takeAt(position);

    return true;
}

bool TreeItem::removeColumns(int position, int columns)
{
    if (position < 0 || position + columns > itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
        itemData.remove(position);

    foreach (TreeItem *child, childItems)
        child->removeColumns(position, columns);

    return true;
}

bool TreeItem::setData(int column, const QVariant &value)
{
    if (column < 0 || column >= itemData.size())
        return false;

    itemData[column] = value;
    return true;
}

bool TreeItem::setDataPtr(EItemDataType type, const QVariant &value)
{
    itemDataType = type;
    itemDataPtr = value;
    return true;
}
