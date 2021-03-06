/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2012 Filipe Saraiva <filipe@kde.org>
 */

#ifndef _PYTHONBACKEND_H
#define _PYTHONBACKEND_H

#include "backend.h"
#include <cantor_pythonbackend_export.h>

class CANTOR_PYTHONBACKEND_EXPORT PythonBackend : public Cantor::Backend
{
  Q_OBJECT
  public:
    explicit PythonBackend(QObject* parent = nullptr, const QList<QVariant>& args = QList<QVariant>());
    ~PythonBackend() override;

    QWidget* settingsWidget(QWidget* parent) const override;

    KConfigSkeleton* config() const override = 0;
    Cantor::Backend::Capabilities capabilities() const override = 0;
};


#endif /* _PYTHONBACKEND_H */
