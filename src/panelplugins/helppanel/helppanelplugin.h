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
    Copyright (C) 2010 Alexander Rieder <alexanderrieder@gmail.com>
 */

#ifndef _HELPPANELPLUGIN_H
#define _HELPPANELPLUGIN_H

#include "panelplugin.h"


class KTextEdit;

class HelpPanelPlugin : public Cantor::PanelPlugin
{
  Q_OBJECT
  public:
    HelpPanelPlugin( QObject* parent, QList<QVariant> args);
    ~HelpPanelPlugin();

    QWidget* widget() Q_DECL_OVERRIDE;

  public Q_SLOTS:
    void setHelpHtml(const QString& help);
    void showHelp(const QString& help);

  private:
    QPointer<KTextEdit> m_edit;

};

#endif /* _HELPPANELPLUGIN_H */
