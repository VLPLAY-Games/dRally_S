#!/usr/bin/env python3
"""IPX tunnelling server for the network race of dRally (the protocol of DOSBox's `ipxnet`).

Death Rally talks IPX, a protocol of old local networks. dRally sends its IPX packets inside UDP
packets to one server, and the server hands each packet on to the player it is addressed to (or
to everybody, for a broadcast). Run this on a computer in your network and give its address to
every player: in SWITCH.CFG on the Switch (`ipx_server` and `ipx_port`), or in the source of the
Linux build.

    python3 ipxnet.py            listen on UDP port 8888 (the port dRally uses by default)
    python3 ipxnet.py 9000       another port

Only Python 3 is needed (no SDL). It does the same as ipxnet.c in this folder, which needs
SDL2 and SDL2_net to build; a player that registers again keeps its place.
"""

import socket
import struct
import sys
import time

DEFAULT_PORT = 8888
MAX_CLIENTS = 16
HEADER = 30                 # an IPX header: checksum 2, length 2, control 1, type 1, destination 12, source 12
BROADCAST_HOST = b"\xff\xff\xff\xff"
ZERO_HOST = b"\x00\x00\x00\x00"

# offsets of the fields of the header
DEST_NET, DEST_HOST, DEST_PORT, DEST_SOCKET = 6, 10, 14, 16
SRC_NET, SRC_HOST, SRC_PORT, SRC_SOCKET = 18, 22, 26, 28


def pack_address(addr):
    """('1.2.3.4', 5678) as the six bytes an IPX node holds here: the address and the port, as they are on the wire."""
    return socket.inet_aton(addr[0]) + struct.pack("!H", addr[1])


def ack_packet(client, server_port):
    """The reply to a registration: it tells the client its own IPX node (its address and port)."""
    packet = bytearray(HEADER)
    packet[0:2] = b"\xff\xff"                           # checksum
    packet[2:4] = struct.pack("!H", HEADER)             # length
    packet[DEST_NET:DEST_NET + 4] = b"\x00\x00\x00\x00"
    packet[DEST_HOST:DEST_HOST + 6] = pack_address(client)
    packet[DEST_SOCKET:DEST_SOCKET + 2] = struct.pack("!H", 2)
    packet[SRC_NET:SRC_NET + 4] = struct.pack("!I", 1)
    packet[SRC_HOST:SRC_HOST + 4] = ZERO_HOST
    packet[SRC_PORT:SRC_PORT + 2] = struct.pack("!H", server_port)
    packet[SRC_SOCKET:SRC_SOCKET + 2] = struct.pack("!H", 2)
    return bytes(packet)


class Server:
    def __init__(self, port, log=print):
        self.port = port
        self.log = log
        self.clients = []                               # the (host, port) of each registered player
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.bind(("", port))

    def handle(self, data, sender):
        """One datagram from `sender`: a registration, or a packet to pass on."""
        if len(data) < HEADER:
            return
        dest_socket = struct.unpack("!H", data[DEST_SOCKET:DEST_SOCKET + 2])[0]
        dest_host = data[DEST_HOST:DEST_HOST + 4]

        # an echo packet (socket 2) with no destination is a registration
        if dest_socket == 2 and dest_host == ZERO_HOST:
            if sender in self.clients:
                self.log("IPXSERVER: Reconnect from %s:%d" % sender)
            elif len(self.clients) < MAX_CLIENTS:
                self.clients.append(sender)
                self.log("IPXSERVER: Connect from %s:%d (%d connected)" % (sender + (len(self.clients),)))
            else:
                self.log("IPXSERVER: full, refused %s:%d" % sender)
                return
            self.sock.sendto(ack_packet(sender, self.port), sender)
            return

        # a packet for somebody: its destination node says who
        src_node = data[SRC_HOST:SRC_HOST + 6]
        if dest_host == BROADCAST_HOST:
            for client in self.clients:
                if pack_address(client) != src_node:
                    self.sock.sendto(data, client)
        else:
            dest_node = data[DEST_HOST:DEST_HOST + 6]
            for client in self.clients:
                if pack_address(client) == dest_node:
                    self.sock.sendto(data, client)

    def run(self):
        self.log("IPX Tunneling Server is running on UDP port %d" % self.port)
        while True:
            try:
                data, sender = self.sock.recvfrom(2048)
            except KeyboardInterrupt:
                break
            except OSError:
                time.sleep(0.01)
                continue
            self.handle(data, sender)
        self.log("IPX Tunneling Server terminated")


def main():
    port = int(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_PORT
    try:
        Server(port).run()
    except OSError as e:
        print("IPX Tunneling Server failed to start: %s" % e)
        if port < 1024:
            print("Try a port number above 1024.")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
