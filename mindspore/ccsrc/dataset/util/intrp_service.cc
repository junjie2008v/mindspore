/**
 * Copyright 2019 Huawei Technologies Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "dataset/util/intrp_service.h"
#include <sstream>
#include "common/utils.h"
#include "dataset/util/services.h"
#include "dataset/util/task_manager.h"

namespace mindspore {
namespace dataset {
IntrpService::IntrpService() : high_water_mark_(0) { (void)ServiceStart(); }

IntrpService::~IntrpService() noexcept {
  MS_LOG(INFO) << "Number of registered resources is " << high_water_mark_ << ".";
  if (!all_intrp_resources_.empty()) {
    try {
      (void)InterruptAll();
    } catch (const std::exception &e) {
      // Ignore all error as we can't throw in the destructor.
    }
  }
  (void)ServiceStop();
}

Status IntrpService::Register(const std::string &name, IntrpResource *res) {
  SharedLock stateLck(&state_lock_);
  // Now double check the state
  if (ServiceState() != STATE::kRunning) {
    return Status(StatusCode::kInterrupted, __LINE__, __FILE__, "Interrupt service is shutting down");
  } else {
    std::lock_guard<std::mutex> lck(mutex_);
    try {
      std::ostringstream ss;
      ss << this_thread::get_id();
      MS_LOG(DEBUG) << "Register resource with name " << name << ". Thread ID " << ss.str() << ".";
      auto it = all_intrp_resources_.emplace(name, res);
      if (it.second == false) {
        return Status(StatusCode::kDuplicateKey, __LINE__, __FILE__, name);
      }
      high_water_mark_++;
    } catch (std::exception &e) {
      RETURN_STATUS_UNEXPECTED(e.what());
    }
  }
  return Status::OK();
}

Status IntrpService::Deregister(const std::string &name) noexcept {
  std::lock_guard<std::mutex> lck(mutex_);
  try {
    std::ostringstream ss;
    ss << this_thread::get_id();
    MS_LOG(DEBUG) << "De-register resource with name " << name << ". Thread ID is " << ss.str() << ".";
    auto it = all_intrp_resources_.find(name);
    if (it != all_intrp_resources_.end()) {
      (void)all_intrp_resources_.erase(it);
    } else {
      MS_LOG(INFO) << "Key " << name << " not found.";
    }
  } catch (std::exception &e) {
    RETURN_STATUS_UNEXPECTED(e.what());
  }
  return Status::OK();
}

Status IntrpService::InterruptAll() noexcept {
  std::lock_guard<std::mutex> lck(mutex_);
  Status rc;
  for (auto const &it : all_intrp_resources_) {
    std::string kName = it.first;
    try {
      Status rc2 = it.second->Interrupt();
      if (rc2.IsError()) {
        rc = rc2;
      }
    } catch (const std::exception &e) {
      // continue the clean up.
    }
  }
  return rc;
}
}  // namespace dataset
}  // namespace mindspore
