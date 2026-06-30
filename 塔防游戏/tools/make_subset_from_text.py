import argparse
import subprocess
import sys


def text_to_gb2312_codes(text: str):
    b = text.encode("gb2312")
    if len(b) % 2 != 0:
        raise ValueError("GB2312 byte length must be even")
    codes = []
    for i in range(0, len(b), 2):
        codes.append((b[i] << 8) | b[i + 1])
    return codes


def main() -> None:
    ap = argparse.ArgumentParser(description="Append GB2312 codes from Chinese text and regenerate e08_fon_subset.c/h.")
    ap.add_argument("--text", required=True, help="Chinese text (GB2312 supported chars).")
    args = ap.parse_args()

    # compute codes
    codes = text_to_gb2312_codes(args.text)
    add_set = set(codes)

    # patch gen_e08_fon_subset.py in-place (simple: append codes if missing)
    gen_path = subprocess.check_output(
        [sys.executable, "-c", "import os;print(os.path.abspath(os.path.join(os.path.dirname(__file__),'gen_e08_fon_subset.py')))",],
        cwd=os.path.dirname(__file__),
        text=True,
    ).strip()

    src = open(gen_path, "r", encoding="utf-8").read().splitlines(True)
    out = []
    in_list = False
    existing = set()
    for line in src:
        if line.strip().startswith("GB2312_CODES") and "=" in line:
            in_list = True
            out.append(line)
            continue
        if in_list:
            if "]" in line:
                # insert new codes before closing bracket
                missing = sorted(add_set - existing)
                if missing:
                    out.append("\n    # Added by make_subset_from_text.py\n")
                    for c in missing:
                        out.append(f"    0x{c:04X},\n")
                in_list = False
                out.append(line)
                continue
            # collect existing codes
            s = line.strip().rstrip(",")
            if s.startswith("0x"):
                try:
                    existing.add(int(s, 16))
                except Exception:
                    pass
        out.append(line)

    open(gen_path, "w", encoding="utf-8").write("".join(out))

    # run generator
    subprocess.check_call([sys.executable, "gen_e08_fon_subset.py"], cwd=os.path.dirname(__file__))
    print("OK: updated codes and regenerated subset.")


if __name__ == "__main__":
    main()

