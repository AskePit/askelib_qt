//! @file

#ifndef MEMORYTEXTEDITOR_H
#define MEMORYTEXTEDITOR_H

#include <std/mask.h>
#include <QPlainTextEdit>
#include "syntax.h"

class QSyntaxHighlighter;

namespace aske {
/*!
 * @brief The TextEditor class
 *
 * @details
 * `QPlainTextEdit` extension which allows 3 modes: Text, Code, Binary.
 * Syntax highlight support.
 * Intelligent "code/text/binary" auto-detection.
 */
class TextEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    //! TextEditor type
    class Type {
    public:
        enum t {
            No   = 0,      //! No content

            Text = 1 << 0, //! Common editor. No lines count, readable font
            Code = 1 << 1, //! Code editor. Lines count, monospaced font
            Hex  = 1 << 2, //! Hex viewer
        };
        static constexpr int count = 3;
        using mask = aske::mask<>; //! Type bitmask
    };

    explicit TextEditor(QWidget *parent = 0);
    explicit TextEditor(Type::mask types = Type::Text, QWidget *parent = 0);

    /*! Set allowed types for text editor.
     *
     * @details
     * Ex.: setTypes(aske::TextEditor::Type::Text | aske::TextEditor::Type::Code)
     */
    void setTypes(Type::mask types);

    /*! Forced switch to text editor type regardless of it's content. */
    void switchToType(Type::t types);

    /*! Load data from `fileName`. Text editor's type will be changed automatically. */
    void openFile(const QString &fileName);

    /*! Save data from text editor to `fileName`. */
    void saveFile(const QString &fileName);

    /*! Save current file.
     *
     * @details
     * This function does nothing if there is no current file.
     */
    void saveFile();

    /*! Types of text editor */
    Type::mask types() { return m_allowedTypes; }

    /*! Current content type */
    Type::t currentType() { return m_currentType; }

public slots:
    /*! This slot should be invoked if text editor's file name was changed. */
    void onFileRenamed(const QString &fileName);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateLook();

    class LineNumberArea : public QWidget
    {
    public:
        LineNumberArea(TextEditor *editor)
            : QWidget(editor)
        {
            textEditor = editor;
        }

        QSize sizeHint() const override {
            return QSize(textEditor->lineNumberAreaWidth(), 0);
        }

    protected:
        void paintEvent(QPaintEvent *event) override {
            textEditor->lineNumberAreaPaintEvent(event);
        }

    private:
        TextEditor *textEditor;
    };

    friend class LineNumberArea;
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);

    void applyHighlighter();
    void applyHighlighter(TextEditorPrivate::Syntax::t syntax);
    void deleteHighlighter();

    LineNumberArea m_lineNumberArea;
    QString m_fileName;

    Type::mask m_allowedTypes {Type::Text | Type::Hex}; //! Types allowed by TextEditor
    Type::t m_currentType {Type::No}; //! Current TextEditorType
    Type::t m_fileType {Type::No}; //! Type of a current file

    QSyntaxHighlighter *m_highlighter {nullptr};
};

} // namespace aske

#endif // MEMORYTEXTEDITOR_H
