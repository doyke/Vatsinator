/*
    glerrors.h
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


#ifndef GLERRORS_H
#define GLERRORS_H

#include <QMap>
#include <QString>
#include <QtOpenGL>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define HERE __FILE__ " : " TOSTRING(__LINE__)

#ifndef NO_DEBUG
# define checkGLErrors(WHERE) checkGLErrorsFunc(WHERE)
# define registerGPUMemoryAlloc(HOWMUCH) registerGPUMemoryAllocFunc(HOWMUCH)
# define unregisterGPUMemoryAlloc(HOWMUCH) unregisterGPUMemoryAllocFunc(HOWMUCH)

void checkGLErrorsFunc(const QString&);
void registerGPUMemoryAllocFunc(long unsigned);
void unregisterGPUMemoryAllocFunc(long unsigned);
void registerExtensionPointer(const QString&, long long unsigned);

extern QString        glErrors;
extern long unsigned  gpuMemoryUsage;

extern QMap<QString, long long unsigned> extensions;

#else
# define checkGLErrors(WHERE)
# define registerGPUMemoryAlloc(HOWMUCH)
# define unregisterGPUMemoryAlloc(HOWMUCH)
#endif

#endif // GLERRORS_H
