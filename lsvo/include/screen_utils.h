//This determines screen-coordinates by "sticking" the screen to the nearest plane that is axis alined to one axis
__device__ __inline__ int2 screenCoord(const optix::float3& hitPoint,const optix::float3& normal, int screenWidth, int screenHeight) {


      float cosX=abs(normal.x);
      float cosY=abs(normal.y);
      float cosZ=abs(normal.z);
      float3 xVec,yVec;
      float maxVal=0;
      if (cosX>cosY)
      {
       
        maxVal=cosX;
        xVec=make_float3(0,1,0);
        yVec=make_float3(0,0,1);

      }
      else
      {
       
        maxVal=cosY;
        xVec=make_float3(1,0,0);
        yVec=make_float3(0,0,1);
      }
      if(cosZ>maxVal)
      {
       
        maxVal=cosZ;
        xVec=make_float3(1,0,0);
        yVec=make_float3(0,1,0);

      }
      int xCoord=(int)10*dot(hitPoint,xVec)*screenWidth;
      int yCoord=(int)10*dot(hitPoint,yVec)*screenHeight;
      xCoord%=screenWidth;
      yCoord%=screenHeight;

      return make_int2(xCoord,yCoord);
}
