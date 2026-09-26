# SpotConnect for Home Assistant

A Home Assistant app repository that turns AirPlay receivers into Spotify
Connect devices using [SpotConnect](https://github.com/philippe44/SpotConnect).
UPnP/DLNA renderers can optionally be enabled in the same app.

## Installation

1. In Home Assistant, open **Settings → Apps → App store**.
2. Open the repository menu and add:

   ```text
   https://github.com/Allcrafter1/ha-spotconnect
   ```

3. Install **SpotConnect**.
4. Keep AirPlay enabled, adjust any optional settings, and start the app.
5. Open Spotify's device picker. Discovered receivers use a trailing `+` by
   default—for example, `Living Room+`.

The app supports Home Assistant OS and Home Assistant Supervised on `amd64`
and `aarch64`. Spotify Premium is required by SpotConnect.

## Warum eine gemeinsame App?

AirPlay ist standardmäßig aktiv und bleibt der primäre Anwendungsfall. Der
UPnP-/DLNA-Prozess nutzt dieselbe Upstream-Version und nur einen kleinen Teil
zusätzlicher Startlogik. Deshalb lässt er sich optional in derselben App
aktivieren, ohne ein zweites Paket oder Image pflegen zu müssen.

## Design

- Uses host networking, as required by SpotConnect for multicast discovery and
  its per-player audio web server.
- Downloads the official, pinned SpotConnect release during the container
  build and verifies the release archive with SHA-256.
- Runs the unmodified, checksum-verified upstream SpotConnect executables.
- Publishes pre-built, signed multi-architecture images to GitHub Container
  Registry with Home Assistant's maintained builder actions.
- Stores runtime configuration and reusable device credentials only in Home
  Assistant's private `/data` volume.
- Does not include Spotify, GitHub, or Home Assistant credentials in source or
  images.

Detailed options and troubleshooting are in
[spotconnect/DOCS.md](spotconnect/DOCS.md).

## Updates and maintenance

The repository checks GitHub's latest SpotConnect release every day. When a
new stable version appears, the workflow verifies that the expected release
archive exists, calculates its SHA-256 checksum, and opens a pull request. It
then runs the Home Assistant linter and builds both `amd64` and `aarch64`
images from that branch. A fully successful update is merged automatically;
the merge publishes the multi-architecture image and Home Assistant sees the
new app version.

If downloading, validation, building, PR creation, or merging fails,
the update is not merged. The workflow leaves the PR open where applicable and
creates a GitHub issue assigned to the repository owner. That assignment
creates a personal GitHub notification; email delivery still follows the
owner's GitHub notification settings. Failed workflow runs are also visible in
the repository's Actions tab.

Home Assistant offers the published version as a normal app update. To also
install it without interaction, enable **Automatic updates** on the installed
SpotConnect app. Dependabot separately opens pull requests for GitHub Actions
dependencies; those are intentionally not auto-merged with SpotConnect
releases.

## Development

Validate the shell scripts and repository metadata locally:

```bash
./tests/validate.sh
```

Every runtime change must also increment `spotconnect/config.yaml`'s app
version. The final numeric suffix is the packaging revision, so a packaging-only
change after `0.20.7-1` becomes `0.20.7-2`.

## License and upstream

SpotConnect is developed by Philippe G. The bundled executables report the
GPL-3.0-or-later terms via `spotraop -t` and `spotupnp -t`; corresponding source
for the pinned version is available from the
[SpotConnect 0.20.8 tag](https://github.com/philippe44/SpotConnect/tree/0.20.8).
This repository contains packaging and supervision code rather than a fork of
SpotConnect.
