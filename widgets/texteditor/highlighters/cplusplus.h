#ifndef CPP_HIGHLIGHTER_H
#define CPP_HIGHLIGHTER_H

#include "clike.h"

QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

namespace aske {

class CppHighlighter : public CLikeHighlighter
{
    Q_OBJECT

public:
    CppHighlighter(QTextDocument *parent = 0);
};

} // namespace aske

#endif // CPP_HIGHLIGHTER_H
