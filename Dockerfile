# Compile sources
FROM ubuntu:25.04 AS builder

RUN apt-get update
RUN apt-get install -y gcc make libsixel-dev libsixel-bin clang

WORKDIR /app
COPY . .

RUN make -f Makefile

# Now generate a small layer for docker

FROM ubuntu:25.04
RUN \
    apt-get update && \
    apt-get install -y libsixel-bin

WORKDIR /app
COPY --from=builder /app/doomsixel .
COPY --from=builder /app/doom1.wad .
CMD ["./doomsixel"]

