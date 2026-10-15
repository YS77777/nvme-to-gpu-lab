#!/usr/bin/env python3
"""Generate a real, deterministic binary file for correctness testing."""

from __future__ import annotations

import argparse
import os
from pathlib import Path


def parse_size(text: str) -> int:
    normalized = text.strip().lower()
    suffixes = (
        ("kib", 1024),
        ("kb", 1024),
        ("k", 1024),
        ("mib", 1024**2),
        ("mb", 1024**2),
        ("m", 1024**2),
        ("gib", 1024**3),
        ("gb", 1024**3),
        ("g", 1024**3),
        ("b", 1),
    )
    for suffix, multiplier in suffixes:
        if normalized.endswith(suffix):
            number = normalized[: -len(suffix)]
            break
    else:
        number = normalized
        multiplier = 1
    if not number.isdigit():
        raise argparse.ArgumentTypeError(f"invalid size: {text}")
    return int(number) * multiplier


def pattern_block(offset: int, length: int) -> bytes:
    if offset < 0 or length < 0:
        raise ValueError("offset and length must be non-negative")
    period = bytes(range(251))
    start = offset % len(period)
    rotated = period[start:] + period[:start]
    repetitions = (length + len(rotated) - 1) // len(rotated)
    return (rotated * repetitions)[:length]


def generate(path: Path, size: int, chunk_size: int, sync: bool = False) -> None:
    if chunk_size <= 0:
        raise ValueError("chunk size must be greater than zero")
    path.parent.mkdir(parents=True, exist_ok=True)
    written = 0
    with path.open("wb") as output:
        while written < size:
            length = min(chunk_size, size - written)
            output.write(pattern_block(written, length))
            written += length
        output.flush()
        if sync:
            os.fsync(output.fileno())


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--size", type=parse_size, default=parse_size("256MiB"))
    parser.add_argument(
        "--chunk-size", type=parse_size, default=parse_size("4MiB")
    )
    parser.add_argument(
        "--fsync", action="store_true", help="flush file data before returning"
    )
    args = parser.parse_args()
    generate(args.output, args.size, args.chunk_size, args.fsync)
    print(f"generated {args.size} bytes at {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
