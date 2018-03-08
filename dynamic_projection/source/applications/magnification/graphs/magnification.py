#!/usr/bin/python

import argparse
import math

class Point(object):
    ''' Stores a point on a coordinates plane '''
    def __init__(self, x, y):
        '''Defines x and y variables'''
        self.x = x
        self.y = y

    def get_xy(self):
        return (self.x, self.y)

    def get_x(self):
        return self.x

    def get_y(self):
        return self.y

    def distance(self, other):
        dx = self.x - other.x
        dy = self.y - other.y
        return math.sqrt(dx**2 + dy**2)

    def __str__(self):
        return "X: %f\tY: %f" % (self.x, self.y)

def linear_transform( center, destination ):
    magnification_factor = 9.0
    scale = 1.0 / magnification_factor;
    dist = center.distance( destination )

    if dist > 0.0:
        normal_vec = ((destination.get_x() - center.get_x())/ dist , (destination.get_y() - center.get_y())/ dist )
        dist = dist * scale
        return Point( center.get_x() + normal_vec[0] * dist, center.get_y() + normal_vec[1] * dist )
    else:
        return Point( center.get_x(), center.get_y() )

def old_fisheye_transform( center, destination ):
    direction = (destination.get_x() - center.get_x(),  destination.get_y() - center.get_y())
    radius = math.sqrt( direction[0]**2 + direction[1]**2)

    distortion = 5.71

    gr = (( distortion + 1 ) * radius) / (distortion * radius + 1 )
    if radius > 0.0:
        ratio = radius / gr
        return Point( center.get_x() + ratio * direction[0], center.get_y() + ratio * direction[1])
    else:
        return Point( center.get_x(), center.get_y() )

def orth_fisheye( center, destination ):
    x_dist = abs(destination.get_x() - center.get_x())
    y_dist = abs(destination.get_y() - center.get_y())

    dist = math.sqrt( x_dist**2 + y_dist**2 )

    if dist < 0.001 :
        return Point( center.get_x(), center.get_y() )

    x_max = 0.33
    y_max = 1.0

    d_max = 0.2

    x_sign = 0
    y_sign = 0

    if x_dist < 0.001 and x_dist > -0.001 :
        x_sign = 0
    else:
        x_sign = (destination.get_x() - center.get_x()) / x_dist
    if y_dist < 0.001 and y_dist > -0.001 :
        y_sign = 0
    else:
        y_sign = (destination.get_y() - center.get_y()) / y_dist

    distortion_factor = 5.71

    gd = ( distortion_factor + 1 ) / ( distortion_factor + (d_max / dist ))

    if ( x_dist < 0.001 and x_dist > -0.001 and y_dist < 0.001 and y_dist > -0.001 ):
        return Point( center.get_x(), center.get_y() )
    elif ( x_dist < 0.001 and x_dist > -0.001 and (y_dist > 0.001 or y_dist < -0.001)):
        gy = ( distortion_factor + 1 ) / (distortion_factor + (y_max / y_dist) )
        return Point( center.get_x(), y_sight * gd * d_max + center.get_y())
    elif (( x_dist > 0.001 or x_dist < -0.001) and y_dist < 0.001 and y_dist > -0.001):
        gx = ( distortion_factor + 1 ) / (distortion_factor + (x_max / x_dist) )
        return Point( x_sign * gd * d_max + center.get_x(), center.get_y() )
    else:
        gx = ( distortion_factor + 1 ) / (distortion_factor + (x_max / x_dist) )
        gy = ( distortion_factor + 1 ) / (distortion_factor + (y_max / y_dist) )
        new_x = x_sign * gd * d_max + center.get_x()
        new_y = y_sign * gd * d_max + center.get_y()
        return Point( new_x, new_y)


def logistic_regression( lower, upper ):
    numerator = (upper[1] + 0.001) - (lower[1] - 0.001)
    lhs = upper[1] -lower[1] + 0.001

    a = (numerator - lhs) / lhs 
    b = math.log( (numerator/0.001 + 1 ) / a) / (lower[0] - upper[0])

    return (a, b)


def logistic_function( input_val, lower, upper ):
    logistic_vars = logistic_regression( lower, upper)
    numerator = ((upper[1] + 0.001) - (lower[1] - 0.001))
    denominator = 1 + logistic_vars[0] * math.exp( logistic_vars[1] * (input_val - upper[0]) )
    return lower[1] + 0.001 + numerator / denominator

def fisheye_transform( center, destination ):
    x_dist = destination.get_x() - center.get_x()
    y_dist = destination.get_y() - center.get_y()
    dist = math.sqrt( x_dist**2 + y_dist**2 )


    large_m = ( 0.1, 9 )
    small_m = ( 0.5, 1 )

    slope = (large_m[1] - small_m[1]) / ( large_m[0] - small_m[0])
    b = large_m[1] - slope * large_m[0]

    current_m = slope * dist + b

    current_m = logistic_function( dist, small_m, large_m)

    computed_r = dist * current_m

    if dist > 0.0:

        radius = computed_r
        power = 5.0
        power_exp = math.exp(power)

        new_dist = power_exp /(power_exp -1) * radius * (1)


        new_x = center.get_x() + x_dist * ( dist / new_dist )
        new_y = center.get_y() + y_dist * ( dist / new_dist )


        new_point = Point(new_x, new_y);

        calc_dist = math.sqrt((new_x - center.get_x())**2 + (new_y - center.get_y())**2) 

        if dist == 0.5:
            print current_m


        if current_m == 9.0:
            print x_dist
            print y_dist
            print computed_r
            print dist
            print new_dist
            print
            print destination
            print new_point
            print
            print calc_dist
            print dist
            print (dist / calc_dist)
            print 

        return new_point
    else:
        return Point( center.get_x(), center.get_y() )




if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Produce data!")
    parser.add_argument( "-i ", "--input-file", required=True, help="Input file" )
    parser.add_argument( "-o ", "--output-file", help = "Output file" )
    args = parser.parse_args()

    print "Input file: %s" % args.input_file
    print "Output file: %s" % args.output_file

    i_file = open(args.input_file, 'r')
    o_file = open(args.output_file, 'w')

    original_points = []
    linear_transform_points = []
    fisheye_transform_points = []
    orth_fisheye_points = []
    old_fisheye_transform_points = []

    center = Point( 0.5, 0.0 )

    for line in i_file:
        vals = line.strip().split()
        x = float(vals[0])
        y = float(vals[1])
        original_points.append(Point(x,y))

    for point in original_points:
        linear_transform_points.append(linear_transform( center, point ))
        fisheye_transform_points.append(fisheye_transform( center, point ))
        orth_fisheye_points.append(orth_fisheye( center, point ))
        old_fisheye_transform_points.append(old_fisheye_transform( center, point ))

    data_file = open("output.dat", 'w')

    o_dists = []
    l_dists = []
    f_dists = []
    of_dists = []
    for i in xrange( len(original_points) ):
        o_file.write( "O: %s\n" % str(original_points[i]))
        o_file.write( "L: %s\n" % str(linear_transform_points[i]))
        o_file.write( "F: %s\n" % str(fisheye_transform_points[i]))
        o_file.write( "OF: %s\n" % str(orth_fisheye_points[i]))
        #o_file.write( "OF: %s\n" % str(old_fisheye_transform_points[i]))

        o_dist = original_points[i].distance( center )
        o_dists.append(o_dist)
        l_dist = linear_transform_points[i].distance( original_points[i] )
        l_dist = linear_transform_points[i].distance( center )
        l_dists.append(l_dist)
        f_dist = fisheye_transform_points[i].distance( original_points[i] )
        f_dist = fisheye_transform_points[i].distance( center )
        f_dists.append(f_dist)
        of_dist = orth_fisheye_points[i].distance( original_points[i] )
        of_dist = orth_fisheye_points[i].distance( center )
        #of_dist = old_fisheye_transform_points[i].distance( original_points[i] )
        of_dists.append(of_dist)

        o_file.write( "O D: %f\n" % o_dist)
        o_file.write( "L D: %f\n" % l_dist)
        o_file.write( "F D: %f\n" % f_dist)
        o_file.write( "OF D: %f\n" % of_dist)
        o_file.write( "\n")

        if o_dist > 0.0:
            
            o_ratio = o_dist / o_dist
            l_ratio = o_dist / l_dist
            f_ratio = o_dist / f_dist
            of_ratio = o_dist / of_dist

            o_file.write( "O R: %f\n" % (o_ratio))
            o_file.write( "L R: %f\n" % (l_ratio))
            o_file.write( "F R: %f\n" % (f_ratio))
            o_file.write( "OF R: %f\n" % (of_ratio))
            data_file.write( "%f\t%f\t%f\t%f\n" % (o_dist, l_ratio, f_ratio, o_ratio))

        #data_file.write( "%f\t%f\t%f\t%f\n" % (o_dist, l_dist, f_dist, of_dist))
        o_file.write( "\n")

    for i in xrange( len(o_dists) ):
        diff = of_dists[i] - l_dists[i]
        o_file.write( "X: %f DIFF: %f\n" % (original_points[i].get_x(), diff ))

    i_file.close()
    o_file.close()
    data_file.close()
