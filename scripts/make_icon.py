#!/usr/bin/env python3
"""Generates the application icon.

The icon is drawn from code rather than shipped as an opaque binary so that it
can be reviewed, tweaked and regenerated. No third-party imaging dependency is
needed: PNG is written with zlib from the standard library (PLAN.md 45).

    python scripts/make_icon.py

Outputs:
    packaging/windows/PrivacyBrowser.ico   (16..256 px, PNG-compressed entries)
    ui/assets/icon.png                     (256 px, used as the window icon)
"""

from __future__ import annotations

import math
import struct
import zlib
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SIZES = (16, 32, 48, 64, 128, 256)
SUPERSAMPLE = 4

# Deep indigo to soft periwinkle: the same family as the UI accent colour.
TOP_COLOUR = (0x7C, 0x93, 0xFF)
BOTTOM_COLOUR = (0x35, 0x3F, 0xA8)


def shield_coverage(u: float, v: float) -> float:
    """Signed coverage of the shield shape at normalised coordinates.

    u runs -1..1 left to right, v runs 0..1 top to bottom.
    """
    if v < 0.0 or v > 1.0:
        return 0.0

    if v <= 0.58:
        half_width = 0.78
        # Round the top corners.
        corner = 0.22
        if v < corner:
            t = (corner - v) / corner
            half_width *= math.sqrt(max(0.0, 1.0 - t * t))
    else:
        t = (v - 0.58) / 0.42
        half_width = 0.78 * math.sqrt(max(0.0, 1.0 - t * t))

    return 1.0 if abs(u) <= half_width else 0.0


def keyhole_coverage(u: float, v: float) -> float:
    """A small keyhole punched out of the shield."""
    # v is scaled twice as far as u in pixels, so the vertical term is doubled
    # to keep the bore of the keyhole circular on screen.
    circle = (u * u + ((v - 0.40) * 2.0) ** 2) <= 0.0256
    stem = abs(u) <= 0.055 and 0.40 <= v <= 0.58
    return 1.0 if (circle or stem) else 0.0


def render(size: int) -> bytes:
    """Returns raw RGBA rows for one square icon."""
    scale = size * SUPERSAMPLE
    rows = []
    for y in range(size):
        row = bytearray()
        for x in range(size):
            alpha_total = 0.0
            colour_total = [0.0, 0.0, 0.0]
            for sy in range(SUPERSAMPLE):
                for sx in range(SUPERSAMPLE):
                    px = (x * SUPERSAMPLE + sx + 0.5) / scale
                    py = (y * SUPERSAMPLE + sy + 0.5) / scale
                    # Inset the shape slightly so it is not flush with the edge.
                    u = (px - 0.5) / 0.44
                    v = (py - 0.06) / 0.88
                    coverage = shield_coverage(u, v)
                    if coverage == 0.0:
                        continue
                    if keyhole_coverage(u, v) > 0.0:
                        continue
                    blend = min(1.0, max(0.0, v))
                    colour = [
                        TOP_COLOUR[i] + (BOTTOM_COLOUR[i] - TOP_COLOUR[i]) * blend
                        for i in range(3)
                    ]
                    # Soft highlight along the top-left edge.
                    highlight = max(0.0, 1.0 - (u + 1.2) ** 2 - (v * 1.6) ** 2)
                    colour = [min(255.0, c + 70.0 * highlight) for c in colour]
                    alpha_total += 1.0
                    for i in range(3):
                        colour_total[i] += colour[i]

            samples = SUPERSAMPLE * SUPERSAMPLE
            alpha = alpha_total / samples
            if alpha <= 0.0:
                row += bytes((0, 0, 0, 0))
            else:
                r, g, b = (int(round(c / alpha_total)) for c in colour_total)
                row += bytes((r, g, b, int(round(alpha * 255))))
        rows.append(bytes(row))
    return b"".join(b"\x00" + row for row in rows)


def png(size: int, raw: bytes) -> bytes:
    def chunk(tag: bytes, payload: bytes) -> bytes:
        return (
            struct.pack(">I", len(payload))
            + tag
            + payload
            + struct.pack(">I", zlib.crc32(tag + payload) & 0xFFFFFFFF)
        )

    header = struct.pack(">IIBBBBB", size, size, 8, 6, 0, 0, 0)
    return (
        b"\x89PNG\r\n\x1a\n"
        + chunk(b"IHDR", header)
        + chunk(b"IDAT", zlib.compress(raw, 9))
        + chunk(b"IEND", b"")
    )


def ico(images: list[tuple[int, bytes]]) -> bytes:
    header = struct.pack("<HHH", 0, 1, len(images))
    offset = len(header) + 16 * len(images)
    directory = b""
    payload = b""
    for size, data in images:
        directory += struct.pack(
            "<BBBBHHII",
            0 if size >= 256 else size,
            0 if size >= 256 else size,
            0,
            0,
            1,
            32,
            len(data),
            offset,
        )
        payload += data
        offset += len(data)
    return header + directory + payload


def main() -> None:
    images = []
    for size in SIZES:
        data = png(size, render(size))
        images.append((size, data))
        if size == 256:
            target = ROOT / "ui" / "assets" / "icon.png"
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(data)
            print(f"wrote {target.relative_to(ROOT)}")

    target = ROOT / "packaging" / "windows" / "PrivacyBrowser.ico"
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_bytes(ico(images))
    print(f"wrote {target.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
