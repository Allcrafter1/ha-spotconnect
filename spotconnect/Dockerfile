# syntax=docker/dockerfile:1
ARG BUILD_FROM=ghcr.io/home-assistant/base:3.23

FROM ${BUILD_FROM} AS downloader

ARG BUILD_ARCH
ARG SPOTCONNECT_VERSION=0.20.11
ARG SPOTCONNECT_SHA256=1895cd4952971f6b35b446c20f67d40dabb55f963d1bd382b566335ad4016313

RUN set -eu; \
    apk add --no-cache curl unzip; \
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
    chmod 0755 /out/spotraop /out/spotupnp

FROM ${BUILD_FROM}

ARG BUILD_ARCH
ARG BUILD_VERSION

COPY --from=downloader /out/spotraop /usr/bin/spotraop
COPY --from=downloader /out/spotupnp /usr/bin/spotupnp
COPY rootfs /

LABEL \
    io.hass.version="${BUILD_VERSION}" \
    io.hass.type="app" \
    io.hass.arch="${BUILD_ARCH}" \
    org.opencontainers.image.title="Home Assistant App: SpotConnect" \
    org.opencontainers.image.description="Bridge AirPlay and optional UPnP players to Spotify Connect" \
    org.opencontainers.image.source="https://github.com/Allcrafter1/ha-spotconnect" \
    org.opencontainers.image.licenses="GPL-3.0-or-later"
