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
#ifndef _VIRTUALFS_H_
#define _VIRTUALFS_H_

#include <string>

#include "littlefs_v2.4.1/lfs.h"
#include "sdcard.h"

#include "translations.h"

#define FILE_COPY_PREFIX "cp_"

#define PATH_SEPARATOR      "/"
#define ROOT_PATH           PATH_SEPARATOR
#define MODELS_PATH         ROOT_PATH "MODELS"      // no trailing slash = important
#define RADIO_PATH          ROOT_PATH "RADIO"       // no trailing slash = important
#define LOGS_PATH           ROOT_PATH "LOGS"
#define SCREENSHOTS_PATH    ROOT_PATH "SCREENSHOTS"
#define SOUNDS_PATH         ROOT_PATH "SOUNDS/en"
#define SOUNDS_PATH_LNG_OFS (sizeof(SOUNDS_PATH)-3)
#define SYSTEM_SUBDIR       "SYSTEM"
#define BITMAPS_PATH        ROOT_PATH "IMAGES"
#define FIRMWARES_PATH      ROOT_PATH "FIRMWARE"
#define AUTOUPDATE_FILENAME FIRMWARES_PATH PATH_SEPARATOR "autoupdate.frsk"
#define EEPROMS_PATH        ROOT_PATH "EEPROM"
#define BACKUP_PATH         ROOT_PATH "BACKUP"
#define SCRIPTS_PATH        ROOT_PATH "SCRIPTS"
#define WIZARD_PATH         SCRIPTS_PATH PATH_SEPARATOR "WIZARD"
#define THEMES_PATH         ROOT_PATH "THEMES"
#define LAYOUTS_PATH        ROOT_PATH "LAYOUTS"
#define WIDGETS_PATH        ROOT_PATH "WIDGETS"
#define WIZARD_NAME         "wizard.lua"
#define SCRIPTS_MIXES_PATH  SCRIPTS_PATH PATH_SEPARATOR "MIXES"
#define SCRIPTS_FUNCS_PATH  SCRIPTS_PATH PATH_SEPARATOR "FUNCTIONS"
#define SCRIPTS_TELEM_PATH  SCRIPTS_PATH PATH_SEPARATOR "TELEMETRY"
#define SCRIPTS_TOOLS_PATH  SCRIPTS_PATH PATH_SEPARATOR "TOOLS"

#define LEN_FILE_PATH_MAX   (sizeof(SCRIPTS_TELEM_PATH)+1)  // longest + "/"
#if 0
#if defined(SDCARD_YAML) || defined(SDCARD_RAW)
#define RADIO_FILENAME      "radio.bin"
const char RADIO_MODELSLIST_PATH[] = RADIO_PATH PATH_SEPARATOR "models.txt";
const char RADIO_SETTINGS_PATH[] = RADIO_PATH PATH_SEPARATOR RADIO_FILENAME;
#if defined(SDCARD_YAML)
const char MODELSLIST_YAML_PATH[] = MODELS_PATH PATH_SEPARATOR "models.yml";
const char FALLBACK_MODELSLIST_YAML_PATH[] = RADIO_PATH PATH_SEPARATOR "models.yml";
const char RADIO_SETTINGS_YAML_PATH[] = RADIO_PATH PATH_SEPARATOR "radio.yml";
#endif
#define    SPLASH_FILE             "splash.png"
#endif

#define MODELS_EXT          ".bin"
#define LOGS_EXT            ".csv"
#define SOUNDS_EXT          ".wav"
#define BMP_EXT             ".bmp"
#define PNG_EXT             ".png"
#define JPG_EXT             ".jpg"
#define SCRIPT_EXT          ".lua"
#define SCRIPT_BIN_EXT      ".luac"
#define TEXT_EXT            ".txt"
#define FIRMWARE_EXT        ".bin"
#define EEPROM_EXT          ".bin"
#define SPORT_FIRMWARE_EXT  ".frk"
#define FRSKY_FIRMWARE_EXT  ".frsk"
#define MULTI_FIRMWARE_EXT  ".bin"
#define ELRS_FIRMWARE_EXT   ".elrs"
#define YAML_EXT            ".yml"

#if defined(COLORLCD)
#define BITMAPS_EXT         BMP_EXT JPG_EXT PNG_EXT
#define LEN_BITMAPS_EXT     4
#else
#define BITMAPS_EXT         BMP_EXT
#endif

#ifdef LUA_COMPILER
  #define SCRIPTS_EXT         SCRIPT_BIN_EXT SCRIPT_EXT
#else
  #define SCRIPTS_EXT         SCRIPT_EXT
#endif

#define GET_FILENAME(filename, path, var, ext) \
  char filename[sizeof(path) + sizeof(var) + sizeof(ext)]; \
  memcpy(filename, path, sizeof(path) - 1); \
  filename[sizeof(path) - 1] = '/'; \
  memcpy(&filename[sizeof(path)], var, sizeof(var)); \
  filename[sizeof(path)+sizeof(var)] = '\0'; \
  strcat(&filename[sizeof(path)], ext)
#endif


class VirtualFS;

struct VfsDir
{
public:
  VfsDir(){}
  ~VfsDir(){}

private:
  friend class VirtualFS;
  VfsDir(const VfsDir&);

  enum DirType {DIR_UNKNOWN, DIR_ROOT, DIR_FAT, DIR_LFS};

  void clear()
  {
    type = DIR_UNKNOWN;
    lfsDir = {0};
    fatDir = {0};

    readIdx = 0;
    firstTime = true;
  }

  DirType type = DIR_UNKNOWN;
  lfs_dir_t lfsDir = {0};
  DIR fatDir = {0};

  size_t readIdx = 0;
  bool firstTime = true;
};

enum VfsType {VFS_TYPE_UNKNOWN, VFS_TYPE_DIR, VFS_TYPE_FILE};
enum class VfsFileType {UNKNOWN, ROOT, FAT, LFS};

struct VfsFileInfo
{
public:
  VfsFileInfo(){}
  ~VfsFileInfo(){}

  std::string getName()
  {
    switch(type)
    {
    case VfsFileType::ROOT: return name;
    case VfsFileType::FAT:  return(!name.empty())?name:fatInfo.fname;
    case VfsFileType::LFS:  return lfsInfo.name;
    }
    return "";
  };

  VfsType getType()
  {
    switch(type)
    {
    case VfsFileType::ROOT:
      return VFS_TYPE_DIR;

    case VfsFileType::FAT:
      if (!name.empty())
        return VFS_TYPE_DIR;
      if(fatInfo.fattrib & AM_DIR)
        return VFS_TYPE_DIR;
      else
        return VFS_TYPE_FILE;
    case VfsFileType::LFS:

      if(lfsInfo.type == LFS_TYPE_DIR)
        return VFS_TYPE_DIR;
      else
        return VFS_TYPE_FILE;
    }
    return VFS_TYPE_UNKNOWN;
  };

private:
  friend class VirtualFS;
  VfsFileInfo(const VfsFileInfo&);

  void clear() {
    type = VfsFileType::UNKNOWN;
    lfsInfo = {0};
    fatInfo = {0};
    name.clear();
  }

  VfsFileType type = VfsFileType::UNKNOWN;
  lfs_info lfsInfo = {0};
  FILINFO fatInfo = {0};

  std::string name;
};

struct VfsFile
{
public:
  VfsFile(){}
  ~VfsFile(){}

private:
  friend class VirtualFS;
  VfsFile(const VfsFile&);


  void clear() {
    type = VfsFileType::UNKNOWN;
    lfs = {0};
    fat = {0};
  }

  VfsFileType type = VfsFileType::UNKNOWN;
  lfs_file lfs = {0};
  FIL fat = {0};
};

class VirtualFS
{
public:
  VirtualFS();
  ~VirtualFS();

  static VirtualFS& instance()
  {
    if( _instance == nullptr)
      _instance = new VirtualFS();
    return *_instance;
  }

  bool format();
  const char * checkAndCreateDirectory(const char * path);
  bool isFileAvailable(const char * path, bool exclDir);
  bool isFilePatternAvailable(const char * path, const char * file, const char * pattern = nullptr, bool exclDir = true, char * match = nullptr);
  char* getFileIndex(char * filename, unsigned int & value);

  const std::string& getCurWorkDir()  { return curWorkDir;}
  int changeDirectory(const std::string& path);
  int openDirectory(VfsDir& dir, const char * path);
  int readDirectory(VfsDir& dir, VfsFileInfo& info, bool firstTime = false);
  int closeDirectory(VfsDir& dir);

  int openFile(VfsFile& file, const std::string& path, int flags);
  int closeFile(VfsFile& file);
  int read(VfsFile& file, void* buf, size_t size, size_t& readSize);
  int write(VfsFile& file, void* buf, size_t size, size_t& written);
  int fileEof(VfsFile& file);

  int rename(const char* oldPath, const char* newPath);
  int copyFile(const std::string& srcFile, const std::string& srcDir,
             const std::string& targetDir, const std::string& targetFile);


  uint32_t flashGetNoSectors();
  uint32_t flashGetSize();
  uint32_t flashGetFreeSectors();

  //#if !defined(BOOT)
  //inline const char * SDCARD_ERROR(FRESULT result)
  //{
  //  if (result == FR_NOT_READY)
  //    return STR_NO_SDCARD;
  //  else
  //    return STR_SDCARD_ERROR;
  //}
  //#endif

  // NOTE: 'size' must = 0 or be a valid character position within 'filename' array -- it is NOT validated
  const char * flashGetBasename(const char * path);

  #if defined(PCBX12S)
    #define OTX_FOURCC 0x3478746F // otx for X12S
  #elif defined(RADIO_T16)
    #define OTX_FOURCC 0x3F78746F // otx for Jumper T16
  #elif defined(RADIO_T18)
    #define OTX_FOURCC 0x4078746F // otx for Jumper T18
  #elif defined(RADIO_TX16S)
    #define OTX_FOURCC 0x3878746F // otx for Radiomaster TX16S
  #elif defined(PCBX10)
    #define OTX_FOURCC 0x3778746F // otx for X10
  #elif defined(PCBX9E)
    #define OTX_FOURCC 0x3578746F // otx for Taranis X9E
  #elif defined(PCBXLITES)
    #define OTX_FOURCC 0x3B78746F // otx for Taranis X-Lite S
  #elif defined(PCBXLITE)
    #define OTX_FOURCC 0x3978746F // otx for Taranis X-Lite
  #elif defined(RADIO_T12)
    #define OTX_FOURCC 0x3D78746F // otx for Jumper T12
  #elif defined(RADIO_TLITE)
    #define OTX_FOURCC 0x4278746F // otx for Jumper TLite
  #elif defined(RADIO_TPRO)
    #define OTX_FOURCC 0x4678746F // otx for Jumper TPro
  #elif defined(RADIO_TX12)
    #define OTX_FOURCC 0x4178746F // otx for Radiomaster TX12
  #elif defined(RADIO_ZORRO)
    #define OTX_FOURCC 0x4778746F // otx for Radiomaster Zorro
  #elif defined(RADIO_T8)
    #define OTX_FOURCC 0x4378746F // otx for Radiomaster T8
  #elif defined(PCBX7)
    #define OTX_FOURCC 0x3678746F // otx for Taranis X7 / X7S / X7 Express / X7S Express
  #elif defined(PCBX9LITES)
    #define OTX_FOURCC 0x3E78746F // otx for Taranis X9-Lite S
  #elif defined(PCBX9LITE)
    #define OTX_FOURCC 0x3C78746F // otx for Taranis X9-Lite
  #elif defined(PCBX9D) || defined(PCBX9DP)
    #define OTX_FOURCC 0x3378746F // otx for Taranis X9D
  #elif defined(PCBNV14)
    #define OTX_FOURCC 0x3A78746F // otx for NV14
  #elif defined(PCBSKY9X)
    #define OTX_FOURCC 0x3278746F // otx for sky9x
  #endif

  unsigned int findNextFileIndex(char * filename, uint8_t size, const char * directory);

  const char * sdCopyFile(const char * src, const char * dest);
  const char * flashCopyFile(const char * srcFilename, const char * srcDir, const char * destFilename, const char * destDir);

  #define LIST_NONE_SD_FILE   1
  #define LIST_SD_FILE_EXT    2
  bool flashListFiles(const char * path, const char * extension, const uint8_t maxlen, const char * selection, uint8_t flags=0);
private:
  static VirtualFS* _instance;

  lfs_config lfsCfg = {0};
  lfs_t lfs = {0};
  std::string curWorkDir = "/";

  void normalizePath(std::string &path);

  VfsDir::DirType getDirTypeAndPath(std::string& path);
};

#endif // _VIRTUALFS_H_
