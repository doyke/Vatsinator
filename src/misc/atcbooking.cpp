/*
 * atcbooking.cpp
 * Copyright (C) 2016 Michał Garapich <michal@garapich.pl>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include "atcbooking.h"
#include <QSharedData>

namespace Vatsinator { namespace Misc {

class AtcBookingData : public QSharedData {
public:
    AtcBookingData() = default;
    explicit AtcBookingData(const QString& callsign) : callsign(callsign) {}
    
    QString callsign;
    QString realName;
    QDate bookedDay;
    QTime timeFrom;
    QTime timeTo;
    bool isTrainingSession = false;
};

AtcBooking::AtcBooking() : d(new AtcBookingData) {}

AtcBooking::AtcBooking(const QString& callsign) :
    d(new AtcBookingData(callsign)) {}

AtcBooking::AtcBooking(const AtcBooking& orig) :
    d(orig.d) {}

AtcBooking::~AtcBooking() {}

AtcBooking& AtcBooking::operator=(const AtcBooking& other)
{
    d = other.d;
    return *this;
}

QString AtcBooking::callsign() const
{
    return d->callsign;
}

void AtcBooking::setCallsign(const QString& callsign)
{
    d->callsign = callsign;
}

QString AtcBooking::realName() const
{
    return d->realName;
}

void AtcBooking::setRealName(const QString& realName)
{
    d->realName = realName;
}

QDate AtcBooking::bookedDay() const
{
    return d->bookedDay;
}

void AtcBooking::setBookedDay(const QDate& bookedDay)
{
    d->bookedDay = bookedDay;
}

QTime AtcBooking::timeFrom() const
{
    return d->timeFrom;
}

void AtcBooking::setTimeFrom(const QTime& timeFrom)
{
    d->timeFrom = timeFrom;
}

QTime AtcBooking::timeTo() const
{
    return d->timeTo;
}

void AtcBooking::setTimeTo(const QTime& timeTo)
{
    d->timeTo = timeTo;
}

bool AtcBooking::isTrainingSession() const
{
    return d->isTrainingSession;
}

void AtcBooking::setIsTrainingSession(bool isTrainingSession)
{
    d->isTrainingSession = isTrainingSession;
}

}} /* namespace Vatsinator::Misc */
