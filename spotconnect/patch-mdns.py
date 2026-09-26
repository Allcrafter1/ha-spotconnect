#!/usr/bin/env python3
"""Switch SpotConnect's RAOP browser to unicast mDNS discovery.

AirPlay bridges such as AirCast can advertise receivers from the same host.
Their multicast announcements are not looped back to SpotConnect.  Binding the
browser to an ephemeral source port and setting the DNS-SD unicast-response bit
makes mDNS responders send a unicast reply.  This works for same-host bridges
and regular network receivers.

The replacements deliberately include adjacent instructions and must match
exactly once.  An upstream binary layout change therefore fails the image build
instead of silently patching the wrong code.
"""

from __future__ import annotations

import argparse
from pathlib import Path


PATCHES = {
    "amd64": (
        (
            bytes.fromhex("ba0100000031ffe8"),
            bytes.fromhex("ba0000000031ffe8"),
        ),
        (
            bytes.fromhex("bec2beb400f6d819c94531c931d283e13c"),
            bytes.fromhex("bec2beb400f6d819c94531c9b20183e13c"),
        ),
    ),
    "aarch64": (
        (
            bytes.fromhex("c1d243b92200805200008052"),
            bytes.fromhex("c1d243b90200805200008052"),
        ),
        (
            bytes.fromhex("004045f93f0000710200805283078052"),
            bytes.fromhex("004045f93f0000712200805283078052"),
        ),
    ),
}


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("architecture", choices=PATCHES)
    parser.add_argument("binary", type=Path)
    args = parser.parse_args()

    data = args.binary.read_bytes()
    for index, (original, replacement) in enumerate(
        PATCHES[args.architecture], start=1
    ):
        matches = data.count(original)
        already_patched = data.count(replacement)

        if matches != 1 or already_patched != 0:
            raise SystemExit(
                f"Refusing mDNS compatibility patch {index}: expected one "
                f"original pattern and no patched pattern, found "
                f"original={matches}, patched={already_patched} in "
                f"{args.binary}"
            )

        data = data.replace(original, replacement, 1)

    args.binary.write_bytes(data)


if __name__ == "__main__":
    main()
