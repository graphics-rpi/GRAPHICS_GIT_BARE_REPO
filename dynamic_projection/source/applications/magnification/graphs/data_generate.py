#!/usr/bin/python

import argparse

if __name__ == '__main__' :
    parser = argparse.ArgumentParser(description="Produce data!")
    parser.add_argument( "-o ", "--output-file", required=True, help="Output file" )
    args = parser.parse_args()

    o_file = open(args.output_file, 'w')
    x = 0.000
    y = 0.000
    while x < 1.001:
        o_file.write( "%f %f\n" % (x, y))
        x += 0.001
