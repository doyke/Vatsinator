/*
    worldmap.h
    Copyright (C) 2012-2013  Michał Garapich michal@garapich.pl

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


#ifndef WORLDMAP_H
#define WORLDMAP_H

#include <QCoreApplication>
#include <QVector>

#include "db/point.h"

#include "singleton.h"

class VertexBufferObject;

class WorldMap : public Singleton<WorldMap> {

  Q_DECLARE_TR_FUNCTIONS(WorldMap);

  struct Polygon {
    QVector<Point>          borders;
    QVector<unsigned short> triangles;
  };
  
public:
  WorldMap();
  
  const QVector<Point> &
  borders() const { return __worldPolygon.borders; }
  
  const QVector<unsigned short> &
  triangles() const { return __worldPolygon.triangles; }

private:
  void __readDatabase();

  Polygon __worldPolygon;


};

#endif // WORLDMAP_H
