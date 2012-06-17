/*
    vatsimdatahandler.cpp
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

#include "vatsimdata/models/controllertablemodel.h"
#include "vatsimdata/models/flighttablemodel.h"

#include "vatsinatorapplication.h"

#include "vatsimdatahandler.h"
#include "defines.h"


VatsimDataHandler::VatsimDataHandler() :
		__flights(new FlightTableModel()),
		__atcs(new ControllerTableModel()),
		__airports(AirportsDatabase::GetSingleton()),
		__firs(FirsDatabase::GetSingleton()),
		__mother(VatsinatorApplication::GetSingleton()) {}

VatsimDataHandler::~VatsimDataHandler() {
	__clearData();
	while (!__uirs.empty())
		delete __uirs.back(), __uirs.pop_back();
	
	delete __atcs;
	delete __flights;
}

void
VatsimDataHandler::init() {
	QFile datFile(VATSINATOR_DAT);
	if (!datFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qDebug() << "File " << VATSINATOR_DAT << " could not be opened!";
		VatsinatorApplication::alert(
			static_cast< QString >("File ") +
			static_cast< QString >(VATSINATOR_DAT) +
			static_cast< QString >(" could not be opened!"));
		return;
	}
	
	QMap< QString, bool > flags;
	flags["[COUNTRY]"] = false;
	flags["[FIR]"] = false;
	flags["[ALIAS]"] = false;
	flags["[UIR]"] = false;
	
	QMap< QString, QString > countries;
	
	while (!datFile.atEnd()) {
		QString line(datFile.readLine());
		
		line = line.simplified();
		
		if (line[0] == '#' || line.isEmpty())
			continue;
		
		if (line[0] == '[') {
			if (!flags.contains(line)) {
				qDebug() << "Flag " << line << " could not be recognized! Check your vatsinator.dat file.";
				continue;
			}
			__clearFlags(flags);
			flags[line] = true;
			continue;
		}
		
		if (flags["[COUNTRY]"]) {
			countries.insert(
				line.section(' ', -1),
				line.section(' ', 0, -2)
			);
		} else if (flags["[FIR]"]) {
			QString icao = line.section(' ', 0, 0);
			
			Fir* currentFir = __firs.findFirByIcao(icao);
			if (currentFir) {
				currentFir->setName(line.section(' ', 1));
				if (currentFir->getIcao() == "UMKK") // fix for Kaliningrad Center
					currentFir->setCountry("Russia");
				else
					currentFir->setCountry(countries[icao.left(2)]);
				currentFir->correctName();
			}
			
			// look for same oceanic fir
			currentFir = __firs.findFirByIcao(icao, true);
			if (currentFir) {
				currentFir->setName(line.section(' ', 1));
				currentFir->setCountry(countries[icao.left(2)]);
				currentFir->correctName();
			}
			
			
		} else if (flags["[ALIAS]"]) {
			QStringList data = line.split(' ');
			QString icao = data[0];
			if (icao.length() != 4) {
				qDebug() << "Word " << icao << " is not an ICAO code and could not be parsed." <<
								"For aliases see [ALIAS] section.";
				continue;
			}
			
			for (int i = 1; i < data.length(); ++i) {
				if (data[i][0] == '{') {
					__aliases.insert(data[i].mid(1), icao);
				} else if (data[i].toUpper() == data[i]) {
					__aliases.insert(data[i], icao);
					//qDebug() << "I: " << data[i] << " = " << icao;
				}
			}
		} else if (flags["[UIR]"]) {
			QStringList data = line.split(' ');
			Uir* uir = new Uir;
			uir->icao = data[0];
			for (int i = 1; i < data.length(); ++i) {
				if (data[i].toUpper() == data[i]) {
					Fir* fir = __firs.findFirByIcao(data[i]);
					if (fir)
						uir->addFir(fir);
					else
						qDebug() << "FIR " << data[i] << " could not be found!";
				} else {
					uir->name.append(data[i] + " ");
				}
			}
			__uirs.push_back(uir);
		}
	}
	
	datFile.close();
}

void
VatsimDataHandler::parseStatusFile(const QString& _statusFile) {
	QStringList tempList = _statusFile.split('\n', QString::SkipEmptyParts);
	for (QString& temp: tempList) {
		if (temp.startsWith(';'))
			continue;
		
		if (temp.startsWith("metar0=")) {
			__metarURL = temp.mid(7);
			__metarURL = __metarURL.simplified();
			continue;
		}
		
		if (temp.startsWith("url0=")) {
			QString url0 = temp.mid(5);
			url0 = url0.simplified();
			__servers.push_back(url0);

			continue;
		}
	}
}

void
VatsimDataHandler::parseDataFile(const QString& _data) {
	__clearData();
	
#ifndef NO_DEBUG
	qDebug() << "Data length: " << _data.length();
#endif
	QStringList tempList = _data.split('\n', QString::SkipEmptyParts);
	
	QMap< QString, bool > flags;
	flags["GENERAL"] = false;
	flags["CLIENTS"] = false;
	flags["PREFILE"] = false;
	
	for (QString& temp: tempList) {
		if (temp.startsWith(';'))
			continue;
		
		if (temp.startsWith('!')) {
			__clearFlags(flags);
			if (temp.simplified() == "!GENERAL:")
				flags["GENERAL"] = true;
			else if (temp.simplified() == "!CLIENTS:")
				flags["CLIENTS"] = true;
			else if (temp.simplified() == "!PREFILE:")
				flags["PREFILE"] = true;
			
			continue;
		}
		
		if (flags["GENERAL"]) {
			if (temp.startsWith("UPDATE")) {
				__dateDataUpdated = QDateTime::fromString(
					temp.split(' ').back().simplified(),
					"yyyyMMddhhmmss"
				);
			}
		} else if (flags["CLIENTS"]) {
			QStringList clientData = temp.split(':');
			if (clientData.size() < 40) {
				emit dataCorrupted();
				return;
			}
			
			if (clientData[3] == "ATC") {
				Controller* atc = new Controller(clientData);
				__atcs->addStaff(atc);
			} else if (clientData[3] == "PILOT") {
				Pilot* pilot = new Pilot(clientData);
				__flights->addFlight(pilot);
			}
		} else if (flags["PREFILE"]) {
			QStringList clientData = temp.split(':');
			if (clientData.size() < 40) {
				emit dataCorrupted();
				return;
			}
			
			Pilot* pilot = new Pilot(clientData, true);
			__flights->addFlight(pilot);
		}
	}
}

const QString &
VatsimDataHandler::getDataUrl() const {
	qsrand(QTime::currentTime().msec());
	return __servers[qrand() % __servers.size()];
}

const Pilot *
VatsimDataHandler::findPilot(const QString& _callsign) const {
	return __flights->findFlightByCallsign(_callsign);
}

Uir *
VatsimDataHandler::findUIR(const QString& _icao) {
	for (Uir* u: __uirs)
		if (u->icao == _icao)
			return u;
		
	return NULL;
}


Airport *
VatsimDataHandler::addActiveAirport(const QString& _icao) {
	if (!__activeAirports.contains(_icao))
		__activeAirports.insert(_icao, new Airport(_icao));
	return __activeAirports[_icao];
}

void
VatsimDataHandler::__clearFlags(QMap< QString, bool >& _flags) {
	for (auto it = _flags.begin(); it != _flags.end(); ++it)
		it.value() = false;
}

void
VatsimDataHandler::__clearData() {
	for (const Pilot* p: __flights->getFlights())
		delete p;
	__flights->clear();
	
	for (const Controller* c: __atcs->getStaff())
		delete c;
	__atcs->clear();
	
	for (Uir* u: __uirs)
		u->clear();
	
	for (auto it = __activeAirports.begin(); it != __activeAirports.end(); ++it)
		delete it.value();
	__activeAirports.clear();
}