#!/usr/bin/perl -w
#
# make_model.pl:
# create model.txt, physical locations of corners on camera calibration target
#

sub main(){
  if (0 && $ARGV[0] =~ /slr/){
    $Nx = 8;
    $Ny = 6;

    #
    # 8.5x11" target
    #
    $dx = 29.375/1000;
    $dy = 29.500/1000;
  } else {
    $Nx = 18;
    $Ny = 12;
    #
    # VCC printed target on 3/16" foam core
    #
    # $dx = 40.400/1000;
    # $dy = 40.545/1000;

    #
    # Bokland printed target on 1/2" Gatorboard
    #
    $dx = 41.667/1000;
    $dy = 41.667/1000;
  }

  $cx =  $dx * ($Nx-1) / 2.0;
  $cy =  $dy * ($Ny-1) / 2.0;

  for ($i=0; $i<$Ny; $i++){
    $y = $i * $dy - $cy;
    for ($j=0; $j<$Nx; $j++){
      $x = $j * $dx - $cx;
      if ($i == 0 && $j == 0 ||
          $i == 0 && $j == $Nx-1 ||
          $i == $Ny-1 && $j == 0 ||
          $i == $Ny-1 && $j == $Nx-1){
        # corner position
      } else {
        print "$x $y\n";
      }
    }
  }

}

exit main();
