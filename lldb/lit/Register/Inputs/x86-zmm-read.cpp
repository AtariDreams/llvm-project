#include <cstdint>

struct alignas(64) zmm_t {
  uint64_t a, b, c, d, e, f, g, h;
};

int main() {
  constexpr zmm_t zmm[] = {
    { 0x0706050403020100, 0x0F0E0D0C0B0A0908,
      0x1716151413121110, 0x1F1E1D1C1B1A1918,
      0x2726252423222120, 0x2F2E2D2C2B2A2928,
      0x3736353433323130, 0x3F3E3D3C3B3A3938, },
    { 0x0807060504030201, 0x100F0E0D0C0B0A09,
      0x1817161514131211, 0x201F1E1D1C1B1A19,
      0x2827262524232221, 0x302F2E2D2C2B2A29,
      0x3837363534333231, 0x403F3E3D3C3B3A39, },
    { 0x0908070605040302, 0x11100F0E0D0C0B0A,
      0x1918171615141312, 0x21201F1E1D1C1B1A,
      0x2928272625242322, 0x31302F2E2D2C2B2A,
      0x3938373635343332, 0x41403F3E3D3C3B3A, },
    { 0x0A09080706050403, 0x1211100F0E0D0C0B,
      0x1A19181716151413, 0x2221201F1E1D1C1B,
      0x2A29282726252423, 0x3231302F2E2D2C2B,
      0x3A39383736353433, 0x4241403F3E3D3C3B, },
    { 0x0B0A090807060504, 0x131211100F0E0D0C,
      0x1B1A191817161514, 0x232221201F1E1D1C,
      0x2B2A292827262524, 0x333231302F2E2D2C,
      0x3B3A393837363534, 0x434241403F3E3D3C, },
    { 0x0C0B0A0908070605, 0x14131211100F0E0D,
      0x1C1B1A1918171615, 0x24232221201F1E1D,
      0x2C2B2A2928272625, 0x34333231302F2E2D,
      0x3C3B3A3938373635, 0x44434241403F3E3D, },
    { 0x0D0C0B0A09080706, 0x1514131211100F0E,
      0x1D1C1B1A19181716, 0x2524232221201F1E,
      0x2D2C2B2A29282726, 0x3534333231302F2E,
      0x3D3C3B3A39383736, 0x4544434241403F3E, },
    { 0x0E0D0C0B0A090807, 0x161514131211100F,
      0x1E1D1C1B1A191817, 0x262524232221201F,
      0x2E2D2C2B2A292827, 0x363534333231302F,
      0x3E3D3C3B3A393837, 0x464544434241403F, },
#if defined(__x86_64__) || defined(_M_X64)
    { 0x0F0E0D0C0B0A0908, 0x1716151413121110,
      0x1F1E1D1C1B1A1918, 0x2726252423222120,
      0x2F2E2D2C2B2A2928, 0x3736353433323130,
      0x3F3E3D3C3B3A3938, 0x4746454443424140, },
    { 0x100F0E0D0C0B0A09, 0x1817161514131211,
      0x201F1E1D1C1B1A19, 0x2827262524232221,
      0x302F2E2D2C2B2A29, 0x3837363534333231,
      0x403F3E3D3C3B3A39, 0x4847464544434241, },
    { 0x11100F0E0D0C0B0A, 0x1918171615141312,
      0x21201F1E1D1C1B1A, 0x2928272625242322,
      0x31302F2E2D2C2B2A, 0x3938373635343332,
      0x41403F3E3D3C3B3A, 0x4948474645444342, },
    { 0x1211100F0E0D0C0B, 0x1A19181716151413,
      0x2221201F1E1D1C1B, 0x2A29282726252423,
      0x3231302F2E2D2C2B, 0x3A39383736353433,
      0x4241403F3E3D3C3B, 0x4A49484746454443, },
    { 0x131211100F0E0D0C, 0x1B1A191817161514,
      0x232221201F1E1D1C, 0x2B2A292827262524,
      0x333231302F2E2D2C, 0x3B3A393837363534,
      0x434241403F3E3D3C, 0x4B4A494847464544, },
    { 0x14131211100F0E0D, 0x1C1B1A1918171615,
      0x24232221201F1E1D, 0x2C2B2A2928272625,
      0x34333231302F2E2D, 0x3C3B3A3938373635,
      0x44434241403F3E3D, 0x4C4B4A4948474645, },
    { 0x1514131211100F0E, 0x1D1C1B1A19181716,
      0x2524232221201F1E, 0x2D2C2B2A29282726,
      0x3534333231302F2E, 0x3D3C3B3A39383736,
      0x4544434241403F3E, 0x4D4C4B4A49484746, },
    { 0x161514131211100F, 0x1E1D1C1B1A191817,
      0x262524232221201F, 0x2E2D2C2B2A292827,
      0x363534333231302F, 0x3E3D3C3B3A393837,
      0x464544434241403F, 0x4E4D4C4B4A494847, },
    { 0x1716151413121110, 0x1F1E1D1C1B1A1918,
      0x2726252423222120, 0x2F2E2D2C2B2A2928,
      0x3736353433323130, 0x3F3E3D3C3B3A3938,
      0x4746454443424140, 0x4F4E4D4C4B4A4948, },
    { 0x1817161514131211, 0x201F1E1D1C1B1A19,
      0x2827262524232221, 0x302F2E2D2C2B2A29,
      0x3837363534333231, 0x403F3E3D3C3B3A39,
      0x4847464544434241, 0x504F4E4D4C4B4A49, },
    { 0x1918171615141312, 0x21201F1E1D1C1B1A,
      0x2928272625242322, 0x31302F2E2D2C2B2A,
      0x3938373635343332, 0x41403F3E3D3C3B3A,
      0x4948474645444342, 0x51504F4E4D4C4B4A, },
    { 0x1A19181716151413, 0x2221201F1E1D1C1B,
      0x2A29282726252423, 0x3231302F2E2D2C2B,
      0x3A39383736353433, 0x4241403F3E3D3C3B,
      0x4A49484746454443, 0x5251504F4E4D4C4B, },
    { 0x1B1A191817161514, 0x232221201F1E1D1C,
      0x2B2A292827262524, 0x333231302F2E2D2C,
      0x3B3A393837363534, 0x434241403F3E3D3C,
      0x4B4A494847464544, 0x535251504F4E4D4C, },
    { 0x1C1B1A1918171615, 0x24232221201F1E1D,
      0x2C2B2A2928272625, 0x34333231302F2E2D,
      0x3C3B3A3938373635, 0x44434241403F3E3D,
      0x4C4B4A4948474645, 0x54535251504F4E4D, },
    { 0x1D1C1B1A19181716, 0x2524232221201F1E,
      0x2D2C2B2A29282726, 0x3534333231302F2E,
      0x3D3C3B3A39383736, 0x4544434241403F3E,
      0x4D4C4B4A49484746, 0x5554535251504F4E, },
    { 0x1E1D1C1B1A191817, 0x262524232221201F,
      0x2E2D2C2B2A292827, 0x363534333231302F,
      0x3E3D3C3B3A393837, 0x464544434241403F,
      0x4E4D4C4B4A494847, 0x565554535251504F, },
    { 0x1F1E1D1C1B1A1918, 0x2726252423222120,
      0x2F2E2D2C2B2A2928, 0x3736353433323130,
      0x3F3E3D3C3B3A3938, 0x4746454443424140,
      0x4F4E4D4C4B4A4948, 0x5756555453525150, },
    { 0x201F1E1D1C1B1A19, 0x2827262524232221,
      0x302F2E2D2C2B2A29, 0x3837363534333231,
      0x403F3E3D3C3B3A39, 0x4847464544434241,
      0x504F4E4D4C4B4A49, 0x5857565554535251, },
    { 0x21201F1E1D1C1B1A, 0x2928272625242322,
      0x31302F2E2D2C2B2A, 0x3938373635343332,
      0x41403F3E3D3C3B3A, 0x4948474645444342,
      0x51504F4E4D4C4B4A, 0x5958575655545352, },
    { 0x2221201F1E1D1C1B, 0x2A29282726252423,
      0x3231302F2E2D2C2B, 0x3A39383736353433,
      0x4241403F3E3D3C3B, 0x4A49484746454443,
      0x5251504F4E4D4C4B, 0x5A59585756555453, },
    { 0x232221201F1E1D1C, 0x2B2A292827262524,
      0x333231302F2E2D2C, 0x3B3A393837363534,
      0x434241403F3E3D3C, 0x4B4A494847464544,
      0x535251504F4E4D4C, 0x5B5A595857565554, },
    { 0x24232221201F1E1D, 0x2C2B2A2928272625,
      0x34333231302F2E2D, 0x3C3B3A3938373635,
      0x44434241403F3E3D, 0x4C4B4A4948474645,
      0x54535251504F4E4D, 0x5C5B5A5958575655, },
    { 0x2524232221201F1E, 0x2D2C2B2A29282726,
      0x3534333231302F2E, 0x3D3C3B3A39383736,
      0x4544434241403F3E, 0x4D4C4B4A49484746,
      0x5554535251504F4E, 0x5D5C5B5A59585756, },
    { 0x262524232221201F, 0x2E2D2C2B2A292827,
      0x363534333231302F, 0x3E3D3C3B3A393837,
      0x464544434241403F, 0x4E4D4C4B4A494847,
      0x565554535251504F, 0x5E5D5C5B5A595857, },
#endif
  };

  asm volatile(
    "vmovaps  0x000(%%rbx), %%zmm0\n\t"
    "vmovaps  0x040(%%rbx), %%zmm1\n\t"
    "vmovaps  0x080(%%rbx), %%zmm2\n\t"
    "vmovaps  0x0C0(%%rbx), %%zmm3\n\t"
    "vmovaps  0x100(%%rbx), %%zmm4\n\t"
    "vmovaps  0x140(%%rbx), %%zmm5\n\t"
    "vmovaps  0x180(%%rbx), %%zmm6\n\t"
    "vmovaps  0x1C0(%%rbx), %%zmm7\n\t"
#if defined(__x86_64__) || defined(_M_X64)
    "vmovaps  0x200(%%rbx), %%zmm8\n\t"
    "vmovaps  0x240(%%rbx), %%zmm9\n\t"
    "vmovaps  0x280(%%rbx), %%zmm10\n\t"
    "vmovaps  0x2C0(%%rbx), %%zmm11\n\t"
    "vmovaps  0x300(%%rbx), %%zmm12\n\t"
    "vmovaps  0x340(%%rbx), %%zmm13\n\t"
    "vmovaps  0x380(%%rbx), %%zmm14\n\t"
    "vmovaps  0x3C0(%%rbx), %%zmm15\n\t"
    "vmovaps  0x400(%%rbx), %%zmm16\n\t"
    "vmovaps  0x440(%%rbx), %%zmm17\n\t"
    "vmovaps  0x480(%%rbx), %%zmm18\n\t"
    "vmovaps  0x4C0(%%rbx), %%zmm19\n\t"
    "vmovaps  0x500(%%rbx), %%zmm20\n\t"
    "vmovaps  0x540(%%rbx), %%zmm21\n\t"
    "vmovaps  0x580(%%rbx), %%zmm22\n\t"
    "vmovaps  0x5C0(%%rbx), %%zmm23\n\t"
    "vmovaps  0x600(%%rbx), %%zmm24\n\t"
    "vmovaps  0x640(%%rbx), %%zmm25\n\t"
    "vmovaps  0x680(%%rbx), %%zmm26\n\t"
    "vmovaps  0x6C0(%%rbx), %%zmm27\n\t"
    "vmovaps  0x700(%%rbx), %%zmm28\n\t"
    "vmovaps  0x740(%%rbx), %%zmm29\n\t"
    "vmovaps  0x780(%%rbx), %%zmm30\n\t"
    "vmovaps  0x7C0(%%rbx), %%zmm31\n\t"
#endif
    "\n\t"
    "int3\n\t"
    :
    : "b"(zmm)
    : "%zmm0", "%zmm1", "%zmm2", "%zmm3", "%zmm4", "%zmm5", "%zmm6", "%zmm7"
#if defined(__x86_64__) || defined(_M_X64)
      , "%zmm8", "%zmm9", "%zmm10", "%zmm11", "%zmm12", "%zmm13", "%zmm14",
      "%zmm15", "%zmm16", "%zmm17", "%zmm18", "%zmm19", "%zmm20", "%zmm21",
      "%zmm22", "%zmm23", "%zmm24", "%zmm25", "%zmm26", "%zmm27", "%zmm28",
      "%zmm29", "%zmm30", "%zmm31"
#endif
  );

  return 0;
}
