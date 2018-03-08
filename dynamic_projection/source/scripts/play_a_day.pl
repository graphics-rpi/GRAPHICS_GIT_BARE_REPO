#!/usr/bin/perl
#
# play a day for EMPAC
#
# play_a_day.pl <day_file>
#
use Time::HiRes qw( time usleep );
use File::Copy;


#
# configurable parameters
#
$day_file = "/ramdisk/specified.date";
#$obj_file = "/home/grfx/slow/foo_1000.obj";
#$mtl_file = "/home/grfx/slow/foo_1000.lsvmtl";
#$north_file = "/home/grfx/slow/foo_1000.north";
$obj_file = "/ramdisk/slow/foo.obj";
$mtl_file = "/ramdisk/slow/foo.lsvmtl";
$north_file = "/ramdisk/tween/foo.north";
$time_file = "/ramdisk/out.time";
$LSVIEW="~/Checkout/archlight3/bin/lsv";
#$LSVIEW="/home/mint/work/cvs/archlight3/bin/lsv";
$lsv_size = "512 512";
$LSVIEW_LOG="lsview_log.txt";
$output_dir="/ramdisk/images/";

# lsv signals
$CMD_UPDATE_TIME="RTMIN";
$CMD_UPDATE_GEOMETRY="NUM35";
$CMD_RELOAD="NUM36";
$CMD_EXIT="NUM37";

#	    " -il naive ".
#	    "ls *.glcam | xargs " .
#	    $LSVIEW . 
#	    " -il lab_hier_cuda".
#	    " -coarse 100 ".

sub start_lsv(){
    $LSV_PID = fork();
    if (! $LSV_PID){
	$lsv_cmd = 
	    $LSVIEW .
	    " -i $obj_file".
	    " -ml $mtl_file".
	    " -size $lsv_size".
	    " -tf $time_file".
	    " -exposure $ARGV[1] ".
	    " -svm final ".
	    " -hidegui".
	    " -vm ";
	    
	# list all the glcam files
	opendir(DIR, "/ramdisk/slow");
	@files = grep(/\.glcam$/,readdir(DIR));
	closedir(DIR);
	# attach all the files to the end
	foreach $file (@files) {
	    $lsv_cmd .= $file . " ";
	}
	
	print $lsv_cmd;
	print "\n";
  sleep 4;
	exec ($lsv_cmd) or die "cannot execute lsv:$!\n";
    }
}

sub cleanup(){
    #blank_projectors;
    kill $CMD_EXIT, $LSV_PID;
}

sub write_time(){
    $hour = int($time / 60);
    $minute = $time - 60 * $hour;
    open (FH, ">$time_file") || die "$!";
    print FH "t $month $day $hour:$minute -1 $north\n";
    close FH;
}

 #parse the north file to get the north direction
open (FH, "<$north_file") || die "$!";
$north = <FH>;
close FH;

#
# parse the day file
#
$found = 0;
open (FH, "<$day_file") || die "$!";
while (<FH>){
    if (/(\d+)\s+(\d+)\s+(\d+:?(?:\d+)?)\s+(\d+:?(?:\d+)?)\s+(\d+:?(?:\d+)?)/){
	$month = $1;
	$day = $2;
	$start = $3;
	$end = $4;
	$interval = $5;
	$found = 1;
    }
}
close FH;

if (!$found){
    print STDERR "Invalid day file\n";
    exit -1;
}
	
($start_hour, $start_minute) = ($start =~ /(\d+):?(\d+)?/);
($end_hour, $end_minute) = ($end =~ /(\d+):?(\d+)?/);
($interval_hour, $interval_minute) = ($interval =~ /(\d+):?(\d+)?/);

$start = 60 * $start_hour + $start_minute;
$end = 60 * $end_hour + $end_minute;
$interval = 60 * $interval_hour + $interval_minute;

#
# signal handler to update frames
#
$time = $start;
$done = 0;
sub next_time(){
    if ($time < $end){
	$time += $interval;
	write_time;

	# send a signal to lsv
	print "send signal $CMD_UPDATE_TIME to pid $LSV_PID\n";
	kill $CMD_UPDATE_TIME, $LSV_PID;

	#kill $CMD_EXIT, $LSV_PID;
	#kill -9, $LSV_PID;
	#start_lsv();
    } else {
	cleanup;
	# wait for a while to let lsv quit
	usleep(1000);
	# make sure lsv ends
	kill -9, $LSV_PID;
	$done = 1;
    }
}

$counter = 0;
sub read_counter() {
    $counter_file = "/ramdisk/images/counter.txt";
    open (FH, "<$counter_file") || die "$!";
    $line = <FH>;
    close FH;

    return $line;
}

#
# play the day
#
#$SIG{$CMD_RELOAD} = \&next_time;
#$SIG{"ALRM"} = \&lsv_timeout;
write_time();
$counter = read_counter();
start_lsv();
sleep(5);  # sleep 5 seconds for the first time
   #exec ("rm /ramdisk/tween/stop");
while (! $done) 
{
  print "in not done while\n";
  #exec ("rm /ramdisk/tween/stop");
  $finish_one_time = 0;
  while (! $finish_one_time) 
  {

	  $new_counter = read_counter();
#   print "in seconde while $counter $new_counter\n";
	  $diff = $new_counter - $counter;
    #  print "diff $diff \n"
	  if($diff == 0) 
    {
	    usleep(2000);
	  } else 
    { # finish one time
	    $finish_one_time = 1;
	    $counter = $new_counter;


      my @old_files = glob "*.ppm";
      my $new_dir = "/ramdisk/images/";

      foreach my $old_file (@old_files)
      {
        move($old_file, $new_dir) or die "Could not move $old_file to $new_dir: $!\n";
      }

#      move ("*.ppm", "/ramdisk/images/") || die "cound not move $old_loc to $new_loc: $!\n";
	  }
  }

  # do for the next time
  print "heading to the next time\n";
  next_time();
}

