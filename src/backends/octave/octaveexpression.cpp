/*
    Copyright (C) 2010 Miha Čančula <miha.cancula@gmail.com>

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
*/

#include "octaveexpression.h"
#include "octavesession.h"
#include "defaultvariablemodel.h"

#include "textresult.h"

#include <KDebug>
#include <QtCore/QFile>
#include <helpresult.h>

#include <config-cantorlib.h>

#ifdef WITH_EPS
#include "epsresult.h"
typedef Cantor::EpsResult OctavePlotResult;
#else
#include "imageresult.h"
typedef Cantor::ImageResult OctavePlotResult;
#endif

static const char* printCommand = "cantor_print();";

OctaveExpression::OctaveExpression(Cantor::Session* session): Expression(session)
{
    m_plotCommands << "plot" << "semilogx" << "semilogy" << "loglog" << "polar"
                   << "mesh" << "contour" << "bar" << "stairs" << "errorbar"
                   << "surf" << "sombrero";
    m_plotCommands << "cantor_plot2d" << "cantor_plot3d";

    m_error = false;
    m_plotPending = false;
}


OctaveExpression::~OctaveExpression()
{

}


void OctaveExpression::interrupt()
{
    setStatus(Interrupted);
}

void OctaveExpression::evaluate()
{
    QString cmd = command();
    QStringList cmdWords = cmd.split(QRegExp("\\b"), QString::SkipEmptyParts);
    if (!cmdWords.contains("help") && !cmdWords.contains("completion_matches"))
    {
        foreach (const QString& plotCmd, m_plotCommands)
        {
            if (cmdWords.contains(plotCmd))
            {
                setPlotPending(true);
                kDebug() << "Executing a plot command";
                break;
            }
        }
    }
    if ( m_plotPending && !cmd.contains("cantor_plot"))
    {
        // This was a manual plot, we have to add a print command
        if (!cmd.endsWith(';') && !cmd.endsWith(','))
        {
            cmd += ',';
        }
        cmd += printCommand;
        setCommand(cmd);
    }
    m_finished = false;
    setStatus(Computing);
    OctaveSession* octaveSession = dynamic_cast<OctaveSession*>(session());
    if (octaveSession)
    {
        octaveSession->runExpression(this);
    }
}


void OctaveExpression::parseOutput ( QString output )
{
    m_resultString += output;
    if (!m_resultString.trimmed().isEmpty())
    {
        if (command().contains("help"))
        {
            setResult(new Cantor::HelpResult(m_resultString));
        }
        else
        {
            setResult(new Cantor::TextResult(m_resultString));
        }
    }
}

void OctaveExpression::parseError(QString error)
{
    kDebug() << error;
    if (false && error.contains("warning"))
    {
        parseOutput(error);
    }
    else
    {
        m_error = true;
        setErrorMessage(error);
        setStatus(Error);
    }
}

void OctaveExpression::parsePlotFile(QString file)
{
    if (QFile::exists(file))
    {
        setResult(new OctavePlotResult(file));
        setPlotPending(false);
        if (m_finished)
        {
            setStatus(Done);
        }
    }
}

void OctaveExpression::finalize()
{
    kDebug() << m_resultString;
    foreach ( const QString& line, m_resultString.split('\n', QString::SkipEmptyParts) )
    {
        if (m_resultString.contains('='))
        {
            kDebug() << line;
            // Probably a new variable
            QStringList parts = line.split('=');
            if (parts.size() >= 2)
            {
                Cantor::DefaultVariableModel* model = dynamic_cast<Cantor::DefaultVariableModel*>(session()->variableModel());
                if (model)
                {
                    model->addVariable(parts.first().trimmed(), parts.last().trimmed());
                }
            }
        }
    }
    kDebug() << m_plotPending << m_error;
    m_finished = true;
    if (!m_plotPending)
    {
        setStatus(m_error ? Error : Done);
    }
}
void OctaveExpression::setPlotPending(bool plot)
{
    m_plotPending = plot;
}