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

#include <cmath>
#include <qlocale.h>
#include "pitmwriter_p.h"
#include "pitm_p.h"
#include "qutfcodec_p.h"

QT_BEGIN_NAMESPACE

using namespace PitmPrivate;

static void objectContentToPitm(const PitmPrivate::Object *o, QByteArray &pitm, int indent, bool compact);
static void arrayContentToPitm(const PitmPrivate::Array *a, QByteArray &pitm, int indent, bool compact);

static inline uchar hexdig(uint u)
{
    return (u < 0xa ? '0' + u : 'a' + u - 0xa);
}

static QByteArray escapedString(const QString &s)
{
    const uchar replacement = '?';
    QByteArray ba(s.length(), Qt::Uninitialized);

    uchar *cursor = reinterpret_cast<uchar *>(const_cast<char *>(ba.constData()));
    const uchar *ba_end = cursor + ba.length();
    const ushort *src = reinterpret_cast<const ushort *>(s.constBegin());
    const ushort *const end = reinterpret_cast<const ushort *>(s.constEnd());

    while (src != end) {
        if (cursor >= ba_end - 6) {
            // ensure we have enough space
            int pos = cursor - (const uchar *)ba.constData();
            ba.resize(ba.size()*2);
            cursor = (uchar *)ba.data() + pos;
            ba_end = (const uchar *)ba.constData() + ba.length();
        }

        uint u = *src++;
        if (u < 0x80) {
            if (u < 0x20 || u == 0x22 || u == 0x5c) {
                *cursor++ = '\\';
                switch (u) {
                case 0x22:
                    *cursor++ = '"';
                    break;
                case 0x5c:
                    *cursor++ = '\\';
                    break;
                case 0x8:
                    *cursor++ = 'b';
                    break;
                case 0xc:
                    *cursor++ = 'f';
                    break;
                case 0xa:
                    *cursor++ = 'n';
                    break;
                case 0xd:
                    *cursor++ = 'r';
                    break;
                case 0x9:
                    *cursor++ = 't';
                    break;
                default:
                    *cursor++ = 'u';
                    *cursor++ = '0';
                    *cursor++ = '0';
                    *cursor++ = hexdig(u>>4);
                    *cursor++ = hexdig(u & 0xf);
               }
            } else {
                *cursor++ = (uchar)u;
            }
        } else {
            if (QUtf8Functions::toUtf8<QUtf8BaseTraits>(u, cursor, src, end) < 0)
                *cursor++ = replacement;
        }
    }

    ba.resize(cursor - (const uchar *)ba.constData());
    return ba;
}

static void valueToPitm(const PitmPrivate::Base *b, const PitmPrivate::Value &v, QByteArray &pitm, int indent, bool compact)
{
    PitmValue::Type type = (PitmValue::Type)(uint)v.type;
    switch (type) {
    case PitmValue::Bool:
        pitm += v.toBoolean() ? "true" : "false";
        break;
    case PitmValue::Double: {
        const double d = v.toDouble(b);
        if (qIsFinite(d)) { // +2 to format to ensure the expected precision
            const double abs = std::abs(d);
            pitm += QByteArray::number(d, abs == static_cast<quint64>(abs) ? 'f' : 'g');
        } else {
            pitm += "null"; // +INF || -INF || NaN (see RFC4627#section2.4)
        }
        break;
    }
    case PitmValue::String:
        pitm += '"';
        pitm += escapedString(v.toString(b));
        pitm += '"';
        break;
    case PitmValue::Array:
        pitm += compact ? "[" : "[\n";
        arrayContentToPitm(static_cast<PitmPrivate::Array *>(v.base(b)), pitm, indent + (compact ? 0 : 1), compact);
        pitm += QByteArray(4*indent, ' ');
        pitm += ']';
        break;
    case PitmValue::Object:
        pitm += compact ? "{" : "{\n";
        objectContentToPitm(static_cast<PitmPrivate::Object *>(v.base(b)), pitm, indent + (compact ? 0 : 1), compact);
        pitm += QByteArray(4*indent, ' ');
        pitm += '}';
        break;
    case PitmValue::Null:
    default:
        pitm += "null";
    }
}

static void arrayContentToPitm(const PitmPrivate::Array *a, QByteArray &pitm, int indent, bool compact)
{
    if (!a || !a->length)
        return;

    QByteArray indentString(4*indent, ' ');

    uint i = 0;
    while (1) {
        pitm += indentString;
        valueToPitm(a, a->at(i), pitm, indent, compact);

        if (++i == a->length) {
            if (!compact)
                pitm += '\n';
            break;
        }

        pitm += compact ? "" : "\n";
    }
}


static void objectContentToPitm(const PitmPrivate::Object *o, QByteArray &pitm, int indent, bool compact)
{
    if (!o || !o->length)
        return;

    QByteArray indentString(4*indent, ' ');

    uint i = 0;
    while (1) {
        PitmPrivate::Entry *e = o->entryAt(i);
        pitm += indentString;
        pitm += escapedString(e->key());
        pitm += " ";
        valueToPitm(o, e->value, pitm, indent, compact);

        if (++i == o->length) {
            if (!compact)
                pitm += '\n';
            break;
        }

        pitm += compact ? "" : "\n";
    }
}

void Writer::objectToPitm(const PitmPrivate::Object *o, QByteArray &pitm, int indent, bool compact)
{
    pitm.reserve(pitm.size() + (o ? (int)o->size : 16));
    pitm += compact ? "{" : "{\n";
    objectContentToPitm(o, pitm, indent + (compact ? 0 : 1), compact);
    pitm += QByteArray(4*indent, ' ');
    pitm += compact ? "}" : "}\n";
}

void Writer::arrayToPitm(const PitmPrivate::Array *a, QByteArray &pitm, int indent, bool compact)
{
    pitm.reserve(pitm.size() + (a ? (int)a->size : 16));
    pitm += compact ? "[" : "[\n";
    arrayContentToPitm(a, pitm, indent + (compact ? 0 : 1), compact);
    pitm += QByteArray(4*indent, ' ');
    pitm += compact ? "]" : "]\n";
}

QT_END_NAMESPACE
