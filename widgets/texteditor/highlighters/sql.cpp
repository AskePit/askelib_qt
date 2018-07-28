#include "sql.h"

#include <QRegularExpression>

namespace aske {

SqlHighlighter::SqlHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    QTextCharFormat KeywordFormat;
    QTextCharFormat CommentFormat;
    QTextCharFormat LiteralFormat;

    KeywordFormat.setForeground(Qt::blue);
    CommentFormat.setForeground(Qt::darkGreen);
    LiteralFormat.setForeground(QColor(0, 0, 170));

    m_colors = {
        { HighlightElement::Keyword, KeywordFormat },
        { HighlightElement::Comment, CommentFormat },
        { HighlightElement::Literal, LiteralFormat },
    };

    //
    // KEYWORDS
    //

    m_keywords = QStringList {
        QStringLiteral("ADD"),
        QStringLiteral("EXCEPT"),
        QStringLiteral("PERCENT"),
        QStringLiteral("ALL"),
        QStringLiteral("EXEC"),
        QStringLiteral("PLAN"),
        QStringLiteral("ALTER"),
        QStringLiteral("EXECUTE"),
        QStringLiteral("PRECISION"),
        QStringLiteral("AND"),
        QStringLiteral("EXISTS"),
        QStringLiteral("PRIMARY"),
        QStringLiteral("ANY"),
        QStringLiteral("EXIT"),
        QStringLiteral("PRINT"),
        QStringLiteral("AS"),
        QStringLiteral("FETCH"),
        QStringLiteral("PROC"),
        QStringLiteral("ASC"),
        QStringLiteral("FILE"),
        QStringLiteral("PROCEDURE"),
        QStringLiteral("AUTHORIZATION"),
        QStringLiteral("FILLFACTOR"),
        QStringLiteral("PUBLIC"),
        QStringLiteral("BACKUP"),
        QStringLiteral("FOR"),
        QStringLiteral("RAISERROR"),
        QStringLiteral("BEGIN"),
        QStringLiteral("FOREIGN"),
        QStringLiteral("READ"),
        QStringLiteral("BETWEEN"),
        QStringLiteral("FREETEXT"),
        QStringLiteral("READTEXT"),
        QStringLiteral("BREAK"),
        QStringLiteral("FREETEXTTABLE"),
        QStringLiteral("RECONFIGURE"),
        QStringLiteral("BROWSE"),
        QStringLiteral("FROM"),
        QStringLiteral("REFERENCES"),
        QStringLiteral("BULK"),
        QStringLiteral("FULL"),
        QStringLiteral("REPLICATION"),
        QStringLiteral("BY"),
        QStringLiteral("FUNCTION"),
        QStringLiteral("RESTORE"),
        QStringLiteral("CASCADE"),
        QStringLiteral("GOTO"),
        QStringLiteral("RESTRICT"),
        QStringLiteral("CASE"),
        QStringLiteral("GRANT"),
        QStringLiteral("RETURN"),
        QStringLiteral("CHECK"),
        QStringLiteral("GROUP"),
        QStringLiteral("REVOKE"),
        QStringLiteral("CHECKPOINT"),
        QStringLiteral("HAVING"),
        QStringLiteral("RIGHT"),
        QStringLiteral("CLOSE"),
        QStringLiteral("HOLDLOCK"),
        QStringLiteral("ROLLBACK"),
        QStringLiteral("CLUSTERED"),
        QStringLiteral("IDENTITY"),
        QStringLiteral("ROWCOUNT"),
        QStringLiteral("COALESCE"),
        QStringLiteral("IDENTITY_INSERT"),
        QStringLiteral("ROWGUIDCOL"),
        QStringLiteral("COLLATE"),
        QStringLiteral("IDENTITYCOL"),
        QStringLiteral("RULE"),
        QStringLiteral("COLUMN"),
        QStringLiteral("IF"),
        QStringLiteral("SAVE"),
        QStringLiteral("COMMIT"),
        QStringLiteral("IN"),
        QStringLiteral("SCHEMA"),
        QStringLiteral("COMPUTE"),
        QStringLiteral("INDEX"),
        QStringLiteral("SELECT"),
        QStringLiteral("CONSTRAINT"),
        QStringLiteral("INNER"),
        QStringLiteral("SESSION_USER"),
        QStringLiteral("CONTAINS"),
        QStringLiteral("INSERT"),
        QStringLiteral("SET"),
        QStringLiteral("CONTAINSTABLE"),
        QStringLiteral("INTERSECT"),
        QStringLiteral("SETUSER"),
        QStringLiteral("CONTINUE"),
        QStringLiteral("INTO"),
        QStringLiteral("SHUTDOWN"),
        QStringLiteral("CONVERT"),
        QStringLiteral("IS"),
        QStringLiteral("SOME"),
        QStringLiteral("CREATE"),
        QStringLiteral("JOIN"),
        QStringLiteral("STATISTICS"),
        QStringLiteral("CROSS"),
        QStringLiteral("KEY"),
        QStringLiteral("SYSTEM_USER"),
        QStringLiteral("CURRENT"),
        QStringLiteral("KILL"),
        QStringLiteral("TABLE"),
        QStringLiteral("CURRENT_DATE"),
        QStringLiteral("LEFT"),
        QStringLiteral("TEXTSIZE"),
        QStringLiteral("CURRENT_TIME"),
        QStringLiteral("LIKE"),
        QStringLiteral("THEN"),
        QStringLiteral("CURRENT_TIMESTAMP"),
        QStringLiteral("LINENO"),
        QStringLiteral("TO"),
        QStringLiteral("CURRENT_USER"),
        QStringLiteral("LOAD"),
        QStringLiteral("TOP"),
        QStringLiteral("CURSOR"),
        QStringLiteral("NATIONAL"),
        QStringLiteral("TRAN"),
        QStringLiteral("DATABASE"),
        QStringLiteral("NOCHECK"),
        QStringLiteral("TRANSACTION"),
        QStringLiteral("DBCC"),
        QStringLiteral("NONCLUSTERED"),
        QStringLiteral("TRIGGER"),
        QStringLiteral("DEALLOCATE"),
        QStringLiteral("NOT"),
        QStringLiteral("TRUNCATE"),
        QStringLiteral("DECLARE"),
        QStringLiteral("NULL"),
        QStringLiteral("TSEQUAL"),
        QStringLiteral("DEFAULT"),
        QStringLiteral("NULLIF"),
        QStringLiteral("UNION"),
        QStringLiteral("DELETE"),
        QStringLiteral("OF"),
        QStringLiteral("UNIQUE"),
        QStringLiteral("DENY"),
        QStringLiteral("OFF"),
        QStringLiteral("UPDATE"),
        QStringLiteral("DESC"),
        QStringLiteral("OFFSETS"),
        QStringLiteral("UPDATETEXT"),
        QStringLiteral("DISK"),
        QStringLiteral("ON"),
        QStringLiteral("USE"),
        QStringLiteral("DISTINCT"),
        QStringLiteral("OPEN"),
        QStringLiteral("USER"),
        QStringLiteral("DISTRIBUTED"),
        QStringLiteral("OPENDATASOURCE"),
        QStringLiteral("VALUES"),
        QStringLiteral("DOUBLE"),
        QStringLiteral("OPENQUERY"),
        QStringLiteral("VARYING"),
        QStringLiteral("DROP"),
        QStringLiteral("OPENROWSET"),
        QStringLiteral("VIEW"),
        QStringLiteral("DUMMY"),
        QStringLiteral("OPENXML"),
        QStringLiteral("WAITFOR"),
        QStringLiteral("DUMP"),
        QStringLiteral("OPTION"),
        QStringLiteral("WHEN"),
        QStringLiteral("ELSE"),
        QStringLiteral("OR"),
        QStringLiteral("WHERE"),
        QStringLiteral("END"),
        QStringLiteral("ORDER"),
        QStringLiteral("WHILE"),
        QStringLiteral("ERRLVL"),
        QStringLiteral("OUTER"),
        QStringLiteral("WITH"),
        QStringLiteral("ESCAPE"),
        QStringLiteral("OVER"),
        QStringLiteral("WRITETEXT"),
    };
}

void SqlHighlighter::highlightBlock(const QString &text)
{
    if(text.startsWith("--")) {
        setFormat(0, text.size(), m_colors[HighlightElement::Comment]);
        return;
    }

    for(const QString &keyword : m_keywords) {
        QRegularExpression expr(QString("\\b%1\\b").arg(keyword), QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatchIterator matchIterator = expr.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), m_colors[HighlightElement::Keyword]);
        }
    }
}

} // aske
