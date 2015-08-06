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

#pragma once

#include <QtGui>
#include <QAction>
#include <QComboBox>
#include <QDialog>
#include <QGroupBox>
#include <QSystemTrayIcon>
#include <QWidget>

#include <QtWebSockets\QtWebSockets>
#include <QtWebSockets\qwebsocket.h>
#include <QtNetwork\QtNetwork>
#include <QtNetwork\qhostaddress.h>
#include <QtWebSockets\qwebsocketserver.h>

#include <memory>	// std::unique_ptr (for the ui)
#include "ui_OBSTrayConfigWindow.h"

class Message;

class OBSTray : public QDialog {
	Q_OBJECT
public:
	OBSTray();
	void setVisible(bool visible) Q_DECL_OVERRIDE;

protected:
	void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
	void SendStartStreamingSignal(Message configs);
	void SendStopStreamingSignal();
	void SendCloseSignal();

private slots:
	void setIcon(int index);
	void AddClient();
	void ProcessRemoteController(QString str);
	
public slots:
	void ToggleVisibility();
	void closeObsTray();

signals:
	void toggleVisibility();
	void closeObs();
	void startStreaming(QString url, QString path, int display,
		int width, int height, int downscale, int bitrate);
	void stopStreaming();

private:
	void createActions();
	void createTrayIcon();

	QPointer<QWebSocketServer>	wbsServer;
	QPointer<QWebSocket>		clientWbSocket;	

	QGroupBox *iconGroupBox;
	QComboBox *iconComboBox;

	QAction *toggleVisibilityAction;
	QAction *setupAction;
	QAction *stopAction;
	QAction *quitAction;

	QSystemTrayIcon *trayIcon;
	QIcon defaultIcon;
	QIcon playingIcon;
	QMenu *trayIconMenu;

	std::unique_ptr<Ui::OBSTrayConfig> ui;
};

class Message{
public:
	Message(QString json_data);
	
	bool isValid;
	QString RawData;
	int MessageID;

	QString Type;

	QString StreamPath;
	QString StreamURL;
	
	int DisplayID;
	int Width;
	int Height;
	int Downscale;
	int BitRate;
};
