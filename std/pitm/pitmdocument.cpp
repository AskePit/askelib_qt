/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "pitmdocument.h"
#include "pitmobject.h"
#include "pitmvalue.h"
#include "pitmarray.h"
#include <qstringlist.h>
#include <qvariant.h>
#include <qdebug.h>
#include "pitmwriter_p.h"
#include "pitmparser_p.h"
#include "pitm_p.h"

QT_BEGIN_NAMESPACE

/*! \class PitmDocument
    \inmodule QtCore
    \ingroup pitm
    \ingroup shared
    \reentrant
    \since 5.0

    \brief The PitmDocument class provides a way to read and write PITM documents.

    PitmDocument is a class that wraps a complete PITM document and can read and
    write this document both from a UTF-8 encoded text based representation as well
    as Qt's own binary format.

    A PITM document can be converted from its text-based representation to a PitmDocument
    using PitmDocument::fromPitm(). toPitm() converts it back to text. The parser is very
    fast and efficient and converts the PITM to the binary representation used by Qt.

    Validity of the parsed document can be queried with !isNull()

    A document can be queried as to whether it contains an array or an object using isArray()
    and isObject(). The array or object contained in the document can be retrieved using
    array() or object() and then read or manipulated.

    A document can also be created from a stored binary representation using fromBinaryData() or
    fromRawData().

    \sa {PITM Support in Qt}, {PITM Save Game Example}
*/

/*!
 * Constructs an empty and invalid document.
 */
PitmDocument::PitmDocument()
    : d(0)
{
}

/*!
 * Creates a PitmDocument from \a object.
 */
PitmDocument::PitmDocument(const PitmObject &object)
    : d(0)
{
    setObject(object);
}

/*!
 * Constructs a PitmDocument from \a array.
 */
PitmDocument::PitmDocument(const PitmArray &array)
    : d(0)
{
    setArray(array);
}

/*!
    \internal
 */
PitmDocument::PitmDocument(PitmPrivate::Data *data)
    : d(data)
{
    Q_ASSERT(d);
    d->ref.ref();
}

/*!
 Deletes the document.

 Binary data set with fromRawData is not freed.
 */
PitmDocument::~PitmDocument()
{
    if (d && !d->ref.deref())
        delete d;
}

/*!
 * Creates a copy of the \a other document.
 */
PitmDocument::PitmDocument(const PitmDocument &other)
{
    d = other.d;
    if (d)
        d->ref.ref();
}

/*!
 * Assigns the \a other document to this PitmDocument.
 * Returns a reference to this object.
 */
PitmDocument &PitmDocument::operator =(const PitmDocument &other)
{
    if (d != other.d) {
        if (d && !d->ref.deref())
            delete d;
        d = other.d;
        if (d)
            d->ref.ref();
    }

    return *this;
}

/*! \enum PitmDocument::DataValidation

  This value is used to tell PitmDocument whether to validate the binary data
  when converting to a PitmDocument using fromBinaryData() or fromRawData().

  \value Validate Validate the data before using it. This is the default.
  \value BypassValidation Bypasses data validation. Only use if you received the
  data from a trusted place and know it's valid, as using of invalid data can crash
  the application.
  */

/*!
 Creates a PitmDocument that uses the first \a size bytes from
 \a data. It assumes \a data contains a binary encoded PITM document.
 The created document does not take ownership of \a data and the caller
 has to guarantee that \a data will not be deleted or modified as long as
 any PitmDocument, PitmObject or PitmArray still references the data.

 \a data has to be aligned to a 4 byte boundary.

 \a validation decides whether the data is checked for validity before being used.
 By default the data is validated. If the \a data is not valid, the method returns
 a null document.

 Returns a PitmDocument representing the data.

 \sa rawData(), fromBinaryData(), isNull(), DataValidation
 */
PitmDocument PitmDocument::fromRawData(const char *data, int size, DataValidation validation)
{
    if (quintptr(data) & 3) {
        qWarning("PitmDocument::fromRawData: data has to have 4 byte alignment");
        return PitmDocument();
    }

    PitmPrivate::Data *d = new PitmPrivate::Data((char *)data, size);
    d->ownsData = false;

    if (validation != BypassValidation && !d->valid()) {
        delete d;
        return PitmDocument();
    }

    return PitmDocument(d);
}

/*!
  Returns the raw binary representation of the data
  \a size will contain the size of the returned data.

  This method is useful to e.g. stream the PITM document
  in it's binary form to a file.
 */
const char *PitmDocument::rawData(int *size) const
{
    if (!d) {
        *size = 0;
        return 0;
    }
    *size = d->alloc;
    return d->rawData;
}

/*!
 Creates a PitmDocument from \a data.

 \a validation decides whether the data is checked for validity before being used.
 By default the data is validated. If the \a data is not valid, the method returns
 a null document.

 \sa toBinaryData(), fromRawData(), isNull(), DataValidation
 */
PitmDocument PitmDocument::fromBinaryData(const QByteArray &data, DataValidation validation)
{
    if (data.size() < (int)(sizeof(PitmPrivate::Header) + sizeof(PitmPrivate::Base)))
        return PitmDocument();

    PitmPrivate::Header h;
    memcpy(&h, data.constData(), sizeof(PitmPrivate::Header));
    PitmPrivate::Base root;
    memcpy(&root, data.constData() + sizeof(PitmPrivate::Header), sizeof(PitmPrivate::Base));

    // do basic checks here, so we don't try to allocate more memory than we can.
    if (h.tag != PitmDocument::BinaryFormatTag || h.version != 1u ||
        sizeof(PitmPrivate::Header) + root.size > (uint)data.size())
        return PitmDocument();

    const uint size = sizeof(PitmPrivate::Header) + root.size;
    char *raw = (char *)malloc(size);
    if (!raw)
        return PitmDocument();

    memcpy(raw, data.constData(), size);
    PitmPrivate::Data *d = new PitmPrivate::Data(raw, size);

    if (validation != BypassValidation && !d->valid()) {
        delete d;
        return PitmDocument();
    }

    return PitmDocument(d);
}

/*!
 Creates a PitmDocument from the QVariant \a variant.

 If the \a variant contains any other type than a QVariantMap,
 QVariantHash, QVariantList or QStringList, the returned document is invalid.

 \sa toVariant()
 */
PitmDocument PitmDocument::fromVariant(const QVariant &variant)
{
    PitmDocument doc;
    switch (variant.type()) {
    case QVariant::Map:
        doc.setObject(PitmObject::fromVariantMap(variant.toMap()));
        break;
    case QVariant::Hash:
        doc.setObject(PitmObject::fromVariantHash(variant.toHash()));
        break;
    case QVariant::List:
        doc.setArray(PitmArray::fromVariantList(variant.toList()));
        break;
    case QVariant::StringList:
        doc.setArray(PitmArray::fromStringList(variant.toStringList()));
        break;
    default:
        break;
    }
    return doc;
}

/*!
 Returns a QVariant representing the Pitm document.

 The returned variant will be a QVariantList if the document is
 a PitmArray and a QVariantMap if the document is a PitmObject.

 \sa fromVariant(), PitmValue::toVariant()
 */
QVariant PitmDocument::toVariant() const
{
    if (!d)
        return QVariant();

    if (d->header->root()->isArray())
        return PitmArray(d, static_cast<PitmPrivate::Array *>(d->header->root())).toVariantList();
    else
        return PitmObject(d, static_cast<PitmPrivate::Object *>(d->header->root())).toVariantMap();
}

/*!
 Converts the PitmDocument to a UTF-8 encoded PITM document.

 \sa fromPitm()
 */
#ifndef QT_PITM_READONLY
QByteArray PitmDocument::toPitm() const
{
    return toPitm(Indented);
}
#endif

/*!
    \enum PitmDocument::PitmFormat

    This value defines the format of the PITM byte array produced
    when converting to a PitmDocument using toPitm().

    \value Indented Defines human readable output as follows:
        \code
        {
            "Array": [
                true,
                999,
                "string"
            ],
            "Key": "Value",
            "null": null
        }
        \endcode

    \value Compact Defines a compact output as follows:
        \code
        {"Array":[true,999,"string"],"Key":"Value","null":null}
        \endcode
  */

/*!
    Converts the PitmDocument to a UTF-8 encoded PITM document in the provided \a format.

    \sa fromPitm(), PitmFormat
 */
#ifndef QT_PITM_READONLY
QByteArray PitmDocument::toPitm(PitmFormat format) const
{
    QByteArray pitm;
    if (!d)
        return pitm;

    if (d->header->root()->isArray())
        PitmPrivate::Writer::arrayToPitm(static_cast<PitmPrivate::Array *>(d->header->root()), pitm, 0, (format == Compact));
    else
        PitmPrivate::Writer::objectToPitm(static_cast<PitmPrivate::Object *>(d->header->root()), pitm, 0, (format == Compact));

    return pitm;
}
#endif

/*!
 Parses \a pitm as a UTF-8 encoded PITM document, and creates a PitmDocument
 from it.

 Returns a valid (non-null) PitmDocument if the parsing succeeds. If it fails,
 the returned document will be null, and the optional \a error variable will contain
 further details about the error.

 \sa toPitm(), PitmParseError, isNull()
 */
PitmDocument PitmDocument::fromPitm(const QByteArray &pitm, PitmParseError *error)
{
    PitmPrivate::Parser parser(pitm.constData(), pitm.length());
    return parser.parse(error);
}

/*!
    Returns \c true if the document doesn't contain any data.
 */
bool PitmDocument::isEmpty() const
{
    if (!d)
        return true;

    return false;
}

/*!
 Returns a binary representation of the document.

 The binary representation is also the native format used internally in Qt,
 and is very efficient and fast to convert to and from.

 The binary format can be stored on disk and interchanged with other applications
 or computers. fromBinaryData() can be used to convert it back into a
 PITM document.

 \sa fromBinaryData()
 */
QByteArray PitmDocument::toBinaryData() const
{
    if (!d || !d->rawData)
        return QByteArray();

    return QByteArray(d->rawData, d->header->root()->size + sizeof(PitmPrivate::Header));
}

/*!
    Returns \c true if the document contains an array.

    \sa array(), isObject()
 */
bool PitmDocument::isArray() const
{
    if (!d)
        return false;

    PitmPrivate::Header *h = (PitmPrivate::Header *)d->rawData;
    return h->root()->isArray();
}

/*!
    Returns \c true if the document contains an object.

    \sa object(), isArray()
 */
bool PitmDocument::isObject() const
{
    if (!d)
        return false;

    PitmPrivate::Header *h = (PitmPrivate::Header *)d->rawData;
    return h->root()->isObject();
}

/*!
    Returns the PitmObject contained in the document.

    Returns an empty object if the document contains an
    array.

    \sa isObject(), array(), setObject()
 */
PitmObject PitmDocument::object() const
{
    if (d) {
        PitmPrivate::Base *b = d->header->root();
        if (b->isObject())
            return PitmObject(d, static_cast<PitmPrivate::Object *>(b));
    }
    return PitmObject();
}

/*!
    Returns the PitmArray contained in the document.

    Returns an empty array if the document contains an
    object.

    \sa isArray(), object(), setArray()
 */
PitmArray PitmDocument::array() const
{
    if (d) {
        PitmPrivate::Base *b = d->header->root();
        if (b->isArray())
            return PitmArray(d, static_cast<PitmPrivate::Array *>(b));
    }
    return PitmArray();
}

/*!
    Sets \a object as the main object of this document.

    \sa setArray(), object()
 */
void PitmDocument::setObject(const PitmObject &object)
{
    if (d && !d->ref.deref())
        delete d;

    d = object.d;

    if (!d) {
        d = new PitmPrivate::Data(0, PitmValue::Object);
    } else if (d->compactionCounter || object.o != d->header->root()) {
        PitmObject o(object);
        if (d->compactionCounter)
            o.compact();
        else
            o.detach2();
        d = o.d;
        d->ref.ref();
        return;
    }
    d->ref.ref();
}

/*!
    Sets \a array as the main object of this document.

    \sa setObject(), array()
 */
void PitmDocument::setArray(const PitmArray &array)
{
    if (d && !d->ref.deref())
        delete d;

    d = array.d;

    if (!d) {
        d = new PitmPrivate::Data(0, PitmValue::Array);
    } else if (d->compactionCounter || array.a != d->header->root()) {
        PitmArray a(array);
        if (d->compactionCounter)
            a.compact();
        else
            a.detach2();
        d = a.d;
        d->ref.ref();
        return;
    }
    d->ref.ref();
}

/*!
    Returns \c true if the \a other document is equal to this document.
 */
bool PitmDocument::operator==(const PitmDocument &other) const
{
    if (d == other.d)
        return true;

    if (!d || !other.d)
        return false;

    if (d->header->root()->isArray() != other.d->header->root()->isArray())
        return false;

    if (d->header->root()->isObject())
        return PitmObject(d, static_cast<PitmPrivate::Object *>(d->header->root()))
                == PitmObject(other.d, static_cast<PitmPrivate::Object *>(other.d->header->root()));
    else
        return PitmArray(d, static_cast<PitmPrivate::Array *>(d->header->root()))
                == PitmArray(other.d, static_cast<PitmPrivate::Array *>(other.d->header->root()));
}

/*!
 \fn bool PitmDocument::operator!=(const PitmDocument &other) const

    returns \c true if \a other is not equal to this document
 */

/*!
    returns \c true if this document is null.

    Null documents are documents created through the default constructor.

    Documents created from UTF-8 encoded text or the binary format are
    validated during parsing. If validation fails, the returned document
    will also be null.
 */
bool PitmDocument::isNull() const
{
    return (d == 0);
}

#if !defined(QT_NO_DEBUG_STREAM) && !defined(QT_PITM_READONLY)
QDebug operator<<(QDebug dbg, const PitmDocument &o)
{
    QDebugStateSaver saver(dbg);
    if (!o.d) {
        dbg << "PitmDocument()";
        return dbg;
    }
    QByteArray pitm;
    if (o.d->header->root()->isArray())
        PitmPrivate::Writer::arrayToPitm(static_cast<PitmPrivate::Array *>(o.d->header->root()), pitm, 0, true);
    else
        PitmPrivate::Writer::objectToPitm(static_cast<PitmPrivate::Object *>(o.d->header->root()), pitm, 0, true);
    dbg.nospace() << "PitmDocument("
                  << pitm.constData() // print as utf-8 string without extra quotation marks
                  << ')';
    return dbg;
}
#endif

QT_END_NAMESPACE
