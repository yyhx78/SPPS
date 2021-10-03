
#ifndef pageMeshDisplay_H
#define pageMeshDisplay_H

#pragma warning( push, 0 )
#include <QWidget.h>
#pragma warning(pop)

class pageMeshDisplay : public QWidget
{
    Q_OBJECT

public:

  // Constructor/Destructor
    pageMeshDisplay(QWidget* parent = NULL, Qt::WindowFlags f = 0);
    ~pageMeshDisplay();

public slots:
	void slotOnCheck_MeshEdges(int);
	void slotOnCheck_FeatureEdges(int);
	void slotOnCheck_GouraodShading(int);
    
    void slotOnR1Clicked();
    void slotOnR2Clicked();
    void slotOnR3Clicked();


protected:
};

#endif
