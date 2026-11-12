#!/usr/bin/env python3
"""Test of the IPX tunnelling server: registration, a packet to one player, a broadcast."""
import os
import socket
import struct
import sys
import threading

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "ipxnet"))
import ipxnet

failures = 0


def check(cond, text):
    global failures
    if not cond:
        print("FAIL: " + text)
        failures += 1


def registration():
    """What dRally sends first: an echo packet (socket 2) with a zero destination and source."""
    p = bytearray(30)
    p[0:2] = b"\xff\xff"
    p[2:4] = struct.pack("!H", 30)
    p[16:18] = struct.pack("!H", 2)
    p[28:30] = struct.pack("!H", 2)
    return bytes(p)


def packet(dest_node, src_node, payload=b"hello"):
    p = bytearray(30)
    p[0:2] = b"\xff\xff"
    p[2:4] = struct.pack("!H", 30 + len(payload))
    p[10:16] = dest_node
    p[16:18] = struct.pack("!H", 0x5001)
    p[22:28] = src_node
    p[28:30] = struct.pack("!H", 0x5001)
    return bytes(p) + payload


def client():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.bind(("127.0.0.1", 0))
    s.settimeout(1.0)
    return s


srv = ipxnet.Server(0, log=lambda m: None)
port = srv.sock.getsockname()[1]
srv.port = port
srv.sock.settimeout(0.05)
stop = []


def loop():
    while not stop:
        try:
            data, sender = srv.sock.recvfrom(2048)
        except socket.timeout:
            continue
        srv.handle(data, sender)


t = threading.Thread(target=loop, daemon=True)
t.start()
server = ("127.0.0.1", port)

a, b = client(), client()
node_a = ipxnet.pack_address(a.getsockname())
node_b = ipxnet.pack_address(b.getsockname())

# registration: the answer names the player's own node
a.sendto(registration(), server)
ack, _ = a.recvfrom(2048)
check(len(ack) == 30, "ack is an IPX header")
check(ack[10:16] == node_a, "ack carries the player's own node")
check(struct.unpack("!H", ack[16:18])[0] == 2, "ack comes from socket 2")
b.sendto(registration(), server)
ack, _ = b.recvfrom(2048)
check(ack[10:16] == node_b, "second player's ack carries its own node")

# registering again does not take another place
a.sendto(registration(), server)
a.recvfrom(2048)
check(len(srv.clients) == 2, "a repeated registration keeps its place")

# a packet to one player reaches only that player
a.sendto(packet(node_b, node_a, b"to-b"), server)
data, _ = b.recvfrom(2048)
check(data.endswith(b"to-b"), "a packet reaches the player it is addressed to")
a.settimeout(0.2)
try:
    a.recvfrom(2048)
    check(False, "the sender does not get its own directed packet")
except socket.timeout:
    pass

# a broadcast reaches everybody but the sender
a.sendto(packet(b"\xff" * 6, node_a, b"all"), server)
data, _ = b.recvfrom(2048)
check(data.endswith(b"all"), "a broadcast reaches the other player")
try:
    a.recvfrom(2048)
    check(False, "the sender does not get its own broadcast")
except socket.timeout:
    pass

# a packet for nobody is dropped; a short datagram is ignored
a.sendto(packet(b"\x01\x02\x03\x04\x05\x06", node_a, b"lost"), server)
a.sendto(b"short", server)
b.settimeout(0.2)
try:
    b.recvfrom(2048)
    check(False, "a packet for nobody is dropped")
except socket.timeout:
    pass

# the table is limited
for _ in range(20):
    c = client()
    c.sendto(registration(), server)
    try:
        c.recvfrom(2048)
    except socket.timeout:
        pass
check(len(srv.clients) == ipxnet.MAX_CLIENTS, "no more than 16 players")

stop.append(1)
if failures:
    print("%d check(s) failed" % failures)
    sys.exit(1)
print("ipxnet: all checks passed")
