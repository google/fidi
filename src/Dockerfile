FROM debian:sid
MAINTAINER Manoj Srivastava, srivasta@google.com

# install
RUN apt-get update && apt-get install -y  \
        libpocofoundation60 \
        libpoconet60        \
        libpocoutil60       \
        && rm -rf /var/lib/apt/lists/*

# extra metadata
LABEL description="Fidi full docker image."

COPY root /
ENTRYPOINT ["fidi_app"]
CMD ["--help"]
