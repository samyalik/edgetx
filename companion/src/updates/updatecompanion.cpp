/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "updatecompanion.h"
#include "appdata.h"

#include <QMessageBox>
#include <QDesktopServices>
#include <QDirIterator>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>

#if defined(__APPLE__)
  #define OS_SUPPORTED_INSTALLER
  #define OS_FILEPATTERN           "edgetx-cpn-osx"
  #define OS_INSTALLER_EXTN        "*.dmg"
  #define OS_INSTALL_QUESTION      tr("Would you like to open the disk image to install the new version of Companion?")
#elif defined(_WIN64)
  #define OS_SUPPORTED_INSTALLER
  #define OS_FILEPATTERN           "edgetx-cpn-win64"
  #define OS_INSTALLER_EXTN        "*.exe"
  #define OS_INSTALL_QUESTION      tr("Would you like to launch the Companion installer?")
#elif defined(_WIN32)
  #define OS_SUPPORTED_INSTALLER
  #define OS_FILEPATTERN           "edgetx-cpn-win32"
  #define OS_INSTALLER_EXTN        "*.exe"
  #define OS_INSTALL_QUESTION      tr("Would you like to launch the Companion installer?")
#elif defined(__linux__) || defined(__linux)
  #define OS_SUPPORTED_INSTALLER
  #define OS_FILEPATTERN           "edgetx-cpn-linux"
  #define OS_INSTALLER_EXTN        "*.AppImage"
#else
  #define OS_INSTALL_MSG           tr("No install process support for your operating system")
#endif

UpdateCompanion::UpdateCompanion(QWidget * parent) :
  UpdateInterface(parent)
{
  setName("Companion");
  setRepo(QString(GH_REPOS_EDGETX).append("/edgetx"));
  setReleasesNightlyName("nightly");

  dfltParams->data.flags = (UPDFLG_Common | UPDFLG_Locked | UPDFLG_AsyncInstall) & ~UPDFLG_CopyDest;

  UpdateParameters::AssetParams &ap = dfltParams->addAsset();
  ap.filterType = UpdateParameters::UFT_Startswith;
  ap.filter = QString("%1").arg(QString(OS_FILEPATTERN));
  ap.maxExpected = 1;
  ap.flags = dfltParams->data.flags;
}

bool UpdateCompanion::asyncInstall()
{
#ifdef OS_SUPPORTED_INSTALLER
  //reportProgress(tr("Run application installer: %1").arg(g.runAppInstaller() ? tr("true") : tr("false")), QtDebugMsg);

  if (!g.runAppInstaller())
    return true;

  progressMessage(tr("Install"));

  assets->setFilterFlags(UPDFLG_AsyncInstall);
  //reportProgress(tr("Asset filter applied: %1 Assets found: %2").arg(updateFlagsToString(UPDFLG_AsyncInstall)).arg(assets->count()), QtDebugMsg);

  if (assets->count() != 1) {
    reportProgress(tr("Expected 1 asset for install but none found"), QtCriticalMsg);
    return false;
  }

  assets->getSetId(0);

  QString installerPath = decompressDir % QString("/A%1/%2").arg(assets->id()).arg(QFileInfo(assets->filename()).completeBaseName());

  QDirIterator it(installerPath, { OS_INSTALLER_EXTN }, QDir::Files, QDirIterator::Subdirectories);

  bool found = false;

  while (it.hasNext())
  {
    installerPath = it.next();
    found = true;
    break;
  }

  if (!found) {
    reportProgress(tr("Installer not found in %1 using filter %2").arg(installerPath).arg(OS_INSTALLER_EXTN), QtCriticalMsg);
    return false;
  }

#if defined(__linux__) || defined(__linux)
  QFile f(installerPath);
  if (!f.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner)) {
    QMessageBox::critical(parentWidget(), CPN_STR_APP_NAME, tr("Unable to set file permissions on the AppImage"));
    return false;
  }

  QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                           QString("%1/%2").arg(QDir::currentPath()).arg(QFileInfo(f).fileName()), tr("Images (*.AppImage)"));
  if (fileName.isEmpty())
    return true;

  QFile dest(fileName);

  if (dest.exists()) {
    if (!dest.remove()) {
      QMessageBox::critical(parentWidget(), CPN_STR_APP_NAME, tr("Unable to delete file %1").arg(fileName));
      return false;
    }
  }

  if (!QFile::copy(installerPath, fileName)) {
    QMessageBox::critical(parentWidget(), CPN_STR_APP_NAME, tr("Unable to copy file to %1").arg(fileName));
    return false;
  }

  if (QMessageBox::question(parentWidget(), CPN_STR_APP_NAME, "Restart Companion?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
    QProcess::startDetached(fileName);
    qApp->exit();
  }

#else

  int ret = QMessageBox::question(parentWidget(), CPN_STR_APP_NAME, OS_INSTALL_QUESTION, QMessageBox::Yes | QMessageBox::No);

  if (ret == QMessageBox::Yes) {
    if (QDesktopServices::openUrl(QUrl::fromLocalFile(installerPath)))
      qApp->exit();
  }

#endif  //  defined(__linux__) || defined(__linux)

  return true;

#else
  QMessageBox::warning(parentWidget(), CPN_STR_APP_NAME, OS_INSTALL_MSG);
  return true;
#endif  //  OS_SUPPORTED_INSTALLER
}

const QString UpdateCompanion::currentRelease()
{
#if defined(VERSION_TAG)
  return UpdateInterface::latestRelease();
#else
  dfltParams->data.currentRelease = QString("EdgeTX v%1.%2 (Custom)").arg(VERSION_MAJOR).arg(VERSION_MINOR);
  runParams->data.currentRelease = dfltParams->data.currentRelease;
  return dfltParams->data.currentRelease;
#endif
}

const QString UpdateCompanion::currentVersion()
{
#if defined(VERSION_TAG)
  return UpdateInterface::latestRelease();
#else
  return QString(QString("v%1.%2").arg(VERSION_MAJOR).arg(VERSION_MINOR));
#endif
}

const bool UpdateCompanion::isLatestRelease()
{
#if defined(VERSION_TAG)
  return UpdateInterface::isLatestRelease();
#else
  return true;
#endif
}

const bool UpdateCompanion::isUpdateAvailable()
{
#if defined(VERSION_TAG)
  return UpdateInterface::isUpdateAvailable();
#else
  return false;
#endif
}
