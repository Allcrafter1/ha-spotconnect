# Changelog

## 0.20.8-8

- Make debug diagnostics report the RAOP browser port and only summarize a
  complete AirCast response instead of a short goodbye record.

## 0.20.8-7

- Accept legitimate same-subnet proxy announcements emitted by AirCast/RCast.
  The architecture-specific compatibility patch is exact-match validated and
  will fail safely if a future upstream binary changes.

## 0.20.8-6

- Preserve the local source address when relaying same-host mDNS responses so
  SpotConnect accepts announcements from AirCast/RCast instead of treating
  them as proxy announcements.
- Keep debug diagnostics concise and avoid logging complete mDNS packets.

## 0.20.8-5

- Log one captured RAOP response at debug level to make same-host mDNS
  compatibility problems diagnosable.

## 0.20.8-4

- Relay same-host multicast mDNS responses to SpotConnect's RAOP browser as a
  fallback for AirCast responders that do not honor unicast response requests.

## 0.20.8-3

- Request explicit unicast mDNS responses so same-host AirCast receivers are
  discoverable even with AirCast's minimal mDNS responder.

## 0.20.8-2

- Add Home Assistant icon and logo artwork.
- Discover same-host AirPlay bridges such as AirCast through legacy-unicast
  mDNS queries.
- Document the interaction with AirCast and Cast Audio Receiver Lab.

## 0.20.8-1

- Update SpotConnect from 0.20.7 to 0.20.8.

## 0.20.7-2

- Remove redundant Supervisor defaults from the app configuration.

## 0.20.7-1

- Initial Home Assistant app release.
- Package SpotConnect 0.20.7 for `amd64` and `aarch64`.
- Enable AirPlay by default and provide optional parallel UPnP/DLNA support.
- Persist configuration and reusable Spotify device credentials in app data.
