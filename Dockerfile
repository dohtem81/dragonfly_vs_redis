FROM dohtem81/gcc_opencv_base:latest
WORKDIR /usr/src

RUN apt-get update
RUN apt-get install build-essential autoconf libtool pkg-config -y

RUN git clone https://github.com/redis/hiredis.git
RUN mkdir /usr/src/hiredis/build
WORKDIR /usr/src/hiredis/build
RUN cmake ../.
RUN make
RUN make install
RUN make clean

RUN mkdir /var/build
WORKDIR /var/build
RUN mkdir /usr/src/app
COPY ./app/ /usr/src/app/
RUN cmake /usr/src/app/.
RUN make

RUN ldconfig

# Run the application
CMD ["/bin/bash"]