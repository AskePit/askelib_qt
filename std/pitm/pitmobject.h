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

#ifndef PITMOBJECT_H
#define PITMOBJECT_H

#include "pitmvalue.h"
#include <QtCore/qiterator.h>
#ifdef Q_COMPILER_INITIALIZER_LISTS
#include <QtCore/qpair.h>
#include <initializer_list>
#endif

QT_BEGIN_NAMESPACE

class QDebug;
template <class Key, class T> class QMap;
typedef QMap<QString, QVariant> QVariantMap;
template <class Key, class T> class QHash;
typedef QHash<QString, QVariant> QVariantHash;

class PitmObject
{
public:
    PitmObject();

#if defined(Q_COMPILER_INITIALIZER_LISTS) || defined(Q_QDOC)
    PitmObject(std::initializer_list<QPair<QString, PitmValue> > args)
    {
        initialize();
        for (std::initializer_list<QPair<QString, PitmValue> >::const_iterator i = args.begin(); i != args.end(); ++i)
            insert(i->first, i->second);
    }
#endif

    ~PitmObject();

    PitmObject(const PitmObject &other);
    PitmObject &operator =(const PitmObject &other);

    static PitmObject fromVariantMap(const QVariantMap &map);
    QVariantMap toVariantMap() const;
    static PitmObject fromVariantHash(const QVariantHash &map);
    QVariantHash toVariantHash() const;

    QStringList keys() const;
    int size() const;
    inline int count() const { return size(); }
    inline int length() const { return size(); }
    bool isEmpty() const;

    PitmValue value(const QString &key) const;
    PitmValue value(QLatin1String key) const;
    PitmValue operator[] (const QString &key) const;
    PitmValue operator[] (QLatin1String key) const { return value(key); }
    PitmValueRef operator[] (const QString &key);
    PitmValueRef operator[] (QLatin1String key);

    void remove(const QString &key);
    PitmValue take(const QString &key);
    bool contains(const QString &key) const;
    bool contains(QLatin1String key) const;

    bool operator==(const PitmObject &other) const;
    bool operator!=(const PitmObject &other) const;

    class const_iterator;

    class iterator
    {
        friend class const_iterator;
        friend class PitmObject;
        PitmObject *o;
        int i;

    public:
        typedef std::random_access_iterator_tag iterator_category;
        typedef int difference_type;
        typedef PitmValue value_type;
        typedef PitmValueRef reference;
        typedef PitmValuePtr pointer;

        Q_DECL_CONSTEXPR inline iterator() : o(Q_NULLPTR), i(0) {}
        Q_DECL_CONSTEXPR inline iterator(PitmObject *obj, int index) : o(obj), i(index) {}

        inline QString key() const { return o->keyAt(i); }
        inline PitmValueRef value() const { return PitmValueRef(o, i); }
        inline PitmValueRef operator*() const { return PitmValueRef(o, i); }
#ifdef Q_QDOC
        inline PitmValueRef* operator->() const;
#else
        inline PitmValueRefPtr operator->() const { return PitmValueRefPtr(o, i); }
#endif
        inline bool operator==(const iterator &other) const { return i == other.i; }
        inline bool operator!=(const iterator &other) const { return i != other.i; }

        inline iterator &operator++() { ++i; return *this; }
        inline iterator operator++(int) { iterator r = *this; ++i; return r; }
        inline iterator &operator--() { --i; return *this; }
        inline iterator operator--(int) { iterator r = *this; --i; return r; }
        inline iterator operator+(int j) const
        { iterator r = *this; r.i += j; return r; }
        inline iterator operator-(int j) const { return operator+(-j); }
        inline iterator &operator+=(int j) { i += j; return *this; }
        inline iterator &operator-=(int j) { i -= j; return *this; }

    public:
        inline bool operator==(const const_iterator &other) const { return i == other.i; }
        inline bool operator!=(const const_iterator &other) const { return i != other.i; }
    };
    friend class iterator;

    class const_iterator
    {
        friend class iterator;
        const PitmObject *o;
        int i;

    public:
        typedef std::random_access_iterator_tag iterator_category;
        typedef int difference_type;
        typedef PitmValue value_type;
        typedef PitmValue reference;
        typedef PitmValuePtr pointer;

        Q_DECL_CONSTEXPR inline const_iterator() : o(Q_NULLPTR), i(0) {}
        Q_DECL_CONSTEXPR inline const_iterator(const PitmObject *obj, int index)
            : o(obj), i(index) {}
        inline const_iterator(const iterator &other)
            : o(other.o), i(other.i) {}

        inline QString key() const { return o->keyAt(i); }
        inline PitmValue value() const { return o->valueAt(i); }
        inline PitmValue operator*() const { return o->valueAt(i); }
#ifdef Q_QDOC
        inline PitmValue* operator->() const;
#else
        inline PitmValuePtr operator->() const { return PitmValuePtr(o->valueAt(i)); }
#endif
        inline bool operator==(const const_iterator &other) const { return i == other.i; }
        inline bool operator!=(const const_iterator &other) const { return i != other.i; }

        inline const_iterator &operator++() { ++i; return *this; }
        inline const_iterator operator++(int) { const_iterator r = *this; ++i; return r; }
        inline const_iterator &operator--() { --i; return *this; }
        inline const_iterator operator--(int) { const_iterator r = *this; --i; return r; }
        inline const_iterator operator+(int j) const
        { const_iterator r = *this; r.i += j; return r; }
        inline const_iterator operator-(int j) const { return operator+(-j); }
        inline const_iterator &operator+=(int j) { i += j; return *this; }
        inline const_iterator &operator-=(int j) { i -= j; return *this; }

        inline bool operator==(const iterator &other) const { return i == other.i; }
        inline bool operator!=(const iterator &other) const { return i != other.i; }
    };
    friend class const_iterator;

    // STL style
    inline iterator begin() { detach2(); return iterator(this, 0); }
    inline const_iterator begin() const { return const_iterator(this, 0); }
    inline const_iterator constBegin() const { return const_iterator(this, 0); }
    inline iterator end() { detach2(); return iterator(this, size()); }
    inline const_iterator end() const { return const_iterator(this, size()); }
    inline const_iterator constEnd() const { return const_iterator(this, size()); }
    iterator erase(iterator it);

    // more Qt
    typedef iterator Iterator;
    typedef const_iterator ConstIterator;
    iterator find(const QString &key);
    iterator find(QLatin1String key);
    const_iterator find(const QString &key) const { return constFind(key); }
    const_iterator find(QLatin1String key) const { return constFind(key); }
    const_iterator constFind(const QString &key) const;
    const_iterator constFind(QLatin1String key) const;
    iterator insert(const QString &key, const PitmValue &value);

    // STL compatibility
    typedef PitmValue mapped_type;
    typedef QString key_type;
    typedef int size_type;

    inline bool empty() const { return isEmpty(); }

private:
    friend class PitmPrivate::Data;
    friend class PitmValue;
    friend class PitmDocument;
    friend class PitmValueRef;

    friend QDebug operator<<(QDebug, const PitmObject &);

    PitmObject(PitmPrivate::Data *data, PitmPrivate::Object *object);
    void initialize();
    // ### Qt 6: remove me and merge with detach2
    void detach(uint reserve = 0);
    bool detach2(uint reserve = 0);
    void compact();

    QString keyAt(int i) const;
    PitmValue valueAt(int i) const;
    void setValueAt(int i, const PitmValue &val);

    PitmPrivate::Data *d;
    PitmPrivate::Object *o;
};

#if !defined(QT_NO_DEBUG_STREAM) && !defined(QT_PITM_READONLY)
QDebug operator<<(QDebug, const PitmObject &);
#endif

QT_END_NAMESPACE

#endif // PITMOBJECT_H
