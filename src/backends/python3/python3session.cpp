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
    Copyright (C) 2015 Minh Ngo <minh@fedoraproject.org>
 */

#include "python3session.h"
#include "settings.h"

Python3Session::Python3Session(Cantor::Backend* backend)
    : PythonSession(backend, 3, QLatin1String("cantor_python3server"))
{
}

bool Python3Session::integratePlots() const
{
    return PythonSettings::integratePlots();
}

QStringList Python3Session::autorunScripts() const
{
    return PythonSettings::autorunScripts();
}

bool Python3Session::variableManagement() const
{
    return PythonSettings::variableManagement();
}

