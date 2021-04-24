"""Generates frames in a test pattern and sends them to a client connected over TCP to LISTEN_PORT. Only supports one
connected TCP client.

USAGE: python tcp_server.py [-w WIDTH] [-h HEIGHT] [-r FRAMERATE] [-f FORMAT] <LISTEN_PORT>

Requires Python 3.
"""

from argparse import ArgumentParser
import socket
import time


def main():
    parser = ArgumentParser(
        description='Serve a test pattern to a connected TCP client.')

    parser.add_argument('--width', type=int,
                        default=640, help='Frame width')
    parser.add_argument('--height', type=int,
                        default=480, help='Frame height')
    parser.add_argument('--frame-rate', type=int,
                        default=30, help='Framerate')
    parser.add_argument('--format', type=lambda s: s.lower(), default='rgb',
                        help='Format (RGB, GRAY8, GRAY16_BE, GRAY16_LE)')
    parser.add_argument('listen_port', type=int,
                        help='A port for listening for connections')
    parser.add_argument('input_file',
                        help='A file to read frame data from')

    args = parser.parse_args()

    if args.format != 'rgb':
        raise ValueError(f'Unsupported format {args.format}')
    pixel_size = 3

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock, \
            open(args.input_file, 'rb') as f:
        sock.bind(('127.0.0.1', args.listen_port))
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.listen(1)
        conn, addr = sock.accept()
        with conn:
            print(f'Connection from {addr}')
            i = 0
            period = 1 / args.frame_rate
            while True:
                t_start = time.time()
                frame = f.read(args.width * args.height * pixel_size)
                if frame == b'':
                    # Seek to the beginning of the file and try again
                    f.seek(0)
                    continue

                conn.sendall(frame)

                time.sleep(max(0, period - (time.time() - t_start)))


if __name__ == '__main__':
    main()
