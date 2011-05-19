#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>

class nucleosome {
public:
    int location;   // Index of the last base pair wrapped around the nucleosome
    bool attached;  // Stores whether it is attached or not.
};

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent = 0);
    ~GLWidget();

public slots:
    void setMissingHistonePercent(int percent);
    void setNucleosomeSpacingVariance(int variance);
    void setCutsPer3kBasePairs(int cuts);

signals:
    void xRotationChanged(int angle);
    void yRotationChanged(int angle);
    void zRotationChanged(int angle);

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    // Updates the model of histone and cut locations based on
    // the now-current values for the private variables.
    void updateModel(void);

private:
    int totalNucleosomes;
    int missingHistonePercent;
    int nucleosomeSpacingVariance;
    int cutsPer3kBasePairs;
    QPoint lastPos; // Last place the mouse was.

    // This is a list of histone center locations, in base pairs.
    QVector<nucleosome>    nucleosomes;

    // This is a list of cut locations, in base pairs.
    QVector<int>    cutLocations;
};

#endif
