/*
    VatsimDataHandler.cpp
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

#include <cstring>
#include <cstdlib>
#include <ctime>

#include <QtGui>

#include "../include/VatsimDataHandler.h"

#include "../include/VatsinatorApplication.h"

#include "../include/defines.h"


VatsimDataHandler::VatsimDataHandler() :
		__airports(AirportsDatabase::GetSingleton()),
		__firs(FirsDatabase::GetSingleton()),
		__mother(VatsinatorApplication::GetSingleton()) {}

VatsimDataHandler::~VatsimDataHandler() {
	while (!__pilots.empty())
		delete __pilots.back(), __pilots.pop_back();
	while (!__atcs.empty())
		delete __atcs.back(), __atcs.pop_back();
	while (!__uirs.empty())
		delete __uirs.back(), __uirs.pop_back();
	for (auto it = __activeAirports.begin(); it != __activeAirports.end(); ++it)
		delete it.value();
}

void
VatsimDataHandler::init() {
	QFile datFile(VATSINATOR_DAT);
	if (!datFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qDebug() << "File " << VATSINATOR_DAT << " could not be opened!";
		return;
	}
	
	QMap< QString, bool > flags;
	flags["[FIR]"] = false;
	flags["[ALIAS]"] = false;
	flags["[UIR]"] = false;
	
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
		
		if (flags["[FIR]"]) {
			QString icao = line.section(' ', 0, 0);
			if (icao.length() != 4) {
				qDebug() << "Word " << icao << " is not an ICAO code and could not be parsed." <<
					"For aliases see [ALIAS] section.";
				continue;
			}
			
			Fir* currentFir = __firs.findFirByIcao(icao);
			if (!currentFir) {
				qDebug() << "Fir " << icao << " could not be found. Try [ALIAS] section!";
				continue;
			}
			if (!currentFir->name.isEmpty() && !currentFir->name.isNull())
				qDebug() << "Found duplicate for " << icao << "!";
			
			// finally:
			currentFir->name = line.section(' ', 1);
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
	while (!__pilots.empty())
		delete __pilots.back(), __pilots.pop_back();
	__pilots.clear();
	while (!__atcs.empty())
		delete __atcs.back(), __atcs.pop_back();
	__atcs.clear();
	for (auto it = __activeAirports.begin(); it != __activeAirports.end(); ++it)
		delete it.value();
	__activeAirports.clear();
	
	qDebug() << "Data length: " << _data.length();
	QStringList tempList = _data.split('\n', QString::SkipEmptyParts);
	
	QMap< QString, bool > flags;
	flags["GENERAL"] = false;
	flags["CLIENTS"] = false;
	
	for (QString& temp: tempList) {
		if (temp.startsWith(';'))
			continue;
		
		if (temp.startsWith('!')) {
			__clearFlags(flags);
			if (temp.simplified() == "!GENERAL:")
				flags["GENERAL"] = true;
			if (temp.simplified() == "!CLIENTS:")
				flags["CLIENTS"] = true;
			
			continue;
		}
		
		if (flags["CLIENTS"]) {
			QStringList clientData = temp.split(':');
			
			if (clientData[3] == "ATC")
				__parseATC(clientData);
			else if (clientData[3] == "PILOT")
				__parsePilot(clientData);
		}
		
	}
}

const QString &
VatsimDataHandler::getDataUrl() {
	srand(time(NULL));
	
	return __servers[rand() % __servers.size()];
}

const Pilot *
VatsimDataHandler::findPilot(const QString& _callsign) {
	for (const Pilot* p: __pilots)
		if (p->callsign == _callsign)
			return p;
	return NULL;
}


/*
 * Ok, this is out important parsing section. Let's set things up:
 * 0 callsign
 * 1 cid
 * 2 realname
 * 3 clienttype
 * 4 frequency
 * 5 latitude
 * 6 longitude
 * 7 altitude
 * 8 groundspeed
 * 9 planned_aircraft
 * 10 planned_tascruise
 * 11 planned_depairport
 * 12 planned_altitude
 * 13 planned_destairport
 * 14 server
 * 15 protrevision
 * 16 rating
 * 17 transponder
 * 18 facilitytype
 * 19 visualrange
 * 20 planned_revision
 * 21 planned_flighttype
 * 22 planned_deptime
 * 23 planned_actdeptime
 * 24 planned_hrsenroute
 * 25 planned_minenroute
 * 26 planned_hrsfuel
 * 27 planned_minfuel
 * 28 planned_altairport
 * 29 planned_remarks
 * 30 planned_route
 * 31 planned_depairport_lat
 * 32 planned_depairport_lon
 * 33 planned_destairport_lat
 * 34 planned_destairport_lon
 * 35 atis_message
 * 36 time_last_atis_received
 * 37 time_logon
 * 38 heading
 * 39 QNH_iHg
 * 40 QNH_Mb
 */
void
VatsimDataHandler::__parseATC(const QStringList& _clientData) {
	if (_clientData.size() < 41) {
		emit dataCorrupted();
		return;
	}
	
	Controller* client = new Controller;
	
	client->callsign = _clientData[0];
	client->pid = _clientData[1].toUInt();
	client->realName = _clientData[2];
	
	client->frequency = _clientData[4];
	client->server = _clientData[14];
	client->rating = _clientData[16].toInt();
	client->atis = _clientData[35];
	
	client->onlineFrom = QDateTime::fromString(_clientData[37], "yyyyMMddhhmmss");
	
	if (client->atis[0] == '$') {
		auto index = client->atis.indexOf('^');
		client->atis.remove(0, index + 2);
	}
	client->atis.replace((QString)'^' + (char)167, "\n");
	
	__atcs.push_back(client);
	
	__setIcaoAndFacility(client);
}

void
VatsimDataHandler::__parsePilot(const QStringList& _clientData) {
	if (_clientData.size() < 41) {
		emit dataCorrupted();
		return;
	}
	
	Pilot* client = new Pilot;
	
	client->callsign = _clientData[0];
	client->pid = _clientData[1].toUInt();
	client->realName = _clientData[2];
	client->position.latitude = _clientData[5].toDouble();
	client->position.longitude = _clientData[6].toDouble();
	client->altitude = _clientData[7].toInt();
	client->groundSpeed = _clientData[8].toInt();
	client->aircraft = _clientData[9];
	client->tas = _clientData[10].toInt();
	client->route.origin = _clientData[11];
	client->route.altitude = _clientData[12];
	client->route.destination = _clientData[13];
	client->server = _clientData[14];
	client->squawk = _clientData[17].toShort();
	client->flightRules = (_clientData[21] == "I") ? IFR : VFR;
	client->remarks = _clientData[29];
	client->route.route = _clientData[30];
	client->onlineFrom = QDateTime::fromString(_clientData[37], "yyyyMMddhhmmss");
	client->heading = _clientData[38].toUInt();
	
	__pilots.push_back(client);
	
	if (!__activeAirports.contains(client->route.origin))
		__activeAirports.insert(client->route.origin,
				new AirportObject(client->route.origin));
		__activeAirports[client->route.origin]->addOutbound(client);
	
	if (!__activeAirports.contains(client->route.destination))
		__activeAirports.insert(client->route.destination,
				new AirportObject(client->route.destination));
		__activeAirports[client->route.destination]->addInbound(client);
	
	__setStatus(client);
}

void
VatsimDataHandler::__setStatus(Pilot* _pilot) {
	if (!_pilot->route.origin.isEmpty()) { // we have flight plan, ok
		const AirportRecord* ap_origin = __activeAirports[_pilot->route.origin]->getData();
		const AirportRecord* ap_arrival = __activeAirports[_pilot->route.destination]->getData();
	
		if ((ap_origin == ap_arrival) && (ap_origin != NULL)) // traffic pattern?
			if (_pilot->groundSpeed < 25) {
				_pilot->flightStatus = DEPARTING;
				return;
			}
	
		if (ap_origin)
			// check if origin airport is in range
			if ((__calcDistance(ap_origin->longitude, ap_origin->latitude,
						_pilot->position.longitude, _pilot->position.latitude) < PILOT_TO_AIRPORT)
						&& (_pilot->groundSpeed < 50)) {
				_pilot->flightStatus = DEPARTING;
				return;
			}
	
		if (ap_arrival)
			// or maybe arrival?
			if ((__calcDistance(ap_arrival->longitude, ap_arrival->latitude,
						_pilot->position.longitude, _pilot->position.latitude) < PILOT_TO_AIRPORT)
						&& (_pilot->groundSpeed < 50)) {
				_pilot->flightStatus = ARRIVED;
				return;
			}
	} else { // no flight plan, we have to check where exactly the pilot is
		if (_pilot->groundSpeed > 30) {
			_pilot->flightStatus = AIRBORNE;
			return;
		}
		
		const AirportRecord* closest = NULL;
		double distance = 0.0;
		for (const AirportRecord& ap: __airports.getAirports()) { // yeah, this is messy
			double temp = __calcDistance(ap.longitude, ap.latitude,
										 _pilot->position.longitude, _pilot->position.latitude);
			if (((temp < distance) && closest) || !closest) {
				closest = &ap;
				distance = temp;
			}
		}
		
		if (closest) { // we found something really close
			if (__calcDistance(closest->longitude, closest->latitude,
						_pilot->position.longitude, _pilot->position.latitude) > PILOT_TO_AIRPORT) {
				_pilot->flightStatus = AIRBORNE;
				return;
			}

			_pilot->route.origin = closest->icao;
			if (!__activeAirports.contains(_pilot->route.origin))
				__activeAirports.insert(_pilot->route.origin, new AirportObject(_pilot->route.origin));
			__activeAirports[_pilot->route.origin]->addOutbound(_pilot);
			__setStatus(_pilot);
			return;
		}
	}
	
	// finally:
	_pilot->flightStatus = AIRBORNE;
}

void
VatsimDataHandler::__setIcaoAndFacility(Controller* _atc) {
	QStringList sections = _atc->callsign.split('_');
	if (sections.back() == "CTR") {
		_atc->facility = CTR;
		_atc->airport = NULL;
		
		QString icao = sections.front();
		
		Fir* fir = __firs.findFirByIcao(icao);
		if (fir)
			fir->addStaff(_atc);
		else {
			// many of USA controllers use just three last letters of position
			// ICAO
			if (icao.length() == 3) {
				fir = __firs.findFirByIcao("K" + icao); 
				if (fir) {
					fir->addStaff(_atc);
					return;
				}
			}
			
			for (QString& alias: __aliases.values(icao)) {
				fir = __firs.findFirByIcao(alias);
				if (fir) {
					fir->addStaff(_atc);
					return;
				}
			}
			
			Uir* uir = __findUIR(icao);
			if (uir) {
				uir->addStaff(_atc);
				return;
			}
			
			qDebug() << "FIR not found: " << icao << "(" << _atc->callsign << ")";
		}
		
		return;
	} else if (sections.back() == "FSS") {
		_atc->facility = FSS;
		_atc->airport = NULL;
		
		QString icao = sections.front();
		
		Fir* fir = __firs.findFirByIcao(icao + "F");
		if (fir)
			fir->addStaff(_atc);
		else {
			Uir* uir = __findUIR(icao);
			if (uir) {
				uir->addStaff(_atc);
				return;
			}
			
			for (QString& alias: __aliases.values(icao)) {
				fir = __firs.findFirByIcao(alias + "F");
				if (fir) {
					fir->addStaff(_atc);
					return;
				}
			}
			
			qDebug() << "FIR not found: " << icao << "(" << _atc->callsign << ")";
		}
		
		return;
	} else if (
			sections.back() == "APP" ||
			sections.back() == "TWR" ||
			sections.back() == "GND" ||
			sections.back() == "DEL" ||
			sections.back() == "ATIS") {
		
		if (sections.back() == "APP")
			_atc->facility = APP;
		else if (sections.back() == "TWR")
			_atc->facility = TWR;
		else if (sections.back() == "GND")
			_atc->facility = GND;
		else if (sections.back() == "DEL")
			_atc->facility = DEL;
		else if (sections.back() == "ATIS")
			_atc->facility = ATIS;
		
		AirportRecord* apShot = __airports.find(sections[0]);
		if (apShot) {
			if (!__activeAirports.contains(sections[0]))
				__activeAirports.insert(sections[0], new AirportObject(sections[0]));
			__activeAirports[sections[0]]->addStaff(_atc);
			_atc->airport = __activeAirports[sections[0]]->getData();
			return;
		} else {
			if (sections[0].length() == 3) {
				QString alias = "K" + sections[0];
				apShot = __airports.find(alias);
				if (apShot) {
					if (!__activeAirports.contains(alias))
						__activeAirports.insert(alias, new AirportObject(alias));
					__activeAirports[alias]->addStaff(_atc);
					_atc->airport = __activeAirports[alias]->getData();
					return;
				}
			}
			
			for (QString& alias: __aliases.values(sections[0])) {
				apShot = __airports.find(alias);
				if (apShot) {
					if (!__activeAirports.contains(alias))
						__activeAirports.insert(alias, new AirportObject(alias));
					__activeAirports[alias]->addStaff(_atc);
					_atc->airport = __activeAirports[alias]->getData();
					return;
				}
			}
		
			qDebug() << "Airport not found: " << sections[0] << "(" << _atc->callsign << ")";
		}
		
		return;
	} else if (sections.back() == "OBS") {
		_atc->facility = OBS;
		_atc->airport = NULL;
		return;
	}
}

void
VatsimDataHandler::__clearFlags(QMap< QString, bool >& _flags) {
	for (auto it = _flags.begin(); it != _flags.end(); ++it)
		it.value() = false;
}

Uir *
VatsimDataHandler::__findUIR(const QString& _icao) {
	for (Uir* u: __uirs)
		if (u->icao == _icao)
			return u;
		
	return NULL;
}
