ext=".jpg"
target="ppm"
 
for i in *.jpg
do
  base=$(basename $i $ext) #to extract only the filename and NOT the extension
  convert $i $base.$target
done
