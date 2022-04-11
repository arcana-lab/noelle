FROM ubuntu:20.04

RUN apt update \
    && DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC apt install -y \
     git \
     wget \
     cmake \
     build-essential \
     libz-dev \
     lsb-release \
     software-properties-common \
    && apt clean

RUN wget https://apt.llvm.org/llvm.sh \
    && chmod +x llvm.sh \
    && ./llvm.sh 9

RUN ln -s /usr/bin/clang-9 /usr/bin/clang \
    && ln -s /usr/bin/clang++-9 /usr/bin/clang++

RUN git clone https://github.com/scampanoni/noelle.git

WORKDIR /noelle

RUN make

ENTRYPOINT ["/usr/bin/bash"]
