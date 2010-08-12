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

#include "advancedplotassistant.h"

#include <QVBoxLayout>
#include <kdialog.h>
#include <kaction.h>
#include <kactioncollection.h>
#include "ui_advancedplotdialog.h"
#include "ui_directivecontainer.h"
#include "cantor_macros.h"
#include "backend.h"
#include "extension.h"

AdvancedPlotAssistant::AdvancedPlotAssistant(QObject* parent, QList<QVariant> args) : Assistant(parent)
{
    Q_UNUSED(args)
}

AdvancedPlotAssistant::~AdvancedPlotAssistant()
{

}

void AdvancedPlotAssistant::initActions()
{
    setXMLFile("cantor_advancedplot_assistant.rc");
    KAction* advplot=new KAction(i18n("Advanced Plotting"), actionCollection());
    actionCollection()->addAction("advancedplot_assistant", advplot);
    connect(advplot, SIGNAL(triggered()), this, SIGNAL(requested()));
}

QStringList AdvancedPlotAssistant::run(QWidget* parent)
{
    QPointer<KDialog> dlg=new KDialog(parent);
    QWidget *widget=new QWidget(dlg);
    Ui::AdvancedPlotAssistantBase base;
    base.setupUi(widget);
    dlg->setMainWidget(widget);

    //Casting the extension to correct type and checking it
    Cantor::AdvancedPlotExtension * pPlotter=dynamic_cast<Cantor::AdvancedPlotExtension*>
        (backend()->extension("AdvancedPlotExtension"));
    if (pPlotter==NULL)
    {
        kDebug()<<"Advanced plotting extension is messed up, that's a bug.";
        return QStringList();
    }

    //Filling up the form accordingly
    Cantor::AdvancedPlotExtension::AcceptorBase *pAcceptor=dynamic_cast<Cantor::AdvancedPlotExtension::AcceptorBase*>(pPlotter);
    if (pAcceptor!=NULL)
    {
        QStringList::const_iterator i=pAcceptor->labels().constBegin();
        foreach (const Cantor::AdvancedPlotExtension::AcceptorBase::widgetProc& wProc, pAcceptor->widgets())
        {
    //         QGroupBox *container=new QGroupBox(NULL);
    //         Ui::directiveContainer uicont;
    //         uicont.setupUi(container);

    /*        QVBoxLayout *layout = new QVBoxLayout;
            wProc(container);
            container->setLayout(layout);*/
            //TODO: find out why group box draws itself silly
            QWidget *container=wProc(NULL);

            base.directivesTabs->addTab(container,(*i));
            ++i; //WARNING: assuming the lists are filled correctly
        }
    }

    QStringList result;
    if( dlg->exec())
    {

    }

    delete dlg;
    return result;
}

K_EXPORT_CANTOR_PLUGIN(advancedplotassistant, AdvancedPlotAssistant)