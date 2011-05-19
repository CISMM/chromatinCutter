#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>
#include "histogram_values_passer.h"

class nucleosome {
public:
    long location;  // Index of the last base pair wrapped around the nucleosome
    bool attached;  // Stores whether it is attached or not.

    const bool operator < (const nucleosome &n) const { return location < n.location; };
    const bool operator < (const long num) const { return location < num; };
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
    void newMinHistogramValue(double val);
    void newMaxHistogramValue(double val);
    void newHistogramCounts(histogram_values_passer);
    void newVersionLabel(QString);

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    // Updates the model of histone and cut locations based on
    // the now-current values for the private variables.
    void updateModel(void);

    // Updates the statistics based on the model.  This reports
    // the new values through signals.
    void updateStatistics(void);

private:
    int bpPerNucleosome;
    int bpPerLinker;
    int totalNucleosomes;
    int missingHistonePercent;
    int nucleosomeSpacingVariance;
    int cutsPer3kBasePairs;
    QPoint lastPos; // Last place the mouse was.

    // This is a list of histone center locations, in base pairs.
    // Then a function to tell whether a specified base-pair location
    // can be cut (it can if it is not inside a nuclesome section).
    QVector<nucleosome>    nucleosomes;
    bool validCutLocation(long loc);

    // This is a list of cut locations, in base pairs.
    QVector<long>    cutLocations;
};

#endif
