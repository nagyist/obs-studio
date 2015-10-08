/******************************************************************************

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
#include <QWidget>
#include <QCheckBox>
#include <QDesktopWidget>
#include <QDialog>

class OBSTrayConfig : public QDialog {
	Q_OBJECT
	/*
		This must have a way of getting from the OBS UI the value of some configs
			(the ones it wants to change)
		Display a window for configuring e.g. display to stream and other things
			(capture mouse?)
	*/

public:
	OBSTrayConfig(int displayId = 1, bool captureMouse = true);

signals:
	void SetConfig(int displayId, bool captureMouse);

public slots:
	void Save(bool close = true);
	
private:
	QComboBox *displaySelection;
	QCheckBox *captureMouse;
};
