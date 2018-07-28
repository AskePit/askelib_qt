#ifndef SHELL_HIGHLIGHTER_H
#define SHELL_HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>

QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

namespace aske {

class ShellHighlighter : public ::QSyntaxHighlighter
{
    Q_OBJECT

public:
    ShellHighlighter(QTextDocument *parent = 0);

protected:
    virtual void highlightBlock(const QString &text) Q_DECL_OVERRIDE;

    enum class HighlightElement {
        Comand,
        Key,
        Comment,
        Var,
        Label,
        Param
    };

    QMap<HighlightElement, QTextCharFormat> m_colors;
};

} // namespace aske

#endif // SHELL_HIGHLIGHTER_H
