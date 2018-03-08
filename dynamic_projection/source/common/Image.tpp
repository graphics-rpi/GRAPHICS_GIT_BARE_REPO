#ifndef __IMAGE_TPP__
#define __IMAGE_TPP__

// full implementation of the templated class
// #included from "Image.h"




// get a number from a PNM file header
inline int get_pnm_number(FILE *fp){
  enum {RESET, NUMBER, COMMENT} state;
  state = RESET;
  int value = 0;
  bool done = false;
  while (!done){
    int c = getc(fp);
    if (isspace(c)){
      switch(state){
      case RESET:
	break;
      case NUMBER:
	done = true;
	break;
      case COMMENT:
	if ('\n' == c ||
	    '\r' == c)
	  state = RESET;
	break;
      }
    } else if (isdigit(c)){
      switch (state){
      case RESET:
	state = NUMBER;
	// fall through
      case NUMBER:
	value = 10 * value + (c - '0');      
	break;
      case COMMENT:
	break;
      }
    } else if ('#' == c){
      state = COMMENT;
    }
  }
  return value;
}


#ifdef MPI_DEFINED
// same as above except read from socket
inline int get_pnm_number(SocketReader & sr){
  enum {RESET, NUMBER, COMMENT} state;
  state = RESET;
  int value = 0;
  bool done = false;
  while (!done){
    int c = sr.Jgetc();
    if (isspace(c)){
      switch(state){
      case RESET:
	break;
      case NUMBER:
	done = true;
	break;
      case COMMENT:
	if ('\n' == c ||
	    '\r' == c)
	  state = RESET;
	break;
      }
    } else if (isdigit(c)){
      switch (state){
      case RESET:
	state = NUMBER;
	// fall through
      case NUMBER:
	value = 10 * value + (c - '0');      
	break;
      case COMMENT:
	break;
      }
    } else if ('#' == c){
      state = COMMENT;
    }
  }
  return value;
}
#endif







// ================================================================

template<> inline void Image<sRGB>::load_ppm(const std::string &filename) {

  //std::cout << "LOADING " << filename << std::endl;
  FILE *fp = fopen(filename.c_str(), "rb");
  if (NULL == fp){
    std::cerr << "unable to open ppm file" << filename.c_str() << "\n";
    throw -1;
  }
  
  char magic[2];
  size_t items_read = fread(magic, sizeof(char), 2, fp);

  if (items_read != 2) { 
    std::cerr << "ERROR READING BYTES\n";
    fclose(fp);
    throw -1;
  }
  if (strncmp(magic, traits.magic(), 2)){
    std::cerr << "Incorrect file type " << magic[0] << " " << magic[1] << "\n";
    fclose(fp);
    throw -1;
  }
  cols = get_pnm_number(fp);
  rows = get_pnm_number(fp);

  //std::cout << "LOADED PPM " << cols << " "<< rows << std::endl;

  int maxval = get_pnm_number(fp);
  if (maxval != traits.pnm_maxval()){
    std::cerr << "Incorrect maxval " << maxval << "\n";
    fclose(fp);
    throw -1;
  }
  data = new sRGB[rows*cols];
  items_read = fread(data, sizeof(sRGB), rows*cols, fp);

  if ((int)items_read != int(rows*cols)) { 
    std::cerr << "ERROR READING BYTES\n";
    fclose(fp);
    throw -1; 
  }
  
  initialized = true;

  min_x = 0;
  max_x = cols-1;
  min_y = 0;
  max_y = rows-1;
  fclose(fp);

  //std::cout << "LOADED PPM " << cols << " "<< rows << std::endl;
}

// ================================================================


METHODDEF(void)
my_error_exit (j_common_ptr /*cinfo*/) {
  std::cerr << "error in jpeg loading... throw exception";
  throw -1;
}


template<> inline void Image<sRGB>::load_jpg(const std::string &filename) {
  FILE *fp = fopen(filename.c_str(), "rb");
  if (NULL == fp){
    std::cerr << "unable to open jpg file" << filename.c_str() << "\n";
    throw -1;
  }
  
  unsigned char r,g,b;
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;

  JSAMPARRAY pJpegBuffer;   	/* Output row buffer */
  int row_stride;   	/* physical row width in output buffer */

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, fp); 
  jerr.error_exit = my_error_exit;

  try {
    (void) jpeg_read_header(&cinfo, TRUE);
    (void) jpeg_start_decompress(&cinfo);
    cols = cinfo.output_width;
    rows = cinfo.output_height;
    data = new sRGB[rows*cols];
    assert (data != NULL);
    
    row_stride = cols * cinfo.output_components ;
    pJpegBuffer = (*cinfo.mem->alloc_sarray)
      ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
    
    int index = 0;
    while (cinfo.output_scanline < cinfo.output_height) {
      (void) jpeg_read_scanlines(&cinfo, pJpegBuffer, 1);
      for (int x=0;x<cols;x++) {
	r = pJpegBuffer[0][cinfo.output_components*x];
	if (cinfo.output_components>2)
	  {
	    g = pJpegBuffer[0][cinfo.output_components*x+1];
	    b = pJpegBuffer[0][cinfo.output_components*x+2];
	  } else {
	  g = r;
	  b = r;
	}
        data[index] = sRGB(r,g,b);
        index++;
      }
    }
    
    (void) jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
  } catch (...) {
    fclose(fp);
    throw -1;
  }  

  min_x = 0.;
  max_x = cols-1;
  min_y = 0.;
  max_y = rows-1;
  fclose(fp);

  initialized = true;
}


// ================================================================

template<> inline void Image<sRGB>::load_png(const std::string &filename) {

  FILE *fp = fopen(filename.c_str(), "rb");
  if (NULL == fp){
    std::cerr << "unable to open png file" << filename.c_str() << "\n";
    throw -1;
  }
  
  // read the header
  png_byte header[8];
  size_t items_read = fread(header, 1, 8, fp);
  if (items_read != 8) {
    assert(0);
    throw -1;
  } 

  if (png_sig_cmp(header, 0, 8)) {
    std::cerr << "error: not a PNG.\n";
    fclose(fp);
    throw -1;
  }

  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
    std::cerr << "error: png_create_read_struct returned 0.\n";
    fclose(fp);
    throw -1; 
  }

  // create png info struct
  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    std::cerr << "error: png_create_info_struct returned 0.\n";
    png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
    fclose(fp);
    throw -1; 
  }

  // create png info struct
  png_infop end_info = png_create_info_struct(png_ptr);
  if (!end_info) {
    std::cerr << "error: png_create_info_struct returned 0.\n";
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
    fclose(fp);
    throw -1; 
  }

  // the code in this if statement gets called if libpng encounters an error
  if (setjmp(png_jmpbuf(png_ptr))) {
    std::cerr << "error from libpng\n";
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    fclose(fp);
    throw -1; 
  }

  // init png reading
  png_init_io(png_ptr, fp);

  // let libpng know you already read the first 8 bytes
  png_set_sig_bytes(png_ptr, 8);

  // read all the info up to the image data
  png_read_info(png_ptr, info_ptr);

  // variables to pass to get info
  int bit_depth, color_type;
  png_uint_32 temp_width, temp_height;

  // get info about png
  png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth, &color_type,
	       NULL, NULL, NULL);

  //  if (width){ *width = temp_width; }
  //if (height){ *height = temp_height; }


  // Update the png info struct.
  png_read_update_info(png_ptr, info_ptr);

  // Row size in bytes.
  int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
  
  // glTexImage2d requires rows to be 4-byte aligned
  rowbytes += 3 - ((rowbytes-1) % 4);
  
  // Allocate the image_data as a big block, to be given to opengl
  png_byte * image_data;
  image_data = (png_byte*)malloc(rowbytes * temp_height * sizeof(png_byte)+15);
  if (image_data == NULL)  {
    std::cerr << "error: could not allocate memory for PNG image data\n";
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    fclose(fp);
    throw -1; 
  }
  
  // row_pointers is for pointing to image_data for reading the png with libpng
  png_bytep * row_pointers = (png_bytep*)malloc(temp_height * sizeof(png_bytep));
  if (row_pointers == NULL) {
    std::cerr << "error: could not allocate memory for PNG row pointers\n";
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    free(image_data);
    fclose(fp);
    throw -1; 
  }
  
  // set the individual row_pointers to point at the correct offsets of image_data
  unsigned int i;
  for (i = 0; i < temp_height; i++) {
    // don't know why this was flipped :(
    //row_pointers[temp_height - 1 - i] = image_data + i * rowbytes;

    // this seems to work though
    row_pointers[i] = image_data + i * rowbytes;
  }
  
  // read the png into image_data through row_pointers
  png_read_image(png_ptr, row_pointers);

  // -------------------------
  // ALLOCATE IMAGE STRUCTURE
  cols = temp_width;
  rows = temp_height;
  data = (sRGB*)image_data; 
  // ------------------------

  free(row_pointers);
  fclose(fp);

  initialized = true;
}

// ================================================================



#ifdef MPI_DEFINED
template<class Pixel>
void Image<Pixel>::load_from_sr(SocketReader & sr){
  char magic[2];
  sr.Jread(magic, 2, 1);
  if (strncmp(magic, traits.magic(), 2)){
    FATAL_ERROR("Incorrect file type '%c%c'", magic[0], magic[1]);
  }
  cols = get_pnm_number(sr);
  rows = get_pnm_number(sr);
  int maxval = get_pnm_number(sr);
  if (maxval != traits.pnm_maxval()){
    FATAL_ERROR("Incorrect maxval %d\n", maxval);
  }
  data = new Pixel[rows*cols];
  while(sr.Jread((char*)data, rows, cols*sizeof(Pixel))==0) {}
  
  min_x = 0.;
  max_x = cols-1;
  min_y = 0.;
  max_y = rows-1;

  initialized = true;

}
#endif







// ================================================================


template<class Pixel> inline void Image<Pixel>::save_ppm(const std::string &filename) const { //, const char *comment = NULL) {
  assert (rows > 0 && cols > 0);
  assert (data != NULL);

  assert (filename != "");
  assert (filename.size() > 4);
  assert (filename.substr(filename.size()-4,4) == ".ppm" ||
	  filename.substr(filename.size()-4,4) == ".pgm");

  FILE *fp;
  //if (NULL == filename){
  //fp = stdout;
  //} else {
  fp = fopen(filename.c_str(), "wb");
  //}
  if (NULL == fp){
    FATAL_ERROR("Unable to open %s for writing", filename.c_str());
  }
  fprintf(fp, "%s\n", traits.magic());
  //  if (NULL != comment){
  //fprintf(fp, "#%s\n", comment);
  //}
  fprintf(fp,"%d %d\n%d\n",
	  cols, rows, traits.pnm_maxval());
  fwrite(data, rows, cols*sizeof(Pixel), fp);
  //fwrite(data, rows, cols*sizeof(sRGB), fp);
  fclose(fp);
}


template<class Pixel> 
inline void Image<Pixel>::save_png(const std::string &filename) const { 
  std::cerr << "CANT SAVE PNG IMAGE OF THIS TYPE" << std::endl;
  throw -1;
}


template<> //class Pixel> 
inline void Image<sRGB>::save_png(const std::string &filename) const { //, const char *comment = NULL) {
  assert (rows > 0 && cols > 0);
  assert (data != NULL);
  FILE *fp;
  
  png_structp png_ptr = NULL;
  png_infop info_ptr = NULL;
  unsigned int /*size_t*/ x, y;
  png_byte ** row_pointers = NULL;
  int pixel_size = 3;
  int depth = 8;
  
  //std::cout << "IN SAVE PNG: " << filename << std::endl;


  fp = fopen (filename.c_str(), "wb");
  if (!fp) {
    std::cerr << "ERROR!  Cannot open this file: " << filename << std::endl;
    throw -1;
  }

  
  png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr == NULL) {
    std::cerr << "PNG PTR ERROR" << std::endl;
    fclose(fp);
    throw -1;
  }
    
  info_ptr = png_create_info_struct (png_ptr);
  if (info_ptr == NULL) {
    png_destroy_write_struct (&png_ptr, &info_ptr);
    fclose(fp);
    throw -1;
  }
  
  /* Set up error handling. */
  if (setjmp (png_jmpbuf (png_ptr))) {
    png_destroy_write_struct (&png_ptr, &info_ptr);
    fclose(fp);
    throw -1;
  }
    
  /* Set image attributes. */
  png_set_IHDR (png_ptr,
		info_ptr,
		cols,//bitmap->width,
		rows,//bitmap->height,
		depth,
		PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);
  
  /* Initialize rows of PNG. */
  row_pointers = (png_byte**)png_malloc (png_ptr, rows/*bitmap->height*/ * sizeof (png_byte *));
  for (y = 0; y < (unsigned int)rows/*bitmap->height*/; ++y) {
    png_byte *row = 
      (png_byte*)png_malloc (png_ptr, sizeof (uint8_t) * cols/*bitmap->width*/ * pixel_size);
    row_pointers[y] = row;
    for (x = 0; x < (unsigned int)cols/*bitmap->width*/; ++x) {

      //pixel_t * pixel = pixel_at (bitmap, x, y);
      sRGB p = (*this)(y,x);
      //sRGB p = (*this)(rows-y-1,x);
      
      *row++ = p.r(); //pixel->red;
      *row++ = p.g(); //pixel->green;
      *row++ = p.b(); //pixel->blue;
    }
  }
  
  /* Write the image data to "fp". */
  png_init_io (png_ptr, fp);
  png_set_rows (png_ptr, info_ptr, row_pointers);
  png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
  
  for (y = 0; y < (unsigned int)rows/*bitmap->height*/; y++) {
    png_free (png_ptr, row_pointers[y]);
  }
  png_free (png_ptr, row_pointers);
}


template<class Pixel> 
inline void Image<Pixel>::save_jpg(const std::string &filename) const { 
  std::cerr << "CANT SAVE JPG IMAGE OF THIS TYPE" << std::endl;
  throw -1;
}


/*template<class Pixel> inline void Image<Pixel>::save_jpg(const std::string &filename) const { //, const char *comment = NULL) {
  assert (rows > 0 && cols > 0);
  assert (data != NULL);
  FILE *fp;

  assert (filename != "");
  assert (filename.size() > 4);
  assert (filename.substr(filename.size()-4,4) == ".jpg" ||
	  filename.substr(filename.size()-5,5) == ".jpeg");
  
  fp = fopen(filename.c_str(), "wb");
  
  if (NULL == fp){
    FATAL_ERROR("Unable to open %s for writing", filename.c_str());
  }
  fprintf(fp, "%s\n", traits.magic());
  //  if (NULL != comment){
  //fprintf(fp, "#%s\n", comment);
  //}
  fprintf(fp,"%d %d\n%d\n",
	  cols, rows, traits.pnm_maxval());
  fwrite(data, rows, cols*sizeof(Pixel), fp);
  //fwrite(data, rows, cols*sizeof(sRGB), fp);
  fclose(fp);
}

*/

// ================================================================
#endif
