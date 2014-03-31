/*
 * lonlat.cpp
 * Copyright (C) 2014  Michał Garapich <michal@garapich.pl>
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

#include <QtGlobal>

#include "lonlat.h"
#include "defines.h"

LonLat::LonLat() : QPointF() {}

LonLat::LonLat(const QPoint& _p): QPointF(_p) {}

LonLat::LonLat(qreal _lon, qreal _lat): QPointF(_lon, _lat) {
//   Q_ASSERT(_lon <= 180.0 && _lon >= 0.0);
//   Q_ASSERT(_lat > -90.0 && _lat <= 90.0);
}
