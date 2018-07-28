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

#ifndef PITMDOCUMENT_H
#define PITMDOCUMENT_H

#include "pitmvalue.h"

QT_BEGIN_NAMESPACE

class QDebug;

namespace PitmPrivate {
    class Parser;
}

struct PitmParseError
{
    enum ParseError {
        NoError = 0,
        UnterminatedObject,
        MissingNameSeparator,
        UnterminatedArray,
        MissingValueSeparator,
        IllegalValue,
        TerminationByNumber,
        IllegalNumber,
        IllegalEscapeSequence,
        IllegalUTF8String,
        UnterminatedString,
        MissingObject,
        DeepNesting,
        DocumentTooLarge,
        GarbageAtEnd
    };

    QString    errorString() const;

    int        offset;
    ParseError error;
};

class PitmDocument
{
public:
#ifdef Q_LITTLE_ENDIAN
    static const uint BinaryFormatTag = ('q') | ('b' << 8) | ('j' << 16) | ('s' << 24);
#else
    static const uint BinaryFormatTag = ('q' << 24) | ('b' << 16) | ('j' << 8) | ('s');
#endif

    PitmDocument();
    explicit PitmDocument(const PitmObject &object);
    explicit PitmDocument(const PitmArray &array);
    ~PitmDocument();

    PitmDocument(const PitmDocument &other);
    PitmDocument &operator =(const PitmDocument &other);

    enum DataValidation {
        Validate,
        BypassValidation
    };

    static PitmDocument fromRawData(const char *data, int size, DataValidation validation = Validate);
    const char *rawData(int *size) const;

    static PitmDocument fromBinaryData(const QByteArray &data, DataValidation validation  = Validate);
    QByteArray toBinaryData() const;

    static PitmDocument fromVariant(const QVariant &variant);
    QVariant toVariant() const;

    enum PitmFormat {
        Indented,
        Compact
    };

    static PitmDocument fromPitm(const QByteArray &pitm, PitmParseError *error = Q_NULLPTR);

#ifdef Q_QDOC
    QByteArray toPitm(PitmFormat format = Indented) const;
#elif !defined(QT_PITM_READONLY)
    QByteArray toPitm() const; //### Merge in Qt6
    QByteArray toPitm(PitmFormat format) const;
#endif

    bool isEmpty() const;
    bool isArray() const;
    bool isObject() const;

    PitmObject object() const;
    PitmArray array() const;

    void setObject(const PitmObject &object);
    void setArray(const PitmArray &array);

    bool operator==(const PitmDocument &other) const;
    bool operator!=(const PitmDocument &other) const { return !(*this == other); }

    bool isNull() const;

private:
    friend class PitmValue;
    friend class PitmPrivate::Data;
    friend class PitmPrivate::Parser;
    friend QDebug operator<<(QDebug, const PitmDocument &);

    PitmDocument(PitmPrivate::Data *data);

    PitmPrivate::Data *d;
};

#if !defined(QT_NO_DEBUG_STREAM) && !defined(QT_PITM_READONLY)
QDebug operator<<(QDebug, const PitmDocument &);
#endif

QT_END_NAMESPACE

#endif // PITMDOCUMENT_H
