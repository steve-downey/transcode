import argparse
import json
import struct
from pathlib import Path


def generate_binary_indices(json_path: Path, output_dir: Path):
    with open(json_path, encoding="utf-8") as f:
        indexes = json.load(f)

    output_dir.mkdir(parents=True, exist_ok=True)

    for index_name, data in indexes.items():
        # Clean up filename (e.g., "shift_jis" to "shift_jis.bin")
        safe_name = index_name.replace("-", "_").lower()
        out_file = output_dir / f"{safe_name}.bin"

        # WPT indexes generally map integers to code points or nulls.
        # We must decide on a byte width. Most WHATWG indices fit in 16-bit (uint16_t)
        # Some, like gb18030, might require 32-bit depending on how you structure the offset.
        # Let's assume uint16_t (H) for standard indices. Use 0xFFFF for null/None.

        with open(out_file, "wb") as bin_out:
            for entry in data:
                if entry is None:
                    # 0xFFFF acts as our sentinel for "no mapping"
                    bin_out.write(struct.pack("<H", 0xFFFF))
                else:
                    bin_out.write(struct.pack("<H", entry))

    print(f"Successfully generated binary indices in {output_dir}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Convert WPT indexes.json to binary files for C++26 #embed"
    )
    parser.add_argument("input", type=Path, help="Path to WPT indexes.json")
    parser.add_argument("outdir", type=Path, help="Output directory for .bin files")
    args = parser.parse_args()

    generate_binary_indices(args.input, args.outdir)
