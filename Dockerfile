# Use an official Ubuntu as a base image
FROM ubuntu:20.04

# Set environment variables to avoid prompts during package installations
ENV DEBIAN_FRONTEND=noninteractive

# Install necessary packages including compilers, git, and dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    g++ \
    cmake \
    curl \
    tar \
    git \
    wget \
    libpcre2-dev \
    libssl-dev \
    libcurl4-openssl-dev \
    zlib1g-dev \
    libgcrypt20-dev \
    libkrb5-dev \
    pkg-config \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Download LibSSH 0.9.5 source tarball and extract it
RUN curl -SL https://www.libssh.org/files/0.9/libssh-0.9.5.tar.xz -o libssh-0.9.5.tar.xz \
    && mkdir -p /opt/libssh-0.9.5 \
    && tar -xJf libssh-0.9.5.tar.xz -C /opt/libssh-0.9.5 --strip-components=1 \
    && rm libssh-0.9.5.tar.xz

# Build and install LibSSH
RUN cd /opt/libssh-0.9.5 \
    && mkdir build && cd build \
    && cmake .. \
    && make \
    && make install \
    && rm -rf /opt/libssh-0.9.5 /opt/libssh-0.9.5/build

# Download OpenSSL 3.0.14 source tarball and extract it
# Download OpenSSL 3.0.14 source tarball and extract it
# Clone OpenSSL repository from GitHub
# Download OpenSSL source tarball and extract it
RUN wget https://www.openssl.org/source/openssl-3.0.14.tar.gz && \
    tar -xf openssl-3.0.14.tar.gz && \
    rm openssl-3.0.14.tar.gz

# Build and install OpenSSL
RUN cd openssl-3.0.14 && \
    ./config --prefix=/usr/local/openssl && \
    make && \
    make install
# Set environment variables for OpenSSL
# Set environment variables for OpenSSL
ENV PATH=/usr/local/openssl/bin:$PATH
ENV LD_LIBRARY_PATH=/usr/local/openssl/lib64:$LD_LIBRARY_PATH
ENV OPENSSL_ROOT_DIR=/usr/local/openssl
ENV OPENSSL_LIBRARIES=/usr/local/ssl/lib

# Update the library cache
RUN ldconfig

RUN openssl version
# Copy libyang from host to Docker image
COPY libyang /opt/libyang

# Build and install libyang
RUN cd /opt/libyang \
    && mkdir build && cd build \
    && cmake .. \
    && make \
    && make install \
    && rm -rf /opt/libyang /opt/libyang/build

COPY ./libnetconf2 /opt/libnetconf2
RUN cd /opt/libnetconf2 && mkdir build && cd build && \
    cmake .. -DOPENSSL_ROOT_DIR=$OPENSSL_ROOT_DIR -DOPENSSL_LIBRARIES=$LD_LIBRARY_PATH && \
    make && make install && \
    rm -rf /opt/libnetconf2 /opt/libnetconf2/build

# Copy sysrepo from host to Docker image
COPY sysrepo /opt/sysrepo

# Build and install sysrepo
RUN cd /opt/sysrepo \
    && mkdir build && cd build \
    && cmake -DCMAKE_BUILD_TYPE=Release .. \
    && make \
    && make install \
    && rm -rf /opt/sysrepo /opt/sysrepo/build

    # Copy netopeer2 from host to Docker image
COPY netopeer2 /opt/netopeer2

# Build and install Netopeer2
RUN cd /opt/netopeer2 \
    && mkdir build && cd build \
    && cmake -DLN2_YANG_MODULE_DIR=$LN2_YANG_MODULE_DIR .. \
    && make \
    && make install \
    && rm -rf /opt/netopeer2 /opt/netopeer2/build

# Cleanup unnecessary files and dependencies
RUN apt-get remove -y --purge \
    build-essential \
    gcc \
    g++ \
    cmake \
    git \
    && apt-get autoremove -y \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# Set environment variables for sysrepo
ENV LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
ENV PATH=/usr/local/bin:$PATH

# Optionally, specify a CMD if you want to run something by default
# CMD ["sysrepoctl", "start"]

# Or, leave CMD empty to run it manually when starting the container
# CMD ["bash"]
