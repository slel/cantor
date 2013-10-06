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
    Copyright (C) 2011 Filipe Saraiva <filipe@kde.org>
 */

#include "scilabsession.h"
#include "scilabexpression.h"
#include "scilabhighlighter.h"
#include "scilabcompletionobject.h"

#include <defaultvariablemodel.h>

#include <kdebug.h>
#include <KProcess>
#include <KDirWatch>

#include <QtCore/QFile>
#include <QTextEdit>
#include <QListIterator>
#include <QDir>
#include <QIODevice>
#include <QByteArray>

#include <settings.h>
#include <qdir.h>

ScilabSession::ScilabSession( Cantor::Backend* backend) : Session(backend),
m_variableModel(new Cantor::DefaultVariableModel(this))
{
    m_process = 0;
    kDebug();
}

ScilabSession::~ScilabSession()
{
    m_process->terminate();
    kDebug();
}

void ScilabSession::login()
{
    kDebug()<<"login";

    QStringList args;

    args << "-nb";

    m_process = new KProcess(this);
    m_process->setProgram(ScilabSettings::self()->path().toLocalFile(), args);

    kDebug() << m_process->program();

    m_process->setOutputChannelMode(KProcess::SeparateChannels);
    m_process->start();

    if(ScilabSettings::integratePlots())
    {
        kDebug() << "integratePlots";

        QString tempPath = QDir::tempPath();

        QString pathScilabOperations = tempPath;
        pathScilabOperations.prepend("chdir('");
        pathScilabOperations.append("');\n");

        kDebug() << "Processing command to change chdir in Scilab. Command " << pathScilabOperations.toLocal8Bit();

        m_process->write(pathScilabOperations.toLocal8Bit());

        m_watch = new KDirWatch(this);
        m_watch->setObjectName("ScilabDirWatch");

        m_watch->addDir(tempPath, KDirWatch::WatchFiles);

        kDebug() << "addDir " <<  tempPath << "? " << m_watch->contains(tempPath.toLocal8Bit());

        QObject::connect(m_watch, SIGNAL(created(QString)), SLOT(plotFileChanged(QString)));
    }

    emit ready();
}

void ScilabSession::logout()
{
    kDebug()<<"logout";

    m_process->write("exit\n");

    m_runningExpressions.clear();
    kDebug() << "m_runningExpressions: " << m_runningExpressions.isEmpty();

    QDir removePlotFigures;
    QListIterator<QString> i(m_listPlotName);

    while(i.hasNext()){
        removePlotFigures.remove(i.next().toLocal8Bit().constData());
    }

    changeStatus(Cantor::Session::Done);
}

void ScilabSession::interrupt()
{
    kDebug()<<"interrupt";

    foreach(Cantor::Expression* e, m_runningExpressions)
        e->interrupt();

    m_runningExpressions.clear();
    changeStatus(Cantor::Session::Done);
}

Cantor::Expression* ScilabSession::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave)
{
    kDebug() << "evaluating: " << cmd;
    ScilabExpression* expr = new ScilabExpression(this);

    QObject::connect(m_process, SIGNAL(readyReadStandardOutput()), SLOT (readOutput()));
    QObject::connect(m_process, SIGNAL(readyReadStandardError()), SLOT (readError()));

    changeStatus(Cantor::Session::Running);

    expr->setFinishingBehavior(behave);
    expr->setCommand(cmd);
    expr->evaluate();

    return expr;
}

void ScilabSession::runExpression(ScilabExpression* expr)
{
    QString command;

    command.prepend("\nprintf('begin-cantor-scilab-command-processing')\n");
    command += expr->command();

    m_currentExpression = expr;

    connect(m_currentExpression, SIGNAL(statusChanged(Cantor::Expression::Status)), this,
            SLOT(currentExpressionStatusChanged(Cantor::Expression::Status)));

    command += "\nprintf('terminated-cantor-scilab-command-processing')\n";
    kDebug() << "Writing command to process" << command;

    m_process->write(command.toLocal8Bit());
}

void ScilabSession::expressionFinished()
{
    kDebug()<<"finished";
    ScilabExpression* expression = qobject_cast<ScilabExpression*>(sender());

    m_runningExpressions.removeAll(expression);
    kDebug() << "size: " << m_runningExpressions.size();
}

void ScilabSession::readError()
{
    kDebug() << "readError";

    QString error = m_process->readAllStandardError();

    kDebug() << "error: " << error;
    m_currentExpression->parseError(error);
}

void ScilabSession::readOutput()
{
    kDebug() << "readOutput";

    while(m_process->bytesAvailable() > 0){
        m_output.append(QString::fromLocal8Bit(m_process->readLine()));
    }

    kDebug() << "output.isNull? " << m_output.isNull();
    kDebug() << "output: " << m_output;

    if(status() != Running || m_output.isNull()){
        return;
    }

    if(m_output.contains("begin-cantor-scilab-command-processing") &&
        m_output.contains("terminated-cantor-scilab-command-processing")){

        m_output.remove("begin-cantor-scilab-command-processing");
        m_output.remove("terminated-cantor-scilab-command-processing");

        m_currentExpression->parseOutput(m_output);

        m_output.clear();
    }
}

void ScilabSession::plotFileChanged(QString filename)
{
    kDebug() << "plotFileChanged filename:" << filename;

    if ((m_currentExpression) && (filename.contains("cantor-export-scilab-figure")))
    {
         kDebug() << "Calling parsePlotFile";
         m_currentExpression->parsePlotFile(filename);

         m_listPlotName.append(filename);
    }
}

void ScilabSession::currentExpressionStatusChanged(Cantor::Expression::Status status)
{
    kDebug() << "currentExpressionStatusChanged: " << status;

    switch (status)
    {
        case Cantor::Expression::Computing:
            break;

        case Cantor::Expression::Interrupted:
            break;

        case Cantor::Expression::Done:
        case Cantor::Expression::Error:
            changeStatus(Done);

            break;
    }
}

QSyntaxHighlighter* ScilabSession::syntaxHighlighter(QObject* parent)
{
    return new ScilabHighlighter(parent);
}

Cantor::CompletionObject* ScilabSession::completionFor(const QString& command, int index)
{
    return new ScilabCompletionObject(command, index, this);
}

QAbstractItemModel* ScilabSession::variableModel()
{
    return m_variableModel;
}

#include "scilabsession.moc"
