

#ifndef TreeModel_H
#define TreeModel_H

#pragma warning( push, 0 )
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include "vtkObject.h"
#pragma warning(pop)

#include "TreeItem.h"
#include "dataModel/Doc.h"

class vtkPlane;

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    TreeModel(QObject *parent = 0);
    ~TreeModel();

    QVariant data(const QModelIndex &index, int role) const;
    TreeItem* item(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole);
    bool setHeaderData(int section, Qt::Orientation orientation,
                       const QVariant &value, int role = Qt::EditRole);

    bool insertColumns(int position, int columns,
                       const QModelIndex &parent = QModelIndex());
    bool removeColumns(int position, int columns,
                       const QModelIndex &parent = QModelIndex());
    bool insertRows(int position, int rows,
                    const QModelIndex &parent = QModelIndex());
    bool removeRows(int position, int rows,
                    const QModelIndex &parent = QModelIndex());
    
    void setupTree();
    
signals:
    void itemCheckStatusChanged(const QModelIndex &);
    
private:
    
    TreeItem *getItem(const QModelIndex &index) const;
    TreeItem *m_rootItem;
    
    TreeItem *m_cuttingPlaneGroupItem;//for easy add/remove clipping planes
};

#endif
