/*
    firdetailsaction.h
    Copyright (C) 2012  Michał Garapich michal@garapich.pl

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


#ifndef FIRDETAILSACTION_H
#define FIRDETAILSACTION_H

#include <QAction>

#include "vatsimdata/fir.h"

class FirDetailsAction : public QAction {
  Q_OBJECT

signals:
  void triggered(const Fir*);

public:
  FirDetailsAction(const Fir*, const QString&, QObject*);

private slots:
  void __handleTriggered();

private:
  const Fir* __current;

};

#endif // FIRDETAILSACTION_H
