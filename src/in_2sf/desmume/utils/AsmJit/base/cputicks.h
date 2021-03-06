// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#pragma once

// [Dependencies - AsmJit]
#include "../base/globals.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

//! \addtogroup asmjit_base_util
//! \{

// ============================================================================
// [asmjit::CpuTicks]
// ============================================================================

//! CPU ticks utilities.
struct CpuTicks {
  //! Get the current CPU ticks for benchmarking (1ms resolution).
  static ASMJIT_API uint32_t now();
};

//! \}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"
