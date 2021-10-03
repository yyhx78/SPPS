#ifndef TreeItem_H
#define TreeItem_H

#pragma warning( push, 0 )
#include <QList>
#include <QVariant>
#include <QVector>
#include "vtkObject.h"
#pragma warning(pop)

template <class T> class VPtr
{
public:
    static T* asPtr(QVariant v)
    {
	return  (T *) v.value<void *>();
    }

    static QVariant asQVariant(T* ptr)
    {
	return qVariantFromValue((void *) ptr);
    }
};

enum EItemDataType {
    eIDT_Unknown,
    eIDT_MeshComponents,
    eIDT_MeshComponent,
    eIDT_Result
};

class TreeItem
{
public:
    TreeItem(const QVector<QVariant> &data, TreeItem *parent = 0);
    ~TreeItem();

    TreeItem *child(int number);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    
    bool insertChildren(int position, int count, int columns);
    TreeItem* addChild(const char* name);

    bool insertColumns(int position, int columns);
    TreeItem *parent();
    bool removeChildren(int position, int count);
    bool removeColumns(int position, int columns);
    int childNumber() const;
    bool setData(int column, const QVariant &value);
    bool setDataPtr(EItemDataType type, const QVariant &value);
    QVariant getDataPtr(EItemDataType &type) const
    {
        type = itemDataType;
        return itemDataPtr;
    };

	//for the checkbox
	bool isChecked() const { return checked; };
	void setChecked( bool set ) { checked = set; };
private:
    QList<TreeItem*> childItems;
    
    //in this item
    QVector<QVariant> itemData;
    
    
	QVariant itemDataPtr; //each item represent a data (mesh fragment, or rlt array)
    EItemDataType itemDataType;

    TreeItem *m_parentItem;

	bool checked;
};

#endif
