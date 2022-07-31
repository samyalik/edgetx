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

#if defined (SPI_FLASH)
#if defined (USE_LITTLEFS)
#include "littlefs_v2.4.1/lfs.h"
#else
#include "tjftl/tjftl.h"
#endif
#endif
#if defined (SDCARD)
#include "sdcard.h"
#endif

#include "translations.h"

#define FILE_COPY_PREFIX "cp_"

#define PATH_SEPARATOR      "/"
#define ROOT_PATH           PATH_SEPARATOR "DEFAULT" PATH_SEPARATOR
#define MODELS_PATH         ROOT_PATH "MODELS"      // no trailing slash = important
#define RADIO_PATH          ROOT_PATH "RADIO"       // no trailing slash = important
#define TEMPLATES_PATH      ROOT_PATH "TEMPLATES"
#define PERS_TEMPL_PATH     TEMPLATES_PATH "/PERSONAL"
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



class VirtualFS;

enum class VfsType { UNKOWN, DIR, FILE };
enum class VfsFileType { UNKNOWN, ROOT, FAT, LFS };

enum class VfsError {
    OK          = 0,    // No error
    IO          = -5,   // Error during device operation
    CORRUPT     = -84,  // Corrupted
    NOENT       = -2,   // No directory entry
    EXIST       = -17,  // Entry already exists
    NOTDIR      = -20,  // Entry is not a dir
    ISDIR       = -21,  // Entry is a dir
    NOTEMPTY    = -39,  // Dir is not empty
    BADF        = -9,   // Bad file number
    FBIG        = -27,  // File too large
    INVAL       = -22,  // Invalid parameter
    NOSPC       = -28,  // No space left on device
    NOMEM       = -12,  // No more memory available
    NOATTR      = -61,  // No data/attr available
    NAMETOOLONG = -36,  // File name too long
    NOT_READY   = -99   // storage not ready
};

#if !defined(BOOT)
inline const char * STORAGE_ERROR(VfsError result)
{
  if (result == VfsError::NOT_READY)
    return STR_NO_SDCARD;
  else
    return STR_SDCARD_ERROR;
}
#endif

enum class VfsOpenFlags {
  NONE          = 0x00,
  READ          = 0x01,
  WRITE         = 0x02,
  OPEN_EXISTING = 0x00,
  CREATE_NEW    = 0x04,
  CREATE_ALWAYS = 0x08,
  OPEN_ALWAYS   = 0x10,
  OPEN_APPEND   = 0x30,
};

VfsOpenFlags operator|(VfsOpenFlags lhs,VfsOpenFlags rhs);
VfsOpenFlags operator&(VfsOpenFlags lhs,VfsOpenFlags rhs);

//for compatibility reasons those are identical to the FAT implementation
enum class VfsFileAttributes {
  NONE = 0x00,
  RDO = 0x01,
  HID = 0x02,
  SYS = 0x04,
  DIR = 0x10,
  ARC = 0x20
};

VfsFileAttributes operator|(VfsFileAttributes lhs,VfsFileAttributes rhs);
VfsFileAttributes operator&(VfsFileAttributes lhs,VfsFileAttributes rhs);

struct VfsDir;

struct VfsFileInfo
{
public:
  VfsFileInfo(){ clear(); }

  std::string getName() const
  {
    switch(type)
    {
    case VfsFileType::ROOT: return name;
#if defined (SDCARD) || (!defined(USE_LITTLEFS) && defined(SPI_FLASH))
    case VfsFileType::FAT:  return(!name.empty())?name:fatInfo.fname;
#endif
#if defined(USE_LITTLEFS)
    case VfsFileType::LFS:  return lfsInfo.name;
#endif
    }
    return "";
  };

  size_t getSize() const
  {
    switch(type)
    {
    case VfsFileType::ROOT: return 0;
#if defined (SDCARD) || (!defined(USE_LITTLEFS) && defined(SPI_FLASH))
    case VfsFileType::FAT:  return fatInfo.fsize;
#endif
#if defined(USE_LITTLEFS)
    case VfsFileType::LFS:  return lfsInfo.size;
#endif
    }
    return 0;
  }

  VfsType getType() const
  {
    switch(type)
    {
    case VfsFileType::ROOT:
      return VfsType::DIR;
#if defined (SDCARD) || (!defined(USE_LITTLEFS) && defined(SPI_FLASH))
    case VfsFileType::FAT:
      if (!name.empty())
        return VfsType::DIR;
      if(fatInfo.fattrib & AM_DIR)
        return VfsType::DIR;
      else
        return VfsType::FILE;
#endif
#if defined(USE_LITTLEFS)
    case VfsFileType::LFS:

      if(lfsInfo.type == LFS_TYPE_DIR)
        return VfsType::DIR;
      else
        return VfsType::FILE;
#endif
    }
    return VfsType::UNKOWN;
  };

  VfsFileAttributes getAttrib()
  {
    switch(type)
    {
    case VfsFileType::ROOT:
      return VfsFileAttributes::DIR;
#if defined (SDCARD) || (!defined(USE_LITTLEFS) && defined(SPI_FLASH))
    case VfsFileType::FAT:
      return (VfsFileAttributes)fatInfo.fattrib;
#endif
#if defined(USE_LITTLEFS)
    case VfsFileType::LFS:
      if(lfsInfo.type == LFS_TYPE_DIR)
        return VfsFileAttributes::DIR;
      return VfsFileAttributes::NONE;
#endif
    }
    return VfsFileAttributes::NONE;
  }

  int getDate(){
    switch(type)
    {
    case VfsFileType::ROOT:
      return 0;
#if defined (SDCARD) || (!defined(USE_LITTLEFS) && defined(SPI_FLASH))
    case VfsFileType::FAT:
      return fatInfo.fdate;
#endif
#if defined(USE_LITTLEFS)
    case VfsFileType::LFS:
      return 0;
#endif
    }
    return 0;
  }
  int getTime()
  {
    switch(type)
    {
    case VfsFileType::ROOT:
      return 0;
#if defined (SDCARD) || (!defined(USE_LITTLEFS) && defined(SPI_FLASH))
    case VfsFileType::FAT:
      return fatInfo.ftime;
#endif
#if defined(USE_LITTLEFS)
    case VfsFileType::LFS:
      return 0;
#endif
    }
    return 0;
  }
private:
  friend class VirtualFS;
  friend struct VfsDir;
  VfsFileInfo(const VfsFileInfo&);

  void clear() {
    type = VfsFileType::UNKNOWN;
#if defined(USE_LITTLEFS)
    lfsInfo = {0};
#endif
    fatInfo = {0};
    name.clear();
  }

  VfsFileType type = VfsFileType::UNKNOWN;
  union {
#if defined(USE_LITTLEFS)
    lfs_info lfsInfo = {0};
#endif
    FILINFO fatInfo;
  };

  std::string name;
};

struct VfsDir
{
public:
  VfsDir() { clear(); }
  ~VfsDir() { if (type != DIR_UNKNOWN) close(); }

  VfsError read(VfsFileInfo& info);
  VfsError close();

private:
  friend class VirtualFS;
  VfsDir(const VfsDir&);

  enum DirType {DIR_UNKNOWN, DIR_ROOT, DIR_FAT, DIR_LFS};

  void clear()
  {
    type = DIR_UNKNOWN;
#if defined(USE_LITTLEFS)
    lfs.dir = {0};
    lfs.handle = nullptr;
#endif
    fat.dir = {0};

    readIdx = 0;
  }
  DirType type = DIR_UNKNOWN;
  union {
#if defined(USE_LITTLEFS)
    struct {
      lfs_dir_t dir;
      lfs* handle;
    } lfs;
#endif
    struct {
      DIR dir;
    } fat;
  };

  size_t readIdx = 0;
};

// for compatibility reasons (audio.h WavContext) the file does not close it self when destructed
// since the must not be a non trivial destructor
struct VfsFile
{
public:
  VfsFile() {clear();}

  VfsError close();

  bool isOpen() const { return type != VfsFileType::UNKNOWN; }
  int size();
  VfsError read(void* buf, size_t size, size_t& readSize);
  char* gets(char* buf, size_t maxLen);
  VfsError write(const void* buf, size_t size, size_t& written);
  VfsError puts(const std::string& str);
  VfsError putc(char c);
  int fprintf(const char* str, ...);

  size_t tell();
  VfsError lseek(size_t offset);
  int eof();

private:
  friend class VirtualFS;
  VfsFile(const VfsFile&);

  void clear() {
    type = VfsFileType::UNKNOWN;
#if defined(USE_LITTLEFS)
    lfs.file = {0};
    lfs.handle = nullptr;
#endif
    fat.file = {0};
  }

  VfsFileType type = VfsFileType::UNKNOWN;
  union {
#if defined(USE_LITTLEFS)
    struct {
      lfs_file file = {0};
      lfs* handle = nullptr;
    } lfs;
#endif
    struct {
      FIL file = {0};
    } fat;
  };

};

extern VfsFile g_oLogFile;


class VirtualFS
{
public:
  VirtualFS();
  ~VirtualFS();

  void stop();
  void restart();

  // NOTE: 'size' must = 0 or be a valid character position within 'filename' array -- it is NOT validated
  static const char* getBasename(const char * path);

  static VirtualFS& instance()
  {
    if( _instance == nullptr)
      _instance = new VirtualFS();
    return *_instance;
  }

  bool format();
  const char * checkAndCreateDirectory(const char * path);
  bool isFileAvailable(const char * path, bool exclDir = false);
  bool isFilePatternAvailable(const char * path, const char * file, const char * pattern = nullptr, bool exclDir = true, char * match = nullptr);
  char* getFileIndex(char * filename, unsigned int & value);

  const std::string& getCurWorkDir() const { return curWorkDir;}

  VfsError unlink(const std::string& path);

  VfsError changeDirectory(const std::string& path);
  VfsError openDirectory(VfsDir& dir, const char * path);
  VfsError makeDirectory(const std::string& path);

  VfsError fstat(const std::string& path, VfsFileInfo& fileInfo);
  VfsError utime(const std::string& path, const VfsFileInfo& fileInfo);
  VfsError openFile(VfsFile& file, const std::string& path, VfsOpenFlags flags);

  VfsError rename(const char* oldPath, const char* newPath);
  VfsError copyFile(const std::string& source, const std::string& destination);
  VfsError copyFile(const std::string& srcFile, const std::string& srcDir,
             const std::string& destDir, const std::string& destFile);


  uint32_t flashGetNoSectors() const;
  uint32_t flashGetSize() const;
  uint32_t flashGetFreeSectors() const;

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
  #define ETX_FOURCC 0x3478746F // etx for X12S
#elif defined(RADIO_T16)
  #define ETX_FOURCC 0x3F78746F // etx for Jumper T16
#elif defined(RADIO_T18)
  #define ETX_FOURCC 0x4078746F // etx for Jumper T18
#elif defined(RADIO_TX16S)
  #define ETX_FOURCC 0x3878746F // etx for Radiomaster TX16S
#elif defined(PCBX10)
  #define ETX_FOURCC 0x3778746F // etx for X10
#elif defined(PCBX9E)
  #define ETX_FOURCC 0x3578746F // etx for Taranis X9E
#elif defined(PCBXLITES)
  #define ETX_FOURCC 0x3B78746F // etx for Taranis X-Lite S
#elif defined(PCBXLITE)
  #define ETX_FOURCC 0x3978746F // etx for Taranis X-Lite
#elif defined(RADIO_T12)
  #define ETX_FOURCC 0x3D78746F // etx for Jumper T12
#elif defined(RADIO_TLITE)
  #define ETX_FOURCC 0x4278746F // etx for Jumper TLite
#elif defined(RADIO_TPRO)
  #define ETX_FOURCC 0x4678746F // etx for Jumper TPro
#elif defined(RADIO_TX12)
  #define ETX_FOURCC 0x4178746F // etx for Radiomaster TX12
#elif defined(RADIO_ZORRO)
  #define ETX_FOURCC 0x4778746F // otx for Radiomaster Zorro
#elif defined(RADIO_T8)
  #define ETX_FOURCC 0x4378746F // etx for Radiomaster T8
#elif defined(PCBX7)
  #define ETX_FOURCC 0x3678746F // etx for Taranis X7 / X7S / X7 Express / X7S Express
#elif defined(PCBX9LITES)
  #define ETX_FOURCC 0x3E78746F // etx for Taranis X9-Lite S
#elif defined(PCBX9LITE)
  #define ETX_FOURCC 0x3C78746F // etx for Taranis X9-Lite
#elif defined(PCBX9D) || defined(PCBX9DP)
  #define ETX_FOURCC 0x3378746F // etx for Taranis X9D
#elif defined(PCBNV14)
  #define ETX_FOURCC 0x3A78746F // otx for NV14
#elif defined(PCBPL18)
  #define ETX_FOURCC 0x4878746F // otx for PL18
#endif

  unsigned int findNextFileIndex(char * filename, uint8_t size, const char * directory);

  #define LIST_NONE_SD_FILE   1
  #define LIST_SD_FILE_EXT    2
  bool flashListFiles(const char * path, const char * extension, const uint8_t maxlen, const char * selection, uint8_t flags=0);
private:
  static VirtualFS* _instance;

#if defined(USE_LITTLEFS)
  lfs_config lfsCfg = {0};
  lfs_t lfs = {0};
#elif defined(SPI_FLASH)
  tjftl_t* tjftl;
#endif
  std::string curWorkDir = "/";

  void normalizePath(std::string &path);

  VfsDir::DirType getDirTypeAndPath(std::string& path);
};

#endif // _VIRTUALFS_H_
