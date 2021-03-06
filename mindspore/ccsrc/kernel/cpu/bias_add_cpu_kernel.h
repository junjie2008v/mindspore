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
#ifndef MINDSPORE_MINDSPORE_CCSRC_KERNEL_CPU_BIAS_ADD_CPU_KERNEL_H_
#define MINDSPORE_MINDSPORE_CCSRC_KERNEL_CPU_BIAS_ADD_CPU_KERNEL_H_

#include <vector>
#include <memory>
#include "kernel/cpu/cpu_kernel.h"
#include "kernel/cpu/cpu_kernel_factory.h"

namespace mindspore {
namespace kernel {
class BiasAddCPUKernel : public CPUKernel {
 public:
  BiasAddCPUKernel() = default;
  ~BiasAddCPUKernel() override = default;

  void InitKernel(const CNodePtr &kernel_node) override;
  bool Launch(const std::vector<AddressPtr> &inputs, const std::vector<AddressPtr> &workspace,
              const std::vector<AddressPtr> &outputs) override;

 private:
  uint8_t data_shape_{0};
  std::vector<size_t> input_shape_;
  std::vector<size_t> bias_shape_;
};
MS_REG_CPU_KERNEL(
  BiasAdd,
  KernelAttr().AddInputAttr(kNumberTypeFloat32).AddInputAttr(kNumberTypeFloat32).AddOutputAttr(kNumberTypeFloat32),
  BiasAddCPUKernel);
}  // namespace kernel
}  // namespace mindspore
#endif  // MINDSPORE_MINDSPORE_CCSRC_KERNEL_CPU_BIAS_ADD_CPU_KERNEL_H_
