#!/usr/bin/env python3
"""Make SpotConnect's RAOP browser compatible with same-host bridges.

AirPlay bridges such as AirCast can advertise receivers from the same host.
Their multicast announcements are not looped back to SpotConnect.  Binding the
browser to an ephemeral source port and setting the DNS-SD unicast-response bit
makes mDNS responders send a unicast reply.  This works for same-host bridges
and regular network receivers.

SpotConnect normally rejects same-subnet announcements made on behalf of a
different address.  That is a useful default for physical receivers, but it
also rejects the legitimate proxy announcements emitted by AirCast/RCast.
The third patch accepts those advertisements after the local-only relay has
delivered them to SpotConnect.

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
        (
            bytes.fromhex(
                "418b4708418b572039d0740e31d023057e12a4000f8423ffffff"
            ),
            bytes.fromhex(
                "418b4708418b572039d0eb0e31d023057e12a4000f8423ffffff"
            ),
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
        (
            bytes.fromhex(
                "400b40b9412340b91f00016bc0000054024b00b00000014a"
                "41484ab91f00016ac0f6ff54"
            ),
            bytes.fromhex(
                "400b40b9412340b91f00016b06000014024b00b00000014a"
                "41484ab91f00016ac0f6ff54"
            ),
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
