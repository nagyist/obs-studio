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

#include <QtWidgets>
#include "obs-tray-window-config.hpp"
#include "obs-tray.hpp"

OBSTrayConfig::OBSTrayConfig(int displayId, bool mouse) : QDialog() {
	QDesktopWidget desktop;
	int screenCount = desktop.screenCount();

	displaySelection = new QComboBox();
	for (int i = 1; i <= screenCount; i++)
		displaySelection->addItem(tr("Display ") + QString::number(i));
	displaySelection->setCurrentIndex(displayId);

	captureMouse = new QCheckBox(tr("Capture mouse"));
	captureMouse->setChecked(mouse);

	QPushButton *saveButton = new QPushButton(tr("Save"));
	connect(saveButton, SIGNAL(clicked()), this, SLOT(Save()));

	QPushButton *closeButton = new QPushButton(tr("Close"));
	connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(displaySelection);
	layout->addWidget(captureMouse);

	QHBoxLayout *buttonsLayout = new QHBoxLayout;
	buttonsLayout->addStretch(1);
	buttonsLayout->addWidget(saveButton);
	buttonsLayout->addWidget(closeButton);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addLayout(layout);
	mainLayout->addStretch(1);
	mainLayout->addSpacing(12);
	mainLayout->addLayout(buttonsLayout);
	setLayout(mainLayout);

	setWindowTitle(tr("Config Dialog"));
}

void OBSTrayConfig::Save(bool close){
	emit SetConfig(displaySelection->currentIndex(), captureMouse->isChecked());

	if (close)
		this->close();
}
