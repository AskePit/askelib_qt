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

#ifndef PITMVALUE_H
#define PITMVALUE_H

#include <QtCore/qglobal.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class QDebug;
class QVariant;
class PitmArray;
class PitmObject;

namespace PitmPrivate {
    class Data;
    class Base;
    class Object;
    class Header;
    class Array;
    class Value;
    class Entry;
}

class PitmValue
{
public:
    enum Type {
        Null =  0x0,
        Bool = 0x1,
        Double = 0x2,
        String = 0x3,
        Array = 0x4,
        Object = 0x5,
        Undefined = 0x80
    };

    PitmValue(Type = Null);
    PitmValue(bool b);
    PitmValue(double n);
    PitmValue(int n);
    PitmValue(qint64 n);
    PitmValue(const QString &s);
    PitmValue(QLatin1String s);
#ifndef QT_NO_CAST_FROM_ASCII
    inline QT_ASCII_CAST_WARN PitmValue(const char *s)
        : d(Q_NULLPTR), t(String) { stringDataFromQStringHelper(QString::fromUtf8(s)); }
#endif
    PitmValue(const PitmArray &a);
    PitmValue(const PitmObject &o);

    ~PitmValue();

    PitmValue(const PitmValue &other);
    PitmValue &operator =(const PitmValue &other);

    static PitmValue fromVariant(const QVariant &variant);
    QVariant toVariant() const;

    Type type() const;
    inline bool isNull() const { return type() == Null; }
    inline bool isBool() const { return type() == Bool; }
    inline bool isDouble() const { return type() == Double; }
    inline bool isString() const { return type() == String; }
    inline bool isArray() const { return type() == Array; }
    inline bool isObject() const { return type() == Object; }
    inline bool isUndefined() const { return type() == Undefined; }

    bool toBool(bool defaultValue = false) const;
    int toInt(int defaultValue = 0) const;
    double toDouble(double defaultValue = 0) const;
    QString toString() const;
    QString toString(const QString &defaultValue) const;
    PitmArray toArray() const;
    PitmArray toArray(const PitmArray &defaultValue) const;
    PitmObject toObject() const;
    PitmObject toObject(const PitmObject &defaultValue) const;

    bool operator==(const PitmValue &other) const;
    bool operator!=(const PitmValue &other) const;

private:
    // avoid implicit conversions from char * to bool
    inline PitmValue(const void *) {}
    friend class PitmPrivate::Value;
    friend class PitmArray;
    friend class PitmObject;
    friend QDebug operator<<(QDebug, const PitmValue &);

    PitmValue(PitmPrivate::Data *d, PitmPrivate::Base *b, const PitmPrivate::Value& v);
    void stringDataFromQStringHelper(const QString &string);

    void detach();

    union {
        quint64 ui;
        bool b;
        double dbl;
        QStringData *stringData;
        PitmPrivate::Base *base;
    };
    PitmPrivate::Data *d; // needed for Objects and Arrays
    Type t;
};

class PitmValueRef
{
public:
    PitmValueRef(PitmArray *array, int idx)
        : a(array), is_object(false), index(idx) {}
    PitmValueRef(PitmObject *object, int idx)
        : o(object), is_object(true), index(idx) {}

    inline operator PitmValue() const { return toValue(); }
    PitmValueRef &operator = (const PitmValue &val);
    PitmValueRef &operator = (const PitmValueRef &val);

    QVariant toVariant() const;
    inline PitmValue::Type type() const { return toValue().type(); }
    inline bool isNull() const { return type() == PitmValue::Null; }
    inline bool isBool() const { return type() == PitmValue::Bool; }
    inline bool isDouble() const { return type() == PitmValue::Double; }
    inline bool isString() const { return type() == PitmValue::String; }
    inline bool isArray() const { return type() == PitmValue::Array; }
    inline bool isObject() const { return type() == PitmValue::Object; }
    inline bool isUndefined() const { return type() == PitmValue::Undefined; }

    inline bool toBool() const { return toValue().toBool(); }
    inline int toInt() const { return toValue().toInt(); }
    inline double toDouble() const { return toValue().toDouble(); }
    inline QString toString() const { return toValue().toString(); }
    PitmArray toArray() const;
    PitmObject toObject() const;

    // ### Qt 6: Add default values
    inline bool toBool(bool defaultValue) const { return toValue().toBool(defaultValue); }
    inline int toInt(int defaultValue) const { return toValue().toInt(defaultValue); }
    inline double toDouble(double defaultValue) const { return toValue().toDouble(defaultValue); }
    inline QString toString(const QString &defaultValue) const { return toValue().toString(defaultValue); }

    inline bool operator==(const PitmValue &other) const { return toValue() == other; }
    inline bool operator!=(const PitmValue &other) const { return toValue() != other; }

private:
    PitmValue toValue() const;

    union {
        PitmArray *a;
        PitmObject *o;
    };
    uint is_object : 1;
    uint index : 31;
};

#ifndef Q_QDOC
// ### Qt 6: Get rid of these fake pointer classes
class PitmValuePtr
{
    PitmValue value;
public:
    explicit PitmValuePtr(const PitmValue& val)
        : value(val) {}

    PitmValue& operator*() { return value; }
    PitmValue* operator->() { return &value; }
};

class PitmValueRefPtr
{
    PitmValueRef valueRef;
public:
    PitmValueRefPtr(PitmArray *array, int idx)
        : valueRef(array, idx) {}
    PitmValueRefPtr(PitmObject *object, int idx)
        : valueRef(object, idx)  {}

    PitmValueRef& operator*() { return valueRef; }
    PitmValueRef* operator->() { return &valueRef; }
};
#endif

#if !defined(QT_NO_DEBUG_STREAM) && !defined(QT_PITM_READONLY)
QDebug operator<<(QDebug, const PitmValue &);
#endif

QT_END_NAMESPACE

#endif // PITMVALUE_H
