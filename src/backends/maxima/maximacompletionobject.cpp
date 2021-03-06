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
    Copyright (C) 2009-2012 Alexander Rieder <alexanderrieder@gmail.com>
 */

#include "maximacompletionobject.h"

#include <QDebug>

#include "maximasession.h"
#include "maximakeywords.h"
#include "maximavariablemodel.h"

MaximaCompletionObject::MaximaCompletionObject(const QString& command, int index,MaximaSession* session) : Cantor::CompletionObject(session)
{
    qDebug() << "MaximaCompletionObject constructor";
    setLine(command, index);
}

void MaximaCompletionObject::fetchIdentifierType()
{
    QStringList userVariableNames=session()->variableModel()->variableNames();
    QStringList userFunctionNames=session()->variableModel()->functions();
    if (std::binary_search(userVariableNames.begin(), userVariableNames.end(), identifier()))
        emit fetchingTypeDone(VariableType);
    else if (std::binary_search(userFunctionNames.begin(), userFunctionNames.end(), identifier()))
        emit fetchingTypeDone(FunctionType);
    else if (std::binary_search(MaximaKeywords::instance()->functions().begin(),
            MaximaKeywords::instance()->functions().end(), identifier()))
        emit fetchingTypeDone(FunctionType);
    else if (std::binary_search(MaximaKeywords::instance()->keywords().begin(),
            MaximaKeywords::instance()->keywords().end(), identifier()))
        emit fetchingTypeDone(KeywordType);
    else
        emit fetchingTypeDone(VariableType);
}

void MaximaCompletionObject::fetchCompletions()
{
    QStringList allCompletions;
    allCompletions<<MaximaKeywords::instance()->variables();
    allCompletions<<MaximaKeywords::instance()->functions();
    allCompletions<<MaximaKeywords::instance()->keywords();
    allCompletions<<session()->variableModel()->variableNames();
    allCompletions<<session()->variableModel()->functions();

    const QString prefix = command();
    QStringList prefixCompletion;
    for (const QString str : allCompletions)
        if (str.startsWith(prefix))
            prefixCompletion << str;

    setCompletions(prefixCompletion);

    emit fetchingDone();
}

bool MaximaCompletionObject::mayIdentifierContain(QChar c) const
{
    return c.isLetter() || c.isDigit() || c == QLatin1Char('_') || c == QLatin1Char('%');
}

bool MaximaCompletionObject::mayIdentifierBeginWith(QChar c) const
{
    return c.isLetter() || c == QLatin1Char('_') || c == QLatin1Char('%');
}
