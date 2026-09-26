# SpotConnect

SpotConnect makes AirPlay receivers available in Spotify's device picker. An
optional UPnP/DLNA bridge can run at the same time.

## Quick start

1. Keep **Enable AirPlay bridge** enabled.
2. Start the app.
3. Wait up to 30 seconds for network discovery.
4. Open Spotify on a device in the same network and select the new device. By
   default, its name is the AirPlay receiver's name followed by `+`.

Spotify Premium is required by SpotConnect.

## Configuration

- **Network interface or IP address**: normally leave this empty. If the app
  binds to the wrong interface, enter the Home Assistant LAN interface (for
  example `enp6s18`) or its LAN IPv4 address.
- **Spotify bitrate**: `320` gives the highest quality. Lower values reduce
  network and CPU use.
- **Device name format**: must include `%s`, which SpotConnect replaces with
  the receiver name.
- **Store reusable Spotify credentials**: recommended. Credentials are kept in
  private, persistent app storage and are not part of backups of this
  repository or container images.
- **AirPlay output codec**: `alac` is the recommended default.
- **Enable UPnP bridge**: also exposes UPnP/DLNA renderers. The default `flc`
  codec provides lossless output and works with many renderers.

## Troubleshooting

Discovery uses the host network because multicast discovery and SpotConnect's
audio web server cannot work through Docker NAT. Home Assistant and the target
receiver must be able to reach each other directly.

If no receiver appears:

1. Check the app log for `adding player` or discovery messages.
2. Wait at least 30 seconds, then reopen Spotify's device picker.
3. Set **Network interface or IP address** to Home Assistant's LAN address.
4. Verify that multicast/mDNS is not blocked between VLANs or Wi-Fi clients.

### AirCast and Cast Audio Receiver Lab

AirCast turns Google Cast receivers into virtual AirPlay receivers. SpotConnect
upstream ignores proxy announcements when AirCast and SpotConnect run on the
same host. This repository deliberately keeps the upstream executable
unmodified, so that same-host conversion path is not supported.

Cast Audio Receiver Lab has no direct port or process conflict with
SpotConnect. It can run at the same time, but its Cast receivers only reach
SpotConnect through AirCast's unsupported same-host proxy path. Native AirPlay
receivers and AirPlay services visible from another network host are unaffected.

Apple TV devices may require SpotConnect's interactive pairing procedure. The
Home Assistant app does not currently automate that procedure; ordinary
AirPlay speakers and software receivers do not need it.

## Privacy and credentials

The app does not require your Spotify username or password in its
configuration. SpotConnect normally receives reusable device credentials
through Spotify's local ZeroConf flow after first use. Those files remain in
Home Assistant's private app data directory.

## Support

For packaging and Home Assistant issues, use this repository's issue tracker.
For SpotConnect engine behavior, consult the
[upstream project](https://github.com/philippe44/SpotConnect).
