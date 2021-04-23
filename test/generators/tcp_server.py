"""Generates frames in a test pattern and sends them to a client connected over TCP to LISTEN_PORT. Only supports one
connected TCP client.

USAGE: python tcp_server.py [-w WIDTH] [-h HEIGHT] [-r FRAMERATE] [-f FORMAT] <LISTEN_PORT>

Requires Python 3.
"""

from argparse import ArgumentParser
import socket
import time

from frame_generation import rgb_gradient


def main():
    parser = ArgumentParser(
        description='Serve a test pattern to a connected TCP client.')

    parser.add_argument('-w', '--width', type=int,
                        default=640, help='Frame width')
    parser.add_argument('-h', '--height', type=int,
                        default=480, help='Frame height')
    parser.add_argument('-r', '--frame-rate', type=int,
                        default=30, help='Framerate')
    parser.add_argument('-f', '--format', type=lambda s: s.lower(), default='rgb',
                        help='Format (RGB, GRAY8, GRAY16_BE, GRAY16_LE)')
    parser.add_argument('listen_port', type=int,
                        help='A port for listening for connections')

    args = parser.parse_args()

    if args.format != 'rgb':
        raise ValueError(f'Unsupported format {args.format')

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.bind(('127.0.0.1', args.port))
        sock.listen(1)
        conn, addr = sock.accept()
        with conn:
            print(f'Connection from {addr}')
            i = 0
            t = time.time()
            period = 1 / args.frame_rate
            while True:
                frame = rgb_gradient(args.width, args.height, i)

                conn.sendall(frame)

                i += 1
                sleep()
