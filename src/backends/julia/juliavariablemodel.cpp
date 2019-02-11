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
    Copyright (C) 2018 Nikita Sirgienko <warquark@gmail.com>
*/

#include "juliavariablemodel.h"
#include "juliaextensions.h"
#include "juliasession.h"

#include <QDebug>
#include <QDBusReply>
#include <QDBusInterface>
#include <QString>

#include "settings.h"

using namespace Cantor;

const QRegularExpression JuliaVariableModel::typeVariableInfo = QRegularExpression(QLatin1String("\\w+\\["));

JuliaVariableModel::JuliaVariableModel(JuliaSession* session):
    DefaultVariableModel(session),
    m_interface(nullptr)
{
}

void JuliaVariableModel::setJuliaServer(QDBusInterface* interface)
{
    m_interface = interface;
}

void JuliaVariableModel::update()
{
    if (!m_interface)
        return;

    m_interface->call(QLatin1String("parseModules"), JuliaSettings::variableManagement());

    const QStringList& variables =
        static_cast<QDBusReply<QStringList>>(m_interface->call(QLatin1String("variablesList"))).value();

    QList<Variable> vars;
    if (JuliaSettings::variableManagement())
    {
        const QStringList& values =
            static_cast<QDBusReply<QStringList>>(m_interface->call(QLatin1String("variableValuesList"))).value();

        for (int i = 0; i < variables.size(); i++)
        {
            if (i >= values.size())
            {
                qWarning() << "Don't have value for variable from julia server response, something wrong!";
                continue;
            }

            const QString& name = variables[i];
            QString value = values[i];
            if (value != JuliaVariableManagementExtension::REMOVED_VARIABLE_MARKER)
            {
                // Register variable
                // We use replace here, because julia return data type for some variables, and we need
                // remove it to make variable view more consistent with the other backends
                // More info: https://bugs.kde.org/show_bug.cgi?id=377771
                vars << Variable{name, value.replace(typeVariableInfo,QLatin1String("["))};
            }
        }
    }
    else
    {
        for (int i = 0; i < variables.size(); i++)
            vars << Variable{variables[i], QString()};
    }
    setVariables(vars);

    const QStringList& newFuncs =
        static_cast<QDBusReply<QStringList>>(m_interface->call(QLatin1String("functionsList"))).value();
    QStringList addedFuncs;
    QStringList removedFuncs;

    //remove the old variables
    int i = 0;
    while (i < m_functions.size())
    {
        //check if this var is present in the new variables
        bool found=false;
        for (const QString& func : newFuncs)
            if(m_functions[i] == func)
            {
                found=true;
                break;
            }

        if(!found)
        {
            removedFuncs<<m_functions[i];
            m_functions.removeAt(i);
        }
        else
            i++;
    }

    for (const QString& func : newFuncs)
    {
        if (!m_functions.contains(func))
        {
            addedFuncs<<func;
            m_functions.append(func);
        }
    }

    emit functionsAdded(addedFuncs);
    emit functionsRemoved(removedFuncs);
}
