set yrange[-1:10]
set title "Magnification data"
set xlabel "x position"
set ylabel "distance from original point"

plot "output.dat" using 1:2 title 'Linear' with lines , \
     "output.dat" using 1:3 title 'Fisheye' with lines , \
     "output.dat" using 1:4 title 'Original' with lines  

pause -1
