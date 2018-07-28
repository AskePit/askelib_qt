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
#include <qstringlist.h>
#include <qdebug.h>
#include <qvariant.h>
#include "pitm_p.h"
#include "pitmwriter_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class PitmObject
    \inmodule QtCore
    \ingroup pitm
    \ingroup shared
    \reentrant
    \since 5.0

    \brief The PitmObject class encapsulates a PITM object.

    A PITM object is a list of key value pairs, where the keys are unique strings
    and the values are represented by a PitmValue.

    A PitmObject can be converted to and from a QVariantMap. You can query the
    number of (key, value) pairs with size(), insert(), and remove() entries from it
    and iterate over its content using the standard C++ iterator pattern.

    PitmObject is an implicitly shared class, and shares the data with the document
    it has been created from as long as it is not being modified.

    You can convert the object to and from text based PITM through PitmDocument.

    \sa {PITM Support in Qt}, {PITM Save Game Example}
*/

/*!
    \typedef PitmObject::Iterator

    Qt-style synonym for PitmObject::iterator.
*/

/*!
    \typedef PitmObject::ConstIterator

    Qt-style synonym for PitmObject::const_iterator.
*/

/*!
    \typedef PitmObject::key_type

    Typedef for QString. Provided for STL compatibility.
*/

/*!
    \typedef PitmObject::mapped_type

    Typedef for PitmValue. Provided for STL compatibility.
*/

/*!
    \typedef PitmObject::size_type

    Typedef for int. Provided for STL compatibility.
*/


/*!
    Constructs an empty PITM object.

    \sa isEmpty()
 */
PitmObject::PitmObject()
    : d(0), o(0)
{
}

/*!
    \fn PitmObject::PitmObject(std::initializer_list<QPair<QString, PitmValue> > args)
    \since 5.4
    Constructs a PitmObject instance initialized from \a args initialization list.
    For example:
    \code
    PitmObject object
    {
        {"property1", 1},
        {"property2", 2}
    };
    \endcode
*/

/*!
    \internal
 */
PitmObject::PitmObject(PitmPrivate::Data *data, PitmPrivate::Object *object)
    : d(data), o(object)
{
    Q_ASSERT(d);
    Q_ASSERT(o);
    d->ref.ref();
}

/*!
    This method replaces part of the PitmObject(std::initializer_list<QPair<QString, PitmValue>> args) body.
    The constructor needs to be inline, but we do not want to leak implementation details
    of this class.
    \note this method is called for an uninitialized object
    \internal
 */

void PitmObject::initialize()
{
    d = 0;
    o = 0;
}

/*!
    Destroys the object.
 */
PitmObject::~PitmObject()
{
    if (d && !d->ref.deref())
        delete d;
}

/*!
    Creates a copy of \a other.

    Since PitmObject is implicitly shared, the copy is shallow
    as long as the object does not get modified.
 */
PitmObject::PitmObject(const PitmObject &other)
{
    d = other.d;
    o = other.o;
    if (d)
        d->ref.ref();
}

/*!
    Assigns \a other to this object.
 */
PitmObject &PitmObject::operator =(const PitmObject &other)
{
    if (d != other.d) {
        if (d && !d->ref.deref())
            delete d;
        d = other.d;
        if (d)
            d->ref.ref();
    }
    o = other.o;

    return *this;
}

/*!
    Converts the variant map \a map to a PitmObject.

    The keys in \a map will be used as the keys in the PITM object,
    and the QVariant values will be converted to PITM values.

    \sa fromVariantHash(), toVariantMap(), PitmValue::fromVariant()
 */
PitmObject PitmObject::fromVariantMap(const QVariantMap &map)
{
    PitmObject object;
    if (map.isEmpty())
        return object;

    object.detach2(1024);

    QVector<PitmPrivate::offset> offsets;
    PitmPrivate::offset currentOffset;
    currentOffset = sizeof(PitmPrivate::Base);

    // the map is already sorted, so we can simply append one entry after the other and
    // write the offset table at the end
    for (QVariantMap::const_iterator it = map.constBegin(); it != map.constEnd(); ++it) {
        QString key = it.key();
        PitmValue val = PitmValue::fromVariant(it.value());

        bool latinOrIntValue;
        int valueSize = PitmPrivate::Value::requiredStorage(val, &latinOrIntValue);

        bool latinKey = PitmPrivate::useCompressed(key);
        int valueOffset = sizeof(PitmPrivate::Entry) + PitmPrivate::qStringSize(key, latinKey);
        int requiredSize = valueOffset + valueSize;

        if (!object.detach2(requiredSize + sizeof(PitmPrivate::offset))) // offset for the new index entry
            return PitmObject();

        PitmPrivate::Entry *e = reinterpret_cast<PitmPrivate::Entry *>(reinterpret_cast<char *>(object.o) + currentOffset);
        e->value.type = val.t;
        e->value.latinKey = latinKey;
        e->value.latinOrIntValue = latinOrIntValue;
        e->value.value = PitmPrivate::Value::valueToStore(val, (char *)e - (char *)object.o + valueOffset);
        PitmPrivate::copyString((char *)(e + 1), key, latinKey);
        if (valueSize)
            PitmPrivate::Value::copyData(val, (char *)e + valueOffset, latinOrIntValue);

        offsets << currentOffset;
        currentOffset += requiredSize;
        object.o->size = currentOffset;
    }

    // write table
    object.o->tableOffset = currentOffset;
    if (!object.detach2(sizeof(PitmPrivate::offset)*offsets.size()))
        return PitmObject();
    memcpy(object.o->table(), offsets.constData(), offsets.size()*sizeof(uint));
    object.o->length = offsets.size();
    object.o->size = currentOffset + sizeof(PitmPrivate::offset)*offsets.size();

    return object;
}

/*!
    Converts this object to a QVariantMap.

    Returns the created map.

    \sa toVariantHash()
 */
QVariantMap PitmObject::toVariantMap() const
{
    QVariantMap map;
    if (o) {
        for (uint i = 0; i < o->length; ++i) {
            PitmPrivate::Entry *e = o->entryAt(i);
            map.insert(e->key(), PitmValue(d, o, e->value).toVariant());
        }
    }
    return map;
}

/*!
    Converts the variant hash \a hash to a PitmObject.
    \since 5.5

    The keys in \a hash will be used as the keys in the PITM object,
    and the QVariant values will be converted to PITM values.

    \sa fromVariantMap(), toVariantHash(), PitmValue::fromVariant()
 */
PitmObject PitmObject::fromVariantHash(const QVariantHash &hash)
{
    // ### this is implemented the trivial way, not the most efficient way

    PitmObject object;
    for (QVariantHash::const_iterator it = hash.constBegin(); it != hash.constEnd(); ++it)
        object.insert(it.key(), PitmValue::fromVariant(it.value()));
    return object;
}

/*!
    Converts this object to a QVariantHash.
    \since 5.5

    Returns the created hash.

    \sa toVariantMap()
 */
QVariantHash PitmObject::toVariantHash() const
{
    QVariantHash hash;
    if (o) {
        hash.reserve(o->length);
        for (uint i = 0; i < o->length; ++i) {
            PitmPrivate::Entry *e = o->entryAt(i);
            hash.insert(e->key(), PitmValue(d, o, e->value).toVariant());
        }
    }
    return hash;
}

/*!
    Returns a list of all keys in this object.

    The list is sorted lexographically.
 */
QStringList PitmObject::keys() const
{
    QStringList keys;
    if (o) {
        keys.reserve(o->length);
        for (uint i = 0; i < o->length; ++i) {
            PitmPrivate::Entry *e = o->entryAt(i);
            keys.append(e->key());
        }
    }
    return keys;
}

/*!
    Returns the number of (key, value) pairs stored in the object.
 */
int PitmObject::size() const
{
    if (!d)
        return 0;

    return o->length;
}

/*!
    Returns \c true if the object is empty. This is the same as size() == 0.

    \sa size()
 */
bool PitmObject::isEmpty() const
{
    if (!d)
        return true;

    return !o->length;
}

/*!
    Returns a PitmValue representing the value for the key \a key.

    The returned PitmValue is PitmValue::Undefined if the key does not exist.

    \sa PitmValue, PitmValue::isUndefined()
 */
PitmValue PitmObject::value(const QString &key) const
{
    if (!d)
        return PitmValue(PitmValue::Undefined);

    bool keyExists;
    int i = o->indexOf(key, &keyExists);
    if (!keyExists)
        return PitmValue(PitmValue::Undefined);
    return PitmValue(d, o, o->entryAt(i)->value);
}

/*!
    \overload
    \since 5.7
*/
PitmValue PitmObject::value(QLatin1String key) const
{
    if (!d)
        return PitmValue(PitmValue::Undefined);

    bool keyExists;
    int i = o->indexOf(key, &keyExists);
    if (!keyExists)
        return PitmValue(PitmValue::Undefined);
    return PitmValue(d, o, o->entryAt(i)->value);
}

/*!
    Returns a PitmValue representing the value for the key \a key.

    This does the same as value().

    The returned PitmValue is PitmValue::Undefined if the key does not exist.

    \sa value(), PitmValue, PitmValue::isUndefined()
 */
PitmValue PitmObject::operator [](const QString &key) const
{
    return value(key);
}

/*!
    \fn PitmValue PitmObject::operator [](QLatin1String key) const

    \overload
    \since 5.7
*/

/*!
    Returns a reference to the value for \a key.

    The return value is of type PitmValueRef, a helper class for PitmArray
    and PitmObject. When you get an object of type PitmValueRef, you can
    use it as if it were a reference to a PitmValue. If you assign to it,
    the assignment will apply to the element in the PitmArray or PitmObject
    from which you got the reference.

    \sa value()
 */
PitmValueRef PitmObject::operator [](const QString &key)
{
    // ### somewhat inefficient, as we lookup the key twice if it doesn't yet exist
    bool keyExists = false;
    int index = o ? o->indexOf(key, &keyExists) : -1;
    if (!keyExists) {
        iterator i = insert(key, PitmValue());
        index = i.i;
    }
    return PitmValueRef(this, index);
}

/*!
    \overload
    \since 5.7
*/
PitmValueRef PitmObject::operator [](QLatin1String key)
{
    // ### optimize me
    return operator[](QString(key));
}

/*!
    Inserts a new item with the key \a key and a value of \a value.

    If there is already an item with the key \a key, then that item's value
    is replaced with \a value.

    Returns an iterator pointing to the inserted item.

    If the value is PitmValue::Undefined, it will cause the key to get removed
    from the object. The returned iterator will then point to end().

    \sa remove(), take(), PitmObject::iterator, end()
 */
PitmObject::iterator PitmObject::insert(const QString &key, const PitmValue &value)
{
    if (value.t == PitmValue::Undefined) {
        remove(key);
        return end();
    }
    PitmValue val = value;

    bool latinOrIntValue;
    int valueSize = PitmPrivate::Value::requiredStorage(val, &latinOrIntValue);

    bool latinKey = PitmPrivate::useCompressed(key);
    int valueOffset = sizeof(PitmPrivate::Entry) + PitmPrivate::qStringSize(key, latinKey);
    int requiredSize = valueOffset + valueSize;

    if (!detach2(requiredSize + sizeof(PitmPrivate::offset))) // offset for the new index entry
        return iterator();

    if (!o->length)
        o->tableOffset = sizeof(PitmPrivate::Object);

    bool keyExists = false;
    int pos = o->indexOf(key, &keyExists);
    if (keyExists)
        ++d->compactionCounter;

    uint off = o->reserveSpace(requiredSize, pos, 1, keyExists);
    if (!off)
        return end();

    PitmPrivate::Entry *e = o->entryAt(pos);
    e->value.type = val.t;
    e->value.latinKey = latinKey;
    e->value.latinOrIntValue = latinOrIntValue;
    e->value.value = PitmPrivate::Value::valueToStore(val, (char *)e - (char *)o + valueOffset);
    PitmPrivate::copyString((char *)(e + 1), key, latinKey);
    if (valueSize)
        PitmPrivate::Value::copyData(val, (char *)e + valueOffset, latinOrIntValue);

    if (d->compactionCounter > 32u && d->compactionCounter >= unsigned(o->length) / 2u)
        compact();

    return iterator(this, pos);
}

/*!
    Removes \a key from the object.

    \sa insert(), take()
 */
void PitmObject::remove(const QString &key)
{
    if (!d)
        return;

    bool keyExists;
    int index = o->indexOf(key, &keyExists);
    if (!keyExists)
        return;

    detach2();
    o->removeItems(index, 1);
    ++d->compactionCounter;
    if (d->compactionCounter > 32u && d->compactionCounter >= unsigned(o->length) / 2u)
        compact();
}

/*!
    Removes \a key from the object.

    Returns a PitmValue containing the value referenced by \a key.
    If \a key was not contained in the object, the returned PitmValue
    is PitmValue::Undefined.

    \sa insert(), remove(), PitmValue
 */
PitmValue PitmObject::take(const QString &key)
{
    if (!o)
        return PitmValue(PitmValue::Undefined);

    bool keyExists;
    int index = o->indexOf(key, &keyExists);
    if (!keyExists)
        return PitmValue(PitmValue::Undefined);

    PitmValue v(d, o, o->entryAt(index)->value);
    detach2();
    o->removeItems(index, 1);
    ++d->compactionCounter;
    if (d->compactionCounter > 32u && d->compactionCounter >= unsigned(o->length) / 2u)
        compact();

    return v;
}

/*!
    Returns \c true if the object contains key \a key.

    \sa insert(), remove(), take()
 */
bool PitmObject::contains(const QString &key) const
{
    if (!o)
        return false;

    bool keyExists;
    o->indexOf(key, &keyExists);
    return keyExists;
}

/*!
    \overload
    \since 5.7
*/
bool PitmObject::contains(QLatin1String key) const
{
    if (!o)
        return false;

    bool keyExists;
    o->indexOf(key, &keyExists);
    return keyExists;
}

/*!
    Returns \c true if \a other is equal to this object.
 */
bool PitmObject::operator==(const PitmObject &other) const
{
    if (o == other.o)
        return true;

    if (!o)
        return !other.o->length;
    if (!other.o)
        return !o->length;
    if (o->length != other.o->length)
        return false;

    for (uint i = 0; i < o->length; ++i) {
        PitmPrivate::Entry *e = o->entryAt(i);
        PitmValue v(d, o, e->value);
        if (other.value(e->key()) != v)
            return false;
    }

    return true;
}

/*!
    Returns \c true if \a other is not equal to this object.
 */
bool PitmObject::operator!=(const PitmObject &other) const
{
    return !(*this == other);
}

/*!
    Removes the (key, value) pair pointed to by the iterator \a it
    from the map, and returns an iterator to the next item in the
    map.

    \sa remove()
 */
PitmObject::iterator PitmObject::erase(PitmObject::iterator it)
{
    Q_ASSERT(d && d->ref.load() == 1);
    if (it.o != this || it.i < 0 || it.i >= (int)o->length)
        return iterator(this, o->length);

    int index = it.i;

    o->removeItems(index, 1);
    ++d->compactionCounter;
    if (d->compactionCounter > 32u && d->compactionCounter >= unsigned(o->length) / 2u)
        compact();

    // iterator hasn't changed
    return it;
}

/*!
    Returns an iterator pointing to the item with key \a key in the
    map.

    If the map contains no item with key \a key, the function
    returns end().
 */
PitmObject::iterator PitmObject::find(const QString &key)
{
    bool keyExists = false;
    int index = o ? o->indexOf(key, &keyExists) : 0;
    if (!keyExists)
        return end();
    detach2();
    return iterator(this, index);
}

/*!
    \overload
    \since 5.7
*/
PitmObject::iterator PitmObject::find(QLatin1String key)
{
    bool keyExists = false;
    int index = o ? o->indexOf(key, &keyExists) : 0;
    if (!keyExists)
        return end();
    detach2();
    return iterator(this, index);
}

/*! \fn PitmObject::const_iterator PitmObject::find(const QString &key) const

    \overload
*/

/*! \fn PitmObject::const_iterator PitmObject::find(QLatin1String key) const

    \overload
    \since 5.7
*/

/*!
    Returns a const iterator pointing to the item with key \a key in the
    map.

    If the map contains no item with key \a key, the function
    returns constEnd().
 */
PitmObject::const_iterator PitmObject::constFind(const QString &key) const
{
    bool keyExists = false;
    int index = o ? o->indexOf(key, &keyExists) : 0;
    if (!keyExists)
        return end();
    return const_iterator(this, index);
}

/*!
    \overload
    \since 5.7
*/
PitmObject::const_iterator PitmObject::constFind(QLatin1String key) const
{
    bool keyExists = false;
    int index = o ? o->indexOf(key, &keyExists) : 0;
    if (!keyExists)
        return end();
    return const_iterator(this, index);
}

/*! \fn int PitmObject::count() const

    \overload

    Same as size().
*/

/*! \fn int PitmObject::length() const

    \overload

    Same as size().
*/

/*! \fn PitmObject::iterator PitmObject::begin()

    Returns an \l{STL-style iterators}{STL-style iterator} pointing to the first item in
    the object.

    \sa constBegin(), end()
*/

/*! \fn PitmObject::const_iterator PitmObject::begin() const

    \overload
*/

/*! \fn PitmObject::const_iterator PitmObject::constBegin() const

    Returns a const \l{STL-style iterators}{STL-style iterator} pointing to the first item
    in the object.

    \sa begin(), constEnd()
*/

/*! \fn PitmObject::iterator PitmObject::end()

    Returns an \l{STL-style iterators}{STL-style iterator} pointing to the imaginary item
    after the last item in the object.

    \sa begin(), constEnd()
*/

/*! \fn PitmObject::const_iterator PitmObject::end() const

    \overload
*/

/*! \fn PitmObject::const_iterator PitmObject::constEnd() const

    Returns a const \l{STL-style iterators}{STL-style iterator} pointing to the imaginary
    item after the last item in the object.

    \sa constBegin(), end()
*/

/*!
    \fn bool PitmObject::empty() const

    This function is provided for STL compatibility. It is equivalent
    to isEmpty(), returning \c true if the object is empty; otherwise
    returning \c false.
*/

/*! \class PitmObject::iterator
    \inmodule QtCore
    \ingroup pitm
    \reentrant
    \since 5.0

    \brief The PitmObject::iterator class provides an STL-style non-const iterator for PitmObject.

    PitmObject::iterator allows you to iterate over a PitmObject
    and to modify the value (but not the key) stored under
    a particular key. If you want to iterate over a const PitmObject, you
    should use PitmObject::const_iterator. It is generally good practice to
    use PitmObject::const_iterator on a non-const PitmObject as well, unless you
    need to change the PitmObject through the iterator. Const iterators are
    slightly faster, and improve code readability.

    The default PitmObject::iterator constructor creates an uninitialized
    iterator. You must initialize it using a PitmObject function like
    PitmObject::begin(), PitmObject::end(), or PitmObject::find() before you can
    start iterating.

    Multiple iterators can be used on the same object. Existing iterators will however
    become dangling once the object gets modified.

    \sa PitmObject::const_iterator, {PITM Support in Qt}, {PITM Save Game Example}
*/

/*! \typedef PitmObject::iterator::difference_type

    \internal
*/

/*! \typedef PitmObject::iterator::iterator_category

    A synonym for \e {std::random_access_iterator_tag} indicating
    this iterator is a random-access iterator.

    \note In Qt versions before 5.6, this was set by mistake to
    \e {std::bidirectional_iterator_tag}.
*/

/*! \typedef PitmObject::iterator::reference

    \internal
*/

/*! \typedef PitmObject::iterator::value_type

    \internal
*/

/*! \typedef PitmObject::iterator::pointer

    \internal
*/

/*! \fn PitmObject::iterator::iterator()

    Constructs an uninitialized iterator.

    Functions like key(), value(), and operator++() must not be
    called on an uninitialized iterator. Use operator=() to assign a
    value to it before using it.

    \sa PitmObject::begin(), PitmObject::end()
*/

/*! \fn PitmObject::iterator::iterator(PitmObject *obj, int index)
    \internal
*/

/*! \fn QString PitmObject::iterator::key() const

    Returns the current item's key.

    There is no direct way of changing an item's key through an
    iterator, although it can be done by calling PitmObject::erase()
    followed by PitmObject::insert().

    \sa value()
*/

/*! \fn PitmValueRef PitmObject::iterator::value() const

    Returns a modifiable reference to the current item's value.

    You can change the value of an item by using value() on
    the left side of an assignment.

    The return value is of type PitmValueRef, a helper class for PitmArray
    and PitmObject. When you get an object of type PitmValueRef, you can
    use it as if it were a reference to a PitmValue. If you assign to it,
    the assignment will apply to the element in the PitmArray or PitmObject
    from which you got the reference.

    \sa key(), operator*()
*/

/*! \fn PitmValueRef PitmObject::iterator::operator*() const

    Returns a modifiable reference to the current item's value.

    Same as value().

    The return value is of type PitmValueRef, a helper class for PitmArray
    and PitmObject. When you get an object of type PitmValueRef, you can
    use it as if it were a reference to a PitmValue. If you assign to it,
    the assignment will apply to the element in the PitmArray or PitmObject
    from which you got the reference.

    \sa key()
*/

/*! \fn PitmValueRef *PitmObject::iterator::operator->() const

    Returns a pointer to a modifiable reference to the current item.
*/

/*!
    \fn bool PitmObject::iterator::operator==(const iterator &other) const
    \fn bool PitmObject::iterator::operator==(const const_iterator &other) const

    Returns \c true if \a other points to the same item as this
    iterator; otherwise returns \c false.

    \sa operator!=()
*/

/*!
    \fn bool PitmObject::iterator::operator!=(const iterator &other) const
    \fn bool PitmObject::iterator::operator!=(const const_iterator &other) const

    Returns \c true if \a other points to a different item than this
    iterator; otherwise returns \c false.

    \sa operator==()
*/

/*! \fn PitmObject::iterator PitmObject::iterator::operator++()

    The prefix ++ operator, \c{++i}, advances the iterator to the
    next item in the object and returns an iterator to the new current
    item.

    Calling this function on PitmObject::end() leads to undefined results.

    \sa operator--()
*/

/*! \fn PitmObject::iterator PitmObject::iterator::operator++(int)

    \overload

    The postfix ++ operator, \c{i++}, advances the iterator to the
    next item in the object and returns an iterator to the previously
    current item.
*/

/*! \fn PitmObject::iterator PitmObject::iterator::operator--()

    The prefix -- operator, \c{--i}, makes the preceding item
    current and returns an iterator pointing to the new current item.

    Calling this function on PitmObject::begin() leads to undefined
    results.

    \sa operator++()
*/

/*! \fn PitmObject::iterator PitmObject::iterator::operator--(int)

    \overload

    The postfix -- operator, \c{i--}, makes the preceding item
    current and returns an iterator pointing to the previously
    current item.
*/

/*! \fn PitmObject::iterator PitmObject::iterator::operator+(int j) const

    Returns an iterator to the item at \a j positions forward from
    this iterator. If \a j is negative, the iterator goes backward.

    \sa operator-()

*/

/*! \fn PitmObject::iterator PitmObject::iterator::operator-(int j) const

    Returns an iterator to the item at \a j positions backward from
    this iterator. If \a j is negative, the iterator goes forward.

    \sa operator+()
*/

/*! \fn PitmObject::iterator &PitmObject::iterator::operator+=(int j)

    Advances the iterator by \a j items. If \a j is negative, the
    iterator goes backward.

    \sa operator-=(), operator+()
*/

/*! \fn PitmObject::iterator &PitmObject::iterator::operator-=(int j)

    Makes the iterator go back by \a j items. If \a j is negative,
    the iterator goes forward.

    \sa operator+=(), operator-()
*/

/*!
    \class PitmObject::const_iterator
    \inmodule QtCore
    \ingroup pitm
    \since 5.0
    \brief The PitmObject::const_iterator class provides an STL-style const iterator for PitmObject.

    PitmObject::const_iterator allows you to iterate over a PitmObject.
    If you want to modify the PitmObject as you iterate
    over it, you must use PitmObject::iterator instead. It is generally
    good practice to use PitmObject::const_iterator on a non-const PitmObject as
    well, unless you need to change the PitmObject through the iterator.
    Const iterators are slightly faster and improve code
    readability.

    The default PitmObject::const_iterator constructor creates an
    uninitialized iterator. You must initialize it using a PitmObject
    function like PitmObject::constBegin(), PitmObject::constEnd(), or
    PitmObject::find() before you can start iterating.

    Multiple iterators can be used on the same object. Existing iterators
    will however become dangling if the object gets modified.

    \sa PitmObject::iterator, {PITM Support in Qt}, {PITM Save Game Example}
*/

/*! \typedef PitmObject::const_iterator::difference_type

    \internal
*/

/*! \typedef PitmObject::const_iterator::iterator_category

    A synonym for \e {std::random_access_iterator_tag} indicating
    this iterator is a random-access iterator.

    \note In Qt versions before 5.6, this was set by mistake to
    \e {std::bidirectional_iterator_tag}.
*/

/*! \typedef PitmObject::const_iterator::reference

    \internal
*/

/*! \typedef PitmObject::const_iterator::value_type

    \internal
*/

/*! \typedef PitmObject::const_iterator::pointer

    \internal
*/

/*! \fn PitmObject::const_iterator::const_iterator()

    Constructs an uninitialized iterator.

    Functions like key(), value(), and operator++() must not be
    called on an uninitialized iterator. Use operator=() to assign a
    value to it before using it.

    \sa PitmObject::constBegin(), PitmObject::constEnd()
*/

/*! \fn PitmObject::const_iterator::const_iterator(const PitmObject *obj, int index)
    \internal
*/

/*! \fn PitmObject::const_iterator::const_iterator(const iterator &other)

    Constructs a copy of \a other.
*/

/*! \fn QString PitmObject::const_iterator::key() const

    Returns the current item's key.

    \sa value()
*/

/*! \fn PitmValue PitmObject::const_iterator::value() const

    Returns the current item's value.

    \sa key(), operator*()
*/

/*! \fn PitmValue PitmObject::const_iterator::operator*() const

    Returns the current item's value.

    Same as value().

    \sa key()
*/

/*! \fn PitmValue *PitmObject::const_iterator::operator->() const

    Returns a pointer to the current item.
*/

/*! \fn bool PitmObject::const_iterator::operator==(const const_iterator &other) const
    \fn bool PitmObject::const_iterator::operator==(const iterator &other) const

    Returns \c true if \a other points to the same item as this
    iterator; otherwise returns \c false.

    \sa operator!=()
*/

/*! \fn bool PitmObject::const_iterator::operator!=(const const_iterator &other) const
    \fn bool PitmObject::const_iterator::operator!=(const iterator &other) const

    Returns \c true if \a other points to a different item than this
    iterator; otherwise returns \c false.

    \sa operator==()
*/

/*! \fn PitmObject::const_iterator PitmObject::const_iterator::operator++()

    The prefix ++ operator, \c{++i}, advances the iterator to the
    next item in the object and returns an iterator to the new current
    item.

    Calling this function on PitmObject::end() leads to undefined results.

    \sa operator--()
*/

/*! \fn PitmObject::const_iterator PitmObject::const_iterator::operator++(int)

    \overload

    The postfix ++ operator, \c{i++}, advances the iterator to the
    next item in the object and returns an iterator to the previously
    current item.
*/

/*! \fn PitmObject::const_iterator &PitmObject::const_iterator::operator--()

    The prefix -- operator, \c{--i}, makes the preceding item
    current and returns an iterator pointing to the new current item.

    Calling this function on PitmObject::begin() leads to undefined
    results.

    \sa operator++()
*/

/*! \fn PitmObject::const_iterator PitmObject::const_iterator::operator--(int)

    \overload

    The postfix -- operator, \c{i--}, makes the preceding item
    current and returns an iterator pointing to the previously
    current item.
*/

/*! \fn PitmObject::const_iterator PitmObject::const_iterator::operator+(int j) const

    Returns an iterator to the item at \a j positions forward from
    this iterator. If \a j is negative, the iterator goes backward.

    This operation can be slow for large \a j values.

    \sa operator-()
*/

/*! \fn PitmObject::const_iterator PitmObject::const_iterator::operator-(int j) const

    Returns an iterator to the item at \a j positions backward from
    this iterator. If \a j is negative, the iterator goes forward.

    This operation can be slow for large \a j values.

    \sa operator+()
*/

/*! \fn PitmObject::const_iterator &PitmObject::const_iterator::operator+=(int j)

    Advances the iterator by \a j items. If \a j is negative, the
    iterator goes backward.

    This operation can be slow for large \a j values.

    \sa operator-=(), operator+()
*/

/*! \fn PitmObject::const_iterator &PitmObject::const_iterator::operator-=(int j)

    Makes the iterator go back by \a j items. If \a j is negative,
    the iterator goes forward.

    This operation can be slow for large \a j values.

    \sa operator+=(), operator-()
*/


/*!
    \internal
 */
void PitmObject::detach(uint reserve)
{
    Q_UNUSED(reserve)
    Q_ASSERT(!reserve);
    detach2(reserve);
}

bool PitmObject::detach2(uint reserve)
{
    if (!d) {
        if (reserve >= PitmPrivate::Value::MaxSize) {
            qWarning("Pitm: Document too large to store in data structure");
            return false;
        }
        d = new PitmPrivate::Data(reserve, PitmValue::Object);
        o = static_cast<PitmPrivate::Object *>(d->header->root());
        d->ref.ref();
        return true;
    }
    if (reserve == 0 && d->ref.load() == 1)
        return true;

    PitmPrivate::Data *x = d->clone(o, reserve);
    if (!x)
        return false;
    x->ref.ref();
    if (!d->ref.deref())
        delete d;
    d = x;
    o = static_cast<PitmPrivate::Object *>(d->header->root());
    return true;
}

/*!
    \internal
 */
void PitmObject::compact()
{
    if (!d || !d->compactionCounter)
        return;

    detach2();
    d->compact();
    o = static_cast<PitmPrivate::Object *>(d->header->root());
}

/*!
    \internal
 */
QString PitmObject::keyAt(int i) const
{
    Q_ASSERT(o && i >= 0 && i < (int)o->length);

    PitmPrivate::Entry *e = o->entryAt(i);
    return e->key();
}

/*!
    \internal
 */
PitmValue PitmObject::valueAt(int i) const
{
    if (!o || i < 0 || i >= (int)o->length)
        return PitmValue(PitmValue::Undefined);

    PitmPrivate::Entry *e = o->entryAt(i);
    return PitmValue(d, o, e->value);
}

/*!
    \internal
 */
void PitmObject::setValueAt(int i, const PitmValue &val)
{
    Q_ASSERT(o && i >= 0 && i < (int)o->length);

    PitmPrivate::Entry *e = o->entryAt(i);
    insert(e->key(), val);
}

#if !defined(QT_NO_DEBUG_STREAM) && !defined(QT_PITM_READONLY)
QDebug operator<<(QDebug dbg, const PitmObject &o)
{
    QDebugStateSaver saver(dbg);
    if (!o.o) {
        dbg << "PitmObject()";
        return dbg;
    }
    QByteArray pitm;
    PitmPrivate::Writer::objectToPitm(o.o, pitm, 0, true);
    dbg.nospace() << "PitmObject("
                  << pitm.constData() // print as utf-8 string without extra quotation marks
                  << ")";
    return dbg;
}
#endif

QT_END_NAMESPACE
