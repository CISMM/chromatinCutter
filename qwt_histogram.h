#ifndef _HISTO_PLOT_H_

#include <qvector.h>
#include <qwt_plot.h>
#include <histogram_values_passer.h>

class Histogram;

class HistoPlot: public QwtPlot
{
    Q_OBJECT

public:
    HistoPlot(QWidget * = NULL);

public slots:
    // Sets the boundaries of the bins, the left side of the minimum bin and the
    // right side of the maximum bin.
    void setMinX(double min_value);
    void setMaxX(double max_value);

    // Sets the counts within each bin, as well as telling us how many bins there
    // are (there is one per entry in the vector).  This is just a QVector of
    // integer counts.
    void setCounts(const histogram_values_passer counts);

private:
    void createGrid();
    void createOrUpdateHistogram();

    double  d_min_value;
    double  d_max_value;

    Histogram   *d_histogram;   // Our histogram plot

    QVector<int>    d_counts;   // The bins and counts in our histogram plot.
};

#endif
