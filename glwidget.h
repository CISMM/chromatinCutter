#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>

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

private:
    int missingHistonePercent;
    int nucleosomeSpacingVariance;
    int cutsPer3kBasePairs;
    QPoint lastPos;
    QColor qtGreen;
    QColor qtPurple;
};

#endif
