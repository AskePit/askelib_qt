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
#include <qvariant.h>
#include <qdebug.h>

#include "pitmwriter_p.h"
#include "pitm_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class PitmArray
    \inmodule QtCore
    \ingroup pitm
    \ingroup shared
    \reentrant
    \since 5.0

    \brief The PitmArray class encapsulates a PITM array.

    A PITM array is a list of values. The list can be manipulated by inserting and
    removing PitmValue's from the array.

    A PitmArray can be converted to and from a QVariantList. You can query the
    number of entries with size(), insert(), and removeAt() entries from it
    and iterate over its content using the standard C++ iterator pattern.

    PitmArray is an implicitly shared class and shares the data with the document
    it has been created from as long as it is not being modified.

    You can convert the array to and from text based PITM through PitmDocument.

    \sa {PITM Support in Qt}, {PITM Save Game Example}
*/

/*!
    \typedef PitmArray::Iterator

    Qt-style synonym for PitmArray::iterator.
*/

/*!
    \typedef PitmArray::ConstIterator

    Qt-style synonym for PitmArray::const_iterator.
*/

/*!
    \typedef PitmArray::size_type

    Typedef for int. Provided for STL compatibility.
*/

/*!
    \typedef PitmArray::value_type

    Typedef for PitmValue. Provided for STL compatibility.
*/

/*!
    \typedef PitmArray::difference_type

    Typedef for int. Provided for STL compatibility.
*/

/*!
    \typedef PitmArray::pointer

    Typedef for PitmValue *. Provided for STL compatibility.
*/

/*!
    \typedef PitmArray::const_pointer

    Typedef for const PitmValue *. Provided for STL compatibility.
*/

/*!
    \typedef PitmArray::reference

    Typedef for PitmValue &. Provided for STL compatibility.
*/

/*!
    \typedef PitmArray::const_reference

    Typedef for const PitmValue &. Provided for STL compatibility.
*/

/*!
    Creates an empty array.
 */
PitmArray::PitmArray()
    : d(0), a(0)
{
}

/*!
    \fn PitmArray::PitmArray(std::initializer_list<PitmValue> args)
    \since 5.4
    Creates an array initialized from \a args initialization list.

    PitmArray can be constructed in a way similar to PITM notation,
    for example:
    \code
    PitmArray array = { 1, 2.2, QString() };
    \endcode
 */

/*!
    \internal
 */
PitmArray::PitmArray(PitmPrivate::Data *data, PitmPrivate::Array *array)
    : d(data), a(array)
{
    Q_ASSERT(data);
    Q_ASSERT(array);
    d->ref.ref();
}

/*!
    This method replaces part of PitmArray(std::initializer_list<PitmValue> args) .
    The constructor needs to be inline, but we do not want to leak implementation details
    of this class.
    \note this method is called for an uninitialized object
    \internal
 */
void PitmArray::initialize()
{
    d = 0;
    a = 0;
}

/*!
    Deletes the array.
 */
PitmArray::~PitmArray()
{
    if (d && !d->ref.deref())
        delete d;
}

/*!
    Creates a copy of \a other.

    Since PitmArray is implicitly shared, the copy is shallow
    as long as the object doesn't get modified.
 */
PitmArray::PitmArray(const PitmArray &other)
{
    d = other.d;
    a = other.a;
    if (d)
        d->ref.ref();
}

/*!
    Assigns \a other to this array.
 */
PitmArray &PitmArray::operator =(const PitmArray &other)
{
    if (d != other.d) {
        if (d && !d->ref.deref())
            delete d;
        d = other.d;
        if (d)
            d->ref.ref();
    }
    a = other.a;

    return *this;
}

/*! \fn PitmArray &PitmArray::operator+=(const PitmValue &value)

    Appends \a value to the array, and returns a reference to the array itself.

    \since 5.3
    \sa append(), operator<<()
*/

/*! \fn PitmArray PitmArray::operator+(const PitmValue &value) const

    Returns an array that contains all the items in this array followed
    by the provided \a value.

    \since 5.3
    \sa operator+=()
*/

/*! \fn PitmArray &PitmArray::operator<<(const PitmValue &value)

    Appends \a value to the array, and returns a reference to the array itself.

    \since 5.3
    \sa operator+=(), append()
*/

/*!
    Converts the string list \a list to a PitmArray.

    The values in \a list will be converted to PITM values.

    \sa toVariantList(), PitmValue::fromVariant()
 */
PitmArray PitmArray::fromStringList(const QStringList &list)
{
    PitmArray array;
    for (QStringList::const_iterator it = list.constBegin(); it != list.constEnd(); ++it)
        array.append(PitmValue(*it));
    return array;
}

/*!
    Converts the variant list \a list to a PitmArray.

    The QVariant values in \a list will be converted to PITM values.

    \sa toVariantList(), PitmValue::fromVariant()
 */
PitmArray PitmArray::fromVariantList(const QVariantList &list)
{
    PitmArray array;
    if (list.isEmpty())
        return array;

    array.detach2(1024);

    QVector<PitmPrivate::Value> values;
    values.resize(list.size());
    PitmPrivate::Value *valueData = values.data();
    uint currentOffset = sizeof(PitmPrivate::Base);

    for (int i = 0; i < list.size(); ++i) {
        PitmValue val = PitmValue::fromVariant(list.at(i));

        bool latinOrIntValue;
        int valueSize = PitmPrivate::Value::requiredStorage(val, &latinOrIntValue);

        if (!array.detach2(valueSize))
            return PitmArray();

        PitmPrivate::Value *v = valueData + i;
        v->type = (val.t == PitmValue::Undefined ? PitmValue::Null : val.t);
        v->latinOrIntValue = latinOrIntValue;
        v->latinKey = false;
        v->value = PitmPrivate::Value::valueToStore(val, currentOffset);
        if (valueSize)
            PitmPrivate::Value::copyData(val, (char *)array.a + currentOffset, latinOrIntValue);

        currentOffset += valueSize;
        array.a->size = currentOffset;
    }

    // write table
    array.a->tableOffset = currentOffset;
    if (!array.detach2(sizeof(PitmPrivate::offset)*values.size()))
        return PitmArray();
    memcpy(array.a->table(), values.constData(), values.size()*sizeof(uint));
    array.a->length = values.size();
    array.a->size = currentOffset + sizeof(PitmPrivate::offset)*values.size();

    return array;
}

/*!
    Converts this object to a QVariantList.

    Returns the created map.
 */
QVariantList PitmArray::toVariantList() const
{
    QVariantList list;

    if (a) {
        list.reserve(a->length);
        for (int i = 0; i < (int)a->length; ++i)
            list.append(PitmValue(d, a, a->at(i)).toVariant());
    }
    return list;
}


/*!
    Returns the number of values stored in the array.
 */
int PitmArray::size() const
{
    if (!d)
        return 0;

    return (int)a->length;
}

/*!
    \fn PitmArray::count() const

    Same as size().

    \sa size()
*/

/*!
    Returns \c true if the object is empty. This is the same as size() == 0.

    \sa size()
 */
bool PitmArray::isEmpty() const
{
    if (!d)
        return true;

    return !a->length;
}

/*!
    Returns a PitmValue representing the value for index \a i.

    The returned PitmValue is \c Undefined, if \a i is out of bounds.

 */
PitmValue PitmArray::at(int i) const
{
    if (!a || i < 0 || i >= (int)a->length)
        return PitmValue(PitmValue::Undefined);

    return PitmValue(d, a, a->at(i));
}

/*!
    Returns the first value stored in the array.

    Same as \c at(0).

    \sa at()
 */
PitmValue PitmArray::first() const
{
    return at(0);
}

/*!
    Returns the last value stored in the array.

    Same as \c{at(size() - 1)}.

    \sa at()
 */
PitmValue PitmArray::last() const
{
    return at(a ? (a->length - 1) : 0);
}

/*!
    Inserts \a value at the beginning of the array.

    This is the same as \c{insert(0, value)} and will prepend \a value to the array.

    \sa append(), insert()
 */
void PitmArray::prepend(const PitmValue &value)
{
    insert(0, value);
}

/*!
    Inserts \a value at the end of the array.

    \sa prepend(), insert()
 */
void PitmArray::append(const PitmValue &value)
{
    insert(a ? (int)a->length : 0, value);
}

/*!
    Removes the value at index position \a i. \a i must be a valid
    index position in the array (i.e., \c{0 <= i < size()}).

    \sa insert(), replace()
 */
void PitmArray::removeAt(int i)
{
    if (!a || i < 0 || i >= (int)a->length)
        return;

    detach2();
    a->removeItems(i, 1);
    ++d->compactionCounter;
    if (d->compactionCounter > 32u && d->compactionCounter >= unsigned(a->length) / 2u)
        compact();
}

/*! \fn void PitmArray::removeFirst()

    Removes the first item in the array. Calling this function is
    equivalent to calling \c{removeAt(0)}. The array must not be empty. If
    the array can be empty, call isEmpty() before calling this
    function.

    \sa removeAt(), removeLast()
*/

/*! \fn void PitmArray::removeLast()

    Removes the last item in the array. Calling this function is
    equivalent to calling \c{removeAt(size() - 1)}. The array must not be
    empty. If the array can be empty, call isEmpty() before calling
    this function.

    \sa removeAt(), removeFirst()
*/

/*!
    Removes the item at index position \a i and returns it. \a i must
    be a valid index position in the array (i.e., \c{0 <= i < size()}).

    If you don't use the return value, removeAt() is more efficient.

    \sa removeAt()
 */
PitmValue PitmArray::takeAt(int i)
{
    if (!a || i < 0 || i >= (int)a->length)
        return PitmValue(PitmValue::Undefined);

    PitmValue v(d, a, a->at(i));
    removeAt(i); // detaches
    return v;
}

/*!
    Inserts \a value at index position \a i in the array. If \a i
    is \c 0, the value is prepended to the array. If \a i is size(), the
    value is appended to the array.

    \sa append(), prepend(), replace(), removeAt()
 */
void PitmArray::insert(int i, const PitmValue &value)
{
    Q_ASSERT (i >= 0 && i <= (a ? (int)a->length : 0));
    PitmValue val = value;

    bool compressed;
    int valueSize = PitmPrivate::Value::requiredStorage(val, &compressed);

    if (!detach2(valueSize + sizeof(PitmPrivate::Value)))
        return;

    if (!a->length)
        a->tableOffset = sizeof(PitmPrivate::Array);

    int valueOffset = a->reserveSpace(valueSize, i, 1, false);
    if (!valueOffset)
        return;

    PitmPrivate::Value &v = (*a)[i];
    v.type = (val.t == PitmValue::Undefined ? PitmValue::Null : val.t);
    v.latinOrIntValue = compressed;
    v.latinKey = false;
    v.value = PitmPrivate::Value::valueToStore(val, valueOffset);
    if (valueSize)
        PitmPrivate::Value::copyData(val, (char *)a + valueOffset, compressed);
}

/*!
    \fn PitmArray::iterator PitmArray::insert(iterator before, const PitmValue &value)

    Inserts \a value before the position pointed to by \a before, and returns an iterator
    pointing to the newly inserted item.

    \sa erase(), insert()
*/

/*!
    \fn PitmArray::iterator PitmArray::erase(iterator it)

    Removes the item pointed to by \a it, and returns an iterator pointing to the
    next item.

    \sa removeAt()
*/

/*!
    Replaces the item at index position \a i with \a value. \a i must
    be a valid index position in the array (i.e., \c{0 <= i < size()}).

    \sa operator[](), removeAt()
 */
void PitmArray::replace(int i, const PitmValue &value)
{
    Q_ASSERT (a && i >= 0 && i < (int)(a->length));
    PitmValue val = value;

    bool compressed;
    int valueSize = PitmPrivate::Value::requiredStorage(val, &compressed);

    if (!detach2(valueSize))
        return;

    if (!a->length)
        a->tableOffset = sizeof(PitmPrivate::Array);

    int valueOffset = a->reserveSpace(valueSize, i, 1, true);
    if (!valueOffset)
        return;

    PitmPrivate::Value &v = (*a)[i];
    v.type = (val.t == PitmValue::Undefined ? PitmValue::Null : val.t);
    v.latinOrIntValue = compressed;
    v.latinKey = false;
    v.value = PitmPrivate::Value::valueToStore(val, valueOffset);
    if (valueSize)
        PitmPrivate::Value::copyData(val, (char *)a + valueOffset, compressed);

    ++d->compactionCounter;
    if (d->compactionCounter > 32u && d->compactionCounter >= unsigned(a->length) / 2u)
        compact();
}

/*!
    Returns \c true if the array contains an occurrence of \a value, otherwise \c false.

    \sa count()
 */
bool PitmArray::contains(const PitmValue &value) const
{
    for (int i = 0; i < size(); i++) {
        if (at(i) == value)
            return true;
    }
    return false;
}

/*!
    Returns the value at index position \a i as a modifiable reference.
    \a i must be a valid index position in the array (i.e., \c{0 <= i <
    size()}).

    The return value is of type PitmValueRef, a helper class for PitmArray
    and PitmObject. When you get an object of type PitmValueRef, you can
    use it as if it were a reference to a PitmValue. If you assign to it,
    the assignment will apply to the character in the PitmArray of PitmObject
    from which you got the reference.

    \sa at()
 */
PitmValueRef PitmArray::operator [](int i)
{
    Q_ASSERT(a && i >= 0 && i < (int)a->length);
    return PitmValueRef(this, i);
}

/*!
    \overload

    Same as at().
 */
PitmValue PitmArray::operator[](int i) const
{
    return at(i);
}

/*!
    Returns \c true if this array is equal to \a other.
 */
bool PitmArray::operator==(const PitmArray &other) const
{
    if (a == other.a)
        return true;

    if (!a)
        return !other.a->length;
    if (!other.a)
        return !a->length;
    if (a->length != other.a->length)
        return false;

    for (int i = 0; i < (int)a->length; ++i) {
        if (PitmValue(d, a, a->at(i)) != PitmValue(other.d, other.a, other.a->at(i)))
            return false;
    }
    return true;
}

/*!
    Returns \c true if this array is not equal to \a other.
 */
bool PitmArray::operator!=(const PitmArray &other) const
{
    return !(*this == other);
}

/*! \fn PitmArray::iterator PitmArray::begin()

    Returns an \l{STL-style iterators}{STL-style iterator} pointing to the first item in
    the array.

    \sa constBegin(), end()
*/

/*! \fn PitmArray::const_iterator PitmArray::begin() const

    \overload
*/

/*! \fn PitmArray::const_iterator PitmArray::constBegin() const

    Returns a const \l{STL-style iterators}{STL-style iterator} pointing to the first item
    in the array.

    \sa begin(), constEnd()
*/

/*! \fn PitmArray::iterator PitmArray::end()

    Returns an \l{STL-style iterators}{STL-style iterator} pointing to the imaginary item
    after the last item in the array.

    \sa begin(), constEnd()
*/

/*! \fn const_iterator PitmArray::end() const

    \overload
*/

/*! \fn PitmArray::const_iterator PitmArray::constEnd() const

    Returns a const \l{STL-style iterators}{STL-style iterator} pointing to the imaginary
    item after the last item in the array.

    \sa constBegin(), end()
*/

/*! \fn void PitmArray::push_back(const PitmValue &value)

    This function is provided for STL compatibility. It is equivalent
    to \l{PitmArray::append()}{append(value)} and will append \a value to the array.
*/

/*! \fn void PitmArray::push_front(const PitmValue &value)

    This function is provided for STL compatibility. It is equivalent
    to \l{PitmArray::prepend()}{prepend(value)} and will prepend \a value to the array.
*/

/*! \fn void PitmArray::pop_front()

    This function is provided for STL compatibility. It is equivalent
    to removeFirst(). The array must not be empty. If the array can be
    empty, call isEmpty() before calling this function.
*/

/*! \fn void PitmArray::pop_back()

    This function is provided for STL compatibility. It is equivalent
    to removeLast(). The array must not be empty. If the array can be
    empty, call isEmpty() before calling this function.
*/

/*! \fn bool PitmArray::empty() const

    This function is provided for STL compatibility. It is equivalent
    to isEmpty() and returns \c true if the array is empty.
*/

/*! \class PitmArray::iterator
    \inmodule QtCore
    \brief The PitmArray::iterator class provides an STL-style non-const iterator for PitmArray.

    PitmArray::iterator allows you to iterate over a PitmArray
    and to modify the array item associated with the
    iterator. If you want to iterate over a const PitmArray, use
    PitmArray::const_iterator instead. It is generally a good practice to
    use PitmArray::const_iterator on a non-const PitmArray as well, unless
    you need to change the PitmArray through the iterator. Const
    iterators are slightly faster and improves code readability.

    The default PitmArray::iterator constructor creates an uninitialized
    iterator. You must initialize it using a PitmArray function like
    PitmArray::begin(), PitmArray::end(), or PitmArray::insert() before you can
    start iterating.

    Most PitmArray functions accept an integer index rather than an
    iterator. For that reason, iterators are rarely useful in
    connection with PitmArray. One place where STL-style iterators do
    make sense is as arguments to \l{generic algorithms}.

    Multiple iterators can be used on the same array. However, be
    aware that any non-const function call performed on the PitmArray
    will render all existing iterators undefined.

    \sa PitmArray::const_iterator
*/

/*! \typedef PitmArray::iterator::iterator_category

  A synonym for \e {std::random_access_iterator_tag} indicating
  this iterator is a random access iterator.
*/

/*! \typedef PitmArray::iterator::difference_type

    \internal
*/

/*! \typedef PitmArray::iterator::value_type

    \internal
*/

/*! \typedef PitmArray::iterator::reference

    \internal
*/

/*! \typedef PitmArray::iterator::pointer

    \internal
*/

/*! \fn PitmArray::iterator::iterator()

    Constructs an uninitialized iterator.

    Functions like operator*() and operator++() should not be called
    on an uninitialized iterator. Use operator=() to assign a value
    to it before using it.

    \sa PitmArray::begin(), PitmArray::end()
*/

/*! \fn PitmArray::iterator::iterator(PitmArray *array, int index)
    \internal
*/

/*! \fn PitmValueRef PitmArray::iterator::operator*() const


    Returns a modifiable reference to the current item.

    You can change the value of an item by using operator*() on the
    left side of an assignment.

    The return value is of type PitmValueRef, a helper class for PitmArray
    and PitmObject. When you get an object of type PitmValueRef, you can
    use it as if it were a reference to a PitmValue. If you assign to it,
    the assignment will apply to the character in the PitmArray of PitmObject
    from which you got the reference.
*/

/*! \fn PitmValueRef *PitmArray::iterator::operator->() const

    Returns a pointer to a modifiable reference to the current item.
*/

/*! \fn PitmValueRef PitmArray::iterator::operator[](int j) const

    Returns a modifiable reference to the item at offset \a j from the
    item pointed to by this iterator (the item at position \c{*this + j}).

    This function is provided to make PitmArray iterators behave like C++
    pointers.

    The return value is of type PitmValueRef, a helper class for PitmArray
    and PitmObject. When you get an object of type PitmValueRef, you can
    use it as if it were a reference to a PitmValue. If you assign to it,
    the assignment will apply to the character in the PitmArray of PitmObject
    from which you got the reference.

    \sa operator+()
*/

/*!
    \fn bool PitmArray::iterator::operator==(const iterator &other) const
    \fn bool PitmArray::iterator::operator==(const const_iterator &other) const

    Returns \c true if \a other points to the same item as this
    iterator; otherwise returns \c false.

    \sa operator!=()
*/

/*!
    \fn bool PitmArray::iterator::operator!=(const iterator &other) const
    \fn bool PitmArray::iterator::operator!=(const const_iterator &other) const

    Returns \c true if \a other points to a different item than this
    iterator; otherwise returns \c false.

    \sa operator==()
*/

/*!
    \fn bool PitmArray::iterator::operator<(const iterator& other) const
    \fn bool PitmArray::iterator::operator<(const const_iterator& other) const

    Returns \c true if the item pointed to by this iterator is less than
    the item pointed to by the \a other iterator.
*/

/*!
    \fn bool PitmArray::iterator::operator<=(const iterator& other) const
    \fn bool PitmArray::iterator::operator<=(const const_iterator& other) const

    Returns \c true if the item pointed to by this iterator is less than
    or equal to the item pointed to by the \a other iterator.
*/

/*!
    \fn bool PitmArray::iterator::operator>(const iterator& other) const
    \fn bool PitmArray::iterator::operator>(const const_iterator& other) const

    Returns \c true if the item pointed to by this iterator is greater
    than the item pointed to by the \a other iterator.
*/

/*!
    \fn bool PitmArray::iterator::operator>=(const iterator& other) const
    \fn bool PitmArray::iterator::operator>=(const const_iterator& other) const

    Returns \c true if the item pointed to by this iterator is greater
    than or equal to the item pointed to by the \a other iterator.
*/

/*! \fn PitmArray::iterator &PitmArray::iterator::operator++()

    The prefix ++ operator, \c{++it}, advances the iterator to the
    next item in the array and returns an iterator to the new current
    item.

    Calling this function on PitmArray::end() leads to undefined results.

    \sa operator--()
*/

/*! \fn PitmArray::iterator PitmArray::iterator::operator++(int)

    \overload

    The postfix ++ operator, \c{it++}, advances the iterator to the
    next item in the array and returns an iterator to the previously
    current item.
*/

/*! \fn PitmArray::iterator &PitmArray::iterator::operator--()

    The prefix -- operator, \c{--it}, makes the preceding item
    current and returns an iterator to the new current item.

    Calling this function on PitmArray::begin() leads to undefined results.

    \sa operator++()
*/

/*! \fn PitmArray::iterator PitmArray::iterator::operator--(int)

    \overload

    The postfix -- operator, \c{it--}, makes the preceding item
    current and returns an iterator to the previously current item.
*/

/*! \fn PitmArray::iterator &PitmArray::iterator::operator+=(int j)

    Advances the iterator by \a j items. If \a j is negative, the
    iterator goes backward.

    \sa operator-=(), operator+()
*/

/*! \fn PitmArray::iterator &PitmArray::iterator::operator-=(int j)

    Makes the iterator go back by \a j items. If \a j is negative,
    the iterator goes forward.

    \sa operator+=(), operator-()
*/

/*! \fn PitmArray::iterator PitmArray::iterator::operator+(int j) const

    Returns an iterator to the item at \a j positions forward from
    this iterator. If \a j is negative, the iterator goes backward.

    \sa operator-(), operator+=()
*/

/*! \fn PitmArray::iterator PitmArray::iterator::operator-(int j) const

    Returns an iterator to the item at \a j positions backward from
    this iterator. If \a j is negative, the iterator goes forward.

    \sa operator+(), operator-=()
*/

/*! \fn int PitmArray::iterator::operator-(iterator other) const

    Returns the number of items between the item pointed to by \a
    other and the item pointed to by this iterator.
*/

/*! \class PitmArray::const_iterator
    \inmodule QtCore
    \brief The PitmArray::const_iterator class provides an STL-style const iterator for PitmArray.

    PitmArray::const_iterator allows you to iterate over a
    PitmArray. If you want to modify the PitmArray as
    you iterate over it, use PitmArray::iterator instead. It is generally a
    good practice to use PitmArray::const_iterator on a non-const PitmArray
    as well, unless you need to change the PitmArray through the
    iterator. Const iterators are slightly faster and improves
    code readability.

    The default PitmArray::const_iterator constructor creates an
    uninitialized iterator. You must initialize it using a PitmArray
    function like PitmArray::constBegin(), PitmArray::constEnd(), or
    PitmArray::insert() before you can start iterating.

    Most PitmArray functions accept an integer index rather than an
    iterator. For that reason, iterators are rarely useful in
    connection with PitmArray. One place where STL-style iterators do
    make sense is as arguments to \l{generic algorithms}.

    Multiple iterators can be used on the same array. However, be
    aware that any non-const function call performed on the PitmArray
    will render all existing iterators undefined.

    \sa PitmArray::iterator
*/

/*! \fn PitmArray::const_iterator::const_iterator()

    Constructs an uninitialized iterator.

    Functions like operator*() and operator++() should not be called
    on an uninitialized iterator. Use operator=() to assign a value
    to it before using it.

    \sa PitmArray::constBegin(), PitmArray::constEnd()
*/

/*! \fn PitmArray::const_iterator::const_iterator(const PitmArray *array, int index)
    \internal
*/

/*! \typedef PitmArray::const_iterator::iterator_category

  A synonym for \e {std::random_access_iterator_tag} indicating
  this iterator is a random access iterator.
*/

/*! \typedef PitmArray::const_iterator::difference_type

    \internal
*/

/*! \typedef PitmArray::const_iterator::value_type

    \internal
*/

/*! \typedef PitmArray::const_iterator::reference

    \internal
*/

/*! \typedef PitmArray::const_iterator::pointer

    \internal
*/

/*! \fn PitmArray::const_iterator::const_iterator(const const_iterator &other)

    Constructs a copy of \a other.
*/

/*! \fn PitmArray::const_iterator::const_iterator(const iterator &other)

    Constructs a copy of \a other.
*/

/*! \fn PitmValue PitmArray::const_iterator::operator*() const

    Returns the current item.
*/

/*! \fn PitmValue *PitmArray::const_iterator::operator->() const

    Returns a pointer to the current item.
*/

/*! \fn PitmValue PitmArray::const_iterator::operator[](int j) const

    Returns the item at offset \a j from the item pointed to by this iterator (the item at
    position \c{*this + j}).

    This function is provided to make PitmArray iterators behave like C++
    pointers.

    \sa operator+()
*/

/*! \fn bool PitmArray::const_iterator::operator==(const const_iterator &other) const

    Returns \c true if \a other points to the same item as this
    iterator; otherwise returns \c false.

    \sa operator!=()
*/

/*! \fn bool PitmArray::const_iterator::operator!=(const const_iterator &other) const

    Returns \c true if \a other points to a different item than this
    iterator; otherwise returns \c false.

    \sa operator==()
*/

/*!
    \fn bool PitmArray::const_iterator::operator<(const const_iterator& other) const

    Returns \c true if the item pointed to by this iterator is less than
    the item pointed to by the \a other iterator.
*/

/*!
    \fn bool PitmArray::const_iterator::operator<=(const const_iterator& other) const

    Returns \c true if the item pointed to by this iterator is less than
    or equal to the item pointed to by the \a other iterator.
*/

/*!
    \fn bool PitmArray::const_iterator::operator>(const const_iterator& other) const

    Returns \c true if the item pointed to by this iterator is greater
    than the item pointed to by the \a other iterator.
*/

/*!
    \fn bool PitmArray::const_iterator::operator>=(const const_iterator& other) const

    Returns \c true if the item pointed to by this iterator is greater
    than or equal to the item pointed to by the \a other iterator.
*/

/*! \fn PitmArray::const_iterator &PitmArray::const_iterator::operator++()

    The prefix ++ operator, \c{++it}, advances the iterator to the
    next item in the array and returns an iterator to the new current
    item.

    Calling this function on PitmArray::end() leads to undefined results.

    \sa operator--()
*/

/*! \fn PitmArray::const_iterator PitmArray::const_iterator::operator++(int)

    \overload

    The postfix ++ operator, \c{it++}, advances the iterator to the
    next item in the array and returns an iterator to the previously
    current item.
*/

/*! \fn PitmArray::const_iterator &PitmArray::const_iterator::operator--()

    The prefix -- operator, \c{--it}, makes the preceding item
    current and returns an iterator to the new current item.

    Calling this function on PitmArray::begin() leads to undefined results.

    \sa operator++()
*/

/*! \fn PitmArray::const_iterator PitmArray::const_iterator::operator--(int)

    \overload

    The postfix -- operator, \c{it--}, makes the preceding item
    current and returns an iterator to the previously current item.
*/

/*! \fn PitmArray::const_iterator &PitmArray::const_iterator::operator+=(int j)

    Advances the iterator by \a j items. If \a j is negative, the
    iterator goes backward.

    \sa operator-=(), operator+()
*/

/*! \fn PitmArray::const_iterator &PitmArray::const_iterator::operator-=(int j)

    Makes the iterator go back by \a j items. If \a j is negative,
    the iterator goes forward.

    \sa operator+=(), operator-()
*/

/*! \fn PitmArray::const_iterator PitmArray::const_iterator::operator+(int j) const

    Returns an iterator to the item at \a j positions forward from
    this iterator. If \a j is negative, the iterator goes backward.

    \sa operator-(), operator+=()
*/

/*! \fn PitmArray::const_iterator PitmArray::const_iterator::operator-(int j) const

    Returns an iterator to the item at \a j positions backward from
    this iterator. If \a j is negative, the iterator goes forward.

    \sa operator+(), operator-=()
*/

/*! \fn int PitmArray::const_iterator::operator-(const_iterator other) const

    Returns the number of items between the item pointed to by \a
    other and the item pointed to by this iterator.
*/


/*!
    \internal
 */
void PitmArray::detach(uint reserve)
{
    Q_UNUSED(reserve)
    Q_ASSERT(!reserve);
    detach2(0);
}

/*!
    \internal
 */
bool PitmArray::detach2(uint reserve)
{
    if (!d) {
        if (reserve >= PitmPrivate::Value::MaxSize) {
            qWarning("Pitm: Document too large to store in data structure");
            return false;
        }
        d = new PitmPrivate::Data(reserve, PitmValue::Array);
        a = static_cast<PitmPrivate::Array *>(d->header->root());
        d->ref.ref();
        return true;
    }
    if (reserve == 0 && d->ref.load() == 1)
        return true;

    PitmPrivate::Data *x = d->clone(a, reserve);
    if (!x)
        return false;
    x->ref.ref();
    if (!d->ref.deref())
        delete d;
    d = x;
    a = static_cast<PitmPrivate::Array *>(d->header->root());
    return true;
}

/*!
    \internal
 */
void PitmArray::compact()
{
    if (!d || !d->compactionCounter)
        return;

    detach2();
    d->compact();
    a = static_cast<PitmPrivate::Array *>(d->header->root());
}


#if !defined(QT_NO_DEBUG_STREAM) && !defined(QT_PITM_READONLY)
QDebug operator<<(QDebug dbg, const PitmArray &a)
{
    QDebugStateSaver saver(dbg);
    if (!a.a) {
        dbg << "PitmArray()";
        return dbg;
    }
    QByteArray pitm;
    PitmPrivate::Writer::arrayToPitm(a.a, pitm, 0, true);
    dbg.nospace() << "PitmArray("
                  << pitm.constData() // print as utf-8 string without extra quotation marks
                  << ")";
    return dbg;
}
#endif

QT_END_NAMESPACE

