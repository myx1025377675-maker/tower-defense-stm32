import os
from typing import List, Tuple


ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
FON_16 = os.path.join(ROOT, "fonts", "GB2312_16.FON")
FON_32 = os.path.join(ROOT, "fonts", "GB2312_32.FON")

OUT_C = os.path.join(ROOT, "Drivers", "User", "Src", "e08_fon_subset.c")
OUT_H = os.path.join(ROOT, "Drivers", "User", "Inc", "e08_fon_subset.h")


# GB2312 codes used by current UI:
GB2312_CODES: List[int] = [
    # 厦门大学
    0xCFC3, 0xC3C5, 0xB4F3, 0xD1A7,
    # 集成电路设计与集成系统
    0xBCAF, 0xB3C9, 0xB5E7, 0xC2B7, 0xC9E8, 0xBCC6, 0xD3EB, 0xCFB5, 0xCDB3,
    # 陈卓阳
    0xB3C2, 0xD7BF, 0xD1F4,
    # 校名/专业/姓名
    0xD0A3, 0xC3FB, 0xD7A8, 0xD2B5, 0xD0D5,

    # Added by make_subset_from_text.py
    0xB3C9,
    0xB4F3,
    0xB5E7,
    0xBAC5,
    0xBCAF,
    0xBCB6,
    0xBCC6,
    0xC2B7,
    0xC2ED,
    0xC3C5,
    0xC3FB,
    0xC4EA,
    0xC8FD,
    0xC9E8,
    0xCDB3,
    0xCFB5,
    0xCFC3,
    0xCFE8,
    0xD0A3,
    0xD0D5,
    0xD1A7,
    0xD2B5,
    0xD3EB,
    0xD4BE,
    0xD7A8,
]


def glyph_offset(code: int, bytes_per_char: int) -> int:
    gb_h = (code >> 8) & 0xFF
    gb_l = code & 0xFF
    idx = (gb_h - 0xA1) * 94 + (gb_l - 0xA1)
    return idx * bytes_per_char


def read_glyphs(path: str, bytes_per_char: int, codes: List[int]) -> List[Tuple[int, bytes]]:
    with open(path, "rb") as f:
        data = f.read()
    out: List[Tuple[int, bytes]] = []
    for c in sorted(set(codes)):
        off = glyph_offset(c, bytes_per_char)
        if off < 0 or off + bytes_per_char > len(data):
            raise RuntimeError(f"code 0x{c:04X} out of range for {os.path.basename(path)}")
        out.append((c, data[off : off + bytes_per_char]))
    return out


def emit_table(name: str, glyphs: List[Tuple[int, bytes]], bytes_per_char: int) -> str:
    lines = [f"const E08_Glyph16 {name}[{len(glyphs)}] = {{"] if bytes_per_char == 32 else [f"const E08_Glyph32 {name}[{len(glyphs)}] = {{"]
    for code, blob in glyphs:
        lines.append(f"  {{ 0x{code:04X}, {{")
        # 12 bytes per line for readability
        for i in range(0, len(blob), 12):
            chunk = blob[i : i + 12]
            lines.append("    " + ", ".join(f"0x{b:02X}" for b in chunk) + ",")
        lines.append("  } },")
    lines.append("};")
    return "\n".join(lines) + "\n"


def main() -> None:
    if not os.path.exists(FON_16) or not os.path.exists(FON_32):
        raise SystemExit(
            "Missing FON files. Put them under: "
            + os.path.join(ROOT, "fonts")
            + " (GB2312_16.FON and GB2312_32.FON)"
        )
    g16 = read_glyphs(FON_16, 32, GB2312_CODES)
    g32 = read_glyphs(FON_32, 128, GB2312_CODES)

    os.makedirs(os.path.dirname(OUT_C), exist_ok=True)
    os.makedirs(os.path.dirname(OUT_H), exist_ok=True)

    with open(OUT_H, "w", encoding="utf-8") as f:
        f.write(
            "#ifndef E08_FON_SUBSET_H\n#define E08_FON_SUBSET_H\n\n#include <stdint.h>\n\n#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n"
            "typedef struct { uint16_t code; uint8_t data[32]; } E08_Glyph16;\n"
            "typedef struct { uint16_t code; uint8_t data[128]; } E08_Glyph32;\n\n"
            "extern const E08_Glyph16 E08_GLYPHS_16[];\nextern const uint32_t E08_GLYPHS_16_COUNT;\n"
            "extern const E08_Glyph32 E08_GLYPHS_32[];\nextern const uint32_t E08_GLYPHS_32_COUNT;\n\n"
            "const uint8_t *E08_FindGlyph16(uint16_t code);\n"
            "const uint8_t *E08_FindGlyph32(uint16_t code);\n\n"
            "#ifdef __cplusplus\n}\n#endif\n\n#endif\n"
        )

    with open(OUT_C, "w", encoding="utf-8") as f:
        f.write("/* Auto-generated: minimal GB2312 glyph subset (no SD/QSPI needed). */\n")
        f.write('#include "e08_fon_subset.h"\n\n')
        f.write(emit_table("E08_GLYPHS_16", g16, 32))
        f.write("const uint32_t E08_GLYPHS_16_COUNT = (uint32_t)(sizeof(E08_GLYPHS_16)/sizeof(E08_GLYPHS_16[0]));\n\n")
        f.write(emit_table("E08_GLYPHS_32", g32, 128))
        f.write("const uint32_t E08_GLYPHS_32_COUNT = (uint32_t)(sizeof(E08_GLYPHS_32)/sizeof(E08_GLYPHS_32[0]));\n\n")
        f.write(
            "const uint8_t *E08_FindGlyph16(uint16_t code) {\n"
            "  uint32_t i;\n"
            "  for (i = 0; i < E08_GLYPHS_16_COUNT; i++) if (E08_GLYPHS_16[i].code == code) return E08_GLYPHS_16[i].data;\n"
            "  return 0;\n}\n\n"
            "const uint8_t *E08_FindGlyph32(uint16_t code) {\n"
            "  uint32_t i;\n"
            "  for (i = 0; i < E08_GLYPHS_32_COUNT; i++) if (E08_GLYPHS_32[i].code == code) return E08_GLYPHS_32[i].data;\n"
            "  return 0;\n}\n"
        )

    print("generated:", OUT_C)
    print("generated:", OUT_H)


if __name__ == "__main__":
    main()

