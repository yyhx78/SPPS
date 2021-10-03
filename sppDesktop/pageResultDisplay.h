
#ifndef pageResultDisplay_H
#define pageResultDisplay_H

#pragma warning( push, 0 )
#include <QComboBox>
#include <QCheckBox>
#include <QWidget.h>
#include <QBasicTimer.h>
#pragma warning(pop)

class pageResultDisplay : public QWidget
{
    Q_OBJECT

public:

  // Constructor/Destructor
    pageResultDisplay(QWidget* parent = NULL, Qt::WindowFlags f = 0);
    ~pageResultDisplay();

    void onResultSelectionChanged();
public slots:
    void slotTimeItemChanged(int);
    void slotTimeValueChanged(int);
    void slotOnCheck_Contour(int);
    void slotOnLineEdit_Finished();
    void slotOnLineEdit_TextChanged(const QString&);

    void slotOnPlay();
    void slotOnPause();
    void slotOnBack();
    void slotOnForward();
protected:
    int m_selectedIndpValue;

    QCheckBox* m_enableIsoCheckBox;
    QLineEdit* m_isoCountLineEdit;
    QComboBox* m_timeComboBox;
    QSlider* m_timeSlider;

    void timerEvent(QTimerEvent *event);
    QBasicTimer *m_pAnimationTimer;

private:
    bool m_isoCountChanged;
    void updateIsoSurfaces();
};

#endif
