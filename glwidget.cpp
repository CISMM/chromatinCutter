#include <QtGui>
#include <QtOpenGL>
#include <QColor>
#include <math.h>
// For rand()
#include <cstdlib>

#include "glwidget.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

//----------------------------------------------------------------------
// Helper functions

static double random_0_1(void)
{
  return static_cast<double>(rand())/RAND_MAX;
}

// Polar method for normal density discussed in Knuth

static double random_normal_sample(void)
{
  double u1, u2, v1, v2;
  double S = 2;
  while (S >= 1) {
    u1 = random_0_1();
    u2 = random_0_1();
    v1 = 2*u1 - 1;
    v2 = 2*u2 - 1;
    S = pow(v1, 2) + pow(v2, 2);
  };
  return v1*sqrt( (-2*log(S))/S );
}

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    missingHistonePercent = 0;
    nucleosomeSpacingVariance = 0;
    cutsPer3kBasePairs = 1;

    // How many nucleosomes to add to our model.
    totalNucleosomes = 1000;

    // Set the initial state of the model.
    updateModel();
}

GLWidget::~GLWidget()
{
}

void GLWidget::updateModel(void)
{
    // Clear the lists of nucleosome locations and cut locations
    // in preparation to making a new model.
    nucleosomes.clear();
    cutLocations.clear();

    // Add nucleosomes into the model.  Each histone causes a chain of
    // DNA 146 base-pairs long to wrap around it to form a nucleosome.
    // The length of DNA between each wrapped nucleosome is on average
    // 20 base-pairs long.  When we add a new nucleosome, we do so by
    // figuring out how long the linker is from the last one (must be
    // at least 1 base-pair long) and add it to the last nucleosome
    // index, then we add a whole nucleosome length and locate the
    // new nucleosome there.
    int i;
    int last_location = 0;
    for (i = 0; i < totalNucleosomes; i++) {
        int linker_length = 20; // XXX Will be based on variance
        int add_length = linker_length + 146;
        int location = last_location + add_length;
        nucleosome n;
        n.location = location;
        n.attached = true;
        nucleosomes.push_back(n);
        last_location = n.location;
    }

    // Figure out which nucleosomes are detached.  We do this by
    // randomly removing them until the specified percent is unattached.
    // if they are all to be detached, we just do that without randomness.
    int num_to_remove = static_cast<int>(totalNucleosomes*(missingHistonePercent/100.0));
    if (num_to_remove >= nucleosomes.size()) {
        for (i = 0; i < nucleosomes.size(); i++) {
            nucleosomes[i].attached = false;
        }
    } else {
        for (i = 0; i < num_to_remove; i++) {
            // Select one at random until we find one that is not yet removed.
            // Then remove it.
            int which;
            do {
                which = static_cast<int>((nucleosomes.size()-1) * random_0_1());
            } while (nucleosomes[which].attached == false);
            nucleosomes[which].attached = false;
        }
    }

}

void GLWidget::setMissingHistonePercent(int percent)
{
    missingHistonePercent = percent;
    updateModel();
    updateGL();
}

void GLWidget::setNucleosomeSpacingVariance(int variance)
{
    nucleosomeSpacingVariance = variance;
    updateModel();
    updateGL();
}

void GLWidget::setCutsPer3kBasePairs(int cuts)
{
    cutsPer3kBasePairs = cuts;
    updateModel();
    updateGL();
}

void GLWidget::initializeGL()
{
    qglClearColor(Qt::black);

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
    int i;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -10.0);

    // Set up rendering state.
    glPointSize(3.0);
    glDisable(GL_TEXTURE_2D);

    // Set the base-pair to full-screen scale.
    glScalef(0.01, 0.01, 1.0);

    // Draw the model, showing the linkers, the histones, and the cuts for the
    // first part of the chain to give an idea of what it looks like.
    // The window is 1 unit high and however wide the aspect ratio requires.
    int last_bp = 0;    // Last base-pair location drawn.
    int last_sl = 0;    // Screen location of this last pair.
    for (i = 0; i < nucleosomes.size(); i++) {
        glColor3f(1.0, 1.0, 1.0);
        int new_bp = nucleosomes[i].location - 146 - last_bp;
        glBegin(GL_LINES);
            glVertex2f(last_bp, 0);
            glVertex2f(new_bp, 0);
        glEnd();

        glColor3f(1.0, 0.0, 0.0);
        glBegin(GL_POINTS);
            glVertex2f(new_bp, 0);
        glEnd();

        last_bp = new_bp;
    }

    // XXX Drawing stuff that was for debugging.
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
    // the aspect ratio of the window.  We also make the left side of the window be at
    // the origin.
    float aspect;
    if ((height <= 0) || (width < 0)) {
        aspect = 1.0;
    } else {
        aspect = static_cast<float>(width)/height;
    }
    glOrtho(0, aspect, -0.5, 0.5, 5.0, 15.0);
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
