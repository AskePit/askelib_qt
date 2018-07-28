#ifndef RUST_HIGHLIGHTER_H
#define RUST_HIGHLIGHTER_H

#include "clike.h"

QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

namespace aske {

class RustHighlighter : public CLikeHighlighter
{
    Q_OBJECT

public:
    RustHighlighter(QTextDocument *parent = 0);

protected:
    virtual void highlightBlock(const QString &text) Q_DECL_OVERRIDE;
    QTextCharFormat m_macrosFormat;
};

} // namespace aske

#endif // RUST_HIGHLIGHTER_H
