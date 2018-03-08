REMESH_LOG=remesh_log.txt

glcam0=/home/grfx/Checkout/archdisplay/projector_A.glcam
glcam1=/home/grfx/Checkout/archdisplay/projector_B.glcam
glcam2=/home/grfx/Checkout/archdisplay/projector_C.glcam
glcam3=/home/grfx/Checkout/archdisplay/projector_D.glcam
glcam4=/home/grfx/Checkout/archdisplay/projector_E.glcam
glcam5=/home/grfx/Checkout/archdisplay/projector_F.glcam

pushd geometry
~/GIT_CHECKOUT/remesher/build/remesh -i ../terrain.army -army_tweening -o foo.obj -p 6 $glcam0 $glcam1 $glcam2 $glcam3 $glcam4 $glcam5 -no_remesh -offline
#~/Checkout/remesher/remesh -i ../barb_test_file.army -army_tweening -o foo.obj -p 6 $glcam0 $glcam1 $glcam2 $glcam3 $glcam4 $glcam5 -no_remesh
cp * ~/tween/
echo DONE REMESHING
popd
