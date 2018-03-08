#include "mesh.h"
#include "../../common/Image.h"

#include <string>
#include <cstdio>

#include "canvas.h"
#include "argparser.h"

#define NUM_VERTICIES 6

const GLfloat Mesh::g_vertex_buffer_data_[] = {
    // Square one
    -2.00f, -2.00f, 0.00f,
    2.00f, -2.00f, 0.00f,
    2.00f,  2.00f, 0.00f,
    2.00f,  2.00f, 0.00f,
    -2.00f,  2.00f, 0.00f,
    -2.00f, -2.00f, 0.00f
};

const GLfloat Mesh::g_uv_buffer_data_[] = {
    // Square one
    0, 0,
    1, 0,
    1, 1,
    1, 1,
    0, 1,
    0, 0
};

Mesh::~Mesh(){
    Cleanup();
}

void Mesh::Normalize( glm::vec4 & input ) const{
    glm::vec3 min = Min();
    glm::vec3 max = Max();
    for( int i = 0; i < 3; i++){
        if (max[i] - min[i] > 0.001f ){
            input[i] = (input[i] - min[i]) / (max[i] - min[i]);
        }
    }
}

glm::vec3 Mesh::Min() const{
    glm::vec3 result = glm::vec3(1000.0f, 1000.0f, 1000.0f);
    for( int i = 0; i < NUM_VERTICIES * 3; i++ ){
        if( g_vertex_buffer_data_[i] < result[i % 3]){
            result[i % 3] = g_vertex_buffer_data_[i];
        }
    }
    return result;
}

glm::vec3 Mesh::Max() const{
    glm::vec3 result = glm::vec3(-1000.0f, -1000.0f, -1000.0f);
    for( int i = 0; i < NUM_VERTICIES * 3; i++ ){
        if( g_vertex_buffer_data_[i] > result[i % 3]){
            result[i % 3] = g_vertex_buffer_data_[i];
        }
    }
    return result;
}

void Mesh::Initialize(){
    printf("Mesh Initialize\n"); 

    // Setup model matrix for use in GLSL
    // model is at origin!
    model_ = glm::mat4(1.0f);

    // Generate and bind VAO buffer ( Vertex Array Object )
    glGenVertexArrays(1, &mesh_vao_);
    glBindVertexArray(mesh_vao_);

    // Generate and bind VBO buffer for verticies ( Vertex Buffer Object ) 
    glGenBuffers(1, &mesh_verticies_vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, mesh_verticies_vbo_);

    // populate VBO buffer for verticies
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data_), g_vertex_buffer_data_, GL_STATIC_DRAW);

    // Generate and bind VBO buffer for UV coordinates ( Vertex Buffer Object ) 
    glGenBuffers(1, &mesh_uv_vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, mesh_uv_vbo_);

    // populate VBO buffer for UV coordinates
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data_), g_uv_buffer_data_, GL_STATIC_DRAW);

    // Setup VAO to prevent constant binding of attributes.

    // Enable and bind the VBO buffer to the first attribute for the vertex shader
    // this buffer deals with vertex positions.
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, mesh_verticies_vbo_);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );

    // Enable and bind the VBO buffer to the second attribute for the vertex shader
    // this buffer deals with texture coordinates.
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, mesh_uv_vbo_);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );

    glBindVertexArray(0);
    return;
}

void Mesh::Draw(){

    // Bind the texture to the 0th slot, link uniform location for fragment shader.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_);

    Canvas::HandleGLError( "BEFORE BIND VAO" );
    glBindVertexArray(mesh_vao_);

    // Draw the triangles!
    Canvas::HandleGLError( "BEFORE DRAW ARRAYS" );
    glDrawArrays(GL_TRIANGLES, 0, 2*3);
    Canvas::HandleGLError( "BEFORE BIND VAO" );
    glBindVertexArray(0);

    return;
}

void Mesh::Cleanup(){
    printf("Mesh Cleanup\n"); 
    glDeleteVertexArrays(1, &mesh_vao_);
    glDeleteBuffers(1, &mesh_verticies_vbo_);
    glDeleteBuffers(1, &mesh_uv_vbo_);
    glDeleteTextures(1, &texture_);
    return;
}

void Mesh::LoadTexture(const std::string &filename){
    printf("Loading image: %s\n", filename.c_str());
    texture_ = loadBMP_custom(filename.c_str());
    /* Image<sRGB> image;
       image.load_from_file(filename.c_str());

       int w = image.getCols();
       int h = image.getRows();
       GLubyte * gl_image = new GLubyte[w*h*3];

       for(int i = 0; i < h; i++ ){
       for( int j = 0; j < h; j++) {
       sRGB c = image(i,j);
       GLubyte r = c.r();
       GLubyte g = c.g();
       GLubyte b = c.b();
       gl_image[i * w * 3 + j * 3 + 0] = r;
       gl_image[i * w * 3 + j * 3 + 1] = g;
       gl_image[i * w * 3 + j * 3 + 2] = b;
       } 
       }

       glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
       HandleGLError("Load Image 1");
       glGenTextures(1, &texture);
       HandleGLError("Load Image 2");

       glActiveTexture(GL_TEXTURE0);
       HandleGLError("Load Image 3");
       glBindTexture(GL_TEXTURE_2D, texture); //exName);
       HandleGLError("Load Image 4");
       glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
       HandleGLError("Load Image 5");
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
       HandleGLError("Load Image 6");
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
       HandleGLError("Load Image 7");
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
       HandleGLError("Load Image 8");
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
       HandleGLError("Load Image 9");

       gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, w, h, GL_RGB, GL_UNSIGNED_BYTE, gl_image);
       HandleGLError("Load Image 10");

       delete [] gl_image;*/
    return;
}
GLuint Mesh::loadBMP_custom(const char * imagepath){

    printf("Reading image %s\n", imagepath);

    // Data read from the header of the BMP file
    unsigned char header[54];
    unsigned int dataPos;
    unsigned int imageSize;
    unsigned int width, height;
    // Actual RGB data
    unsigned char * data;

    // Open the file
    FILE * file = fopen(imagepath,"rb");
    if (!file)  {printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); return 0;}

    // Read the header, i.e. the 54 first bytes

    // If less than 54 byes are read, problem
    if ( fread(header, 1, 54, file)!=54 ){ 
        printf("Not a correct BMP file\n");
        return 0;
    }
    // A BMP files always begins with "BM"
    if ( header[0]!='B' || header[1]!='M' ){
        printf("Not a correct BMP file\n");
        return 0;
    }
    // Make sure this is a 24bpp file
    if ( *(int*)&(header[0x1E])!=0  ) {printf("Not a correct BMP file\n");    return 0;}
    if ( *(int*)&(header[0x1C])!=24 ) {printf("Not a correct BMP file\n");    return 0;}

    // Read the information about the image
    dataPos    = *(int*)&(header[0x0A]);
    imageSize  = *(int*)&(header[0x22]);
    width      = *(int*)&(header[0x12]);
    height     = *(int*)&(header[0x16]);

    // Some BMP files are misformatted, guess missing information
    if (imageSize==0)    imageSize=width*height*3; // 3 : one byte for each Red, Green and Blue component
    if (dataPos==0)      dataPos=54; // The BMP header is done that way

    // Create a buffer
    data = new unsigned char [imageSize];

    // Read the actual data from the file into the buffer
    fread(data,1,imageSize,file);

    // Everything is in memory now, the file wan be closed
    fclose (file);

    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Give the image to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

    // OpenGL has now copied the data. Free our own version
    delete [] data;

    // Poor filtering, or ...
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 

    // ... nice trilinear filtering.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER /*GL_REPEAT*/);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER /*GL_REPEAT*/);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
    glGenerateMipmap(GL_TEXTURE_2D);

    // Return the ID of the texture we just created
    return textureID;
}
