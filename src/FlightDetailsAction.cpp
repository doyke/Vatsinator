/*
    FlightDetailsAction.cpp
    Copyright (C) 2012  Michał Garapich garrappachc@gmail.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QtGui>

#include "../include/FlightDetailsAction.h"

#include "../include/Client.h"

FlightDetailsAction::FlightDetailsAction(const Client* _client, QObject* _parent) :
		QAction("Flight details", _parent),
		__current(_client) {
	
	connect(this, SIGNAL(triggered()), this, SLOT(handleTriggered()));
}

FlightDetailsAction::FlightDetailsAction(const QString& _text, const Client* _client, QObject* _parent) :
		QAction(_text, _parent),
		__current(_client) {
	
	connect(this, SIGNAL(triggered()), this, SLOT(handleTriggered()));
}

void
FlightDetailsAction::handleTriggered() {
	emit clicked(__current);
}


