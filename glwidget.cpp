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
    bpPerLinker = 20;
    bpPerNucleosome = 146;

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
    // DNA bpPerNucleosome base-pairs long to wrap around it to form a nucleosome.
    // The length of DNA between each wrapped nucleosome is on average
    // bpPerLinker base-pairs long.  When we add a new nucleosome, we do so by
    // figuring out how long the linker is from the last one (must be
    // at least 1 base-pair long) and add it to the last nucleosome
    // index, then we add a whole nucleosome length and locate the
    // new nucleosome there.
    int i;
    long last_location = 0;
    for (i = 0; i < totalNucleosomes; i++) {

        // Select a linker length.  It will Gaussian distributed based on
        // the variance, with a mean at the specified linker length.  If the
        // length is less than 1, we pick another random number to avoid
        // this case.  Also, if it tries to go above twice the linker length
        // then we reject it; this will avoid increasing the mean separation.
        int linker_length = bpPerLinker; // XXX Will be based on variance
        if (nucleosomeSpacingVariance > 0) do {
            linker_length = bpPerLinker + random_normal_sample() * sqrt(nucleosomeSpacingVariance);
        } while ((linker_length <= 0) || (linker_length >= 2*bpPerLinker));

        // Add the length onto the existing DNA strand and put a nucleosome
        // there.
        int add_length = linker_length + bpPerNucleosome;
        long location = last_location + add_length;
        nucleosome n;
        n.location = location;
        n.attached = true;
        nucleosomes.push_back(n);
        last_location = n.location;
    }

    // Sort the list of nucleosomes by location (this should already be in order,
    // but we make sure).
    qSort(nucleosomes);

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

    // Select locations for the cuts.  First figure out how many there are total and then
    // put them all in.  Don't allow cuts that would fall within a wrapped nucleosome.
    // We compute the number of base pairs by multiplying the expected amount of DNA
    // per nucleosome (including linker) by the number of nucleosomes.
    long num_bps = (bpPerNucleosome + bpPerLinker) * totalNucleosomes;
    int num_cuts = (num_bps/3.0e3) * cutsPer3kBasePairs;
    for (i = 0; i < num_cuts; i++) {

        // Keep trying until we find a valid cut location
        long try_cut;
        do {
            try_cut = random_0_1() * num_bps;
        } while (!validCutLocation(try_cut));
        cutLocations.push_back(try_cut);
    }

    // Sort the cut locations, to make it faster to process them during graphics and
    // histogram formation.
    qSort(cutLocations);
}

// Returns true if the specified location is a valid cut location
// (a base pair that is not inside a wrapped nucleosome) and false if
// it is inside a wrapped nucleosome.
bool GLWidget::validCutLocation(long loc)
{
    // Find the first nucleosome that is at a location that is equal to
    // or larger than the location.
    nucleosome  which;
    which.location = loc;
    QVector<nucleosome>::ConstIterator i = qLowerBound(nucleosomes, which);

    // If there is one, then see if we're within the wrapped base-pair region.
    // Then see if this histone is wrapped.
    // If so, return false.
    if ( (i != nucleosomes.end()) && (*i).attached ) {
        long nuc_loc = (*i).location;
        if (nuc_loc - bpPerNucleosome <= loc) {
            return false;
        }
    }

    // We're in the clear.
    return true;
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

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_MULTISAMPLE);
    static GLfloat lightPosition[4] = { 0.5, 5.0, 7.0, 1.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    // Makes the colors for the primitives be what we want.
    glDisable(GL_LIGHTING);
}

void GLWidget::paintGL()
{
    int i;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -10.0);

    // Set up rendering state.
    glPointSize(5.0);
    glDisable(GL_TEXTURE_2D);

    // Set the base-pair to full-screen scale.
    glScalef(1.0/bpPerNucleosome, 1.0/bpPerNucleosome, 1.0);

//    float scale = 1.0 / log(cutsPer3kBasePairs+9);
//    glScalef(scale,scale,scale);

    // Draw the model, showing the linkers, the nucleosomes, and the cuts for the
    // first part of the chain to give an idea of what it looks like.
    // The window is 1 unit high and however wide the aspect ratio requires.
    // Set the scale so that the bpPerNucleosome base pairs span from the top of the display
    // to the bottom.
    int last_bp = 0;    // Last base-pair location drawn.
    int last_sl = 0;    // Screen location of this last pair.
    int next_cut_index = 0; // Index of the next cut location to draw.
    for (i = 0; i < nucleosomes.size(); i++) {

        // Draw the line from the previous nucleosome to this one.
        glColor3f(1.0, 1.0, 1.0);
        int inc_bp = nucleosomes[i].location - last_bp;
        int new_sl = last_sl + inc_bp - bpPerNucleosome;
        glBegin(GL_LINES);
            glVertex2f(last_sl, 0);
            glVertex2f(new_sl, 0);
        glEnd();

        // Draw this nucleosome, either in wrapped form or unwrapped.
        glColor3f(0.3, 1.0, 0.3);
        if (nucleosomes[i].attached) {
            glBegin(GL_POINTS);
                glVertex2f(new_sl, 0);
            glEnd();
        } else {
            glBegin(GL_LINES);
                glVertex2f(new_sl, bpPerNucleosome/2.0);
                glVertex2f(new_sl, -bpPerNucleosome/2.0);
            glEnd();
        }

        // Draw any cut lines that fall between the last and the
        // current base-pair value.
        glColor3f(1.0, 0.3, 0.3);
        while ( (next_cut_index < cutLocations.size())
                && (cutLocations[next_cut_index] <= last_bp + inc_bp) ) {

            // If this cut is before the start of the nucleosome, then we
            // draw it vertically across the DNA.
            float halfcut = bpPerLinker/2.0;    // Half length of cut line
            if (last_bp + inc_bp - bpPerNucleosome > cutLocations[next_cut_index]) {
                long xloc = last_sl + (cutLocations[next_cut_index] - last_bp);
                glBegin(GL_LINES);
                    glVertex3f(xloc, halfcut, 1.0);
                    glVertex3f(xloc, -halfcut, 1.0);
                glEnd();
            } else {
                // If this is cut within the chromosome, then draw it horizontally
                // the fraction of the way from the bottom of the screen to the top
                // that it is along the nucleosomal DNA (this is an abstract representation).
                long yloc = -bpPerNucleosome/2 + (last_bp + inc_bp - cutLocations[next_cut_index]);
                glBegin(GL_LINES);
                    glVertex3f(new_sl - halfcut, yloc, 1.0);
                    glVertex3f(new_sl + halfcut, yloc, 1.0);
                glEnd();
            }

            next_cut_index++;
        }

        last_bp += inc_bp;
        last_sl = new_sl;
    }
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
