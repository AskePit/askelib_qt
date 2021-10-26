#include "ini.h"

namespace aske {

IniHighlighter::IniHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    m_sectionFormat.setForeground(Qt::darkGreen);
    m_keyFormat.setForeground(QColor(120, 120, 255));
    m_commentFormat.setForeground(QColor(30, 130, 160));
}

void IniHighlighter::highlightBlock(const QString &text)
{
    QStringView r(text);
    r = r.trimmed();

    if(r.startsWith('[')) {
        setFormat(0, static_cast<int>(text.size()), m_sectionFormat);
    } else if(r.startsWith(';') || r.startsWith('#')) {
        setFormat(0, static_cast<int>(text.size()), m_commentFormat);
    } else {
        qsizetype i = text.indexOf('=');
        if(i != -1) {
            setFormat(0, i+1, m_keyFormat);
        }
    }
}

} // aske
