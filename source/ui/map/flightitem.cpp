/*
 * flightitem.cpp
 * Copyright (C) 2014-2015  Michał Garapich <michal@garapich.pl>
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

#include <QtGui>

#include "db/airportdatabase.h"
#include "storage/settingsmanager.h"
#include "ui/map/mapconfig.h"
#include "ui/map/mapscene.h"
#include "ui/map/modelmatcher.h"
#include "ui/userinterface.h"
#include "vatsimdata/pilot.h"
#include "vatsimdata/airport.h"
#include "vatsimdata/vatsimdatahandler.h"
#include "vatsinatorapplication.h"

#include "flightitem.h"

FlightItem::FlightItem(const Pilot* pilot, QObject* parent) :
    MapItem(parent),
    __scene(qobject_cast<MapScene * >(parent)),
    __pilot(pilot),
    __position(pilot->position())
{
    
}

FlightItem::~FlightItem()
{
    
}

bool
FlightItem::isVisible() const
{
    return data()->phase() == Pilot::Airborne && !data()->isPrefiledOnly();
}

bool
FlightItem::isLabelVisible() const
{
    return __scene->settings().view.pilot_labels.always;
}

const LonLat&
FlightItem::position() const
{
    return __position;
}

void
FlightItem::draw(QPainter* painter, const QTransform& transform) const
{
    if (__model.isNull()) {
        QTransform t;
        t.rotate(static_cast<float>(data()->heading()));
        __model = __scene->modelMatcher()->match(__pilot->aircraft()).transformed(t, Qt::SmoothTransformation);
    }
    
    Q_ASSERT(!__model.isNull());
    
    QRectF rect(QPointF(0.0, 0.0), __model.size());
    rect.moveCenter(position() * transform);
    
    painter->drawPixmap(rect, __model, QRectF(QPointF(0.0, 0.0), __model.size()));
}

QString
FlightItem::tooltipText() const
{
    QString callsign = data()->callsign();
    QString desc = QStringLiteral("%1 (%2)").arg(data()->realName(), data()->aircraft());
    
    QString from;
    const Airport* ap = data()->origin();
    
    if (ap)
        from = QString(ap->icao()) % QString(" ") % ap->city();
    else
        from = tr("(unknown)");
        
    QString to;
    ap = data()->destination();
    
    if (ap)
        to = QString(ap->icao()) % QString(" ") % ap->city();
    else
        to = tr("(unknown)");
        
    QString gs = tr("Ground speed: %1 kts").arg(QString::number(data()->groundSpeed()));
    QString alt = tr("Altitude: %1 ft").arg(QString::number(data()->altitude()));
    
    return QString("<p style='white-space:nowrap'><center>"
                   % callsign % "<br />"
                   % desc % "<br />"
                   % from % " > " % to % "<br />"
                   % gs % "<br />"
                   % alt
                   % "</center></p>");
}

void
FlightItem::showDetails() const
{
    vApp()->userInterface()->showDetails(data());
}
