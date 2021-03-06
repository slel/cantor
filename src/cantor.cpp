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
#include "cantor.h"

#include <KActionCollection>
#include <KConfigDialog>
#include <KConfigGroup>
#include <KMessageBox>
#include <KShortcutsDialog>
#include <KStandardAction>
#include <KNS3/DownloadDialog>
#include <KParts/ReadWritePart>
#include <KRecentFilesAction>

#include <QApplication>
#include <QCloseEvent>
#include <QDebug>
#include <QDockWidget>
#include <QDir>
#include <QFileDialog>
#include <QPushButton>
#include <QGraphicsView>

#include "lib/backend.h"
#include "lib/panelpluginhandler.h"
#include "lib/panelplugin.h"
#include "lib/worksheetaccess.h"

#include "settings.h"
#include "ui_settings.h"
#include "backendchoosedialog.h"

CantorShell::CantorShell() : KParts::MainWindow(), m_part(nullptr)
{
    // set the shell's ui resource file
    setXMLFile(QLatin1String("cantor_shell.rc"));

    // then, setup our actions
    setupActions();

    createGUI(nullptr);

    m_tabWidget=new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);
    m_tabWidget->setDocumentMode(true);
    setCentralWidget(m_tabWidget);

    connect(m_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(activateWorksheet(int)));
    connect(m_tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));

    // apply the saved mainwindow settings, if any, and ask the mainwindow
    // to automatically save settings if changed: window size, toolbar
    // position, icon size, etc.
    setAutoSaveSettings();

    setDockOptions(QMainWindow::AnimatedDocks|QMainWindow::AllowTabbedDocks|QMainWindow::VerticalTabs);

    updateNewSubmenu();
}

CantorShell::~CantorShell()
{
    if (m_recentProjectsAction)
        m_recentProjectsAction->saveEntries(KSharedConfig::openConfig()->group(QLatin1String("Recent Files")));

    if (!m_newBackendActions.isEmpty())
    {
        unplugActionList(QLatin1String("new_worksheet_with_backend_list"));
        qDeleteAll(m_newBackendActions);
        m_newBackendActions.clear();
    }

    unplugActionList(QLatin1String("view_show_panel_list"));
    qDeleteAll(m_panels);
    m_panels.clear();
}

void CantorShell::load(const QUrl &url)
{
    if (!m_part||!m_part->url().isEmpty() || m_part->isModified() )
    {
        addWorksheet(QString());
        m_tabWidget->setCurrentIndex(m_parts.size()-1);
    }
    if (!m_part->openUrl( url ))
        closeTab(m_tabWidget->currentIndex());
    if (m_recentProjectsAction)
        m_recentProjectsAction->addUrl(url);
}

void CantorShell::setupActions()
{
    QAction* openNew = KStandardAction::openNew(this, SLOT(fileNew()), actionCollection());
    openNew->setPriority(QAction::LowPriority);
    QAction* open = KStandardAction::open(this, SLOT(fileOpen()), actionCollection());
    open->setPriority(QAction::LowPriority);
    m_recentProjectsAction = KStandardAction::openRecent(this, &CantorShell::load, actionCollection());
    m_recentProjectsAction->setPriority(QAction::LowPriority);
    m_recentProjectsAction->loadEntries(KSharedConfig::openConfig()->group(QLatin1String("Recent Files")));

    KStandardAction::close (this,  SLOT(closeTab()),  actionCollection());

    KStandardAction::quit(qApp, SLOT(closeAllWindows()), actionCollection());

    createStandardStatusBarAction();
    //setStandardToolBarMenuEnabled(true);

    KStandardAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
    KStandardAction::configureToolbars(this, SLOT(configureToolbars()), actionCollection());

    KStandardAction::preferences(this, SLOT(showSettings()), actionCollection());

    QAction * downloadExamples = new QAction(i18n("Download Example Worksheets"), actionCollection());
    downloadExamples->setIcon(QIcon::fromTheme(QLatin1String("get-hot-new-stuff")));
    actionCollection()->addAction(QLatin1String("file_example_download"),  downloadExamples);
    connect(downloadExamples, SIGNAL(triggered()), this,  SLOT(downloadExamples()));

    QAction * openExample =new QAction(i18n("&Open Example"), actionCollection());
    openExample->setIcon(QIcon::fromTheme(QLatin1String("document-open")));
    actionCollection()->addAction(QLatin1String("file_example_open"), openExample);
    connect(openExample, SIGNAL(triggered()), this, SLOT(openExample()));

    QAction* toPreviousTab = new QAction(i18n("Go to previous worksheet"), actionCollection());
    actionCollection()->addAction(QLatin1String("go_to_previous_tab"), toPreviousTab);
    actionCollection()->setDefaultShortcut(toPreviousTab, Qt::CTRL+Qt::Key_PageDown);
    connect(toPreviousTab, &QAction::triggered, toPreviousTab, [this](){
        const int index = m_tabWidget->currentIndex()-1;
        if (index >= 0)
            m_tabWidget->setCurrentIndex(index);
        else
            m_tabWidget->setCurrentIndex(m_tabWidget->count()-1);
    });
    addAction(toPreviousTab);

    QAction* toNextTab = new QAction(i18n("Go to next worksheet"), actionCollection());
    actionCollection()->addAction(QLatin1String("go_to_next_tab"), toNextTab);
    actionCollection()->setDefaultShortcut(toNextTab, Qt::CTRL+Qt::Key_PageUp);
    connect(toNextTab, &QAction::triggered, toNextTab, [this](){
        const int index = m_tabWidget->currentIndex()+1;
        if (index < m_tabWidget->count())
            m_tabWidget->setCurrentIndex(index);
        else
            m_tabWidget->setCurrentIndex(0);
    });
    addAction(toNextTab);
}

void CantorShell::saveProperties(KConfigGroup & /*config*/)
{
    // the 'config' object points to the session managed
    // config file.  anything you write here will be available
    // later when this app is restored
}

void CantorShell::readProperties(const KConfigGroup & /*config*/)
{
    // the 'config' object points to the session managed
    // config file.  this function is automatically called whenever
    // the app is being restored.  read in here whatever you wrote
    // in 'saveProperties'
}

/*!
 * called when one of the "new backend" action or the "New" action are called
 * adds a new worksheet with the backend assossiated with the called action
 * or opens the "Choose Backend" dialog, respectively.
 */
void CantorShell::fileNew()
{
    QAction* a = static_cast<QAction*>(sender());
    const QString& backendName = a->data().toString();
    if (!backendName.isEmpty())
    {
        addWorksheet(backendName);
        return;
    }

    //"New" action was called -> open the "Choose Backend" dialog.
    addWorksheet();
}

void CantorShell::optionsConfigureKeys()
{
    KShortcutsDialog dlg( KShortcutsEditor::AllActions, KShortcutsEditor::LetterShortcutsDisallowed, this );
    dlg.addCollection( actionCollection(), i18n("Cantor") );
    if (m_part)
        dlg.addCollection( m_part->actionCollection(), i18n("Cantor") );
    dlg.configure( true );
}

void CantorShell::fileOpen()
{
    // this slot is called whenever the File->Open menu is selected,
    // the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
    // button is clicked
    static const QString& worksheetFilter = i18n("Cantor Worksheet (*.cws)");
    static const QString& notebookFilter = i18n("Jupyter Notebook (*.ipynb)");
    QString filter;
    if (m_previousFilter == notebookFilter)
        filter = notebookFilter + QLatin1String(";;") + worksheetFilter;
    else
        filter = worksheetFilter + QLatin1String(";;") + notebookFilter;
    QUrl url = QFileDialog::getOpenFileUrl(this, i18n("Open file"), QUrl(), filter, &m_previousFilter);

    if (url.isEmpty() == false)
    {
        // About this function, the style guide
        // says that it should open a new window if the document is _not_
        // in its initial state.  This is what we do here..
        /*if ( m_part->url().isEmpty() && ! m_part->isModified() )
        {
            // we open the file in this window...
            load( url );
        }
        else
        {
            // we open the file in a new window...
            CantorShell* newWin = new CantorShell;
            newWin->load( url );
            newWin->show();
            }*/
        load( url );
    }
}

void CantorShell::addWorksheet()
{
    bool hasBackend = false;
    for (auto* b : Cantor::Backend::availableBackends())
    {
        if(b->isEnabled())
        {
            hasBackend = true;
            break;
        }
    }

    if(hasBackend) //There is no point in asking for the backend, if no one is available
    {
        QString backend = Settings::self()->defaultBackend();
        if (backend.isEmpty())
        {
            QPointer<BackendChooseDialog> dlg=new BackendChooseDialog(this);
            if(dlg->exec())
            {
                backend = dlg->backendName();
                addWorksheet(backend);
            }

            delete dlg;
        }
        else
        {
            addWorksheet(backend);
        }

    }else
    {
        QTextBrowser *browser=new QTextBrowser(this);
        QString backendList=QLatin1String("<ul>");
        int backendListSize = 0;
        foreach(Cantor::Backend* b, Cantor::Backend::availableBackends())
        {
            if(!b->requirementsFullfilled()) //It's disabled because of missing dependencies, not because of some other reason(like eg. nullbackend)
            {
                backendList+=QString::fromLatin1("<li>%1: <a href=\"%2\">%2</a></li>").arg(b->name(), b->url());
                ++backendListSize;
            }
        }
        browser->setHtml(i18np("<h1>No Backend Found</h1>\n"             \
                               "<div>You could try:\n"                   \
                               "  <ul>"                                  \
                               "    <li>Changing the settings in the config dialog;</li>" \
                               "    <li>Installing packages for the following program:</li>" \
                               "     %2 "                                \
                               "  </ul> "                                 \
                               "</div> "
                              , "<h1>No Backend Found</h1>\n"             \
                               "<div>You could try:\n"                   \
                               "  <ul>"                                  \
                               "    <li>Changing the settings in the config dialog;</li>" \
                               "    <li>Installing packages for one of the following programs:</li>" \
                               "     %2 "                                \
                               "  </ul> "                                 \
                               "</div> "
                              , backendListSize, backendList
                              ));

        browser->setObjectName(QLatin1String("ErrorMessage"));
        m_tabWidget->addTab(browser, i18n("Error"));
    }
}

void CantorShell::addWorksheet(const QString& backendName)
{
    static int sessionCount=1;

    // this routine will find and load our Part.  it finds the Part by
    // name which is a bad idea usually.. but it's alright in this
    // case since our Part is made for this Shell
    KPluginLoader loader(QLatin1String("cantorpart"));
    KPluginFactory* factory = loader.factory();
    if (factory)
    {
        if (!backendName.isEmpty())
        {
            Cantor::Backend* backend = Cantor::Backend::getBackend(backendName);
            if (!backend)
            {
                KMessageBox::error(this, i18n("Backend %1 is not installed", backendName), i18n("Cantor"));
                return;
            }
            else
            {
                if (!backend->isEnabled())
                {
                    KMessageBox::error(this, i18n("%1 backend installed, but inactive. Please check installation and Cantor settings", backendName), i18n("Cantor"));
                    return;
                }
            }
        }

        // now that the Part is loaded, we cast it to a Part to get our hands on it
        KParts::ReadWritePart* part = factory->create<KParts::ReadWritePart>(m_tabWidget, QVariantList()<<backendName);
        if (part)
        {
            connect(part, SIGNAL(setCaption(QString,QIcon)), this, SLOT(setTabCaption(QString,QIcon)));
            connect(part, SIGNAL(worksheetSave(QUrl)), this, SLOT(onWorksheetSave(QUrl)));
            m_parts.append(part);

            int tab = m_tabWidget->addTab(part->widget(), i18n("Session %1", sessionCount++));
            m_tabWidget->setCurrentIndex(tab);
            // Setting focus on worksheet view, because Qt clear focus of added widget inside addTab
            // This fix https://bugs.kde.org/show_bug.cgi?id=395976
            part->widget()->findChild<QGraphicsView*>()->setFocus();
        }
        else
        {
            qDebug()<<"error creating part ";
        }
    }
    else
    {
        // if we couldn't find our Part, we exit since the Shell by
        // itself can't do anything useful
        KMessageBox::error(this, i18n("Failed to find the Cantor Part with error %1", loader.errorString()));
        qApp->quit();
        // we return here, cause qApp->quit() only means "exit the
        // next time we enter the event loop...
        return;
    }

}

void CantorShell::activateWorksheet(int index)
{
    QObject* pluginHandler=m_part->findChild<QObject*>(QLatin1String("PanelPluginHandler"));
    if (pluginHandler)
        disconnect(pluginHandler,SIGNAL(pluginsChanged()), this, SLOT(updatePanel()));

    // Save part state before change worksheet
    if (m_part)
    {
        QStringList visiblePanelNames;
        foreach (QDockWidget* doc, m_panels)
        {
            if (doc->widget() && doc->widget()->isVisible())
                visiblePanelNames << doc->objectName();
        }
        m_pluginsVisibility[m_part] = visiblePanelNames;
    }

    m_part=findPart(m_tabWidget->widget(index));
    if(m_part)
    {
        createGUI(m_part);

        QObject* pluginHandler=m_part->findChild<QObject*>(QLatin1String("PanelPluginHandler"));
        connect(pluginHandler, SIGNAL(pluginsChanged()), this, SLOT(updatePanel()));
        updatePanel();
    }
    else
        qDebug()<<"selected part doesn't exist";

    m_tabWidget->setCurrentIndex(index);
}

void CantorShell::setTabCaption(const QString& caption, const QIcon& icon)
{
    if (caption.isEmpty()) return;

    KParts::ReadWritePart* part=dynamic_cast<KParts::ReadWritePart*>(sender());
    if (part)
    {
        m_tabWidget->setTabText(m_tabWidget->indexOf(part->widget()), caption);
        m_tabWidget->setTabIcon(m_tabWidget->indexOf(part->widget()), icon);
    }
}

void CantorShell::closeTab(int index)
{
    if (!reallyClose(false))
    {
        return;
    }

    QWidget* widget = nullptr;
    if (index >= 0)
    {
        widget = m_tabWidget->widget(index);
    }
    else if (m_part)
    {
        widget = m_part->widget();
    }

    if (!widget)
    {
        qWarning() << "Could not find widget by tab index" << index;
        return;
    }


    m_tabWidget->removeTab(index);

    if(widget->objectName()==QLatin1String("ErrorMessage"))
    {
        widget->deleteLater();
    }else
    {
        KParts::ReadWritePart* part= findPart(widget);
        if(part)
        {
            m_parts.removeAll(part);
            m_pluginsVisibility.remove(part);
            delete part;
        }
    }

    if (m_tabWidget->count() == 0)
        setCaption(QString());
    updatePanel();
}

bool CantorShell::reallyClose(bool checkAllParts) {
    if(checkAllParts && m_parts.count() > 1) {
        bool modified = false;
        foreach( KParts::ReadWritePart* const part, m_parts)
        {
            if(part->isModified()) {
                modified = true;
                break;
            }
        }
        if(!modified) return true;
        int want_save = KMessageBox::warningYesNo( this,
            i18n("Multiple unsaved Worksheets are opened. Do you want to close them?"),
            i18n("Close Cantor"));
        switch (want_save) {
            case KMessageBox::Yes:
                return true;
            case KMessageBox::No:
                return false;
        }
    }
    if (m_part && m_part->isModified() ) {
        int want_save = KMessageBox::warningYesNoCancel( this,
            i18n("The current project has been modified. Do you want to save it?"),
            i18n("Save Project"));
        switch (want_save) {
            case KMessageBox::Yes:
                m_part->save();
                if(m_part->waitSaveComplete()) {
                    return true;
                } else {
                    m_part->setModified(true);
                    return false;
                }
            case KMessageBox::Cancel:
                return false;
            case KMessageBox::No:
                return true;
        }
    }
    return true;
}

void CantorShell::closeEvent(QCloseEvent* event) {
    if(!reallyClose()) {
        event->ignore();
    } else {
        KParts::MainWindow::closeEvent(event);
    }
}

void CantorShell::showSettings()
{
    KConfigDialog *dialog = new KConfigDialog(this,  QLatin1String("settings"), Settings::self());
    QWidget *generalSettings = new QWidget;
    Ui::SettingsBase base;
    base.setupUi(generalSettings);
    base.kcfg_DefaultBackend->addItems(Cantor::Backend::listAvailableBackends());

    dialog->addPage(generalSettings, i18n("General"), QLatin1String("preferences-other"));
    foreach(Cantor::Backend* backend, Cantor::Backend::availableBackends())
    {
        if (backend->config()) //It has something to configure, so add it to the dialog
            dialog->addPage(backend->settingsWidget(dialog), backend->config(), backend->name(),  backend->icon());
    }

    dialog->show();
}

void CantorShell::downloadExamples()
{
    KNS3::DownloadDialog dialog;
    dialog.exec();
    foreach (const KNS3::Entry& e,  dialog.changedEntries())
    {
        qDebug() << "Changed Entry: " << e.name();
    }
}

void CantorShell::openExample()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1String("/examples");
    if (dir.isEmpty()) return;
    QDir().mkpath(dir);

    QStringList files=QDir(dir).entryList(QDir::Files);
    QPointer<QDialog> dlg=new QDialog(this);
    QListWidget* list=new QListWidget(dlg);
    foreach(const QString& file, files)
    {
        QString name=file;
        name.remove(QRegExp(QLatin1String("-.*\\.hotstuff-access$")));
        list->addItem(name);
    }

    QVBoxLayout *mainLayout = new QVBoxLayout;
    dlg->setLayout(mainLayout);
    mainLayout->addWidget(list);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    mainLayout->addWidget(buttonBox);

    buttonBox->button(QDialogButtonBox::Ok)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOkButton));
    buttonBox->button(QDialogButtonBox::Cancel)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));

    connect(buttonBox, SIGNAL(accepted()), dlg, SLOT(accept()) );
    connect(buttonBox, SIGNAL(rejected()), dlg, SLOT(reject()) );

    if (dlg->exec()==QDialog::Accepted&&list->currentRow()>=0)
    {
        const QString& selectedFile=files[list->currentRow()];
        QUrl url = QUrl::fromLocalFile(QDir(dir).absoluteFilePath(selectedFile));

        qDebug()<<"loading file "<<url;
        load(url);
    }

    delete dlg;
}

KParts::ReadWritePart* CantorShell::findPart(QWidget* widget)
{
    foreach( KParts::ReadWritePart* const part, m_parts)
    {
        if(part->widget()==widget)
            return part;
    }
    return nullptr;
}

void CantorShell::updatePanel()
{
    unplugActionList(QLatin1String("view_show_panel_list"));

    //remove all of the previous panels (but do not delete the widgets)
    foreach(QDockWidget* dock, m_panels)
    {
        QWidget* widget=dock->widget();
        if(widget!=nullptr)
        {
            widget->setParent(this);
            widget->hide();
        }
        dock->deleteLater();
    }
    m_panels.clear();

    QList<QAction*> panelActions;

    Cantor::PanelPluginHandler* handler=m_part->findChild<Cantor::PanelPluginHandler*>(QLatin1String("PanelPluginHandler"));
    if(!handler)
    {
        qDebug()<<"no PanelPluginHandle found for this part";
        return;
    }

    QDockWidget* last=nullptr;

    QList<Cantor::PanelPlugin*> plugins=handler->plugins();
    const bool isNewWorksheet = !m_pluginsVisibility.contains(m_part);
    foreach(Cantor::PanelPlugin* plugin, plugins)
    {
        if(plugin==nullptr)
        {
            qDebug()<<"somethings wrong";
            continue;
        }

        qDebug()<<"adding panel for "<<plugin->name();
        plugin->setParentWidget(this);

        QDockWidget* docker=new QDockWidget(plugin->name(), this);
        docker->setObjectName(plugin->name());
        docker->setWidget(plugin->widget());
        addDockWidget ( Qt::RightDockWidgetArea,  docker );

        // Set visibility for dock from saved info
        if (isNewWorksheet)
        {
            if (plugin->showOnStartup())
                docker->show();
            else
                docker->hide();
        }
        else
        {
            if (m_pluginsVisibility[m_part].contains(plugin->name()))
                docker->show();
            else
                docker->hide();
        }

        if(last!=nullptr)
            tabifyDockWidget(last, docker);
        last=docker;

        connect(plugin, &Cantor::PanelPlugin::visibilityRequested, this, &CantorShell::pluginVisibilityRequested);

        m_panels.append(docker);

        //Create the action to show/hide this panel
        panelActions<<docker->toggleViewAction();

    }

    plugActionList(QLatin1String("view_show_panel_list"), panelActions);

    updateNewSubmenu();
}

void CantorShell::updateNewSubmenu()
{
    unplugActionList(QLatin1String("new_worksheet_with_backend_list"));
    qDeleteAll(m_newBackendActions);
    m_newBackendActions.clear();

    foreach (Cantor::Backend* backend, Cantor::Backend::availableBackends())
    {
        if (!backend->isEnabled())
            continue;
        QAction * action = new QAction(QIcon::fromTheme(backend->icon()), backend->name(), nullptr);
        action->setData(backend->name());
        connect(action, SIGNAL(triggered()), this, SLOT(fileNew()));
        m_newBackendActions << action;
    }
    plugActionList(QLatin1String("new_worksheet_with_backend_list"), m_newBackendActions);
}

Cantor::WorksheetAccessInterface* CantorShell::currentWorksheetAccessInterface()
{
    Cantor::WorksheetAccessInterface* wa=m_part->findChild<Cantor::WorksheetAccessInterface*>(Cantor::WorksheetAccessInterface::Name);

    if (!wa)
        qDebug()<<"failed to access worksheet access interface for current part";

    return wa;
}

void CantorShell::pluginVisibilityRequested()
{
    Cantor::PanelPlugin* plugin = static_cast<Cantor::PanelPlugin*>(sender());
    for (QDockWidget* docker: m_panels)
    {
        if (plugin->name() == docker->windowTitle())
        {
            if (docker->isHidden())
                docker->show();
            docker->raise();
        }
    }
}

void CantorShell::onWorksheetSave(const QUrl& url)
{
    if (m_recentProjectsAction)
        m_recentProjectsAction->addUrl(url);
}
