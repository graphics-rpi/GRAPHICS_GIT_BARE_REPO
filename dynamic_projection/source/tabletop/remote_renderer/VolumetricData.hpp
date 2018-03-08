#include <Image.h>
#include <math.h>
#define GL_GLEXT_PROTOTYPES
#define GL_API
#include <GL/gl.h>

class VolumetricData {
public:
  VolumetricData() {
    data = NULL;
  }

  ~VolumetricData() {
    if (data){
      delete [] data;
    }
  }

  void load(int rows, int cols, int first_slice, int last_slice,
            const char *file_template){
    char filename[1024];
    this->rows = rows;
    this->cols = cols;
    this->slices = last_slice - first_slice + 1;


    data = new sRGB[rows*cols*slices];
    byte *line = new byte[cols*2];
    for (int i=first_slice; i<=last_slice; i++){
      snprintf(filename, 1024, file_template, i);
      FILE *fp = fopen(filename, "rb");
      assert (NULL != fp);
      for (int row=0; row<rows; row++){
        fread(line, 2, cols, fp);
        for (int col=0; col<cols; col++){
          int val = (int(line[2*col]) << 8) + int(line[2*col+1]);
          double gamma = 1.8;
	  gamma = 2.2;
          double v = pow(double(val)/255., gamma);
          byte b = std::max(std::min(int(v), 255), 0);
          data[col + row * cols + (i-first_slice) * rows * cols] = sRGB(b,b,b);
        }
      }
      fclose(fp);
    }
    delete [] line;
  }

  byte *getData(){
    return (byte*)data;
  }

  int getRows(){
    return rows;
  }

  int getCols(){
    return cols;
  }

  int getSlices(){
    return slices;
  }

  uint getTextureID(){
    return texture_id;
  }

  void initTexture(){
    glEnable(GL_TEXTURE_3D);
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_3D, texture_id);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_MODULATE); 
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
    GLfloat color[3] = {0.f, 0.f, 0.f};
    glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, color);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB8, cols,
                 rows, slices, 1, GL_RGB, 
                 GL_UNSIGNED_BYTE, (byte*)data);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
  }

private:
  int rows, cols, slices;
  sRGB *data;
  uint texture_id;
  VolumetricData(const VolumetricData &v);
  VolumetricData& operator=(const VolumetricData &v);
};


