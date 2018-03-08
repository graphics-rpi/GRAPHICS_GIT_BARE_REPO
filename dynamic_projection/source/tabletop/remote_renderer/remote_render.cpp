#include <mpi.h>
#include "shader.h"
#include "remote_render.h"
#include "SocketReader.h"


char large_buffer[UNCOMP_BUFF_SIZE];
char uncompressed_buffer[UNCOMP_BUFF_SIZE];
char compressed_buffer[COMP_BUFF_SIZE];
char *walls[5];
string sides[5];
bool texture_changed[5] = {true, true, true, true, true};
bool renderGeom=true;
bool puzzle,puzzlefirst;
int RanK;
int numGenTextures=0;

void RemoteProjector::LoadShader(){    
  // init glew for shaders
  GLenum err = glewInit();
  if (GLEW_OK != err){ 
    printf("Error: %s\r\n", glewGetErrorString(err));
  }

  if(!(GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader)){     
    std::cerr << "Error: missing GL extensions" << std::endl;
    return;
  }

  // load vertex shaders
  sh_weights = new ShaderSet();
  sh_no_weights = new ShaderSet();

  // load fragment shaders
  //CHANGED
  sh_weights->loadVertexShader("../remote_renderer/vert.glsl");
  //CHANGED
  sh_no_weights->loadVertexShader("../remote_renderer/vert.glsl");

  // load frag shader with color weights
  //CHANGED
  sh_weights->loadFragmentShader("../remote_renderer/weight_colors.glsl");
      
  // read RBG weights from config file
  float r, g, b;
  r = g = b = 1;
    
  char filename[265];
  sprintf(filename, "color_weights/color_weights_%d.dat", RanK); //projector_id);
  std::ifstream fin(filename);
  fin >> r >> g >> b;
    
  (*ARGS->output) << "RGB = " << r << " " << g << " " << b << "\r\n";
    
  sh_weights->initUniform3f("weights", r, g, b);
    
  //CHANGED
  sh_no_weights->loadFragmentShader("../remote_renderer/frag.glsl");

  if(use_color_weights){
    sh_weights->enable();
  }
  else{
    sh_no_weights->enable();
  }
}

void RemoteProjector::LoadCompressedTexture_z(GLuint texture_idx, SocketReader & sr, RRTextureMapMap & rrmap) {

  //(*ARGS->output) << "loading texturez: " << texture_idx << "\r" << std::endl;


  //If we're using stored textures    
  if(ARGS->use_stored_textures)
    {
      char filename[100] ; 
      sprintf(filename, "%s%s", "/ramdisk/images/surface_camera_", rrmap.getTextureFilename(texture_idx).c_str());
      newLoadTexture(texture_idx,  filename);  

      return;
    }

  //If we're not using stored textures    
  bool first=false;
  assert(GL_NO_ERROR==glGetError());
   
  int height,width, bytes;
  int compressed_size;
  char line[LINE_SIZE]; 
  sr.Jgets(line,LINE_SIZE); 

  unsigned long restored_size;
  sscanf(line,"%i %i %i",&height, &width,&compressed_size);

  //Need to abort calls if we're using stored textures
  if(ARGS->use_stored_textures)


    glEnable(GL_TEXTURE_2D);    

  assert(GL_NO_ERROR==glGetError());    
  if (textures.size() < (texture_idx+1)){
    textures.resize(texture_idx+1,0);
  }
  if(0==textures.at(texture_idx))
    {
      GLuint texture_id;
      glGenTextures(1, &texture_id);
      first=true;
      textures.at(texture_idx) = texture_id;
      numGenTextures++;
 
    }

  glBindTexture(GL_TEXTURE_2D, textures.at(texture_idx));
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);    
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
  
 

  unsigned long unc=UNCOMP_BUFF_SIZE;
 
  assert(GL_NO_ERROR==glGetError());
  unsigned long long_compressed_size=compressed_size;
  MPI_Status status;

#ifdef USE_MPI
  MPI_Recv(    compressed_buffer, compressed_size, MPI_BYTE, 8, 2, MPI_COMM_WORLD, &status);
#else
  sr.Jread((char*)compressed_buffer, 1, compressed_size);
#endif
  //MPI_Send(file_to_send, compressed_size, MPI_BYTE, rank, 2, MPI_COMM_WORLD);

  int ret_val2 = uncompress((Bytef*)uncompressed_buffer,  &unc,
			    (Bytef*)compressed_buffer, long_compressed_size);

  assert(Z_OK==ret_val2);


  //----------------
  // dump the texture buffer to an image
  //Image<sRGB> received_texture(512, 512, (sRGB*)uncompressed_buffer);
  //char received_filename[256];
  //sprintf(received_filename, "/ramdisk/received_%i.ppm", texture_idx);
  //received_texture.write(received_filename);
  //printf("received texture saved\r\n");
  //----------------
  
  assert (Z_OK == ret_val2);
  if(first){
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 

		 width, height, 0,
		 GL_RGB, GL_UNSIGNED_BYTE,
		 (void*)uncompressed_buffer);
  }
  else{
    glTexSubImage2D(GL_TEXTURE_2D, 0,

		    0, 0, width, height,
		    GL_RGB, GL_UNSIGNED_BYTE,
		    (void*)uncompressed_buffer);
  }
  assert(GL_NO_ERROR==glGetError());

}



void RemoteProjector::ParseCommandStream(SocketReader & sr, const LI::Window& w){

  (*ARGS->output) << "In RemoteProjector::ParseCommandStream()\r" << std::endl;

  int count=0;

  MPI_Comm_rank ( MPI_COMM_WORLD, &RanK );

  LoadShader();

  // set the blending id to be the rank
  mesh.setBlendingID(RanK);
  mesh.setBlendingBrightness(1);

  char textline[LINE_SIZE];
  puzzle=false;
  puzzlefirst=true;
  bool once_per_five=false;
  v3d center;
  v3d targetCenter;
  double targetRadius;
  int score1, score2;
  int rows, cols, first_slice, last_slice;
  double x_court_dim, z_court_dim;
  char file_template[1024];
  int texture_id;
  int blending_id;
  int pen_detected, penVertical, targetVertical;
  int rgb_hack;

  std::vector<GLuint> space_invaders_textures;

  while(true){
    //usleep(100000);
    count++;

    //printf("before jgets\r\n");
    sr.Jgets(textline, LINE_SIZE);


    std::string text_string = textline;

    if (RanK==0) {
      (*ARGS->output) << "In RemoteProjector::ParseCommandStream() MAIN LOOP " << RanK << " '" 
		<< text_string.substr(0,text_string.size()-1) << "'\r" << std::endl;
    }
    //printf("textline %s \r\n", textline);

 
    if (!strcmp(textline, "BEGIN OBJ_FILE\n")){ 

      mesh.Load(sr);
    } else if (!strcmp(textline, "BEGIN TEXTURE_MAP_MAP\n")){ 
 
      mesh.loadTextureMapMap(sr);
    } else if (!strcmp(textline, "BEGIN BLE_FILE\n")){ 

      mesh.loadBleFile(sr);

    } else if (1 == sscanf(textline, "SWITCH TO PUZZLE %d\n",&rgb_hack)){

      puzzle=true;
      mesh.LoadPuzzleFile(sr, sides, texture_changed);
      LoadPuzzleTextures(rgb_hack);
      puzzlefirst=false;


    }else if (1 == sscanf(textline, "SWITCH TO MULTIDISPLAY %d\n",&rgb_hack)){

      mesh.LoadMultidisplay(sr, sides, texture_changed);

    } else if (!strcmp(textline, "DISABLE BLENDING\n")){ 
  
      mesh.disableBlending();

    } else if(!strcmp(textline, "TOGGLE COLOR WEIGHTS\n")){
      ToggleColorWeights();
    } else if (!strcmp(textline, "BEGIN MTL_FILE\n")){ 
  

      mesh.loadMtlFile(sr, volume_texture_enable);

    } else if (7 == sscanf(textline, "PONG %lf %lf %lf %lf %lf %d %d",
			   &center.x(), &center.y(), &center.z(),
			   &x_court_dim, &z_court_dim,
			   &score1, &score2)){
	
      pong(center, x_court_dim, z_court_dim, score1, score2);

    } else if(10 == sscanf(textline, "PEN_DEMO %d %lf %lf %lf %d %lf %lf %lf %d %lf",
			   &pen_detected, &center.x(), &center.y(), 
			   &center.z(), &penVertical, &targetCenter.x(), 
			   &targetCenter.y(), &targetCenter.z(), &targetVertical,
			   &targetRadius)){
      pen_demo(pen_detected, center, penVertical, targetCenter, targetVertical, targetRadius);
    } else if(std::string(textline).find("SPACE_INVADERS") == 0){
      if(space_invaders_textures.empty()){
	load_space_invaders_textures(space_invaders_textures);
      }
      space_invaders(std::string(textline), space_invaders_textures);
    } else if (1 == sscanf(textline, "LOAD TEXTURE %d", &texture_id)){
  
#ifdef COMPRESSED_TEX

      LoadCompressedTexture_z( texture_id,sr, mesh.getRRTextureMapMap());
        
#else
      //printf("loading compressed texture %i\r\n",texture_id);
      Image<sRGB> image(sr);
      DisplayImage(image);
      //glXWaitVideoSyncSGI(1, 0, &throwaway);
      w.flushDisplay();
      InitTexture(texture_id, image);
#endif
      //sr.send_ack();
      //sr.send_ack_to_all();
    }/* else if (1 == sscanf(textline, "UPDATE TEXTURE %d", &texture_id)){
	printf("up tex\r\n");
	#ifdef COMPRESSED_TEX
	printf("load compressed tex \r\n");
	LoadCompressedTexture(texture_id, sr);
   
	#else
	Image<sRGB> image(sr);
	UpdateTexture(texture_id, image);
	#endif

	//sr.send_ack_to_all();

	} */

    else if (5 == sscanf(textline, "LOAD 3DTEXTURE %d %d %d %d %1024s", 
			 &rows, &cols, &first_slice, &last_slice, 
			 file_template)){
      volume_data.load(rows, cols, first_slice, last_slice, file_template);
      volume_data.initTexture();

    } else if (!strcmp(textline, "ENABLE VOLUME_TEXTURE\n")){ 

      volume_texture_enable = true;

    } else if (!strcmp(textline, "DISABLE VOLUME_TEXTURE\n")){ 

      volume_texture_enable = false;

    } else if (1 == sscanf(textline, "BLENDING ID %d", &blending_id)){

      mesh.setBlendingID(blending_id);

    } else if (!strcmp(textline, "GLCAM\n")){

      gl_projector.Load(sr);
      gl_projector.setOpenGLCamera();

    } else if (!strcmp(textline, "ADD GLCAM\n")){

      //gl_projectors.push_back();
      //gl_projectors[gl_projectors.size()-1].Load(sr);

    } else if (!strcmp(textline, "DISPLAY IMAGE\n")){

     
      Image<sRGB> image(sr);
      DisplayImage(image);
      w.flushDisplay();

    } else if (!strcmp(textline, "BLANK\n")){
      Blank();	

      w.flushGL();
      w.flushDisplay();
	
      //Make sure all processes are here before taking a picture
      MPI_Barrier(MPI_COMM_WORLD);

      //Once the picture is taken it is safe to turn the projectors back on
      MPI_Barrier(MPI_COMM_WORLD);

    } else if (!strcmp(textline, "RENDER\n")){

      Render();

    } else if (!strcmp(textline, "FLIP\n")){

      w.flushGL();    
      MPI_Barrier(MPI_COMM_WORLD);
      w.flushDisplay();

    } else if (!strcmp(textline, "QUIT\n")){
 
      MPI_Finalize();
      exit(0);
 
    } else{
      printf("textline %s \r\n", textline);
      printf("missed all conditions\r\n");
      assert(0);
    }
  }
}



void RemoteProjector::Blank(){
  assert(GL_NO_ERROR==glGetError());
  glClearColor(0.0,0.0,0.0,0); 
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  
  //printf("renderers blanking %i\r\n", RanK);
  assert(GL_NO_ERROR==glGetError());

}




void RemoteProjector::Render(){
  assert(GL_NO_ERROR==glGetError());
  glClearColor(0.0,0.0,0.0,0);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  assert(GL_NO_ERROR==glGetError());
  // render with sRGB                                                                                                                  
#ifdef GL_EXT_framebuffer_sRGB
  //glEnable(GL_FRAMEBUFFER_SRGB_EXT);                                                                                                 
  assert(GL_NO_ERROR==glGetError());
#endif
  assert(GL_NO_ERROR==glGetError());
  v3d center;
  gl_projector.getCenter(center.x(), center.y(), center.z());
  assert(GL_NO_ERROR==glGetError());
                                                                                                
  mesh.Render(center, textures,          volume_data.getTextureID(),             volume_texture_enable);
                                                                                               

#ifdef GL_EXT_framebuffer_sRGB
  //  glDisable(GL_FRAMEBUFFER_SRGB_EXT);                                                                                              
#endif
  assert(GL_NO_ERROR==glGetError());
  
  /*
  // visualizing the depth buffer
  // save the depth buffer to a file
  float* depthData = new float[1024 * 768];
  glReadPixels(0, 0, 1024, 768, GL_DEPTH_COMPONENT, GL_FLOAT, depthData);


  //Pixel p[1024 * 768];
  
  float n = 0.1;
  float f = 10.0;
  
  for (int i = 0; i < 1024 * 768; i++) {
    float z = depthData[i];
    depthData[i] = z * 255;//(2.0 * n) / (f + n - z * (f - n)) * 255;
    //std::cout << depthData[i] << std::endl;
  }

  //glDrawPixels(1024, 768, GL_LUMINANCE, GL_FLOAT, depthData);
  Image<sRGB> image(768, 1024, depthData);
  image.write("/home/greg/depth.ppm");
  delete [] depthData;
  */

  // start frankensteining in the shadowmap
  //renderToFbo(center);


  assert(GL_NO_ERROR==glGetError());
  
  // draw text over everything else
  //drawCircularString("Welcome");
  
}


void RemoteProjector::renderToFbo(v3d center) {
  // ----------------------------------
  //            FRANKENSTEIN;
  //                 or,
  //        the modern prometheus
  //            in one volume
  // ----------------------------------
  // attempt to render the depth buffer seperately to a frame buffer object
  // some screen vars
  int RENDER_WIDTH = 1024;
  int RENDER_HEIGHT = 768;
  int SHADOW_MAP_RATIO = 2;
  GLuint fboId;
  GLuint depthTextureId;

  // first set up the fbo 
  int shadowMapWidth = RENDER_WIDTH * SHADOW_MAP_RATIO;
  int shadowMapHeight = RENDER_HEIGHT * SHADOW_MAP_RATIO;
	
  //GLfloat borderColor[4] = {0,0,0,0};
	
  GLenum FBOstatus;
	
  // Try to use a texture depth component
  glGenTextures(1, &depthTextureId);
  glBindTexture(GL_TEXTURE_2D, depthTextureId);
	
  // GL_LINEAR does not make sense for depth texture. However, next tutorial shows usage of GL_LINEAR and PCF
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
  // Remove artefact on the edges of the shadowmap
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	
  //glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor );
	
	
	
  // No need to force GL_DEPTH_COMPONENT24, drivers usually give you the max precision if available 
  glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
	
  // create a framebuffer object
  glGenFramebuffersEXT(1, &fboId);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);
	
  // Instruct openGL that we won't bind a color texture with the currently binded FBO
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
	
  // attach the texture to FBO depth attachment point
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D, depthTextureId, 0);
	
  // check FBO status
  FBOstatus = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  if(FBOstatus != GL_FRAMEBUFFER_COMPLETE_EXT)
    printf("GL_FRAMEBUFFER_COMPLETE_EXT failed, CANNOT use FBO\n");
	
  // switch back to window-system-provided framebuffer
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  
  // DONE setting up

  //First step: Render from the light POV to a FBO, story depth values only
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,fboId);	//Rendering offscreen	
  //Using the fixed pipeline to render to the depthbuffer
  glUseProgramObjectARB(0);
  // In the case we render the shadowmap to a higher resolution, the viewport must be modified accordingly.
  glViewport(0,0,RENDER_WIDTH * SHADOW_MAP_RATIO, RENDER_HEIGHT * SHADOW_MAP_RATIO);
  // Clear previous frame values
  glClear( GL_DEPTH_BUFFER_BIT);
  // this is where we would swap in the matrixes for the other projecters once this one works
  // swap swap swap
  // Culling switching, rendering only backface, this is done to avoid self-shadowing
  glCullFace(GL_FRONT);
  // now draw everything
  mesh.Render(center, textures, volume_data.getTextureID(), volume_texture_enable);
  //Save modelview/projection matrice into texture7, also add a biais
  static double modelView[16];
  static double projection[16];
	
  // This is matrix transform every coordinate x,y,z
  // x = x* 0.5 + 0.5 
  // y = y* 0.5 + 0.5 
  // z = z* 0.5 + 0.5 
  // Moving from unit cube [-1,1] to [0,1]  
  const GLdouble bias[16] = {	
    0.5, 0.0, 0.0, 0.0, 
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 0.5, 0.0,
    0.5, 0.5, 0.5, 1.0};
	
  // Grab modelview and transformation matrices
  glGetDoublev(GL_MODELVIEW_MATRIX, modelView);
  glGetDoublev(GL_PROJECTION_MATRIX, projection);
	
	
  glMatrixMode(GL_TEXTURE);
  glActiveTextureARB(GL_TEXTURE7);
	
  glLoadIdentity();	
  glLoadMatrixd(bias);
	
  // concatating all matrice into one.
  glMultMatrixd (projection);
  glMultMatrixd (modelView);
	
  // Go back to normal matrix mode
  glMatrixMode(GL_MODELVIEW);
  
  glUseProgramObjectARB(0);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-RENDER_WIDTH/2,RENDER_WIDTH/2,-RENDER_HEIGHT/2,RENDER_HEIGHT/2,1,20);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glColor4f(1,1,1,1);
  glActiveTextureARB(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D,depthTextureId);
  glEnable(GL_TEXTURE_2D);
  glTranslated(0,0,-1);
  glBegin(GL_QUADS);
  glTexCoord2d(0,0);glVertex3f(0,0,0);
  glTexCoord2d(1,0);glVertex3f(RENDER_WIDTH/2,0,0);
  glTexCoord2d(1,1);glVertex3f(RENDER_WIDTH/2,RENDER_HEIGHT/2,0);
  glTexCoord2d(0,1);glVertex3f(0,RENDER_HEIGHT/2,0);
	 
  glEnd();
  glDisable(GL_TEXTURE_2D);

  // add more from the shadowmap example here i think


  float* depthData = new float[1024 * 768];
  glReadPixels(0, 0, 1024, 768, GL_DEPTH_COMPONENT, GL_FLOAT, depthData);


  float n = 0.1;
  float f = 10.0;
  
  for (int i = 0; i < 1024 * 768; i++) {
    float z = depthData[i];
    depthData[i] = z * 255;//(2.0 * n) / (f + n - z * (f - n)) * 255;
    //std::cout << depthData[i] << std::endl;
  }

  //glDrawPixels(1024, 768, GL_LUMINANCE, GL_FLOAT, depthData);
  Image<sRGB> image(768, 1024, depthData);
  image.write("/home/greg/depth.ppm");
  delete [] depthData;

  // haven't yet checked out how to get back to normal rendering mode so crash
  assert(0);
}
