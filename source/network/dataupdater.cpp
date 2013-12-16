/*
 * dataupdater.cpp
 * Copyright (C) 2013  Michał Garapich <michal@garapich.pl>
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
#include <QtNetwork>

#include "network/filedownloader.h"

#include "storage/filemanager.h"
#include "storage/unzipper.h"

#include "netconfig.h"
#include "vatsinatorapplication.h"

#include "dataupdater.h"
#include "defines.h"

static const QString PackageUrl = QString(NetConfig::Vatsinator::repoUrl()) % QString("packages/latest.zip");
static const QString ManifestUrl = QString(NetConfig::Vatsinator::repoUrl()) % QString("packages/Manifest");

namespace {
  
  /**
   * Moves the file.
   */
  bool moveFile(const QString& _oldLocation, const QString& _newLocation) {
    QFile file(_oldLocation);
    if (!file.open(QIODevice::ReadWrite)) {
      VatsinatorApplication::log("DataUpdater: failed accessing file %s", qPrintable(_oldLocation));
      return false;
    }
    
    file.close();
    
    QFile oldFile(_newLocation);
    if (oldFile.exists())
      oldFile.remove();
    
    QFileInfo fileInfo = QFileInfo(_newLocation);
    if (!QDir(fileInfo.path()).exists())
      QDir().mkpath(fileInfo.path());
    
    bool result = file.rename(_newLocation);
    if (result)
      VatsinatorApplication::log("DataUpdater: moved file %s -> %s", qPrintable(_oldLocation), qPrintable(_newLocation));
    else
      VatsinatorApplication::log("DataUpdater: failed moving %s -> %s", qPrintable(_oldLocation), qPrintable(_newLocation));
    
    return result;
  }
  
  /**
   * Removes the dir recursively.
   * http://stackoverflow.com/questions/11050977/removing-a-non-empty-folder-in-qt
   */
  bool removeDir(const QString& _dirName) {
    bool result = true;
    QDir dir(_dirName);
    
    if (dir.exists()) {
      for (QFileInfo file: dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
        if (file.isDir()) {
          VatsinatorApplication::log("DataUpdater: removing dir %s", qPrintable(file.absoluteFilePath()));
          result = removeDir(file.absoluteFilePath());
        } else {
          VatsinatorApplication::log("DataUpdater: removing file %s", qPrintable(file.absoluteFilePath()));
          result = QFile::remove(file.absoluteFilePath());
        }
        
        if (!result)
          return result;
      }
      
      result = dir.rmdir(_dirName);
    }
    
    return result;
  }
  
  bool checksumMatches(const QString& _fileName, const QByteArray& _md5) {
    QFile file(_fileName);
    if (!file.open(QIODevice::ReadOnly))
      return false;
    
    auto hash = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5).toHex();
    file.close();
    
    return hash == _md5;
  }
  
}

DataUpdater::DataUpdater(QObject* _parent) :
    QObject(_parent),
    __unzipper(new Unzipper()) {
  
  /* Give the Unzipper its own thread */
  QThread* thread = new QThread();
  __unzipper->moveToThread(thread);
  connect(__unzipper,   SIGNAL(unzipped()),
          this,         SLOT(__filesUnzipped()));
  connect(__unzipper,   SIGNAL(error(QString)),
          this,         SLOT(__unzipError(QString)));
  connect(this,         SIGNAL(readyToUnzip()),
          __unzipper,   SLOT(unzip()));
  thread->start();
}

DataUpdater::~DataUpdater() {
  QThread* thread = __unzipper->thread();
  __unzipper->deleteLater();
  thread->quit();
  thread->wait();
  delete thread;
}

void
DataUpdater::update() {
  VatsinatorApplication::log("DataUpdater: updating...");
  
  FileDownloader* fd = new FileDownloader();
  
  connect(fd,   SIGNAL(finished(QString)),
          this, SLOT(__unzipPackage(QString)));
  connect(fd,   SIGNAL(finished(QString)),
          fd,   SLOT(deleteLater()));
  connect(fd,   SIGNAL(error(QString)),
          this, SLOT(__fetchError(QString)));
  connect(fd,   SIGNAL(error(QString)),
          fd,   SLOT(deleteLater()));
  
  fd->fetch(QUrl(PackageUrl));
}

bool
DataUpdater::__checksumsOk(const QString& _fileName) {
  QFile file(_fileName);
  
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    VatsinatorApplication::log("DataUpdater: cannot open %s for reading!", qPrintable(_fileName));
    return false;
  }
  
  QHash<QString, QString> md5sums;
  
  while (!file.atEnd()) {
    QString line = QString::fromUtf8(file.readLine()).simplified();
    
    if (line.isEmpty())
      continue;
    
    if (QRegExp("\\d{8}").exactMatch(line)) { // manifest date
      VatsinatorApplication::log("DataUpdater: manifest date: %s", qPrintable(line));
      continue;
    }
    
    QStringList lineSplitted = line.split(QRegExp("\\s+"));
    Q_ASSERT(lineSplitted.length() == 2);
    
    lineSplitted[1].replace(QString("./"), QString(""));
    
    // insert filename <-> md5sum pair
    md5sums.insert(lineSplitted[1], lineSplitted[0]);
  }
  
  file.close();
  
  for (const QString& f: __unzipper->fileList()) {
    if (!md5sums.contains(f)) {
      VatsinatorApplication::log("DataUpdater: could not find the md5 sum for file %s!", qPrintable(f));
      return false;
    }
    
    if (!checksumMatches(__unzipper->targetDir() + f, md5sums[f].toUtf8())) {
      VatsinatorApplication::log("DataUpdater: checksum failed for %s", qPrintable(f));
      return false;
    }
  }
  
  return true;
}

bool
DataUpdater::__moveFiles() {
  for (const QString& file: __unzipper->fileList()) {
    if (!moveFile(__unzipper->targetDir() % file, FileManager::localDataPath() % file))
      return false;
  }
  
  return true;
}

void
DataUpdater::__cleanup() {
  removeDir(__unzipper->targetDir());
  
  if (QFile(__unzipper->fileName()).exists())
    QFile::remove(__unzipper->fileName());
}

void
DataUpdater::__unzipPackage(QString _fileName) {
  __unzipper->setFileName(_fileName);
  emit readyToUnzip();
}

void
DataUpdater::__fetchError(QString _error) {
  VatsinatorApplication::log("DataUpdater: fetch error: %s", qPrintable(_error));
  emit failed();
}

void
DataUpdater::__filesUnzipped() {
  VatsinatorApplication::log("DataUpdater: files unzipped.");
  
  FileDownloader* fd = new FileDownloader();
  
  connect(fd,   SIGNAL(finished(QString)),
          this, SLOT(__checkManifest(QString)));
  connect(fd,   SIGNAL(finished(QString)),
          fd,   SLOT(deleteLater()));
  connect(fd,   SIGNAL(error(QString)),
          this, SLOT(__fetchError(QString)));
  connect(fd,   SIGNAL(error(QString)),
          fd,   SLOT(deleteLater()));
  
  fd->fetch(QUrl(ManifestUrl));
}

void
DataUpdater::__unzipError(QString _errStr) {
  VatsinatorApplication::log("DataUpdater: unzip error: %s", qPrintable(_errStr));
  emit failed();
}

void
DataUpdater::__checkManifest(QString _fileName) {
  if (!__checksumsOk(_fileName)) {
    __cleanup();
    emit failed();
    return;
  }
  
  if (!__moveFiles()) {
    __cleanup();
    emit failed();
    return;
  }
  
  moveFile(_fileName, FileManager::localDataPath() % "Manifest");
  __cleanup();
  emit updated();
}