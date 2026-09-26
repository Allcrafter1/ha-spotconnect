# syntax=docker/dockerfile:1
ARG BUILD_FROM=ghcr.io/home-assistant/base:3.23

FROM ${BUILD_FROM} AS downloader

ARG BUILD_ARCH
ARG SPOTCONNECT_VERSION=0.20.8
ARG SPOTCONNECT_SHA256=fad415ea437365bc1dd639d9255ec141a1d59c6e69f0f01a614cfa53e164f171

COPY patch-mdns.py mdns-relay.c /usr/local/src/

RUN set -eu; \
    apk add --no-cache build-base curl python3 unzip; \
    case "${BUILD_ARCH}" in \
      amd64) release_arch="x86_64" ;; \
      aarch64) release_arch="aarch64" ;; \
      *) echo "Unsupported architecture: ${BUILD_ARCH}" >&2; exit 1 ;; \
    esac; \
    archive="/tmp/SpotConnect-${SPOTCONNECT_VERSION}.zip"; \
    curl --fail --location --retry 3 \
      --output "${archive}" \
      "https://github.com/philippe44/SpotConnect/releases/download/${SPOTCONNECT_VERSION}/SpotConnect-${SPOTCONNECT_VERSION}.zip"; \
    echo "${SPOTCONNECT_SHA256}  ${archive}" | sha256sum -c -; \
    mkdir -p /out; \
    unzip -j "${archive}" \
      "spotraop-linux-${release_arch}-static" \
      "spotupnp-linux-${release_arch}-static" \
      -d /out; \
    mv "/out/spotraop-linux-${release_arch}-static" /out/spotraop; \
    mv "/out/spotupnp-linux-${release_arch}-static" /out/spotupnp; \
    python3 /usr/local/src/patch-mdns.py "${BUILD_ARCH}" /out/spotraop; \
    cc -Os -Wall -Wextra -Werror \
      -o /out/mdns-relay /usr/local/src/mdns-relay.c; \
    chmod 0755 /out/mdns-relay /out/spotraop /out/spotupnp

FROM ${BUILD_FROM}

ARG BUILD_ARCH
ARG BUILD_VERSION

COPY --from=downloader /out/spotraop /usr/bin/spotraop
COPY --from=downloader /out/spotupnp /usr/bin/spotupnp
COPY --from=downloader /out/mdns-relay /usr/bin/mdns-relay
COPY rootfs /

LABEL \
    io.hass.version="${BUILD_VERSION}" \
    io.hass.type="app" \
    io.hass.arch="${BUILD_ARCH}" \
    org.opencontainers.image.title="Home Assistant App: SpotConnect" \
    org.opencontainers.image.description="Bridge AirPlay and optional UPnP players to Spotify Connect" \
    org.opencontainers.image.source="https://github.com/Allcrafter1/ha-spotconnect" \
    org.opencontainers.image.licenses="GPL-3.0-or-later"
