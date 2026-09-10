# One toolchain for attendees and CI: GCC 14, CMake, Ninja, TBB, marp-cli.
FROM gcc:14
RUN apt-get update && apt-get install -y --no-install-recommends \
      cmake ninja-build libtbb-dev nodejs npm chromium && \
    npm install -g @marp-team/marp-cli && \
    rm -rf /var/lib/apt/lists/*
ENV CHROME_PATH=/usr/bin/chromium
WORKDIR /work
CMD ["bash"]
