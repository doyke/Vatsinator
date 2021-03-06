/*
 * map.cpp
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

#include "map.h"
#include "gui/airportitem.h"
#include "gui/firitem.h"
#include "gui/flightitem.h"
#include "gui/mapdrawerplugin.h"
#include "gui/modelmatcher.h"
#include "core/resourcefile.h"
#include "core/pluginfinder.h"
#include <QtQuick>
#include <QtGui>

using namespace Vatsinator::Core;
using namespace Vatsinator::Gui;

namespace Vatsinator { namespace Imports {

Map::Map(QQuickItem* parent) :
    QQuickPaintedItem(parent),
    m_renderer(new MapRenderer(this)),
    m_scene(new MapScene(this))
{
    connect(m_renderer, &MapRenderer::centerChanged, this, &Map::centerChanged);
    connect(m_renderer, &MapRenderer::zoomChanged, this, &Map::zoomChanged);
    
    setFlag(QQuickItem::ItemHasContents);
    setFillColor(Qt::white);
    setOpaquePainting(true);
    
#if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 1))
    /* https://bugreports.qt.io/browse/QTBUG-52901 */
    setRenderTarget(FramebufferObject);
#endif
    
    connect(m_renderer, &MapRenderer::updated, this, &QQuickItem::update);
    
    m_renderer->setScene(m_scene);
    
    // TODO Selected plugin from options
    MapDrawerPlugin* mapPlugin = nullptr;
    auto plugins = PluginFinder::pluginsForIid(qobject_interface_iid<MapDrawerPlugin*>());
    if (plugins.length() > 0)
        mapPlugin = qobject_cast<MapDrawerPlugin*>(PluginFinder::plugin(plugins.first()));
    
    if (mapPlugin)
        m_renderer->setMapDrawer(mapPlugin->create(m_renderer));
    
    connect(qApp, &QGuiApplication::aboutToQuit, this, &Map::saveMapState);
    connect(qApp, &QGuiApplication::applicationStateChanged, this, &Map::handleAppStateChanged);
    restoreMapState();
    
    connect(this, &QQuickItem::windowChanged, this, &Map::updateViewport);
}

Map::~Map() {}

void Map::paint(QPainter* painter)
{
    // TODO Optimize, use Scene Graph maybe?
    QSize size(width(), height());
    QImage img(size * qApp->primaryScreen()->devicePixelRatio(), QImage::Format_ARGB32_Premultiplied);
    img.setDevicePixelRatio(qApp->primaryScreen()->devicePixelRatio());
    m_renderer->paint(&img);

    painter->drawImage(QRect(QPoint(0, 0), size), img);

//    QPaintDevice* device = painter->device();
//    painter->end();
//    m_renderer->paint(device);
//    painter->begin(device);
}

void Map::setServerTracker(ServerTracker* serverTracker)
{
    m_serverTracker = serverTracker;
    m_scene->track(serverTracker);
    emit serverTrackerChanged(m_serverTracker);
}

void Map::setModelMatcher(ModelMatcher *modelMatcher)
{
    m_scene->setModelMatcher(modelMatcher);
    emit modelMatcherChanged(modelMatcher);
}

void Map::setControls(QQuickItem* controls)
{
    if (m_controls)
        disconnect(m_controls);
    
    bool c = connect(controls, SIGNAL(zoomUpdated(qreal)), this, SLOT(updateZoom(qreal)));
    if (!c)
        qWarning("Could not connect to %s::zoomUpdated(qreal)", controls->metaObject()->className());
    
    c = connect(controls, SIGNAL(positionUpdated(int, int)), this, SLOT(updatePosition(int,int)));
    if (!c)
        qWarning("Could not connect to %s::positionUpdated(int, int)", controls->metaObject()->className());
    
    c = connect(controls, SIGNAL(clicked(int, int)), this, SLOT(handleClicked(int,int)));
    if (!c)
        qWarning("Could not connect to %s::clicked(int, int)", controls->metaObject()->className());
    
    m_controls = controls;
    emit controlsChanged(m_controls);
}

void Map::updateZoom(qreal factor)
{
    m_renderer->setZoom(m_renderer->zoom() * factor);
}

void Map::updatePosition(int x, int y)
{
    WorldTransform t = m_renderer->transform();
    QPoint center = t.map(m_renderer->center());
    QPoint diff(x, y);
    center -= diff;
    
    LonLat llcenter = t.map(center).bound();
    m_renderer->setCenter(llcenter);
}

MapItem* Map::underPoint(int x, int y)
{
    Q_ASSERT(m_renderer);
    
    QPoint q(x, y);
    LonLat pos = m_renderer->transform().map(q);
    MapItem* item = m_renderer->scene()->nearest(pos.bound());
    if (item) {
        QSize size = item->size();
        int m = qApp->primaryScreen()->devicePixelRatio();
        size = size.expandedTo(QSize(m, m));
        QPoint p = renderer()->transform().map(item->position());
        QRect r(p.x() - size.width() / 2, p.y() - size.height() / 2, size.width(), size.height());
        return r.contains(q) ? item : nullptr;
    } else {
        return nullptr;
    }
}

void Map::handleClicked(int x, int y)
{
    MapItem* clicked = underPoint(x, y);
    if (FlightItem* flightItem = qobject_cast<FlightItem*>(clicked)) {
        emit flightSelected(QVariant::fromValue<FlightItem*>(flightItem));
    } else if (AirportItem* airportItem = qobject_cast<AirportItem*>(clicked)) {
        emit airportSelected(QVariant::fromValue<AirportItem*>(airportItem));
    } else if (FirItem* firItem = qobject_cast<FirItem*>(clicked)) {
        emit firSelected(QVariant::fromValue<FirItem*>(firItem));
    }
}

void Map::saveMapState()
{
    QSettings s;
    s.setValue("map/center", this->center());
    s.setValue("map/zoom", this->zoom());
}

void Map::restoreMapState()
{
    QSettings s;
    QPointF center = s.value("map/center").toPointF();
    setCenter(center);
    qreal zoom = s.value("map/zoom", 1.0).toReal();
    setZoom(zoom);
}

void Map::handleAppStateChanged(Qt::ApplicationState state)
{
    if (state == Qt::ApplicationActive)
        restoreMapState();
    else
        saveMapState();
}

void Map::updateViewport(QQuickWindow* window)
{
    m_renderer->setViewport(window->size());
}

}} /* namespace Vatsinator::Imports */
