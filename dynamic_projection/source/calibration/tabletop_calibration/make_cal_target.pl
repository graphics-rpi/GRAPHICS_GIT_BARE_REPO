#!/usr/bin/perl
#
# make_cal_target.pl: create planar camera calibration target
#

sub letter_write_ps_header(){
    print PS <<"EOF";
%!PS-Adobe-2.0
%%Orientation: Landscape
%%BeginFeature: *PageSize Letter
<</PageSize[612 792]/ImagingBBox null/Orientation 3>>setpagedevice
%%EndFeature

EOF
}

sub write_ps_header(){
    print PS <<"EOF";
%!PS-Adobe-2.0
%%Orientation: Landscape
%%BeginFeature: *PageSize archD
<</PageSize[1728 2592]/ImagingBBox null/Orientation 3>>setpagedevice
%%EndFeature
%[/CropBox [0 0 2592 1728] /PAGES pdfmark
EOF
}

sub target1($$$$$){
    my ($cx, $cy, $r, $linewidth, $rgbcolor) = @_;
    $cx = 72 * $cx / 25.4;
    $cy = 72 * $cy / 25.4;
    $r = 72 * $r / 25.4;
    $linewidth = 72 * $linewidth / 25.4;
    print PS <<"EOF";

% draw top pie section
0 setlinewidth
$cx $cy newpath moveto
$cx $r add $cy lineto
$cx $cy $r 0 90 arc
$cx $cy lineto closepath
$rgbcolor setrgbcolor fill

% draw bottom pie section
0 setlinewidth
$cx $cy newpath moveto
$cx $r sub $cy lineto
$cx $cy $r 180 270 arc
$cx $cy lineto closepath
$rgbcolor setrgbcolor fill

% draw circle outline
$rgbcolor setrgbcolor
$linewidth setlinewidth 
$cx $cy $r 0 360 arc closepath
stroke
EOF
}

sub target2($$$$$){
    my ($cx, $cy, $r, $linewidth, $rgbcolor) = @_;
    $cx = 72 * $cx / 25.4;
    $cy = 72 * $cy / 25.4;
    $r = 72 * $r / 25.4;
    $linewidth = 72 * $linewidth / 25.4;
    print PS <<"EOF";

% draw top pie section
0 setlinewidth
$cx $cy newpath moveto
$cx $cy $r sub lineto
$cx $cy $r 270 360 arc
$cx $cy lineto closepath
$rgbcolor setrgbcolor fill

% draw bottom pie section
0 setlinewidth
$cx $cy newpath moveto
$cx $cy $r add lineto
$cx $cy $r 90 180 arc
$cx $cy lineto closepath
$rgbcolor setrgbcolor fill

% draw circle outline
$rgbcolor setrgbcolor
$linewidth setlinewidth 
$cx $cy $r 0 360 arc closepath
stroke
EOF
}

sub disk($$$$$){
    my ($cx, $cy, $r, $linewidth, $rgbcolor) = @_;
    $cx = 72 * $cx / 25.4;
    $cy = 72 * $cy / 25.4;
    $r = 72 * $r / 25.4;
    $linewidth = 72 * $linewidth / 25.4;
    print PS <<"EOF";

% draw solid circle
$rgbcolor setrgbcolor
$linewidth setlinewidth 
$cx $cy $r 0 360 arc closepath
$rgbcolor setrgbcolor fill
% draw circle outline
$rgbcolor setrgbcolor
$linewidth setlinewidth 
$cx $cy $r 0 360 arc closepath
stroke
EOF
}

sub  origin($$$$){
  my ($origin_x, $origin_y, $cross_thickness, $cross_length) = @_;
  $origin_x = 72 * $origin_x / 25.4;
  $origin_y = 72 * $origin_y / 25.4;
  $cross_thickness = 72 * $cross_thickness / 25.4;
  $cross_length = 0.5 * 72 * $cross_length / 25.4;
  print PS <<"EOF";

0 0 0 setrgbcolor
$cross_thickness setlinewidth
$origin_x $cross_length sub $origin_y moveto
$origin_x $cross_length add $origin_y lineto
stroke

$origin_x $origin_y $cross_length sub moveto
$origin_x $origin_y $cross_length add lineto
stroke
EOF
}

#
# main code
#
sub main(){
#  $page_width_mm = 279;
#  $page_height_mm = 216;

  $page_width_mm = 25.4 * 2592 / 72;
  $page_height_mm = 25.4 * 1728 / 72;

#  $nx = 8;
#  $ny = 6;

  $nx = 18;
  $ny = 12;

  $xmargin = 2.5 * 25.4;
  $ymargin = 3.5 * 25.4;

  $sp = 3.25;
  $r = ($page_width_mm - 2*$xmargin)  / (3 + $sp * $nx);
  $disk_r = $r * 1.5;

  $linewidth = 0.15 * $r;

  $cross_thickness = 0.5; # mm
  $cross_length = 10; 
  $origin_x = $xmargin + $r * $sp * 5.5;
  $origin_y = $ymargin + $r * $sp * 8.5;

  open(PS, ">$ARGV[0]") || die "$!";
  open(MODEL, ">$ARGV[1]") || die "$!";

  write_ps_header();
  origin($origin_x, $origin_y, $cross_thickness, $cross_length);
  for ($j=0; $j<$ny; $j++){
    for ($i=0; $i<$nx; $i++){
      $cx = $xmargin + $r * $sp * $j;
      $cy = $ymargin + $r * $sp * $i;
      if ($i == 0 && $j == 0){
        $rgbcolor = "1 0 0";
        disk($cx, $cy, $disk_r, $linewidth, $rgbcolor);
      } elsif ($i == 0 && $j == $ny-1){
        $rgbcolor = "0 1 0";
        disk($cx, $cy, $disk_r, $linewidth, $rgbcolor);
      } elsif ($i == $nx-1 && $j == 0){
        $rgbcolor = "0 0 1";
        disk($cx, $cy, $disk_r, $linewidth, $rgbcolor);
      } elsif ($i == $nx-1 && $j == $ny-1){
        $rgbcolor = "0.5 0 0.5";
        disk($cx, $cy, $disk_r, $linewidth, $rgbcolor);
      } else {
        printf "Model is broken\n";
        $x = ($cx - $r * (1.5 + $sp * $ny /2)) / 1000;
        $y = ($cy - $r * (1.5 + $sp * $nx /2)) / 1000;
        printf MODEL "%+8.5f %+8.5f\n", $y, $x;
        $rgbcolor = "0 0 0";
        if (($i + $j) % 2){
          target1($cx, $cy, $r, $linewidth, $rgbcolor);
        } else {
          target2($cx, $cy, $r, $linewidth, $rgbcolor);
        }
      }
    }
  }

  close PS;
  close MODEL;

  return 0;
}

exit main();


__END__
ps2pdf -dAutoRotatePages=/None test.ps
pdftoppm -r 90 test.pdf > test.ppm
pnmpad -white -left=17 -right=17 -top=2 -bottom=1 test.ppm > projector_cal_target.ppm
