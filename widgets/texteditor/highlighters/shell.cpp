#include "shell.h"

#include <QRegularExpressionMatchIterator>

namespace aske {

ShellHighlighter::ShellHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    QTextCharFormat CommandFormat;
    QTextCharFormat KeyFormat;
    QTextCharFormat CommentFormat;
    QTextCharFormat VarFormat;
    QTextCharFormat LabelFormat;
    QTextCharFormat ParamFormat;

    CommandFormat.setFontWeight(QFont::Bold);
    CommandFormat.setForeground(Qt::darkMagenta);
    KeyFormat.setForeground(QColor(120, 120, 255));
    CommentFormat.setForeground(QColor(30, 130, 160));
    VarFormat.setForeground(QColor(85, 140, 46));
    LabelFormat.setForeground(Qt::blue);
    LabelFormat.setFontWeight(QFont::Bold);
    ParamFormat.setForeground(QColor(0, 103, 124));

    m_colors = {
        { HighlightElement::Comand, CommandFormat },
        { HighlightElement::Key, KeyFormat },
        { HighlightElement::Comment, CommentFormat },
        { HighlightElement::Var, VarFormat },
        { HighlightElement::Label, LabelFormat },
        { HighlightElement::Param, ParamFormat },
    };
}

void ShellHighlighter::highlightBlock(const QString &text_)
{
    if(text_.startsWith('#') || text_.startsWith("rem ", Qt::CaseInsensitive)) {
        setFormat(0, text_.size(), m_colors[HighlightElement::Comment]);
        return;
    }

    QStringRef text(&text_);
    auto comands = text.split('|', QString::SkipEmptyParts);

    for(const QStringRef &comand : comands) {
        int i = 0;
        while(i < comand.size() && comand.at(i).isSpace()) { // eat whitespace
            ++i;
        }
        if(comand[0] == ':') {
            break;
        }
        while(i < comand.size() && !comand.at(i).isSpace()) { // eat first word
            ++i;
        }

        setFormat(comand.position(), i, m_colors[HighlightElement::Comand]);
    }

    auto doRegexp = [&text_, this](HighlightElement el, const QString &regexp){
        QRegularExpressionMatchIterator matchIterator = QRegularExpression(regexp).globalMatch(text_);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), m_colors[el]);
        }
    };

    doRegexp(HighlightElement::Key, R"(-{1,2}[\w\d_]+)");
    doRegexp(HighlightElement::Var, R"((\$|%{1,2})[\w\d_~]+%?)");
    doRegexp(HighlightElement::Label, R"([\s]*:[\w\d_]+)");
    doRegexp(HighlightElement::Param, R"(\s/[\w\d]+)");
}

} // aske
