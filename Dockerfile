FROM debian:stretch

RUN apt-get update && apt-get install -y \
  build-essential autoconf libtool \
  git \
  pkg-config \
  && apt-get clean

RUN apt-get install -y \
  gdb \
  valgrind

RUN apt-get install -y wget
RUN cd /tmp && wget https://static.rust-lang.org/rustup/dist/x86_64-unknown-linux-gnu/rustup-init
RUN cd /tmp && chmod +x rustup-init && ./rustup-init -y --default-toolchain nightly
RUN echo 'export PATH="$HOME/.cargo/bin:$PATH"' >> /root/.bashrc
