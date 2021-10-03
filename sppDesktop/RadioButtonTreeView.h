#ifndef KBIBTEX_GUI_RADIOBUTTONTREEVIEW_H
#define KBIBTEX_GUI_RADIOBUTTONTREEVIEW_H

#pragma warning( push, 0 )
#include <QTreeView>
#include <QStyledItemDelegate>
#pragma warning( pop)

/**
* @author Thomas Fischer <fischer@unix-ag.uni-kl.de>
*
* This class is a refinement of QTreeView, as it adds support
* for radio buttons for elements in the view.
* To use this view, set RadioButtonItemDelegate as the item delegate
* and use a model that respondes to the roles IsRadioRole and
* RadioSelectedRole. The role IsRadioRole returns a boolean value
* packed in a QVariant if a QModelIndex should have a radio button,
* RadioSelectedRole is boolean value as well, determining if a
* radio button is selected or not.
* This class will take care that if a QModelIndex receives a mouse
* click or a space key press, RadioSelectedRole will be set true for
* this QModelIndex and all sibling indices will be set to false.
*/
class RadioButtonTreeView : public QTreeView
{
    Q_OBJECT
public:
	RadioButtonTreeView(QWidget *parent);

	void uncheckSibling(const QModelIndex &index);
    
signals:

public slots:
    void slotSelectionChanged();

protected:
	void mousePressEvent(QMouseEvent *event);
//	void mouseReleaseEvent(QMouseEvent *event);
//	void keyReleaseEvent(QKeyEvent *event);

    
private:
//	void switchRadioFlag(QModelIndex &index);
};

#endif // KBIBTEX_GUI_RADIOBUTTONTREEVIEW_H
