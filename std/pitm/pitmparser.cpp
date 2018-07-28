/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Copyright (C) 2016 Intel Corporation.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QT_BOOTSTRAPPED
#include <qcoreapplication.h>
#endif
#include <qdebug.h>
#include "pitmparser_p.h"
#include "pitm_p.h"
#include "qutfcodec_p.h"

//#define PARSER_DEBUG
#ifdef PARSER_DEBUG
static int indent = 0;
#define BEGIN qDebug() << QByteArray(4*indent++, ' ').constData() << "pos=" << current
#define END --indent
#define DEBUG qDebug() << QByteArray(4*indent, ' ').constData()
#else
#define BEGIN if (1) ; else qDebug()
#define END do {} while (0)
#define DEBUG if (1) ; else qDebug()
#endif

static const int nestingLimit = 1024;

QT_BEGIN_NAMESPACE

// error strings for the PITM parser
#define PITMERR_OK          QT_TRANSLATE_NOOP("PitmParseError", "no error occurred")
#define PITMERR_UNTERM_OBJ  QT_TRANSLATE_NOOP("PitmParseError", "unterminated object")
#define PITMERR_MISS_NSEP   QT_TRANSLATE_NOOP("PitmParseError", "missing name separator")
#define PITMERR_UNTERM_AR   QT_TRANSLATE_NOOP("PitmParseError", "unterminated array")
#define PITMERR_MISS_VSEP   QT_TRANSLATE_NOOP("PitmParseError", "missing value separator")
#define PITMERR_ILLEGAL_VAL QT_TRANSLATE_NOOP("PitmParseError", "illegal value")
#define PITMERR_END_OF_NUM  QT_TRANSLATE_NOOP("PitmParseError", "invalid termination by number")
#define PITMERR_ILLEGAL_NUM QT_TRANSLATE_NOOP("PitmParseError", "illegal number")
#define PITMERR_STR_ESC_SEQ QT_TRANSLATE_NOOP("PitmParseError", "invalid escape sequence")
#define PITMERR_STR_UTF8    QT_TRANSLATE_NOOP("PitmParseError", "invalid UTF8 string")
#define PITMERR_UTERM_STR   QT_TRANSLATE_NOOP("PitmParseError", "unterminated string")
#define PITMERR_MISS_OBJ    QT_TRANSLATE_NOOP("PitmParseError", "object is missing after a comma")
#define PITMERR_DEEP_NEST   QT_TRANSLATE_NOOP("PitmParseError", "too deeply nested document")
#define PITMERR_DOC_LARGE   QT_TRANSLATE_NOOP("PitmParseError", "too large document")
#define PITMERR_GARBAGEEND  QT_TRANSLATE_NOOP("PitmParseError", "garbage at the end of the document")

/*!
    \class PitmParseError
    \inmodule QtCore
    \ingroup pitm
    \ingroup shared
    \reentrant
    \since 5.0

    \brief The PitmParseError class is used to report errors during PITM parsing.

    \sa {PITM Support in Qt}, {PITM Save Game Example}
*/

/*!
    \enum PitmParseError::ParseError

    This enum describes the type of error that occurred during the parsing of a PITM document.

    \value NoError                  No error occurred
    \value UnterminatedObject       An object is not correctly terminated with a closing curly bracket
    \value MissingNameSeparator     A comma separating different items is missing
    \value UnterminatedArray        The array is not correctly terminated with a closing square bracket
    \value MissingValueSeparator    A colon separating keys from values inside objects is missing
    \value IllegalValue             The value is illegal
    \value TerminationByNumber      The input stream ended while parsing a number
    \value IllegalNumber            The number is not well formed
    \value IllegalEscapeSequence    An illegal escape sequence occurred in the input
    \value IllegalUTF8String        An illegal UTF8 sequence occurred in the input
    \value UnterminatedString       A string wasn't terminated with a quote
    \value MissingObject            An object was expected but couldn't be found
    \value DeepNesting              The PITM document is too deeply nested for the parser to parse it
    \value DocumentTooLarge         The PITM document is too large for the parser to parse it
    \value GarbageAtEnd             The parsed document contains additional garbage characters at the end

*/

/*!
    \variable PitmParseError::error

    Contains the type of the parse error. Is equal to PitmParseError::NoError if the document
    was parsed correctly.

    \sa ParseError, errorString()
*/


/*!
    \variable PitmParseError::offset

    Contains the offset in the input string where the parse error occurred.

    \sa error, errorString()
*/

/*!
  Returns the human-readable message appropriate to the reported PITM parsing error.

  \sa error
 */
QString PitmParseError::errorString() const
{
    const char *sz = "";
    switch (error) {
    case NoError:
        sz = PITMERR_OK;
        break;
    case UnterminatedObject:
        sz = PITMERR_UNTERM_OBJ;
        break;
    case MissingNameSeparator:
        sz = PITMERR_MISS_NSEP;
        break;
    case UnterminatedArray:
        sz = PITMERR_UNTERM_AR;
        break;
    case MissingValueSeparator:
        sz = PITMERR_MISS_VSEP;
        break;
    case IllegalValue:
        sz = PITMERR_ILLEGAL_VAL;
        break;
    case TerminationByNumber:
        sz = PITMERR_END_OF_NUM;
        break;
    case IllegalNumber:
        sz = PITMERR_ILLEGAL_NUM;
        break;
    case IllegalEscapeSequence:
        sz = PITMERR_STR_ESC_SEQ;
        break;
    case IllegalUTF8String:
        sz = PITMERR_STR_UTF8;
        break;
    case UnterminatedString:
        sz = PITMERR_UTERM_STR;
        break;
    case MissingObject:
        sz = PITMERR_MISS_OBJ;
        break;
    case DeepNesting:
        sz = PITMERR_DEEP_NEST;
        break;
    case DocumentTooLarge:
        sz = PITMERR_DOC_LARGE;
        break;
    case GarbageAtEnd:
        sz = PITMERR_GARBAGEEND;
        break;
    }
#ifndef QT_BOOTSTRAPPED
    return QCoreApplication::translate("PitmParseError", sz);
#else
    return QLatin1String(sz);
#endif
}

using namespace PitmPrivate;

Parser::Parser(const char *pitm, int length)
    : head(pitm), pitm(pitm), data(0), dataLength(0), current(0), nestingLevel(0), lastError(PitmParseError::NoError)
{
    end = pitm + length;
}



/*

begin-array     = ws %x5B ws  ; [ left square bracket

begin-object    = ws %x7B ws  ; { left curly bracket

end-array       = ws %x5D ws  ; ] right square bracket

end-object      = ws %x7D ws  ; } right curly bracket

name-separator  = ws %x3A ws  ; : colon

value-separator = ws %x2C ws  ; , comma

Insignificant whitespace is allowed before or after any of the six
structural characters.

ws = *(
          %x20 /              ; Space
          %x09 /              ; Horizontal tab
          %x0A /              ; Line feed or New line
          %x0D                ; Carriage return
      )

*/

enum {
    Space = 0x20,
    Tab = 0x09,
    LineFeed = 0x0a,
    Return = 0x0d,
    BeginArray = 0x5b,
    BeginObject = 0x7b,
    EndArray = 0x5d,
    EndObject = 0x7d,
    NameSeparator = 0x3a,
    ValueSeparator = 0x2c,
    Quote = 0x22
};

void Parser::eatBOM()
{
    // eat UTF-8 byte order mark
    uchar utf8bom[3] = { 0xef, 0xbb, 0xbf };
    if (end - pitm > 3 &&
        (uchar)pitm[0] == utf8bom[0] &&
        (uchar)pitm[1] == utf8bom[1] &&
        (uchar)pitm[2] == utf8bom[2])
        pitm += 3;
}

bool Parser::eatSpace()
{
    while (pitm < end) {
        if (*pitm > Space)
            break;
        if (*pitm != Space &&
            *pitm != Tab &&
            *pitm != LineFeed &&
            *pitm != Return)
            break;
        ++pitm;
    }
    return (pitm < end);
}

char Parser::nextToken()
{
    if (!eatSpace())
        return 0;
    char token = *pitm;
    switch (token) {
    case BeginArray:
    case BeginObject:
    case NameSeparator:
    case ValueSeparator:
    case EndArray:
    case EndObject:
    case Quote:
        ++pitm;
        break;
    default:
        break;
    }
    return token;
}

/*
    PITM-text = object / array
*/
PitmDocument Parser::parse(PitmParseError *error)
{
#ifdef PARSER_DEBUG
    indent = 0;
    qDebug(">>>>> parser begin");
#endif
    // allocate some space
    dataLength = qMax(end - pitm, (ptrdiff_t) 256);
    data = (char *)malloc(dataLength);

    // fill in Header data
    PitmPrivate::Header *h = (PitmPrivate::Header *)data;
    h->tag = PitmDocument::BinaryFormatTag;
    h->version = 1u;

    current = sizeof(PitmPrivate::Header);

    eatBOM();
    char token = nextToken();

    DEBUG << hex << (uint)token;
    if (token == BeginArray) {
        if (!parseArray())
            goto error;
    } else if (token == BeginObject) {
        if (!parseObject())
            goto error;
    } else {
        lastError = PitmParseError::IllegalValue;
        goto error;
    }

    eatSpace();
    if (pitm < end) {
        lastError = PitmParseError::GarbageAtEnd;
        goto error;
    }

    END;
    {
        if (error) {
            error->offset = 0;
            error->error = PitmParseError::NoError;
        }
        PitmPrivate::Data *d = new PitmPrivate::Data(data, current);
        return PitmDocument(d);
    }

error:
#ifdef PARSER_DEBUG
    qDebug(">>>>> parser error");
#endif
    if (error) {
        error->offset = pitm - head;
        error->error  = lastError;
    }
    free(data);
    return PitmDocument();
}


void Parser::ParsedObject::insert(uint offset) {
    const PitmPrivate::Entry *newEntry = reinterpret_cast<const PitmPrivate::Entry *>(parser->data + objectPosition + offset);
    int min = 0;
    int n = offsets.size();
    while (n > 0) {
        int half = n >> 1;
        int middle = min + half;
        if (*entryAt(middle) >= *newEntry) {
            n = half;
        } else {
            min = middle + 1;
            n -= half + 1;
        }
    }
    if (min < offsets.size() && *entryAt(min) == *newEntry) {
        offsets[min] = offset;
    } else {
        offsets.insert(min, offset);
    }
}

/*
    object = begin-object [ member *( value-separator member ) ]
    end-object
*/

static bool isFirstKeyChar(char c)
{
    return isalpha(c) || c == '_';
}

static bool isKeyChar(char c)
{
    return isalpha(c) || isdigit(c) || c == '_';
}

bool Parser::parseObject()
{
    if (++nestingLevel > nestingLimit) {
        lastError = PitmParseError::DeepNesting;
        return false;
    }

    int objectOffset = reserveSpace(sizeof(PitmPrivate::Object));
    if (objectOffset < 0)
        return false;
    BEGIN << "parseObject pos=" << objectOffset << current << pitm;

    ParsedObject parsedObject(this, objectOffset);

    char token = nextToken();
    while (isFirstKeyChar(token)) {
        int off = current - objectOffset;
        if (!parseMember(objectOffset))
            return false;
        parsedObject.insert(off);
        token = nextToken();
        if (token == EndObject) {
            break;
        }
    }

    DEBUG << "end token=" << token;
    if (token != EndObject) {
        lastError = PitmParseError::UnterminatedObject;
        return false;
    }

    DEBUG << "numEntries" << parsedObject.offsets.size();
    int table = objectOffset;
    // finalize the object
    if (parsedObject.offsets.size()) {
        int tableSize = parsedObject.offsets.size()*sizeof(uint);
        table = reserveSpace(tableSize);
        if (table < 0)
            return false;

#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
        memcpy(data + table, parsedObject.offsets.constData(), tableSize);
#else
        offset *o = (offset *)(data + table);
        for (int i = 0; i < parsedObject.offsets.size(); ++i)
            o[i] = parsedObject.offsets[i];

#endif
    }

    PitmPrivate::Object *o = (PitmPrivate::Object *)(data + objectOffset);
    o->tableOffset = table - objectOffset;
    o->size = current - objectOffset;
    o->is_object = true;
    o->length = parsedObject.offsets.size();

    DEBUG << "current=" << current;
    END;

    --nestingLevel;
    return true;
}

/*
    member = string name-separator value
*/
bool Parser::parseMember(int baseOffset)
{
    int entryOffset = reserveSpace(sizeof(PitmPrivate::Entry));
    if (entryOffset < 0)
        return false;
    BEGIN << "parseMember pos=" << entryOffset;

    bool latin1;
    if (!parseString(&latin1))
        return false;

    if (!eatSpace()) {
        lastError = PitmParseError::UnterminatedObject;
        return false;
    }
    PitmPrivate::Value val;
    if (!parseValue(&val, baseOffset))
        return false;

    // finalize the entry
    PitmPrivate::Entry *e = (PitmPrivate::Entry *)(data + entryOffset);
    e->value = val;
    e->value.latinKey = latin1;

    END;
    return true;
}

namespace {
    struct ValueArray {
        static const int prealloc = 128;
        ValueArray() : data(stackValues), alloc(prealloc), size(0) {}
        ~ValueArray() { if (data != stackValues) free(data); }

        inline bool grow() {
            alloc *= 2;
            if (data == stackValues) {
                PitmPrivate::Value *newValues = static_cast<PitmPrivate::Value *>(malloc(alloc*sizeof(PitmPrivate::Value)));
                if (!newValues)
                    return false;
                memcpy(newValues, data, size*sizeof(PitmPrivate::Value));
                data = newValues;
            } else {
                void *newValues = realloc(data, alloc * sizeof(PitmPrivate::Value));
                if (!newValues)
                    return false;
                data = static_cast<PitmPrivate::Value *>(newValues);
            }
            return true;
        }
        bool append(const PitmPrivate::Value &v) {
            if (alloc == size && !grow())
                return false;
            data[size] = v;
            ++size;
            return true;
        }

        PitmPrivate::Value stackValues[prealloc];
        PitmPrivate::Value *data;
        int alloc;
        int size;
    };
}

/*
    array = begin-array [ value *( value-separator value ) ] end-array
*/
bool Parser::parseArray()
{
    BEGIN << "parseArray";

    if (++nestingLevel > nestingLimit) {
        lastError = PitmParseError::DeepNesting;
        return false;
    }

    int arrayOffset = reserveSpace(sizeof(PitmPrivate::Array));
    if (arrayOffset < 0)
        return false;

    ValueArray values;

    if (!eatSpace()) {
        lastError = PitmParseError::UnterminatedArray;
        return false;
    }
    if (*pitm == EndArray) {
        nextToken();
    } else {
        while (1) {
            if (!eatSpace()) {
                lastError = PitmParseError::UnterminatedArray;
                return false;
            }
            PitmPrivate::Value val;
            if (!parseValue(&val, arrayOffset))
                return false;
            if (!values.append(val)) {
                lastError = PitmParseError::DocumentTooLarge;
                return false;
            }
            if (!eatSpace()) {
                lastError = PitmParseError::UnterminatedArray;
                return false;
            }
            if (*pitm == EndArray)
                break;
            /*else if (token != ValueSeparator) {
                if (!eatSpace())
                    lastError = PitmParseError::UnterminatedArray;
                else
                    lastError = PitmParseError::MissingValueSeparator;
                return false;
            }*/
        }
    }

    ++pitm;
    DEBUG << "size =" << values.size;
    int table = arrayOffset;
    // finalize the object
    if (values.size) {
        int tableSize = values.size*sizeof(PitmPrivate::Value);
        table = reserveSpace(tableSize);
        if (table < 0)
            return false;
        memcpy(data + table, values.data, tableSize);
    }

    PitmPrivate::Array *a = (PitmPrivate::Array *)(data + arrayOffset);
    a->tableOffset = table - arrayOffset;
    a->size = current - arrayOffset;
    a->is_object = false;
    a->length = values.size;

    DEBUG << "current=" << current;
    END;

    --nestingLevel;
    return true;
}

/*
value = false / null / true / object / array / number / string

*/

bool Parser::parseValue(PitmPrivate::Value *val, int baseOffset)
{
    BEGIN << "parse Value" << pitm;
    val->_dummy = 0;

    switch (*pitm++) {
        case 'n':
        if (end - pitm < 4) {
            lastError = PitmParseError::IllegalValue;
            return false;
        }
        if (*pitm++ == 'u' &&
            *pitm++ == 'l' &&
            *pitm++ == 'l') {
            val->type = PitmValue::Null;
            DEBUG << "value: null";
            END;
            return true;
        }
        lastError = PitmParseError::IllegalValue;
        return false;
    case 't':
        if (end - pitm < 4) {
            lastError = PitmParseError::IllegalValue;
            return false;
        }
        if (*pitm++ == 'r' &&
            *pitm++ == 'u' &&
            *pitm++ == 'e') {
            val->type = PitmValue::Bool;
            val->value = true;
            DEBUG << "value: true";
            END;
            return true;
        }
        lastError = PitmParseError::IllegalValue;
        return false;
    case 'f':
        if (end - pitm < 5) {
            lastError = PitmParseError::IllegalValue;
            return false;
        }
        if (*pitm++ == 'a' &&
            *pitm++ == 'l' &&
            *pitm++ == 's' &&
            *pitm++ == 'e') {
            val->type = PitmValue::Bool;
            val->value = false;
            DEBUG << "value: false";
            END;
            return true;
        }
        lastError = PitmParseError::IllegalValue;
        return false;
    case Quote:
        val->type = PitmValue::String;
        if (current - baseOffset >= Value::MaxSize) {
            lastError = PitmParseError::DocumentTooLarge;
            return false;
        }
        val->value = current - baseOffset;
        bool latin1;
        stringState = true;
        if (!parseString(&latin1))
            return false;
        val->latinOrIntValue = latin1;
        DEBUG << "value: string";
        END;
        return true;
    case BeginArray:
        val->type = PitmValue::Array;
        if (current - baseOffset >= Value::MaxSize) {
            lastError = PitmParseError::DocumentTooLarge;
            return false;
        }
        val->value = current - baseOffset;
        if (!parseArray())
            return false;
        DEBUG << "value: array";
        END;
        return true;
    case BeginObject:
        val->type = PitmValue::Object;
        if (current - baseOffset >= Value::MaxSize) {
            lastError = PitmParseError::DocumentTooLarge;
            return false;
        }
        val->value = current - baseOffset;
        if (!parseObject())
            return false;
        DEBUG << "value: object";
        END;
        return true;
    case ValueSeparator:
        // Essentially missing value, but after a colon, not after a comma
        // like the other MissingObject errors.
        lastError = PitmParseError::IllegalValue;
        return false;
    case EndObject:
    case EndArray:
        lastError = PitmParseError::MissingObject;
        return false;
    default:
        --pitm;
        if (!parseNumber(val, baseOffset))
            return false;
        DEBUG << "value: number";
        END;
    }
    return true;
}





/*
        number = [ minus ] int [ frac ] [ exp ]
        decimal-point = %x2E       ; .
        digit1-9 = %x31-39         ; 1-9
        e = %x65 / %x45            ; e E
        exp = e [ minus / plus ] 1*DIGIT
        frac = decimal-point 1*DIGIT
        int = zero / ( digit1-9 *DIGIT )
        minus = %x2D               ; -
        plus = %x2B                ; +
        zero = %x30                ; 0

*/

bool Parser::parseNumber(PitmPrivate::Value *val, int baseOffset)
{
    BEGIN << "parseNumber" << pitm;
    val->type = PitmValue::Double;

    const char *start = pitm;
    bool isInt = true;

    // minus
    if (pitm < end && *pitm == '-')
        ++pitm;

    // int = zero / ( digit1-9 *DIGIT )
    if (pitm < end && *pitm == '0') {
        ++pitm;
    } else {
        while (pitm < end && *pitm >= '0' && *pitm <= '9')
            ++pitm;
    }

    // frac = decimal-point 1*DIGIT
    if (pitm < end && *pitm == '.') {
        isInt = false;
        ++pitm;
        while (pitm < end && *pitm >= '0' && *pitm <= '9')
            ++pitm;
    }

    // exp = e [ minus / plus ] 1*DIGIT
    if (pitm < end && (*pitm == 'e' || *pitm == 'E')) {
        isInt = false;
        ++pitm;
        if (pitm < end && (*pitm == '-' || *pitm == '+'))
            ++pitm;
        while (pitm < end && *pitm >= '0' && *pitm <= '9')
            ++pitm;
    }

    if (pitm >= end) {
        lastError = PitmParseError::TerminationByNumber;
        return false;
    }

    QByteArray number(start, pitm - start);
    DEBUG << "numberstring" << number;

    if (isInt) {
        bool ok;
        int n = number.toInt(&ok);
        if (ok && n < (1<<25) && n > -(1<<25)) {
            val->int_value = n;
            val->latinOrIntValue = true;
            END;
            return true;
        }
    }

    bool ok;
    union {
        quint64 ui;
        double d;
    };
    d = number.toDouble(&ok);

    if (!ok) {
        lastError = PitmParseError::IllegalNumber;
        return false;
    }

    int pos = reserveSpace(sizeof(double));
    if (pos < 0)
        return false;
    qToLittleEndian(ui, (uchar*)(data + pos));
    if (current - baseOffset >= Value::MaxSize) {
        lastError = PitmParseError::DocumentTooLarge;
        return false;
    }
    val->value = pos - baseOffset;
    val->latinOrIntValue = false;

    END;
    return true;
}

/*

        string = quotation-mark *char quotation-mark

        char = unescaped /
               escape (
                   %x22 /          ; "    quotation mark  U+0022
                   %x5C /          ; \    reverse solidus U+005C
                   %x2F /          ; /    solidus         U+002F
                   %x62 /          ; b    backspace       U+0008
                   %x66 /          ; f    form feed       U+000C
                   %x6E /          ; n    line feed       U+000A
                   %x72 /          ; r    carriage return U+000D
                   %x74 /          ; t    tab             U+0009
                   %x75 4HEXDIG )  ; uXXXX                U+XXXX

        escape = %x5C              ; \

        quotation-mark = %x22      ; "

        unescaped = %x20-21 / %x23-5B / %x5D-10FFFF
 */
static inline bool addHexDigit(char digit, uint *result)
{
    *result <<= 4;
    if (digit >= '0' && digit <= '9')
        *result |= (digit - '0');
    else if (digit >= 'a' && digit <= 'f')
        *result |= (digit - 'a') + 10;
    else if (digit >= 'A' && digit <= 'F')
        *result |= (digit - 'A') + 10;
    else
        return false;
    return true;
}

static inline bool scanEscapeSequence(const char *&pitm, const char *end, uint *ch)
{
    ++pitm;
    if (pitm >= end)
        return false;

    DEBUG << "scan escape" << (char)*pitm;
    uint escaped = *pitm++;
    switch (escaped) {
    case '"':
        *ch = '"'; break;
    case '\\':
        *ch = '\\'; break;
    case '/':
        *ch = '/'; break;
    case 'b':
        *ch = 0x8; break;
    case 'f':
        *ch = 0xc; break;
    case 'n':
        *ch = 0xa; break;
    case 'r':
        *ch = 0xd; break;
    case 't':
        *ch = 0x9; break;
    case 'u': {
        *ch = 0;
        if (pitm > end - 4)
            return false;
        for (int i = 0; i < 4; ++i) {
            if (!addHexDigit(*pitm, ch))
                return false;
            ++pitm;
        }
        return true;
    }
    default:
        // this is not as strict as one could be, but allows for more Pitm files
        // to be parsed correctly.
        *ch = escaped;
        return true;
    }
    return true;
}

static inline bool scanUtf8Char(const char *&pitm, const char *end, uint *result)
{
    const uchar *&src = reinterpret_cast<const uchar *&>(pitm);
    const uchar *uend = reinterpret_cast<const uchar *>(end);
    uchar b = *src++;
    int res = QUtf8Functions::fromUtf8<QUtf8BaseTraits>(b, result, src, uend);
    if (res < 0) {
        // decoding error, backtrack the character we read above
        --pitm;
        return false;
    }

    return true;
}

bool Parser::parseString(bool *latin1)
{
    *latin1 = true;

    const char *start = pitm;
    int outStart = current;

    // try to write out a latin1 string

    int stringPos = reserveSpace(2);
    if (stringPos < 0)
        return false;

    BEGIN << "parse string stringPos=" << stringPos << pitm;
    while (pitm < end) {
        uint ch = 0;
        if ((!stringState && !isKeyChar(*pitm)) || (stringState && *pitm == '"'))
            break;
        else if (*pitm == '\\') {
            if (!scanEscapeSequence(pitm, end, &ch)) {
                lastError = PitmParseError::IllegalEscapeSequence;
                return false;
            }
        } else {
            if (!scanUtf8Char(pitm, end, &ch)) {
                lastError = PitmParseError::IllegalUTF8String;
                return false;
            }
        }
        // bail out if the string is not pure latin1 or too long to hold as a latin1string (which has only 16 bit for the length)
        if (ch > 0xff || pitm - start >= 0x8000) {
            *latin1 = false;
            break;
        }
        int pos = reserveSpace(1);
        if (pos < 0)
            return false;
        DEBUG << "  " << ch << (char)ch;
        data[pos] = (uchar)ch;
    }
    if(stringState) {
        ++pitm;
        stringState = false;
    }
    DEBUG << "end of string";
    if (pitm >= end) {
        lastError = PitmParseError::UnterminatedString;
        return false;
    }

    // no unicode string, we are done
    if (*latin1) {
        // write string length
        *(PitmPrivate::qle_ushort *)(data + stringPos) = ushort(current - outStart - sizeof(ushort));
        int pos = reserveSpace((4 - current) & 3);
        if (pos < 0)
            return false;
        while (pos & 3)
            data[pos++] = 0;
        END;
        return true;
    }

    *latin1 = false;
    DEBUG << "not latin";

    pitm = start;
    current = outStart + sizeof(int);

    while (pitm < end) {
        uint ch = 0;
        if (*pitm == '"')
            break;
        else if (*pitm == '\\') {
            if (!scanEscapeSequence(pitm, end, &ch)) {
                lastError = PitmParseError::IllegalEscapeSequence;
                return false;
            }
        } else {
            if (!scanUtf8Char(pitm, end, &ch)) {
                lastError = PitmParseError::IllegalUTF8String;
                return false;
            }
        }
        if (QChar::requiresSurrogates(ch)) {
            int pos = reserveSpace(4);
            if (pos < 0)
                return false;
            *(PitmPrivate::qle_ushort *)(data + pos) = QChar::highSurrogate(ch);
            *(PitmPrivate::qle_ushort *)(data + pos + 2) = QChar::lowSurrogate(ch);
        } else {
            int pos = reserveSpace(2);
            if (pos < 0)
                return false;
            *(PitmPrivate::qle_ushort *)(data + pos) = (ushort)ch;
        }
    }
    ++pitm;

    if (pitm >= end) {
        lastError = PitmParseError::UnterminatedString;
        return false;
    }

    // write string length
    *(PitmPrivate::qle_int *)(data + stringPos) = (current - outStart - sizeof(int))/2;
    int pos = reserveSpace((4 - current) & 3);
    if (pos < 0)
        return false;
    while (pos & 3)
        data[pos++] = 0;
    END;
    return true;
}

QT_END_NAMESPACE
