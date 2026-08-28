# syntax=docker/dockerfile:1.7
#
# Commit-image Dockerfile for the FreeRTOS-Plus-TCP Mayhem integration. Builds on the org C/C++
# base image (clang/llvm + libFuzzer + the build-contract ENV). Everything the build needs beyond
# the base is installed here (ruby/unifdef/ninja for the upstream CTest suite) then the build runs
# as the non-root `mayhem` user via mayhem/build.sh.
#
# Build (from the repo root):
#   DOCKER_BUILDKIT=1 docker build -f mayhem/Dockerfile \
#       --build-arg COMMIT=$(git rev-parse HEAD) -t freertos-commit:test .

FROM ghcr.io/savantenvs/base:20260621

ARG COMMIT
ARG REPO_URL
# SOURCE_DATE_EPOCH stays an ARG (never an ENV): an empty ENV breaks clang and any in-image rebuild.
ARG SOURCE_DATE_EPOCH
# Sanitizer override (undefaulted so the base ENV flows through unless --build-arg overrides it).
ARG SANITIZER_FLAGS
ARG COVERAGE_FLAGS=
ARG MAYHEM_JOBS=

# The upstream unit tests (test/unit-test) are generated with CMock (ruby) + unifdef and built with
# ninja. Install these as root before dropping to the mayhem user for the build.
USER root
RUN [ -n "$SOURCE_DATE_EPOCH" ] && repro-sources-list.sh || true ; \
    apt-get update && apt-get install -yq --no-install-recommends \
        ruby unifdef ninja-build \
    && rm -rf /var/lib/apt/lists/*

USER mayhem
WORKDIR /mayhem
COPY --chown=2000:2000 . /mayhem

RUN bash mayhem/build.sh

# Run the full upstream unit-test suite at build time; a failing suite fails the image build.
RUN if [ -f mayhem/test.sh ]; then bash mayhem/test.sh; else echo "no mayhem/test.sh"; fi

LABEL org.opencontainers.image.source="${REPO_URL}" \
      org.opencontainers.image.revision="${COMMIT}" \
      rlenv.tier="commit" \
      rlenv.source_date_epoch="${SOURCE_DATE_EPOCH}" \
      rlenv.user="mayhem"

USER mayhem
WORKDIR /mayhem
