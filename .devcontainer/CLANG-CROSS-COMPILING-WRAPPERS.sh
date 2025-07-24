#!/bin/bash

set -eux; \
mkdir -p /usr/local/bin; \
echo '#!/bin/sh' > /usr/local/bin/i686-linux-gnu-clang; \
echo 'exec clang --target=i686-linux-gnu --sysroot=/usr/i686-linux-gnu "$@"' >> /usr/local/bin/i686-linux-gnu-clang; \
chmod +x /usr/local/bin/i686-linux-gnu-clang; \
\
echo '#!/bin/sh' > /usr/local/bin/i686-linux-gnu-clang++; \
echo 'exec clang++ --target=i686-linux-gnu --sysroot=/usr/i686-linux-gnu "$@"' >> /usr/local/bin/i686-linux-gnu-clang++; \
chmod +x /usr/local/bin/i686-linux-gnu-clang++; \
\
echo '#!/bin/sh' > /usr/local/bin/arm-linux-gnueabihf-clang; \
echo 'exec clang --target=arm-linux-gnueabihf --sysroot=/usr/arm-linux-gnueabihf "$@"' >> /usr/local/bin/arm-linux-gnueabihf-clang; \
chmod +x /usr/local/bin/arm-linux-gnueabihf-clang; \
\
echo '#!/bin/sh' > /usr/local/bin/arm-linux-gnueabihf-clang++; \
echo 'exec clang++ --target=arm-linux-gnueabihf --sysroot=/usr/arm-linux-gnueabihf "$@"' >> /usr/local/bin/arm-linux-gnueabihf-clang++; \
chmod +x /usr/local/bin/arm-linux-gnueabihf-clang++; \
\
echo '#!/bin/sh' > /usr/local/bin/aarch64-linux-gnu-clang; \
echo 'exec clang --target=aarch64-linux-gnu --sysroot=/usr/aarch64-linux-gnu "$@"' >> /usr/local/bin/aarch64-linux-gnu-clang; \
chmod +x /usr/local/bin/aarch64-linux-gnu-clang; \
\
echo '#!/bin/sh' > /usr/local/bin/aarch64-linux-gnu-clang++; \
echo 'exec clang++ --target=aarch64-linux-gnu --sysroot=/usr/aarch64-linux-gnu "$@"' >> /usr/local/bin/aarch64-linux-gnu-clang++; \
chmod +x /usr/local/bin/aarch64-linux-gnu-clang++