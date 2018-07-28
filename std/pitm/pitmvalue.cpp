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

#include "pitmobject.h"
#include "pitmvalue.h"
#include "pitmarray.h"
#include <qvariant.h>
#include <qstringlist.h>
#include <qdebug.h>

#include "pitm_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class PitmValue
    \inmodule QtCore
    \ingroup pitm
    \ingroup shared
    \reentrant
    \since 5.0

    \brief The PitmValue class encapsulates a value in PITM.

    A value in PITM can be one of 6 basic types:

    PITM is a format to store structured data. It has 6 basic data types:

    \list
    \li bool PitmValue::Bool
    \li double PitmValue::Double
    \li string PitmValue::String
    \li array PitmValue::Array
    \li object PitmValue::Object
    \li null PitmValue::Null
    \endlist

    A value can represent any of the above data types. In addition, PitmValue has one special
    flag to represent undefined values. This can be queried with isUndefined().

    The type of the value can be queried with type() or accessors like isBool(), isString(), and so on.
    Likewise, the value can be converted to the type stored in it using the toBool(), toString() and so on.

    Values are strictly typed internally and contrary to QVariant will not attempt to do any implicit type
    conversions. This implies that converting to a type that is not stored in the value will return a default
    constructed return value.

    \section1 PitmValueRef

    PitmValueRef is a helper class for PitmArray and PitmObject.
    When you get an object of type PitmValueRef, you can
    use it as if it were a reference to a PitmValue. If you assign to it,
    the assignment will apply to the element in the PitmArray or PitmObject
    from which you got the reference.

    The following methods return PitmValueRef:
    \list
    \li \l {PitmArray}::operator[](int i)
    \li \l {PitmObject}::operator[](const QString & key) const
    \endlist

    \sa {PITM Support in Qt}, {PITM Save Game Example}
*/

/*!
    Creates a PitmValue of type \a type.

    The default is to create a Null value.
 */
PitmValue::PitmValue(Type type)
    : ui(0), d(0), t(type)
{
}

/*!
    \internal
 */
PitmValue::PitmValue(PitmPrivate::Data *data, PitmPrivate::Base *base, const PitmPrivate::Value &v)
    : d(0)
{
    t = (Type)(uint)v.type;
    switch (t) {
    case Undefined:
    case Null:
        dbl = 0;
        break;
    case Bool:
        b = v.toBoolean();
        break;
    case Double:
        dbl = v.toDouble(base);
        break;
    case String: {
        QString s = v.toString(base);
        stringData = s.data_ptr();
        stringData->ref.ref();
        break;
    }
    case Array:
    case Object:
        d = data;
        this->base = v.base(base);
        break;
    }
    if (d)
        d->ref.ref();
}

/*!
    Creates a value of type Bool, with value \a b.
 */
PitmValue::PitmValue(bool b)
    : d(0), t(Bool)
{
    this->b = b;
}

/*!
    Creates a value of type Double, with value \a n.
 */
PitmValue::PitmValue(double n)
    : d(0), t(Double)
{
    this->dbl = n;
}

/*!
    \overload
    Creates a value of type Double, with value \a n.
 */
PitmValue::PitmValue(int n)
    : d(0), t(Double)
{
    this->dbl = n;
}

/*!
    \overload
    Creates a value of type Double, with value \a n.
    NOTE: the integer limits for IEEE 754 double precision data is 2^53 (-9007199254740992 to +9007199254740992).
    If you pass in values outside this range expect a loss of precision to occur.
 */
PitmValue::PitmValue(qint64 n)
    : d(0), t(Double)
{
    this->dbl = double(n);
}

/*!
    Creates a value of type String, with value \a s.
 */
PitmValue::PitmValue(const QString &s)
    : d(0), t(String)
{
    stringDataFromQStringHelper(s);
}

/*!
    \fn PitmValue::PitmValue(const char *s)

    Creates a value of type String with value \a s, assuming
    UTF-8 encoding of the input.

    You can disable this constructor by defining \c
    QT_NO_CAST_FROM_ASCII when you compile your applications.

    \since 5.3
 */

void PitmValue::stringDataFromQStringHelper(const QString &string)
{
    stringData = *(QStringData **)(&string);
    stringData->ref.ref();
}

/*!
    Creates a value of type String, with value \a s.
 */
PitmValue::PitmValue(QLatin1String s)
    : d(0), t(String)
{
    // ### FIXME: Avoid creating the temp QString below
    QString str(s);
    stringDataFromQStringHelper(str);
}

/*!
    Creates a value of type Array, with value \a a.
 */
PitmValue::PitmValue(const PitmArray &a)
    : d(a.d), t(Array)
{
    base = a.a;
    if (d)
        d->ref.ref();
}

/*!
    Creates a value of type Object, with value \a o.
 */
PitmValue::PitmValue(const PitmObject &o)
    : d(o.d), t(Object)
{
    base = o.o;
    if (d)
        d->ref.ref();
}


/*!
    Destroys the value.
 */
PitmValue::~PitmValue()
{
    if (t == String && stringData && !stringData->ref.deref())
        free(stringData);

    if (d && !d->ref.deref())
        delete d;
}

/*!
    Creates a copy of \a other.
 */
PitmValue::PitmValue(const PitmValue &other)
{
    t = other.t;
    d = other.d;
    ui = other.ui;
    if (d)
        d->ref.ref();

    if (t == String && stringData)
        stringData->ref.ref();
}

/*!
    Assigns the value stored in \a other to this object.
 */
PitmValue &PitmValue::operator =(const PitmValue &other)
{
    PitmValue copy(other);
    // swap(copy);
    qSwap(dbl, copy.dbl);
    qSwap(d,   copy.d);
    qSwap(t,   copy.t);
    return *this;
}

/*!
    \fn bool PitmValue::isNull() const

    Returns \c true if the value is null.
*/

/*!
    \fn bool PitmValue::isBool() const

    Returns \c true if the value contains a boolean.

    \sa toBool()
 */

/*!
    \fn bool PitmValue::isDouble() const

    Returns \c true if the value contains a double.

    \sa toDouble()
 */

/*!
    \fn bool PitmValue::isString() const

    Returns \c true if the value contains a string.

    \sa toString()
 */

/*!
    \fn bool PitmValue::isArray() const

    Returns \c true if the value contains an array.

    \sa toArray()
 */

/*!
    \fn bool PitmValue::isObject() const

    Returns \c true if the value contains an object.

    \sa toObject()
 */

/*!
    \fn bool PitmValue::isUndefined() const

    Returns \c true if the value is undefined. This can happen in certain
    error cases as e.g. accessing a non existing key in a PitmObject.
 */


/*!
    Converts \a variant to a PitmValue and returns it.

    The conversion will convert QVariant types as follows:

    \table
    \header
        \li Source type
        \li Destination type
    \row
        \li
            \list
                \li QMetaType::Nullptr
            \endlist
        \li PitmValue::Null
    \row
        \li
            \list
                \li QMetaType::Bool
            \endlist
        \li PitmValue::Bool
    \row
        \li
            \list
                \li QMetaType::Int
                \li QMetaType::UInt
                \li QMetaType::LongLong
                \li QMetaType::ULongLong
                \li QMetaType::Float
                \li QMetaType::Double
            \endlist
        \li PitmValue::Double
    \row
        \li
            \list
                \li QMetaType::QString
            \endlist
        \li PitmValue::String
    \row
        \li
            \list
                \li QMetaType::QStringList
                \li QMetaType::QVariantList
            \endlist
        \li PitmValue::Array
    \row
        \li
            \list
                \li QMetaType::QVariantMap
                \li QMetaType::QVariantHash
            \endlist
        \li PitmValue::Object
    \endtable

    For all other QVariant types a conversion to a QString will be attempted. If the returned string
    is empty, a Null PitmValue will be stored, otherwise a String value using the returned QString.

    \sa toVariant()
 */
PitmValue PitmValue::fromVariant(const QVariant &variant)
{
    switch (variant.userType()) {
    case QVariant::Bool:
        return PitmValue(variant.toBool());
    case QVariant::Int:
    case QMetaType::Float:
    case QVariant::Double:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    case QVariant::UInt:
        return PitmValue(variant.toDouble());
    case QVariant::String:
        return PitmValue(variant.toString());
    case QVariant::StringList:
        return PitmValue(PitmArray::fromStringList(variant.toStringList()));
    case QVariant::List:
        return PitmValue(PitmArray::fromVariantList(variant.toList()));
    case QVariant::Map:
        return PitmValue(PitmObject::fromVariantMap(variant.toMap()));
    case QVariant::Hash:
        return PitmValue(PitmObject::fromVariantHash(variant.toHash()));
    default:
        break;
    }
    QString string = variant.toString();
    if (string.isEmpty())
        return PitmValue();
    return PitmValue(string);
}

/*!
    Converts the value to a \l {QVariant::}{QVariant()}.

    The PitmValue types will be converted as follows:

    \value Null     QMetaType::Nullptr
    \value Bool     QMetaType::Bool
    \value Double   QMetaType::Double
    \value String   QString
    \value Array    QVariantList
    \value Object   QVariantMap
    \value Undefined \l {QVariant::}{QVariant()}

    \sa fromVariant()
 */
QVariant PitmValue::toVariant() const
{
    switch (t) {
    case Bool:
        return b;
    case Double:
        return dbl;
    case String:
        return toString();
    case Array:
        return d ?
               PitmArray(d, static_cast<PitmPrivate::Array *>(base)).toVariantList() :
               QVariantList();
    case Object:
        return d ?
               PitmObject(d, static_cast<PitmPrivate::Object *>(base)).toVariantMap() :
               QVariantMap();
    case Null:
    case Undefined:
        break;
    }
    return QVariant();
}

/*!
    \enum PitmValue::Type

    This enum describes the type of the PITM value.

    \value Null     A Null value
    \value Bool     A boolean value. Use toBool() to convert to a bool.
    \value Double   A double. Use toDouble() to convert to a double.
    \value String   A string. Use toString() to convert to a QString.
    \value Array    An array. Use toArray() to convert to a PitmArray.
    \value Object   An object. Use toObject() to convert to a PitmObject.
    \value Undefined The value is undefined. This is usually returned as an
                    error condition, when trying to read an out of bounds value
                    in an array or a non existent key in an object.
*/

/*!
    Returns the type of the value.

    \sa PitmValue::Type
 */
PitmValue::Type PitmValue::type() const
{
    return t;
}

/*!
    Converts the value to a bool and returns it.

    If type() is not bool, the \a defaultValue will be returned.
 */
bool PitmValue::toBool(bool defaultValue) const
{
    if (t != Bool)
        return defaultValue;
    return b;
}

/*!
    Converts the value to an int and returns it.

    If type() is not Double or the value is not a whole number,
    the \a defaultValue will be returned.
 */
int PitmValue::toInt(int defaultValue) const
{
    if (t == Double && int(dbl) == dbl)
        return int(dbl);
    return defaultValue;
}

/*!
    Converts the value to a double and returns it.

    If type() is not Double, the \a defaultValue will be returned.
 */
double PitmValue::toDouble(double defaultValue) const
{
    if (t != Double)
        return defaultValue;
    return dbl;
}

/*!
    Converts the value to a QString and returns it.

    If type() is not String, the \a defaultValue will be returned.
 */
QString PitmValue::toString(const QString &defaultValue) const
{
    if (t != String)
        return defaultValue;
    stringData->ref.ref(); // the constructor below doesn't add a ref.
    QStringDataPtr holder = { stringData };
    return QString(holder);
}

/*!
    Converts the value to a QString and returns it.

    If type() is not String, a null QString will be returned.

    \sa QString::isNull()
 */
QString PitmValue::toString() const
{
    if (t != String)
        return QString();
    stringData->ref.ref(); // the constructor below doesn't add a ref.
    QStringDataPtr holder = { stringData };
    return QString(holder);
}

/*!
    Converts the value to an array and returns it.

    If type() is not Array, the \a defaultValue will be returned.
 */
PitmArray PitmValue::toArray(const PitmArray &defaultValue) const
{
    if (!d || t != Array)
        return defaultValue;

    return PitmArray(d, static_cast<PitmPrivate::Array *>(base));
}

/*!
    \overload

    Converts the value to an array and returns it.

    If type() is not Array, a \l{PitmArray::}{PitmArray()} will be returned.
 */
PitmArray PitmValue::toArray() const
{
    return toArray(PitmArray());
}

/*!
    Converts the value to an object and returns it.

    If type() is not Object, the \a defaultValue will be returned.
 */
PitmObject PitmValue::toObject(const PitmObject &defaultValue) const
{
    if (!d || t != Object)
        return defaultValue;

    return PitmObject(d, static_cast<PitmPrivate::Object *>(base));
}

/*!
    \overload

    Converts the value to an object and returns it.

    If type() is not Object, the \l {PitmObject::}{PitmObject()} will be returned.
*/
PitmObject PitmValue::toObject() const
{
    return toObject(PitmObject());
}

/*!
    Returns \c true if the value is equal to \a other.
 */
bool PitmValue::operator==(const PitmValue &other) const
{
    if (t != other.t)
        return false;

    switch (t) {
    case Undefined:
    case Null:
        break;
    case Bool:
        return b == other.b;
    case Double:
        return dbl == other.dbl;
    case String:
        return toString() == other.toString();
    case Array:
        if (base == other.base)
            return true;
        if (!base)
            return !other.base->length;
        if (!other.base)
            return !base->length;
        return PitmArray(d, static_cast<PitmPrivate::Array *>(base))
                == PitmArray(other.d, static_cast<PitmPrivate::Array *>(other.base));
    case Object:
        if (base == other.base)
            return true;
        if (!base)
            return !other.base->length;
        if (!other.base)
            return !base->length;
        return PitmObject(d, static_cast<PitmPrivate::Object *>(base))
                == PitmObject(other.d, static_cast<PitmPrivate::Object *>(other.base));
    }
    return true;
}

/*!
    Returns \c true if the value is not equal to \a other.
 */
bool PitmValue::operator!=(const PitmValue &other) const
{
    return !(*this == other);
}

/*!
    \internal
 */
void PitmValue::detach()
{
    if (!d)
        return;

    PitmPrivate::Data *x = d->clone(base);
    x->ref.ref();
    if (!d->ref.deref())
        delete d;
    d = x;
    base = static_cast<PitmPrivate::Object *>(d->header->root());
}


/*!
    \class PitmValueRef
    \inmodule QtCore
    \reentrant
    \brief The PitmValueRef class is a helper class for PitmValue.

    \internal

    \ingroup pitm

    When you get an object of type PitmValueRef, if you can assign to it,
    the assignment will apply to the character in the string from
    which you got the reference. That is its whole purpose in life.

    You can use it exactly in the same way as a reference to a PitmValue.

    The PitmValueRef becomes invalid once modifications are made to the
    string: if you want to keep the character, copy it into a PitmValue.

    Most of the PitmValue member functions also exist in PitmValueRef.
    However, they are not explicitly documented here.
*/


PitmValueRef &PitmValueRef::operator =(const PitmValue &val)
{
    if (is_object)
        o->setValueAt(index, val);
    else
        a->replace(index, val);

    return *this;
}

PitmValueRef &PitmValueRef::operator =(const PitmValueRef &ref)
{
    if (is_object)
        o->setValueAt(index, ref);
    else
        a->replace(index, ref);

    return *this;
}

QVariant PitmValueRef::toVariant() const
{
    return toValue().toVariant();
}

PitmArray PitmValueRef::toArray() const
{
    return toValue().toArray();
}

PitmObject PitmValueRef::toObject() const
{
    return toValue().toObject();
}

PitmValue PitmValueRef::toValue() const
{
    if (!is_object)
        return a->at(index);
    return o->valueAt(index);
}

#if !defined(QT_NO_DEBUG_STREAM) && !defined(QT_PITM_READONLY)
QDebug operator<<(QDebug dbg, const PitmValue &o)
{
    QDebugStateSaver saver(dbg);
    switch (o.t) {
    case PitmValue::Undefined:
        dbg << "PitmValue(undefined)";
        break;
    case PitmValue::Null:
        dbg << "PitmValue(null)";
        break;
    case PitmValue::Bool:
        dbg.nospace() << "PitmValue(bool, " << o.toBool() << ')';
        break;
    case PitmValue::Double:
        dbg.nospace() << "PitmValue(double, " << o.toDouble() << ')';
        break;
    case PitmValue::String:
        dbg.nospace() << "PitmValue(string, " << o.toString() << ')';
        break;
    case PitmValue::Array:
        dbg.nospace() << "PitmValue(array, ";
        dbg << o.toArray();
        dbg << ')';
        break;
    case PitmValue::Object:
        dbg.nospace() << "PitmValue(object, ";
        dbg << o.toObject();
        dbg << ')';
        break;
    }
    return dbg;
}
#endif

QT_END_NAMESPACE
