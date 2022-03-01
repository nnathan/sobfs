# sobfs

sobfs is an obfuscator using symmetric-key cryptography.

sobfs is inspired by [this write-up](https://gist.github.com/gmurdocca/88857b58dc4668d88b0d0fae6ebf8b64) on how an IDS middlebox was defeated using rot-13 encryption.

Since it is trivial for an IDS to blacklist the 26 possible banners using rot encryption, I looked for a more robust way to defeat IDS middleboxes. Using symmetric-key encryption on a stream fits the bill. Rejecting all possible banner cominbations that can be encrypted would be infeasible and performance degrading for an IDS.

The symmetric-key primitive used is the [gimli](https://gimli.cr.yp.to) lightweight cipher. The C implementation used is copied from Taylor R. Campbell and can be originally [found here](https://mumble.net/~campbell/tmp/gimli.c). The permutation is used as essentially a 256-bit stream cipher.

# How to use

The basic premise is you start sobfs at both the client and server of a tcp (udp untested) socket and the underlying communication exiting the client and server will be encrypted as strings indistinguishable from random.

We will use the same test setup as the above write up on defeating IDS middleboxes with rot-13. We assume you're running a Linux client in a restricted network that can access Citrix on port tcp/8090 to the outside world. However the meddling middlebox will reject cleartext OpenSSH connections. On the Internet side is a Linux server which you have full control to run an SSH server on port 8090. As a visual aid, here's a diagram:

```text
┌───────────────────────────────────────────────────┐                            ┌───────────────────────────┐
│Restricted Network                                 │                            │Internet                   │
│                                                   │                            │                           │
│     ┌─────────────────┐     ┌─────────────────┐   │      tcp/8090 (citrix)     │  ┌─────────────────┐      │
│     │                 │     │                 │   │          permitted         │  │                 │      │
│     │  Linux Client   │─────│  IDS/Firewall   │───┼────────────────────────────┼──│  Linux Server   │      │
│     │                 │     │                 │   │                            │  │                 │      │
│     └─────────────────┘     └─────────────────┘   │                            │  └─────────────────┘      │
│                                                   │                            │                           │
└───────────────────────────────────────────────────┘                            └───────────────────────────┘
```

sobfs relies on [socat v2](http://www.dest-unreach.org/socat/). Socat v2 appears to be less popular than v1 and is finnicky to obtain and install. Therefore we use a docker image containing socat v2 to handle our comms.

## Generate a key

We will need a symmetric key to use on the sobfs client and server side.

```sh
dd if=/dev/urandom bs=1 count=32 2>/dev/null | xxd -p -c 32
```

For the examples below we will use the key `97d9b2032735d1ad9f8fe22a6c97c3f0015ced183c4b557d49c11f54b7bafa9f`.

## On the Server

First we start up the docker container and map the name `host.docker.internal` inside the container to expose the parent host. We also expose the port 8090 from the container so that it's accessible over the Internet.

```sh
# start docker
docker run -it -p 8090:8090 --add-host host.docker.internal:$(ip addr show docker0 | grep -Po 'inet \K[\d.]+') timotto/docker-socat2 bash
```

Then we compile sobfs in the docker container:

```sh
# install sobfs
cd ~
git clone https://github.com/nnathan/sobfs.git
cd sobfs
make
```

Then we start socat in the docker container to start sofbs for both the read stream and the write stream of the TCP connection.

```sh
socat "exec1:./sobfs 97d9b2032735d1ad9f8fe22a6c97c3f0015ced183c4b557d49c11f54b7bafa9f % exec1:./sobfs 97d9b2032735d1ad9f8fe22a6c97c3f0015ced183c4b557d49c11f54b7bafa9f | TCP4-LISTEN:8090,reuseaddr,fork" TCP4:host.docker.internal:22
```

This starts a listener on port 8090 that accepts connections that send/receive through sobfs while proxying to OpenSSH on the local host (through the mapped docker host).

## On the Client

We start up another docker container and map the `host.docker.internal` inside the container to the parent host.

```sh
# start docker
docker run -it -p 10000:10000 --add-host host.docker.internal:$(ip addr show docker0 | grep -Po 'inet \K[\d.]+') timotto/docker-socat2 bash
```

Then we compile sobfs in the docker container:

```sh
# install sobfs
cd ~
git clone https://github.com/nnathan/sobfs.git
cd sobfs
make
```

Then we start socat in the docker container to start a listener on port 10000 when connected will send/receive via sofbs to the server over port 8090.

```sh
socat TCP4-LISTEN:10000,reuseaddr,fork "exec1:./sobfs 97d9b2032735d1ad9f8fe22a6c97c3f0015ced183c4b557d49c11f54b7bafa9f % exec1
:./sobfs 97d9b2032735d1ad9f8fe22a6c97c3f0015ced183c4b557d49c11f54b7bafa9f | TCP4-CONNECT:linux-server-hostname:8090"
```

## Lastly

On the client to connect to the server over 8090/tcp with the comms obfuscated by sobfs, run this on the host (outside of the docker container):

```sh
ssh user@localhost -p 10000
```

# Limitations

* Currently both client and server have to use the same endianness. Adding endian conversion routines is planned.
* Performance is reduced by an order of magnitude when doing high-throughput connections such as scp for a large file transfer.
* Unsure if this is a limitation, but this hasn't been tested with UDP sockets.
