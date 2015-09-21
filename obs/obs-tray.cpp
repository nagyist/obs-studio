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
#include "obs-app.hpp"

#include <iostream>

// exibe corretamente strings acentuadas
#define ptbr QString::fromLatin1

using namespace rapidjson;

OBSTray::OBSTray(){
	isConnected = false;
	isStreaming = false;
	
	//cria websocket que recebera comandos do mconf
	wsServer = new QWebSocketServer(QStringLiteral(""),
		QWebSocketServer::NonSecureMode, this);

	if (wsServer->listen(QHostAddress::Any, 2424)) {
		connect(wsServer, SIGNAL(newConnection()), this, SLOT(onClientConnected()));
	}

	defaultIcon = QIcon(":/settings/images/settings/video-display-3.png");
	playingIcon = QIcon(":/settings/images/settings/network.png");

	CreateActions();
	
	// Prevents the application from exiting when there are no windows open
	qApp->setQuitOnLastWindowClosed(false);

	this->setIcon(defaultIcon);
	CreateTrayIcon();
	this->show();
}

void OBSTray::onClientConnected(){
	wsClient = wsServer->nextPendingConnection();

	connect(wsClient, SIGNAL(textMessageReceived(QString)), this, SLOT(onMessageReceived(QString)));
	connect(wsClient, SIGNAL(disconnected()), this, SLOT(onClientDisconnected()));

	wsClient->sendTextMessage(tr("{ \"version\": \"unknown\" }"));
	isConnected = true;
}

void OBSTray::CreateActions(){
	infoAction = new QAction(tr("Show info"), this);
	connect(infoAction, SIGNAL(triggered()), this, SLOT(ShowInfo()));

	toggleVisibilityAction = new QAction(tr("Toggle"), this);
	connect(toggleVisibilityAction, SIGNAL(triggered()),
		this, SLOT(ToggleVisibility()));

	quitAction = new QAction(tr("&Sair"), this);
	connect(quitAction, SIGNAL(triggered()), this, SLOT(Close()));
}

void OBSTray::CreateTrayIcon(){
	trayIconMenu = new QMenu(nullptr);
	trayIconMenu->addAction(infoAction);
	trayIconMenu->addAction(toggleVisibilityAction);
	trayIconMenu->addSeparator();
	trayIconMenu->addAction(quitAction);

	this->setContextMenu(trayIconMenu);
	this->setToolTip(tr("Mconf Deskshare"));

	connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
		this, SLOT(onActivated(QSystemTrayIcon::ActivationReason)));
}

void OBSTray::onActivated(QSystemTrayIcon::ActivationReason reason){
	if (reason == ActivationReason::DoubleClick)
		ShowInfo();
}

void OBSTray::onMessageReceived(QString str){
	//processa comando recebido

	Message m(str);

	if (!m.isValid)
		return;
	
	if (m.Type == "Toggle")
		ToggleVisibility();

	else if (m.Type == "StartStreaming")
		SendStartStreamingSignal(m);

	else if (m.Type == "StopStreaming")
		SendStopStreamingSignal();

	else if (m.Type == "Close")
		SendCloseSignal();
}

void OBSTray::onClientDisconnected(){
	if (isStreaming)
		SendStopStreamingSignal();

	isConnected = false;
}

void OBSTray::ShowInfo(){
	QString message;
	
	message += tr("Mconf Deskshare app is ");
	if (!isConnected) message += tr("not ");
	message += tr("connected to the Mconf client\n\n");

	message += tr("Your screen is ");
	if (!isStreaming) message += tr("not ");
	message += tr("being shared\n\n");

	if (isStreaming){
		message += tr("URL:\n\t");
		message += streamURL;

		message += tr("\n\nPath:\n\t");
		message += streamPath;
	}

	QMessageBox::information(nullptr, tr("Status information"), message, QMessageBox::Ok);
}

void OBSTray::ToggleVisibility(){
	emit signal_toggleVisibility();
}

void OBSTray::SendStartStreamingSignal(Message c){
	if (!c.isValid) return;
	
	/* sample
{"type": "StartStreaming", "streamPath": "path", "streamName": "url", "displayId": 1, 
"width": 1280, "height": 1024, "swidth": 800, "sheight": 600, "bitrate": 1000}
	*/

	/* current
{"type": "StartStreaming", "streamPath": "path", "streamName": "url?", "displayId": 0, 
"bitrate": 1000, "fps": 15, "width":800, "height": 600, "messageid":"4"}
	*/

	streamURL = c.StreamName;
	streamPath = c.StreamPath;

	emit signal_startStreaming(c.StreamName, c.StreamPath,
		c.DisplayID, c.Width, c.Height, c.SWidth, c.SHeight, c.Bitrate);

	isStreaming = true;

	showMessage(tr("Mconf Deskshare"), tr("Streaming initiated"),
		QSystemTrayIcon::Information, balloonDuration);
}

void OBSTray::SendStopStreamingSignal(bool showBalloon){
	emit signal_stopStreaming();
	isStreaming = false;

	if (showBalloon)
		showMessage(tr("Mconf Deskshare"), tr("Streaming stopped"),
		QSystemTrayIcon::Information, balloonDuration);
}

void OBSTray::SendCloseSignal(){
	if (isStreaming)
		SendStopStreamingSignal(false);
	
	this->hide();

	emit signal_close();
}

void OBSTray::Close(){
	QMessageBox::StandardButton reallyCloseObs;
	reallyCloseObs = QMessageBox::question(nullptr, tr("OBSTray"),
		ptbr("Tem certeza que deseja encerrar o OBS?"));

	if (reallyCloseObs == QMessageBox::Yes)
		SendCloseSignal();
}

OBSTray::~OBSTray() {
	this->hide();
}

void OBSTray::setTrayIcon(int index)
{
	QIcon icon = iconComboBox->itemIcon(index);
	this->setIcon(icon);

	this->setToolTip(iconComboBox->itemText(index));
}

Message::Message() {
	isValid = true;		RawData = "";
	MessageID = 0;		Type = "";
	StreamPath = "";	StreamName = "";
	DisplayID = 0;		Width = 0;	Height = 0;
	SWidth = 0;			Bitrate = 0;
	SHeight = 0;
}

Message::Message(QString str) {
	Message();

	RawData = str;
	ReadFrom(str.toStdString());
}

void Message::ReadFrom(std::string data){
	bool debug = false;
	Document d;
	d.Parse(data.c_str());

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
			StreamPath = d["streamPath"].GetString();
			StreamName = d["streamName"].GetString();

			DisplayID = d["displayId"].GetInt();
			Width = d["width"].GetInt();
			Height = d["height"].GetInt();
			SWidth = d["swidth"].GetInt();
			SHeight = d["sheight"].GetInt();
			Bitrate = d["bitrate"].GetInt();
		}
		catch (std::exception e){
			isValid = false;
		}

	}
}
