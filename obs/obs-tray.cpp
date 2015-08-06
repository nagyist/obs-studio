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

#include "rapidjson/document.h"
#include "rapidjson/reader.h"

#include "obs-tray.hpp"
#include "ui_OBSTrayConfigWindow.h"

#include <iostream>

// exibe corretamente strings acentuadas
#define ptbr QString::fromLatin1

using namespace rapidjson;

OBSTray::OBSTray() : ui(new Ui::OBSTrayConfig)
{
	ui->setupUi(this);

	//cria websocket que recebera comandos do mconf
	wbsServer = new QWebSocketServer(QStringLiteral(""),
		QWebSocketServer::NonSecureMode, this);
	if (wbsServer->listen(QHostAddress::Any, 2424)) {
		connect(wbsServer, SIGNAL(newConnection()), this, SLOT(AddClient()));
	}

	defaultIcon = QIcon(":/settings/images/settings/video-display-3.png");
	playingIcon = QIcon(":/settings/images/settings/network.png");

	toggleVisibilityAction = new QAction(tr("Toggle"), this);
	connect(toggleVisibilityAction, SIGNAL(triggered()),
		this, SLOT(ToggleVisibility()));

	stopAction = new QAction(tr("Parar"), this);
	connect(stopAction, SIGNAL(triggered()), this, SLOT(hide()));

	setupAction = new QAction(ptbr("Configuração"), this);
	connect(setupAction, SIGNAL(triggered()), this, SLOT(show()));

	quitAction = new QAction(tr("&Sair"), this);
	connect(quitAction, SIGNAL(triggered()), this, SLOT(closeObsTray()));

	createTrayIcon();

	trayIcon->setIcon(defaultIcon);
	trayIcon->show();

	setWindowTitle(tr("OBSTray"));
}

void OBSTray::AddClient()
{
	clientWbSocket = wbsServer->nextPendingConnection();
	connect(clientWbSocket, SIGNAL(textMessageReceived(QString)), this, SLOT(ProcessRemoteController(QString)));
}

void OBSTray::ProcessRemoteController(QString str)
{
	//processa comando recebido
	//validação/segurança (?)
	Message m(str);

	if (!m.isValid)
		return;

	if (m.Type == "Toggle")
		ToggleVisibility();

	else if (m.Type == "StopStreaming")
		SendStopStreamingSignal();

	else if (m.Type == "Close")
		SendCloseSignal();
	
	// como dizer pro obs o endereço da transmissão?
	//StartStreaming();
}

void OBSTray::ToggleVisibility(){
	emit toggleVisibility();
}

void OBSTray::SendStartStreamingSignal(Message c){
	emit startStreaming(c.StreamURL, c.StreamPath,
		c.DisplayID, c.Width, c.Height, c.Downscale, c.BitRate);
}

void OBSTray::SendStopStreamingSignal(){
	emit stopStreaming();
}

void OBSTray::SendCloseSignal(){
	trayIcon->hide();

	emit closeObs();
}

void OBSTray::setVisible(bool visible)
{
	QDialog::setVisible(visible);
}

void OBSTray::closeEvent(QCloseEvent *event)
{
	if (trayIcon->isVisible()) {
		QMessageBox::information(this, tr("OBSTray"),
			ptbr("OBSTray continuará executando em segundo "
			"plano aguardando o início da transmissão."));
		hide();
		event->ignore();
	}
}

void OBSTray::closeObsTray()
{
	QMessageBox::StandardButton reallyCloseObs;
	reallyCloseObs = QMessageBox::question(this, tr("OBSTray"),
		ptbr("Tem certeza que deseja encerrar o OBS?"),
		QMessageBox::Yes | QMessageBox::No);
	if (reallyCloseObs == QMessageBox::Yes)
		SendCloseSignal();
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
	trayIconMenu->addAction(toggleVisibilityAction);
	//trayIconMenu->addAction(stopAction);
	trayIconMenu->addAction(setupAction);
	trayIconMenu->addSeparator();
	trayIconMenu->addAction(quitAction);

	trayIcon = new QSystemTrayIcon(this);
	trayIcon->setContextMenu(trayIconMenu);
}

Message::Message(QString str){
	isValid = true;		RawData = "";
	MessageID = 0;		Type = "";
	StreamPath = "";	StreamURL = "";
	DisplayID = 0;		Width = 0;	Height = 0;
	Downscale = 0;		BitRate = 0;

	bool debug = false;
	
	std::string data = str.toStdString();

	Document d;
	d.Parse(data.c_str());

	RawData = str;

	try{
		Type = d["type"].GetString();
		
		if (d.HasMember("id"))
			MessageID = d["id"].GetInt();

		if (d.HasMember("debug"))
			debug = d["debug"].GetBool();
	}
	catch (std::exception e){
		isValid = false;
	}

	if (debug)
		QMessageBox::information(nullptr, "Message",
			data.c_str(), QMessageBox::StandardButton::Ok);

	if (Type == "StartStreaming"){
		try{
			StreamPath = d["streampath"].GetString();
			StreamURL = d["streamurl"].GetString();

			DisplayID = d["displayid"].GetInt();
			Width = d["width"].GetInt();
			Height = d["height"].GetInt();
			Downscale = d["downscale"].GetInt();
			BitRate = d["bitrate"].GetInt();
		}
		catch(std::exception e){
			isValid = false;
		}
		
	}
}
