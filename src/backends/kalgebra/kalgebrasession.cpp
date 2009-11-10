/*************************************************************************************
*  Copyright (C) 2009 by Aleix Pol <aleixpol@kde.org>                               *
*                                                                                   *
*  This program is free software; you can redistribute it and/or                    *
*  modify it under the terms of the GNU General Public License                      *
*  as published by the Free Software Foundation; either version 2                   *
*  of the License, or (at your option) any later version.                           *
*                                                                                   *
*  This program is distributed in the hope that it will be useful,                  *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
*  GNU General Public License for more details.                                     *
*                                                                                   *
*  You should have received a copy of the GNU General Public License                *
*  along with this program; if not, write to the Free Software                      *
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
*************************************************************************************/

#include "kalgebrasession.h"
#include "kalgebraexpression.h"
#include "kalgebratabcompletionobject.h"
#include <analitzagui/algebrahighlighter.h>
#include <analitza/analitza.h>
#include <QTextEdit>

#include <kdebug.h>
#include "kalgebrasyntaxhelpobject.h"
#include <analitzagui/operatorsmodel.h>

KAlgebraSession::KAlgebraSession( Cantor::Backend* backend)
    : Session(backend)
{
    m_analitza = new Analitza::Analitza;
    m_operatorsModel = new OperatorsModel;
    m_operatorsModel->setVariables(m_analitza->variables());
}

KAlgebraSession::~KAlgebraSession()
{
    delete m_analitza;
}

void KAlgebraSession::login()
{
    changeStatus(Cantor::Session::Done);
    emit ready();
}

void KAlgebraSession::logout()
{}

void KAlgebraSession::interrupt()
{
    changeStatus(Cantor::Session::Done);
}

Cantor::Expression* KAlgebraSession::evaluateExpression(const QString& cmd,
                                                        Cantor::Expression::FinishingBehavior behave)
{
    KAlgebraExpression* expr=new KAlgebraExpression(this);
    expr->setFinishingBehavior(behave);

    changeStatus(Cantor::Session::Running);
    expr->setCommand(cmd);
    expr->evaluate();
    changeStatus(Cantor::Session::Done);

    m_operatorsModel->setVariables(m_analitza->variables());
    return expr;
}

Cantor::TabCompletionObject* KAlgebraSession::tabCompletionFor(const QString& command)
{
    return new KAlgebraTabCompletionObject(command, this);
}

Cantor::SyntaxHelpObject* KAlgebraSession::syntaxHelpFor(const QString& cmd)
{
    return new KAlgebraSyntaxHelpObject(cmd, this);
}

OperatorsModel* KAlgebraSession::operatorsModel()
{
    return m_operatorsModel;
}


QSyntaxHighlighter* KAlgebraSession::syntaxHighlighter(QTextEdit* parent)
{
    return new AlgebraHighlighter(parent->document());
}