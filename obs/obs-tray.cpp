/******************************************************************************
    Copyright (C) 2013 by Hugh Bailey <obs.jim@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include <QtWidgets/QAction>
#include <QtWidgets/QWidget>
#include <QIcon>
#include <QMenu>
#include <QMessageBox>

#include "obs-tray.hpp"

OBSTray::OBSTray()
{
	defaultIcon = QIcon(":/settings/images/settings/video-display-3.png");
	playingIcon = QIcon(":/settings/images/settings/network.png");

	stopAction = new QAction(tr("Parar"), this);
	connect(stopAction, SIGNAL(triggered()), this, SLOT(hide()));

	setupAction = new QAction(tr("Configuração"), this);
	connect(setupAction, SIGNAL(triggered()), this, SLOT(showMaximized()));

	quitAction = new QAction(tr("&Sair"), this);
	connect(quitAction, SIGNAL(triggered()), this, SLOT(closeObsTray()));

	createTrayIcon();

	trayIcon->setIcon(defaultIcon);
	trayIcon->show();

	setWindowTitle(tr("OBSTray"));
}

void OBSTray::setVisible(bool visible)
{
	QDialog::setVisible(visible);
}

void OBSTray::closeEvent(QCloseEvent *event)
{
	if (trayIcon->isVisible()) {
		QMessageBox::information(this, tr("OBSTray"),
			tr("OBSTray continuará executando em segundo "
			"plano aguardando o início da transmissão."));
		hide();
		event->ignore();
	}
}

void OBSTray::closeObsTray()
{
	QMessageBox::StandardButton reallyCloseObs;
	reallyCloseObs = QMessageBox::question(this, tr("OBSTray"), "Deseja sair do OBS?",
		QMessageBox::Yes | QMessageBox::No);
	if (reallyCloseObs == QMessageBox::Yes) {
		emit closeObs();
	}
}

void OBSTray::setIcon(int index)
{
	QIcon icon = iconComboBox->itemIcon(index);
	trayIcon->setIcon(icon);
	setWindowIcon(icon);

	trayIcon->setToolTip(iconComboBox->itemText(index));
}

void OBSTray::createTrayIcon()
{
	trayIconMenu = new QMenu(this);
	trayIconMenu->addAction(stopAction);
	trayIconMenu->addAction(setupAction);
	trayIconMenu->addSeparator();
	trayIconMenu->addAction(quitAction);

	trayIcon = new QSystemTrayIcon(this);
	trayIcon->setContextMenu(trayIconMenu);
}