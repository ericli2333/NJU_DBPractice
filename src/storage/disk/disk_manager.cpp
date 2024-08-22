/*------------------------------------------------------------------------------
 - Copyright (c) 2024. Websoft research group, Nanjing University.
 -
 - This program is free software: you can redistribute it and/or modify
 - it under the terms of the GNU General Public License as published by
 - the Free Software Foundation, either version 3 of the License, or
 - (at your option) any later version.
 -
 - This program is distributed in the hope that it will be useful,
 - but WITHOUT ANY WARRANTY; without even the implied warranty of
 - MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 - GNU General Public License for more details.
 -
 - You should have received a copy of the GNU General Public License
 - along with this program.  If not, see <https://www.gnu.org/licenses/>.
 -----------------------------------------------------------------------------*/

//
// Created by ziqi on 2024/7/17.
//

#include <filesystem>
#include <fcntl.h>
#include <unistd.h>
#include "disk_manager.h"
#include "../../common/config.h"
#include "../../../common/error.h"

namespace wsdb {
void DiskManager::CreateFile(const std::string &fname)
{
  if (FileExists(fname)) {
    throw WSDBException(WSDB_FILE_EXISTS, Q(DiskManager), Q(CreateFile), fname);
  }
  std::ofstream file(fname);
  if (!file) {
    WSDB_FETAL(DiskManager, CreateFile, "Create file failed");
  }
  file.close();
}

void DiskManager::DestroyFile(const std::string &fname)
{
  if (!FileExists(fname)) {
    throw WSDBException(WSDB_FILE_NOT_EXISTS, Q(DiskManager), Q(DestroyFile), fname);
  }
  int ret = unlink(fname.c_str());
  if (ret < 0) {
    throw WSDBException(WSDB_FILE_DELETE_ERROR, Q(DiskManager), Q(DestroyFile), fname);
  }
}

auto DiskManager::OpenFile(const std::string &fname) -> file_id_t
{
  if (!FileExists(fname))
    throw WSDBException(WSDB_FILE_NOT_EXISTS, Q(DiskManager), Q(OpenFile), fname);
  if (name_fid_map_.find(fname) != name_fid_map_.end()) {
    throw WSDBException(WSDB_FILE_REOPEN, Q(DiskManager), Q(OpenFile), fname);
  } else {
    int fd = open(fname.c_str(), O_RDWR);
    if (fd == -1) {
      throw WSDBException(WSDB_FILE_NOT_OPEN, Q(DiskManager), Q(OpenFile), fname);
    }
    name_fid_map_.insert(std::make_pair(fname, fd));
    fid_name_map_.insert(std::make_pair(fd, fname));
    return fd;
  }
}

void DiskManager::CloseFile(file_id_t fid)
{
  if (fid_name_map_.find(fid) == fid_name_map_.end()) {
    throw WSDBException(WSDB_FILE_NOT_OPEN, Q(DiskManager), Q(CloseFile), fmt::format("fid: {}", fid));
  } else {
    name_fid_map_.erase(fid_name_map_[fid]);
    fid_name_map_.erase(fid);
    close(fid);
  }
}

void DiskManager::WritePage(file_id_t fid, page_id_t page_id, const char *data)
{
  WSDB_ASSERT(DiskManager, WritePage(), fid_name_map_.find(fid) != fid_name_map_.end(), fmt::format("fid: {}", fid));
  lseek(fid, static_cast<off_t>(page_id) * static_cast<off_t>(PAGE_SIZE), SEEK_SET);
  if (write(fid, data, PAGE_SIZE) != PAGE_SIZE) {
    throw WSDBException(
        WSDB_PAGE_WRITE_ERROR, Q(DiskManager), Q(WritePage), fmt::format("fid: {}, page_id: {}", fid, page_id));
  }
}

void DiskManager::ReadPage(file_id_t fid, page_id_t page_id, char *data)
{
  WSDB_ASSERT(DiskManager, ReadPage(), fid_name_map_.find(fid) != fid_name_map_.end(), fmt::format("fid: {}", fid));
  lseek(fid, static_cast<off_t>(page_id) * static_cast<off_t>(PAGE_SIZE), SEEK_SET);
  if (read(fid, data, PAGE_SIZE) < 0) {
    throw WSDBException(
        WSDB_PAGE_READ_ERROR, Q(DiskManager), Q(ReadPage), fmt::format("fid: {}, page_id: {}", fid, page_id));
  }
}

void DiskManager::ReadFile(file_id_t fid, char *data, size_t size, size_t offset, int type)
{
  WSDB_ASSERT(DiskManager, ReadFile(), fid_name_map_.find(fid) != fid_name_map_.end(), "File not Opened");
  lseek(fid, static_cast<off_t>(offset), type);
  read(fid, data, size);
}

void DiskManager::WriteFile(file_id_t fid, const char *data, size_t size, int type)
{
  WSDB_ASSERT(DiskManager, WriteFile(), fid_name_map_.find(fid) != fid_name_map_.end(), "File not Opened");
  WSDB_ASSERT(DiskManager, WriteFile(), type == SEEK_CUR || type == SEEK_SET || type == SEEK_END, "Invalid Type");
  lseek(fid, 0, type);
  write(fid, data, size);
}

void DiskManager::WriteLog(const std::string &log_file, const std::string &log_string) {}

void DiskManager::ReadLog(const std::string &log_file, std::string &log_string) {}

auto DiskManager::GetFileId(const std::string &fname) -> file_id_t
{
  auto it = name_fid_map_.find(fname);
  if (it != name_fid_map_.end()) {
    return it->second;
  } else {
    return INVALID_TABLE_ID;
  }
}

auto DiskManager::GetFileName(file_id_t fid) -> std::string
{
  auto it = fid_name_map_.find(fid);
  if (it != fid_name_map_.end()) {
    return it->second;
  } else {
    throw WSDBException(WSDB_FILE_NOT_OPEN, Q(DiskManager), Q(GetFileName), fmt::format("fid: {}", fid));
  }
}

auto DiskManager::FileExists(const std::string &fname) -> bool { return std::filesystem::exists(fname); }

}  // namespace wsdb