#include <stdlib.h>
#include <qpen.h>
#include <qwt_plot_layout.h>
#include <qwt_legend.h>
#include <qwt_legend_item.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_histogram.h>
#include <qwt_column_symbol.h>
#include <qwt_series_data.h>
#include <qwt_scale_map.h>
#include <qwt_scale_engine.h>
#include "qwt_histogram.h"

class Histogram: public QwtPlotHistogram
{
public:
    Histogram(const QString &, const QColor &,
              double min_x, double max_x);

    void setColor(const QColor &);
    void setValues(uint numValues, const double *);

private:
    double  d_min_x;    // Left-hand border
    double  d_max_x;    // Right-hand border
};

Histogram::Histogram(const QString &title, const QColor &symbolColor,
                     double min_x, double max_x):
    QwtPlotHistogram(title)
    , d_min_x(min_x)
    , d_max_x(max_x)
{
    setStyle(QwtPlotHistogram::Columns);

    setColor(symbolColor);
}

void Histogram::setColor(const QColor &symbolColor)
{
    QColor color = symbolColor;
    color.setAlpha(180);

    setPen(QPen(Qt::black));
    setBrush(QBrush(color));

    QwtColumnSymbol *symbol = new QwtColumnSymbol(QwtColumnSymbol::Box);
    symbol->setFrameStyle(QwtColumnSymbol::Raised);
    symbol->setLineWidth(2);
    symbol->setPalette(QPalette(color));
    setSymbol(symbol);
}

void Histogram::setValues(uint numValues, const double *values)
{
    double step_size = (d_max_x - d_min_x) / numValues;
    QVector<QwtIntervalSample> samples(numValues);
    for ( uint i = 0; i < numValues; i++ )
    {
        QwtInterval interval(double(d_min_x + i*step_size), d_min_x + (i+1)*step_size);
        interval.setBorderFlags(QwtInterval::ExcludeMaximum);
        
        samples[i] = QwtIntervalSample(values[i], interval);
    }

    setData(new QwtIntervalSeriesData(samples));
}

HistoPlot::HistoPlot(QWidget *parent)
    : QwtPlot(parent)
    , d_min_value(100)
    , d_max_value(200)
    , d_histogram(NULL)
{
    setTitle("Gel Density");

    setCanvasBackground(QColor(Qt::gray));
    plotLayout()->setAlignCanvasToScales(true);

    setAxisTitle(QwtPlot::yLeft, "Counts");
    setAxisTitle(QwtPlot::xBottom, "Base Pairs");
    setAxisScaleEngine(QwtPlot::xBottom, new QwtLog10ScaleEngine);

    // Fill in the counts vector with what we show by default,
    // which is nothing.  When we get a report telling us about
    // counts, then we want to redraw.

    createGrid();
    createOrUpdateHistogram();
    // replot();

    setAutoReplot(true);
}

void HistoPlot::createGrid()
{
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableX(false);
    grid->enableY(true);
    grid->enableXMin(false);
    grid->enableYMin(false);
    grid->setMajPen(QPen(Qt::black, 0, Qt::DotLine));
    grid->attach(this);
}

void HistoPlot::createOrUpdateHistogram()
{
    // If we already have a histogram, get rid of it.
    if (d_histogram) {
        d_histogram->detach();
        delete d_histogram;
        d_histogram = NULL;
    }

    // Create a new histogram based on our data.  We put the
    // values into a double array and then pass the array and its
    // counts into the Histogram to be displayed.
    if (d_counts.size() > 0) {
        double *vals = new double[d_counts.size()];
        int i;
        for (i = 0; i < d_counts.size(); i++) {
            vals[i] = d_counts[i];
        }

        d_histogram = new Histogram("Ignored", Qt::red, d_min_value, d_max_value);
        d_histogram->setValues(d_counts.size(), vals);
        d_histogram->attach(this);

        delete [] vals;
    }
}

void HistoPlot::setMinX(double min_value)
{
    d_min_value = min_value;
    createOrUpdateHistogram();
}

void HistoPlot::setMaxX(double max_value)
{
    d_max_value = max_value;
    createOrUpdateHistogram();
}

void HistoPlot::setCounts(const histogram_values_passer counts)
{
    d_counts = counts;
    //printf("dbg: Got %d counts in HistoPlot::setCounts()\n", counts.size());
    createOrUpdateHistogram();
}
