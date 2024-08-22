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
// Created by ziqi on 2024/7/18.
//

#include "executor_delete.h"

namespace wsdb {
;
DeleteExecutor::DeleteExecutor(AbstractExecutorUptr child, TableHandle *tbl, std::list<IndexHandle *> indexes)
    : AbstractExecutor(DML), child_(std::move(child)), tbl_(tbl), indexes_(std::move(indexes)), is_end_(false)
{
  std::vector<RTField> fields(1);
  fields[0]   = RTField{.field_ = {.field_name_ = "deleted", .field_size_ = sizeof(int), .field_type_ = TYPE_INT}};
  out_schema_ = std::make_unique<RecordSchema>(fields);
}
void DeleteExecutor::Init() { WSDB_FETAL(DeleteExecutor, Init(), "DeleteExecutor does not support Init"); }

void DeleteExecutor::Next() { WSDB_STUDENT_TODO(L2, t1, DeleteExecutor, Next()); }

auto DeleteExecutor::IsEnd() const -> bool { WSDB_STUDENT_TODO(L2, t1, DeleteExecutor, IsEnd()); }
}  // namespace wsdb