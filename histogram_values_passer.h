// This file is a horrible hack to let us use Qt Designer to create the object
// of a type needed to pass values to the histogram.  It basically encapsulates
// a templated class.

#ifndef _HISTOGRAM_VALUES_PASSER_H_
#define _HISTOGRAM_VALUES_PASSER_H_

#include <qvector.h>

class histogram_values_passer: public QVector<int>
{
};

#endif
