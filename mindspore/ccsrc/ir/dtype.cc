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

#include "ir/dtype.h"
#include <string>
#include <cstdlib>
#include <algorithm>
#include "utils/log_adapter.h"
#include "pipeline/static_analysis/abstract_value.h"
#include "pybind_api/api_register.h"
#include "pybind_api/export_flags.h"

namespace mindspore {
TypePtr Keyword::DeepCopy() const {
  if (IsGeneric()) {
    return std::make_shared<Keyword>();
  } else {
    MS_EXCEPTION_IF_NULL(value_);
    std::string key = key_;
    return std::make_shared<Keyword>(key, value_->DeepCopy());
  }
}

std::string Keyword::ToString() const {
  std::ostringstream buffer;
  if (IsGeneric()) {
    buffer << "Keyword";
  } else {
    MS_EXCEPTION_IF_NULL(value_);
    buffer << "Keyword[";
    buffer << "key : " << key_;
    buffer << "value : " << value_->ToString();
    buffer << "]";
  }
  return buffer.str();
}

bool Keyword::operator==(const Type &other) const {
  if (!IsSameObjectType(*this, other)) {
    return false;
  }
  const auto &other_keyword = static_cast<const Keyword &>(other);
  return (other_keyword.key_ == key_ && *other_keyword.value_ == *value_);
}

std::string Keyword::DumpText() const { return ToString(); }

TypePtr Slice::DeepCopy() const {
  if (IsGeneric()) {
    return std::make_shared<Slice>();
  } else {
    MS_EXCEPTION_IF_NULL(start_);
    MS_EXCEPTION_IF_NULL(stop_);
    MS_EXCEPTION_IF_NULL(step_);
    auto copy = std::make_shared<Slice>(start_->DeepCopy(), stop_->DeepCopy(), step_->DeepCopy());
    return copy;
  }
}

std::string Slice::ToString() const {
  std::ostringstream buffer;
  if (IsGeneric()) {
    buffer << "Slice";
  } else {
    MS_EXCEPTION_IF_NULL(start_);
    MS_EXCEPTION_IF_NULL(stop_);
    MS_EXCEPTION_IF_NULL(step_);
    buffer << "Slice[";
    buffer << start_->ToString() << " : ";
    buffer << stop_->ToString() << " : ";
    buffer << step_->ToString();
    buffer << "]";
  }
  return buffer.str();
}

bool Slice::operator==(const Type &other) const {
  if (!IsSameObjectType(*this, other)) {
    return false;
  }
  auto other_slice = static_cast<const Slice &>(other);
  return (*start_ == *other_slice.start_ && *stop_ == *other_slice.stop_ && *step_ == *other_slice.step_);
}

std::string Slice::DumpText() const { return ToString(); }

TypePtr TensorType::DeepCopy() const {
  MS_EXCEPTION_IF_NULL(element_type_);
  if (IsGeneric()) {
    return std::make_shared<TensorType>();
  } else {
    return std::make_shared<TensorType>(element_type_->DeepCopy());
  }
}

std::string TensorType::ToString() const {
  if (element_type_ == nullptr) {
    return "Tensor";
  } else {
    return "Tensor[" + element_type_->ToString() + "]";
  }
}

std::string TensorType::DumpText() const {
  if (element_type_ == nullptr) {
    return "Tensor";
  } else {
    return "Tensor(" + element_type_->DumpText() + ")";
  }
}

bool TensorType::operator==(const Type &other) const {
  if (!IsSameObjectType(*this, other)) {
    return false;
  }
  auto other_elem_type = static_cast<const TensorType &>(other).element_type_;
  // When element_type_ = nullptr, which means any type of Array.
  if (element_type_ == nullptr && other_elem_type == nullptr) {
    return true;
  } else if (element_type_ == nullptr || other_elem_type == nullptr) {
    return false;
  }
  return *element_type_ == *other_elem_type;
}

Function::Function() : Object(kObjectTypeFunction) {
  args_ = std::vector<TypePtr>();
  retval_ = nullptr;
}

Function::Function(const std::vector<TypePtr> &args, const TypePtr retval)
    : Object(kObjectTypeFunction, false), args_(args), retval_(retval) {}

TypePtr Function::DeepCopy() const {
  if (IsGeneric()) {
    return std::make_shared<Function>();
  } else {
    TypePtrList args;
    TypePtr retval = nullptr;
    (void)std::transform(args_.begin(), args_.end(), std::back_inserter(args),
                         [](const TypePtr &arg) { return arg->DeepCopy(); });
    if (retval_ != nullptr) {
      retval = retval_->DeepCopy();
    }
    return std::make_shared<Function>(args, retval);
  }
}

bool Function::operator==(const Type &other) const {
  if (!IsSameObjectType(*this, other)) {
    return false;
  }

  const auto &other_function = static_cast<const Function &>(other);
  if ((retval_ != nullptr) && (other_function.retval_ != nullptr)) {
    if (*retval_ != *other_function.retval_) {
      return false;
    }
  } else if ((retval_ == nullptr) && (other_function.retval_ != nullptr)) {
    return false;
  }
  if (args_.size() != other_function.args_.size()) {
    return false;
  }
  for (size_t i = 0; i < args_.size(); ++i) {
    if (*args_[i] != *other_function.args_[i]) return false;
  }
  return true;
}

std::string Function::ToString() const {
  std::ostringstream buffer;
  if (IsGeneric()) {
    buffer << "Func";
  } else {
    buffer << "Func[(";
    bool begin = true;
    for (auto &attr : args_) {
      if (!begin) {
        buffer << ", ";
      } else {
        begin = false;
      }
      buffer << attr->ToString();
    }
    buffer << ")";
    if (retval_ != nullptr) {
      buffer << ", " << retval_->ToString() << "]";
    } else {
      buffer << "]";
    }
  }
  return buffer.str();
}

TypePtr TypeAnything::DeepCopy() const { return kAnyType; }

TypePtr JTagged::DeepCopy() const {
  MS_EXCEPTION_IF_NULL(subtype_);
  if (IsGeneric()) {
    return std::make_shared<JTagged>();
  } else {
    auto subtype = subtype_->DeepCopy();
    return std::make_shared<JTagged>(subtype);
  }
}

std::string JTagged::ToString() const {
  MS_EXCEPTION_IF_NULL(subtype_);
  std::ostringstream buffer;
  if (IsGeneric()) {
    buffer << "JT";
  } else {
    buffer << "JT[";
    buffer << subtype_->ToString() << "]";
  }
  return buffer.str();
}

std::string JTagged::DumpText() const {
  MS_EXCEPTION_IF_NULL(subtype_);
  std::ostringstream buffer;
  if (IsGeneric()) {
    buffer << "JT";
  } else {
    buffer << "JT[";
    buffer << subtype_->DumpText() << "]";
  }
  return buffer.str();
}

std::ostream &operator<<(std::ostream &os, const std::shared_ptr<Problem> problem) {
  MS_EXCEPTION_IF_NULL(problem);
  os << problem->ToString();
  return os;
}

std::size_t TypeHasher::operator()(TypePtr const &type) const {
  MS_EXCEPTION_IF_NULL(type);
  std::size_t hash = std::hash<size_t>()(type->type_id());
  return hash;
}

std::size_t TypeListHasher::operator()(const TypePtrList &type_list) const {
  std::size_t hash_sum = 0;
  for (auto &type : type_list) {
    auto type_id = static_cast<std::size_t>(type->type_id());
    hash_sum = hash_combine(hash_sum, type_id);
  }
  return hash_sum;
}

bool TypeEqual::operator()(TypePtr const &t1, TypePtr const &t2) const {
  MS_EXCEPTION_IF_NULL(t1);
  MS_EXCEPTION_IF_NULL(t2);
  return t1->type_id() == t2->type_id();
}

bool TypeListEqual::operator()(TypePtrList const &lhs, TypePtrList const &rhs) const {
  if (lhs.size() != rhs.size()) {
    return false;
  }
  std::size_t size = lhs.size();
  for (std::size_t i = 0; i < size; ++i) {
    MS_EXCEPTION_IF_NULL(lhs[i]);
    MS_EXCEPTION_IF_NULL(rhs[i]);
    if (*lhs[i] != *rhs[i]) {
      return false;
    }
  }
  return true;
}

TypePtr TypeIdToType(TypeId id) {
  switch (id) {
    case kNumberTypeFloat16:
      return kFloat16;
    case kNumberTypeFloat:
    case kNumberTypeFloat32:
      return kFloat32;
    case kNumberTypeFloat64:
      return kFloat64;
    case kNumberTypeInt8:
      return kInt8;
    case kNumberTypeInt16:
      return kInt16;
    case kNumberTypeInt32:
      return kInt32;
    case kNumberTypeInt64:
      return kInt64;
    case kNumberTypeUInt8:
      return kUInt8;
    case kNumberTypeUInt16:
      return kUInt16;
    case kNumberTypeUInt32:
      return kUInt32;
    case kNumberTypeUInt64:
      return kUInt64;
    case kNumberTypeBool:
      return kBool;
    case kMetaTypeExternal:
      return kTypeExternal;
    case kMetaTypeAnything:
      return kAnyType;
    case kMetaTypeNone:
      return kTypeNone;
    case kObjectTypeEnvType:
      return kTypeEnv;
    case kObjectTypeRefKey:
      return kRefKeyType;
    case kObjectTypeRef:
      return kRefType;
    case kTypeUnknown:
      return kTypeNone;
    default:
      MS_LOG(EXCEPTION) << "Not support the type: " << id;
  }
}

namespace {
template <typename T>
TypePtr StringToNumberType(const std::string &type_name, const std::string &num_type_name) {
  TypePtr type = nullptr;
  if (type_name == num_type_name) {
    type = std::make_shared<T>();
  } else {
    try {
      if (num_type_name.size() >= type_name.size()) {
        MS_LOG(EXCEPTION) << "Convert type is error, type_name(" << type_name << "), num_type_name(" << num_type_name
                          << ")";
      }
      auto bits = std::stoi(type_name.substr(num_type_name.size()));
      type = std::make_shared<T>(bits);
    } catch (const std::exception &e) {
      MS_LOG(EXCEPTION) << num_type_name << " convert from string error " << e.what();
    }
  }
  return type;
}

std::vector<TypePtr> StringToVectorOfType(const std::string &type_names) {
  std::vector<TypePtr> types;
  if (type_names.length() == 0) {
    return types;
  }
  std::string::size_type start = 0;
  std::string::size_type end = type_names.find_first_of(',');
  while (end != std::string::npos) {
    types.push_back(StringToType(type_names.substr(start, end)));
    // Skip ',' to find the next element.
    start = end + 1;
    end = type_names.find_first_of(',', start);
  }
  if (start >= type_names.size()) {
    MS_LOG(EXCEPTION) << "Type name is empty string.";
  }
  types.push_back(StringToType(type_names.substr(start)));
  return types;
}

TypePtr TensorStrToType(const std::string &type_name) {
  TypePtr type = nullptr;
  if (type_name == "Tensor") {
    type = std::make_shared<TensorType>();
  } else {
    try {
      auto start = type_name.find_first_of('[') + 1;
      auto end = type_name.find_last_of(']');
      if (start >= type_name.size()) {
        return nullptr;
      }
      auto element_str = type_name.substr(start, end - start);
      auto element_type = StringToType(element_str);
      if (element_type == nullptr) {
        return nullptr;
      }
      type = std::make_shared<TensorType>(element_type);
    } catch (const std::exception &e) {
      MS_LOG(EXCEPTION) << type_name << " convert from string error " << e.what();
    }
  }

  return type;
}

TypePtr ListStrToType(const std::string &type_name) {
  TypePtr type = nullptr;
  if (type_name == "List") {
    type = std::make_shared<List>();
  } else {
    try {
      auto start = type_name.find_first_of('[') + 1;
      auto end = type_name.find_last_of(']');
      if (start >= type_name.size()) {
        return nullptr;
      }
      std::string element_strs = type_name.substr(start, end - start);
      std::vector<TypePtr> element_types = StringToVectorOfType(element_strs);
      bool wrong =
        std::any_of(element_types.begin(), element_types.end(), [](const TypePtr &x) { return x == nullptr; });
      if (wrong) {
        return nullptr;
      }
      type = std::make_shared<List>(element_types);
    } catch (const std::exception &e) {
      MS_LOG(EXCEPTION) << type_name << " convert from string error " << e.what();
    }
  }

  return type;
}

TypePtr TupleStrToType(const std::string &type_name) {
  TypePtr type = nullptr;
  if (type_name == "Tuple") {
    type = std::make_shared<Tuple>();
  } else {
    try {
      size_t start = type_name.find_first_of('[') + 1;
      size_t end = type_name.find_last_of(']');
      if (start >= type_name.size()) {
        return nullptr;
      }
      std::string element_strs = type_name.substr(start, end - start);
      std::vector<TypePtr> element_types = StringToVectorOfType(element_strs);
      bool wrong =
        std::any_of(element_types.begin(), element_types.end(), [](const TypePtr &x) { return x == nullptr; });
      if (wrong) {
        return nullptr;
      }
      type = std::make_shared<Tuple>(element_types);
    } catch (const std::exception &e) {
      MS_LOG(EXCEPTION) << type_name << " convert from string error " << e.what();
    }
  }
  return type;
}

TypePtr FunctionStrToType(const std::string &type_name) {
  TypePtr type = nullptr;

  if (type_name == "Function") {
    type = std::make_shared<Function>();
  } else {
    try {
      // format: [(para1, para2, para3, ...) retval]
      size_t start = type_name.find_first_of('[') + 1;
      size_t end = type_name.find_last_of(']');
      if (start >= type_name.size()) {
        return nullptr;
      }
      std::string str_all = type_name.substr(start, end - start);
      size_t start_a = str_all.find_first_of('(') + 1;
      size_t end_a = str_all.find_last_of(')');
      if (start_a >= str_all.size()) {
        return nullptr;
      }
      std::string str_args = str_all.substr(start_a, end_a - start_a);
      // bypass " " between ")" and retval
      start = end_a + 2;
      if (start >= str_all.size()) {
        return nullptr;
      }
      std::string str_retval = str_all.substr(start);

      std::vector<TypePtr> args_type = StringToVectorOfType(str_args);
      TypePtr retval = StringToType(str_retval);
      bool wrong = std::any_of(args_type.begin(), args_type.end(), [](const TypePtr &x) { return x == nullptr; });
      if (retval == nullptr || wrong) {
        return nullptr;
      }
      type = std::make_shared<Function>(args_type, retval);
    } catch (const std::exception &e) {
      MS_LOG(EXCEPTION) << type_name << " convert from string error " << e.what();
    }
  }
  return type;
}
}  // namespace

TypePtr StringToType(const std::string &type_name) {
  TypePtr type = nullptr;
  if (type_name.compare("None") == 0) {
    type = std::make_shared<TypeNone>();
  } else if (type_name.compare("Ellipsis") == 0) {
    type = std::make_shared<Ellipsis>();
  } else if (type_name.compare("TypeType") == 0) {
    type = std::make_shared<TypeType>();
  } else if (type_name.compare("SymbolicKeyType") == 0) {
    type = std::make_shared<SymbolicKeyType>();
  } else if (type_name.compare("RefKeyType") == 0) {
    type = std::make_shared<RefKeyType>();
  } else if (type_name.compare("EnvType") == 0) {
    type = std::make_shared<EnvType>();
  } else if (type_name.compare("Number") == 0) {
    type = std::make_shared<Number>();
  } else if (type_name.compare("Bool") == 0) {
    type = std::make_shared<Bool>();
  } else if (type_name.compare(0, strlen("Int"), "Int") == 0) {
    type = StringToNumberType<Int>(type_name, "Int");
  } else if (type_name.compare(0, strlen("UInt"), "UInt") == 0) {
    type = StringToNumberType<UInt>(type_name, "UInt");
  } else if (type_name.compare(0, strlen("Float"), "Float") == 0) {
    type = StringToNumberType<Float>(type_name, "Float");
  } else if (type_name.compare(0, strlen("Tensor"), "Tensor") == 0) {
    type = TensorStrToType(type_name);
  } else if (type_name.compare(0, strlen("List"), "List") == 0) {
    type = ListStrToType(type_name);
  } else if (type_name.compare(0, strlen("Tuple"), "Tuple") == 0) {
    type = TupleStrToType(type_name);
  } else if (type_name.compare("Slice") == 0) {
    type = std::make_shared<Slice>();
  } else if (type_name.compare("Dictionary") == 0) {
    type = std::make_shared<Dictionary>();
  } else if (type_name.compare("String") == 0) {
    type = std::make_shared<String>();
  } else if (type_name.compare("Problem") == 0) {
    type = std::make_shared<Problem>();
  } else if (type_name.compare(0, strlen("Function"), "Function") == 0) {
    type = FunctionStrToType(type_name);
  } else {
    // - unsupported to convert
    // Class
    // SymbolicType
    // JTagged
    // Anything
    // External
    // Problem
    MS_LOG(EXCEPTION) << "Unsupported type name: " << type_name << "!";
  }
  return type;
}

bool IsIdentidityOrSubclass(TypePtr const &x, TypePtr const &base_type) {
  if (x == nullptr || base_type == nullptr) {
    MS_LOG(ERROR) << "Type is nullptr.";
    return false;
  }
  if (base_type->type_id() == kTypeUnknown || x->type_id() == kTypeUnknown) {
    return false;
  } else if (!(base_type->IsGeneric())) {
    return *(base_type) == *(x);
  } else if (base_type->type_id() == x->type_id()) {
    return true;
  } else if (base_type->type_id() == x->generic_type_id()) {
    return true;
  } else if (base_type->type_id() == x->object_type()) {
    return true;
  } else if (base_type->type_id() == x->meta_type()) {
    return true;
  } else {
    return false;
  }
}

bool IsSubType(TypePtr const &t1, TypePtr const &t2) {
  MS_EXCEPTION_IF_NULL(t1);
  if (t1->type_id() == kTypeUnknown) {
    return false;
  } else if (t2 != nullptr) {
    return IsIdentidityOrSubclass(t1, t2);
  } else {
    return true;
  }
}

REGISTER_PYBIND_DEFINE(
  typing, ([](py::module *const m) {
    auto m_sub = m->def_submodule("typing", "submodule for dtype");
    py::enum_<TypeId>(m_sub, "TypeId");
    (void)m_sub.def("is_subclass", &IsIdentidityOrSubclass, "is equal or subclass");
    (void)m_sub.def("load_type", &TypeIdToType, "load type");
    (void)m_sub.def(
      "dump_type", [](const TypePtr &t) { return t->type_id(); }, "dump type");
    (void)py::class_<Type, std::shared_ptr<Type>>(m_sub, "Type")
      .def_readonly(PYTHON_DTYPE_FLAG, &mindspore::Type::parse_info_)
      .def("__eq__",
           [](const TypePtr &t1, const TypePtr &t2) {
             if (t1 != nullptr && t2 != nullptr) {
               return *t1 == *t2;
             }
             return false;
           })
      .def("__hash__", &Type::hash)
      .def("__str__", &Type::ToString)
      .def("__repr__", &Type::ReprString)
      .def("__deepcopy__", [](const TypePtr &t, py::dict) {
        if (t == nullptr) {
          return static_cast<TypePtr>(nullptr);
        }
        return t->DeepCopy();
      });
    (void)py::class_<Number, Type, std::shared_ptr<Number>>(m_sub, "Number").def(py::init());
    (void)py::class_<Bool, Type, std::shared_ptr<Bool>>(m_sub, "Bool")
      .def(py::init())
      .def(py::pickle(
        [](const Bool &) {  // __getstate__
          return py::make_tuple();
        },
        [](const py::tuple &) {  // __setstate__
          return std::make_shared<Bool>();
        }));
    (void)py::class_<Int, Type, std::shared_ptr<Int>>(m_sub, "Int")
      .def(py::init())
      .def(py::init<int>(), py::arg("nbits"))
      .def(py::pickle(
        [](const Int &t) {  // __getstate__
          /* Return a tuple that fully encodes the state of the object */
          return py::make_tuple(py::int_(t.nbits()));
        },
        [](const py::tuple &t) {  // __setstate__
          if (t.size() != 1) {
            throw std::runtime_error("Invalid state!");
          }
          /* Create a new C++ instance */
          Int data(t[0].cast<py::int_>());
          return data;
        }));
    (void)py::class_<UInt, Type, std::shared_ptr<UInt>>(m_sub, "UInt")
      .def(py::init())
      .def(py::init<int>(), py::arg("nbits"))
      .def(py::pickle(
        [](const UInt &t) {  // __getstate__
          /* Return a tuple that fully encodes the state of the object */
          return py::make_tuple(py::int_(t.nbits()));
        },
        [](const py::tuple &t) {  // __setstate__
          if (t.size() != 1) {
            throw std::runtime_error("Invalid state!");
          }
          /* Create a new C++ instance */
          UInt data(t[0].cast<py::int_>());
          return data;
        }));
    (void)py::class_<Float, Type, std::shared_ptr<Float>>(m_sub, "Float")
      .def(py::init())
      .def(py::init<int>(), py::arg("nbits"))
      .def(py::pickle(
        [](const Float &t) {  // __getstate__
          /* Return a tuple that fully encodes the state of the object */
          return py::make_tuple(py::int_(t.nbits()));
        },
        [](const py::tuple &t) {  // __setstate__
          if (t.size() != 1) {
            throw std::runtime_error("Invalid state!");
          }
          /* Create a new C++ instance */
          Float data(t[0].cast<py::int_>());
          return data;
        }));
    (void)py::class_<List, Type, std::shared_ptr<List>>(m_sub, "List")
      .def(py::init())
      .def(py::init<std::vector<TypePtr>>(), py::arg("elements"));
    (void)py::class_<Tuple, Type, std::shared_ptr<Tuple>>(m_sub, "Tuple")
      .def(py::init())
      .def(py::init<std::vector<TypePtr>>(), py::arg("elements"));
    (void)py::class_<TensorType, Type, std::shared_ptr<TensorType>>(m_sub, "TensorType")
      .def(py::init())
      .def(py::init<TypePtr>(), py::arg("element"))
      .def("element_type", &TensorType::element)
      .def(py::pickle(
        [](const TensorType &t) {  // __getstate__
          /* Return a tuple that fully encodes the state of the object */
          return py::make_tuple(py::int_(static_cast<int>(t.element()->type_id())));
        },
        [](const py::tuple &t) {  // __setstate__
          if (t.size() != 1) {
            throw std::runtime_error("Invalid state!");
          }
          /* Create a new C++ instance */
          TensorType data(TypeIdToType(TypeId(static_cast<int>(t[0].cast<py::int_>()))));
          return data;
        }));
    (void)py::class_<Function, Type, std::shared_ptr<Function>>(m_sub, "Function")
      .def(py::init())
      .def(py::init<std::vector<TypePtr>, TypePtr>(), py::arg("args"), py::arg("retval"));
    (void)py::class_<Class, Type, std::shared_ptr<Class>>(m_sub, "Class").def(py::init());
    (void)py::class_<SymbolicKeyType, Type, std::shared_ptr<SymbolicKeyType>>(m_sub, "SymbolicKeyType").def(py::init());
    (void)py::class_<EnvType, Type, std::shared_ptr<EnvType>>(m_sub, "EnvType").def(py::init());
    (void)py::class_<TypeNone, Type, std::shared_ptr<TypeNone>>(m_sub, "TypeNone").def(py::init());
    (void)py::class_<TypeType, Type, std::shared_ptr<TypeType>>(m_sub, "TypeType").def(py::init());
    (void)py::class_<String, Type, std::shared_ptr<String>>(m_sub, "String").def(py::init());
    (void)py::class_<RefKeyType, Type, std::shared_ptr<RefKeyType>>(m_sub, "RefKeyType").def(py::init());
    (void)py::class_<RefType, Type, std::shared_ptr<RefType>>(m_sub, "RefType").def(py::init());
    (void)py::class_<TypeAnything, Type, std::shared_ptr<TypeAnything>>(m_sub, "TypeAnything").def(py::init());
  }));

const TypePtr kTypeExternal = std::make_shared<External>();
const TypePtr kTypeEnv = std::make_shared<EnvType>();
const TypePtr kTypeType = std::make_shared<TypeType>();
const TypePtr kTensorType = std::make_shared<TensorType>();
const TypePtr kString = std::make_shared<String>();
}  // namespace mindspore
