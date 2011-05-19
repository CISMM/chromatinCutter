#include <QtGui>
#include <QtOpenGL>
#include <math.h>

#include "glwidget.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    missingHistonePercent = 0;
    nucleosomeSpacingVariance = 0;
    cutsPer3kBasePairs = 1;

    qtGreen = QColor::fromCmykF(0.40, 0.0, 1.0, 0.0);
    qtPurple = QColor::fromCmykF(0.39, 0.39, 0.0, 0.0);
}

GLWidget::~GLWidget()
{
}

void GLWidget::setMissingHistonePercent(int percent)
{
    missingHistonePercent = percent;
    updateGL();
}

void GLWidget::setNucleosomeSpacingVariance(int variance)
{
    nucleosomeSpacingVariance = variance;
    updateGL();
}

void GLWidget::setCutsPer3kBasePairs(int cuts)
{
    cutsPer3kBasePairs = cuts;
    updateGL();
}

void GLWidget::initializeGL()
{
    qglClearColor(qtPurple.dark());

    // Construct the model here.
    // XXX

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_MULTISAMPLE);
    static GLfloat lightPosition[4] = { 0.5, 5.0, 7.0, 1.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -10.0);
    float scale = 1.0 / log(cutsPer3kBasePairs+9);
    float width = 1 + missingHistonePercent / 100.0;
    float height = 1 + nucleosomeSpacingVariance / 225.0;
    glScalef(scale,scale,scale);
    glBegin(GL_QUADS);
        glVertex3d(-width, -height, 0.0);
        glVertex3d( width, -height, 0.0);
        glVertex3d( width,  height, 0.0);
        glVertex3d(-width,  height, 0.0);
    glEnd();
}

void GLWidget::resizeGL(int width, int height)
{
//    int side = qMin(width, height);
//    glViewport((width - side) / 2, (height - side) / 2, side, side);
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Make the window one unit high (-0.5 to 0.5) and have an aspect ratio that matches
    // the aspect ratio of the window.
    float aspect;
    if ((height <= 0) || (width < 0)) {
        aspect = 1.0;
    } else {
        aspect = static_cast<float>(width)/height;
    }
    glOrtho(-aspect/2, aspect/2, -0.5, 0.5, 5.0, 15.0);
    glMatrixMode(GL_MODELVIEW);
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        // XXX
    } else if (event->buttons() & Qt::RightButton) {
        // XXX
    }
    lastPos = event->pos();
}
