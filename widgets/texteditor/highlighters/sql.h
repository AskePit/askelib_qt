#ifndef SQL_HIGHLIGHTER_H
#define SQL_HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>

QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

namespace aske {

class SqlHighlighter : public ::QSyntaxHighlighter
{
    Q_OBJECT

public:
    SqlHighlighter(QTextDocument *parent = 0);

protected:
    virtual void highlightBlock(const QString &text) Q_DECL_OVERRIDE;

    enum class HighlightElement {
        Keyword,
        Comment,
        Literal,
    };

    QMap<HighlightElement, QTextCharFormat> m_colors;
    QStringList m_keywords;
};

} // namespace aske

#endif // SQL_HIGHLIGHTER_H
