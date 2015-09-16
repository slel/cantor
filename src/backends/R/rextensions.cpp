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
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
 */

#include "rextensions.h"

#include <KLocalizedString>

RScriptExtension::RScriptExtension(QObject* parent) : Cantor::ScriptExtension(parent)
{

}

RScriptExtension::~RScriptExtension()
{

}

QString RScriptExtension::runExternalScript(const QString& path)
{
    return QString::fromLatin1("source(\"%1\")").arg(path);
}

QString RScriptExtension::scriptFileFilter()
{
    return i18n("R script file (*.R)");
}

QString RScriptExtension::highlightingMode()
{
    return QLatin1String("r script");
}

RPlotExtension::RPlotExtension(QObject* parent) : Cantor::AdvancedPlotExtension(parent)
{
}
// TODO: injection prevention
QString RPlotExtension::accept(const Cantor::PlotTitleDirective& directive) const
{
    return QLatin1String("main=\"")+directive.title()+QLatin1String("\"");
}

QString RPlotExtension::accept(const Cantor::OrdinateScaleDirective& directive) const
{
    return QLatin1String("ylim=range(")+QString::number(directive.min())+QLatin1String(",")+QString::number(directive.max())+QLatin1String(")");
}

QString RPlotExtension::accept(const Cantor::AbscissScaleDirective& directive) const
{
    return QLatin1String("xlim=range(")+QString::number(directive.min())+QLatin1String(",")+QString::number(directive.max())+QLatin1String(")");
}
