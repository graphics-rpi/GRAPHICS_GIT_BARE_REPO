#!/usr/bin/perl
#
# make_cal_target.pl: create planar camera calibration target
#

sub write_ps_header(){
    print <<"EOF";
%!PS-Adobe-2.0
%%Orientation: Landscape
%%BeginFeature: *PageSize Legal
<</PageSize[612 792]/ImagingBBox null/Orientation 3>>setpagedevice
%%EndFeature

EOF
}

sub target1($$$$$){
    my ($cx, $cy, $r, $linewidth, $rgbcolor) = @_;
    $cx = 72 * $cx / 25.4;
    $cy = 72 * $cy / 25.4;
    $r = 72 * $r / 25.4;
    $linewidth = 72 * $linewidth / 25.4;
    print <<"EOF";

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
    print <<"EOF";

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
    print <<"EOF";

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


#
# main code
#

$page_width_mm = 279;
$page_height_mm = 216;

$nx = 8;
$ny = 6;

$sp = 3.25;
$r = $page_width_mm / (4 + $sp * $nx);

$linewidth = 0.15 * $r;

write_ps_header();
for ($i=0; $i<$nx; $i++){
    for ($j=0; $j<$ny; $j++){
	$cx = $r * (3 + $sp * $j);
	$cy = $r * (3 + $sp * $i);
	if ($i == 0 && $j == 0){
	    $rgbcolor = "1 0 0";
	    disk($cx, $cy, $r, $linewidth, $rgbcolor);
	} elsif ($i == 0 && $j == $ny-1){
	    $rgbcolor = "0 1 0";
	    disk($cx, $cy, $r, $linewidth, $rgbcolor);
	} elsif ($i == $nx-1 && $j == 0){
	    $rgbcolor = "0 0 1";
	    disk($cx, $cy, $r, $linewidth, $rgbcolor);
	} elsif ($i == $nx-1 && $j == $ny-1){
	    $rgbcolor = "0.5 0 0.5";
	    disk($cx, $cy, $r, $linewidth, $rgbcolor);
	} else {
	    $rgbcolor = "0 0 0";
	    if (($i + $j) % 2){
		target1($cx, $cy, $r, $linewidth, $rgbcolor);
	    } else {
		target2($cx, $cy, $r, $linewidth, $rgbcolor);
	    }
	}
    }
}



__END__
ps2pdf -dAutoRotatePages=/None test.ps
