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
    Copyright (C) 2013 Filipe Saraiva <filipe@kde.org>
 */

#include "importpackageassistant.h"

#include <kdialog.h>
#include <kaction.h>
#include <kdebug.h>
#include <kactioncollection.h>
#include "cantor_macros.h"
#include "backend.h"
#include "extension.h"
#include "ui_importpackagedlg.h"

ImportPackageAssistant::ImportPackageAssistant(QObject* parent, QList<QVariant> args) : Assistant(parent)
{
    Q_UNUSED(args)
}

ImportPackageAssistant::~ImportPackageAssistant()
{

}

void ImportPackageAssistant::initActions()
{
    setXMLFile("cantor_import_package_assistant.rc");

    KAction* importpackage = new KAction(i18n("Import Package"), actionCollection());
    actionCollection()->addAction("importpackage_assistant", importpackage);
    connect(importpackage, SIGNAL(triggered()), this, SIGNAL(requested()));
}

QStringList ImportPackageAssistant::run(QWidget* parent)
{
    QPointer<KDialog> dlg=new KDialog(parent);

    QWidget* widget=new QWidget(dlg);

    Ui::ImportPackageAssistantBase base;
    base.setupUi(widget);

    dlg->setMainWidget(widget);

    QStringList result;
    if( dlg->exec())
    {
        const QString& m = base.package->text();

        Cantor::PackagingExtension* ext = dynamic_cast<Cantor::PackagingExtension*>(backend()->extension("PackagingExtension"));
        result << ext->importPackage(m);
    }

    delete dlg;
    return result;
}

K_EXPORT_CANTOR_PLUGIN(importpackageassistant, ImportPackageAssistant)