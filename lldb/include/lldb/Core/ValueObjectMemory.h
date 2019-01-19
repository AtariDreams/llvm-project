//===-- ValueObjectMemory.h -----------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef liblldb_ValueObjectMemory_h_
#define liblldb_ValueObjectMemory_h_

#include "lldb/Core/Address.h"
#include "lldb/Core/ValueObject.h"
#include "lldb/Symbol/CompilerType.h"
#include "lldb/Utility/ConstString.h"
#include "lldb/lldb-defines.h"
#include "lldb/lldb-enumerations.h"
#include "lldb/lldb-forward.h"
#include "llvm/ADT/StringRef.h"

#include <stddef.h>
#include <stdint.h>

namespace lldb_private {
class ExecutionContextScope;
}

namespace lldb_private {

//----------------------------------------------------------------------
// A ValueObject that represents memory at a given address, viewed as some
// set lldb type.
//----------------------------------------------------------------------
class ValueObjectMemory : public ValueObject {
public:
  ~ValueObjectMemory() override;

  static lldb::ValueObjectSP Create(ExecutionContextScope *exe_scope,
                                    llvm::StringRef name,
                                    const Address &address,
                                    lldb::TypeSP &type_sp);

  static lldb::ValueObjectSP Create(ExecutionContextScope *exe_scope,
                                    llvm::StringRef name,
                                    const Address &address,
                                    const CompilerType &ast_type);

  uint64_t GetByteSize() override;

  ConstString GetTypeName() override;

  ConstString GetDisplayTypeName() override;

  size_t CalculateNumChildren(uint32_t max) override;

  lldb::ValueType GetValueType() const override;

  bool IsInScope() override;

  lldb::ModuleSP GetModule() override;

protected:
  bool UpdateValue() override;

  CompilerType GetCompilerTypeImpl() override;

  Address m_address; ///< The variable that this value object is based upon
  lldb::TypeSP m_type_sp;
  CompilerType m_compiler_type;

private:
  ValueObjectMemory(ExecutionContextScope *exe_scope, llvm::StringRef name,
                    const Address &address, lldb::TypeSP &type_sp);

  ValueObjectMemory(ExecutionContextScope *exe_scope, llvm::StringRef name,
                    const Address &address, const CompilerType &ast_type);
  //------------------------------------------------------------------
  // For ValueObject only
  //------------------------------------------------------------------
  DISALLOW_COPY_AND_ASSIGN(ValueObjectMemory);
};

} // namespace lldb_private

#endif // liblldb_ValueObjectMemory_h_
