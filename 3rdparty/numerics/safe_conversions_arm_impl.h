// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_NUMERICS_SAFE_CONVERSIONS_ARM_IMPL_H_
#define BASE_NUMERICS_SAFE_CONVERSIONS_ARM_IMPL_H_

#include <cassert>
#include <limits>
#include <type_traits>

#include "safe_conversions_impl.h"

namespace chromium {
namespace base {
namespace internal {

// Fast saturation to a destination type.
template <typename Dst, typename Src>
struct SaturateFastAsmOp {
  static constexpr bool is_supported =
      kEnableAsmCode && std::is_signed<Src>::value &&
      std::is_integral<Dst>::value && std::is_integral<Src>::value &&
      IntegerBitsPlusSign<Src>::value <= IntegerBitsPlusSign<int32_t>::value &&
      IntegerBitsPlusSign<Dst>::value <= IntegerBitsPlusSign<int32_t>::value &&
      !IsTypeInRangeForNumericType<Dst, Src>::value;

  __attribute__((always_inline)) static Dst Do(Src value) {
    int32_t src = value;
    typename std::conditional<std::is_signed<Dst>::value, int32_t,
                              uint32_t>::type result;
    if (std::is_signed<Dst>::value) {
      asm("ssat %[dst], %[shift], %[src]"
          : [dst] "=r"(result)
          : [src] "r"(src), [shift] "n"(IntegerBitsPlusSign<Dst>::value <= 32
                                            ? IntegerBitsPlusSign<Dst>::value
                                            : 32));
    } else {
      asm("usat %[dst], %[shift], %[src]"
          : [dst] "=r"(result)
          : [src] "r"(src), [shift] "n"(IntegerBitsPlusSign<Dst>::value < 32
                                            ? IntegerBitsPlusSign<Dst>::value
                                            : 31));
    }
    return static_cast<Dst>(result);
  }
};

}  // namespace internal
}  // namespace base
}  // namespace chromium

#endif  // BASE_NUMERICS_SAFE_CONVERSIONS_ARM_IMPL_H_
