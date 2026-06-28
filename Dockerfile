FROM ghcr.io/wiiu-env/devkitppc:20260126 AS final

COPY --from=ghcr.io/wiiu-env/libmocha:20260126 /artifacts $DEVKITPRO
COPY --from=ghcr.io/stroopwafelcfw/libstroopwafel:20260202 /artifacts $DEVKITPRO


WORKDIR project
