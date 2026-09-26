#!/usr/bin/env python3
"""Switch SpotConnect's RAOP browser to legacy-unicast mDNS queries.

AirPlay bridges such as AirCast can advertise receivers from the same host.
Their multicast announcements are not looped back to SpotConnect.  Binding the
browser to an ephemeral source port makes mDNS responders send a unicast reply,
which works for same-host bridges and regular network receivers.

The replacements deliberately include adjacent instructions and must match
exactly once.  An upstream binary layout change therefore fails the image build
instead of silently patching the wrong code.
"""

from __future__ import annotations

import argparse
from pathlib import Path


PATCHES = {
    "amd64": (
        bytes.fromhex("ba0100000031ffe8"),
        bytes.fromhex("ba0000000031ffe8"),
    ),
    "aarch64": (
        bytes.fromhex("c1d243b92200805200008052"),
        bytes.fromhex("c1d243b90200805200008052"),
    ),
}


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("architecture", choices=PATCHES)
    parser.add_argument("binary", type=Path)
    args = parser.parse_args()

    original, replacement = PATCHES[args.architecture]
    data = args.binary.read_bytes()
    matches = data.count(original)
    already_patched = data.count(replacement)

    if matches != 1 or already_patched != 0:
        raise SystemExit(
            "Refusing mDNS compatibility patch: expected one original pattern "
            f"and no patched pattern, found original={matches}, "
            f"patched={already_patched} in {args.binary}"
        )

    args.binary.write_bytes(data.replace(original, replacement, 1))


if __name__ == "__main__":
    main()
