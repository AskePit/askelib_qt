#ifndef INI_HIGHLIGHTER_H
#define INI_HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>

QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

namespace aske {

class IniHighlighter : public ::QSyntaxHighlighter
{
    Q_OBJECT

public:
    IniHighlighter(QTextDocument *parent = 0);

protected:
    virtual void highlightBlock(const QString &text) Q_DECL_OVERRIDE;

    QTextCharFormat m_sectionFormat;
    QTextCharFormat m_keyFormat;
    QTextCharFormat m_commentFormat;
};


} // namespace aske

#endif // INI_HIGHLIGHTER_H
