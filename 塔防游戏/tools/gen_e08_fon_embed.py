import os


BASE = r"C:\Users\Lenovo\Desktop\week8code_qianrushi\老师发的文件\字库文件及汉字取模教程"
FILES = [
    ("GB2312_16.FON", "E08_GB2312_16_FON"),
    ("GB2312_32.FON", "E08_GB2312_32_FON"),
]

OUT_C = r"C:\Users\Lenovo\Desktop\week8code_qianrushi\E08_LTDC_QSPI_Font\Drivers\User\Src\e08_fon_embed.c"
OUT_H = r"C:\Users\Lenovo\Desktop\week8code_qianrushi\E08_LTDC_QSPI_Font\Drivers\User\Inc\e08_fon_embed.h"


def emit_array(data: bytes, name: str) -> str:
    lines = []
    for i in range(0, len(data), 12):
        chunk = data[i : i + 12]
        lines.append("    " + ", ".join(f"0x{b:02X}" for b in chunk) + ",")
    return f"const unsigned char {name}[{len(data)}] = {{\n" + "\n".join(lines) + "\n};\n"


def main() -> None:
    parts = []
    for fname, sym in FILES:
        path = os.path.join(BASE, fname)
        with open(path, "rb") as f:
            data = f.read()
        parts.append((sym, data))

    os.makedirs(os.path.dirname(OUT_C), exist_ok=True)
    os.makedirs(os.path.dirname(OUT_H), exist_ok=True)

    with open(OUT_H, "w", encoding="utf-8") as f:
        f.write("#ifndef E08_FON_EMBED_H\n#define E08_FON_EMBED_H\n\n#include <stdint.h>\n\n#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n")
        for sym, data in parts:
            f.write(f"extern const unsigned char {sym}[{len(data)}];\n")
            f.write(f"#define {sym}_SIZE ({len(data)}U)\n")
        f.write("\n#ifdef __cplusplus\n}\n#endif\n\n#endif\n")

    with open(OUT_C, "w", encoding="utf-8") as f:
        f.write("/* Auto-generated from GB2312_16/32.FON for no-SD fallback. */\n")
        f.write('#include "e08_fon_embed.h"\n\n')
        for sym, data in parts:
            f.write(emit_array(data, sym))

    print("generated:", OUT_C)
    print("generated:", OUT_H)


if __name__ == "__main__":
    main()

