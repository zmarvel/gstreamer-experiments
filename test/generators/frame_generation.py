
def rgb_gradient(width, height, step=0):
    frame = bytearray(3*width*height)
    # Corresponds to gstreamer's RGB
    # We want to go from all red to all blue
    period = 90  # cycle in number of frames/steps
    step %= period
    r_ratio = ((period - step) / period)
    b_ratio = (step / period)
    r = int(255.999 * r_ratio)
    b = int(255.999 * b_ratio)
    for row in range(height):
        for col in range(width):
            g_ratio = (col + row * 2) / (width + height * 2)
            g = int(255.999 * g_ratio)
            i = row * width + col
            frame[3*i] = r
            frame[3*i+1] = g
            frame[3*i+2] = b
    print(int(frame[0]), int(frame[1]), int(frame[2]))
    print(int(frame[7680]), int(frame[7680+1]), int(frame[7680+2]))
    return frame


def gray_gradient(width, height, bytes=2, endianness='big', step=0):
    # Defaults correspond to gstreamer's GRAY16_BE
    pass


# TODO YUV420


if __name__ == '__main__':
    import argparse

    def main():
        parser = argparse.ArgumentParser()
        parser.add_argument('--width', type=int, default=640)
        parser.add_argument('--height', type=int, default=480)
        parser.add_argument('-f', '--format', type=lambda s: s.lower(), default='rgb',
                            help='Pixel format (only RGB supported for now)')
        parser.add_argument('num_frames', type=int)
        parser.add_argument('out_path')

        args = parser.parse_args()

        with open(args.out_path, 'w+b') as f:
            for i in range(args.num_frames):
                print(i)
                f.write(rgb_gradient(args.width, args.height, i))

    main()
