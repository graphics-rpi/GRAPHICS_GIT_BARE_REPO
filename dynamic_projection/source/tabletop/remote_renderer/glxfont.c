/* 
   glxfont.c - OpenGL rendering of X fonts. 

   cc -o glxfont glxfont.c -lglut -lGLU -lGL -lXmu -lX11
*/

/* 
   Copyright (c) Gerard Lanois, 1998.

   This program is freely distributable without licensing fees and is
   provided without guarantee or warrantee expressed or implied. This
   program is -not- in the public domain. 
*/

#include <cstdio>
#include <cstring> /* fprintf(), stderr */

#include <GL/gl.h>
#include <GL/glut.h>

#include <X11/Intrinsic.h>    /* Display, Window */
#include <GL/glx.h>           /* GLXContext */

static GLuint font_base;

void
init_font(GLuint base, char* f)
{
   Display* display;
   XFontStruct* font_info;
   int first;
   int last;

   /* Need an X Display before calling any Xlib routines. */
   display = XOpenDisplay(0);
   if (display == 0) {
      fprintf(stderr, "XOpenDisplay() failed.  Exiting.\n");
      //exit(-1);
   } 
   else {
 
      /* Load the font. */
      font_info = XLoadQueryFont(display, f);
      if (!font_info) {
         fprintf(stderr, "XLoadQueryFont() failed - Exiting.\n");
         exit(-1);
      }
      else {
         /* Tell GLX which font & glyphs to use. */
         first = font_info->min_char_or_byte2;
         last  = font_info->max_char_or_byte2;
         glXUseXFont(font_info->fid, first, last-first+1, base+first);
      }
      XCloseDisplay(display);
      display = 0;
   }
}


void
print_string(GLuint base, char* s)
{
   if (!glIsList(font_base)) {
      fprintf(stderr, "print_string(): Bad display list. - Exiting.\n");
      exit (-1);
   }
   else if (s && strlen(s)) {
      glPushAttrib(GL_LIST_BIT);
      glListBase(base);
      glCallLists(strlen(s), GL_UNSIGNED_BYTE, (GLubyte *)s);
      glPopAttrib();
   }
}


void 
my_init(char* f)
{
   font_base = glGenLists(256);
   if (!glIsList(font_base)) {
      fprintf(stderr, "my_init(): Out of display lists. - Exiting.\n");
      exit (-1);
   }
   else {
      init_font(font_base, f);
   }
}

void 
my_reshape(int w, int h)
{
   GLdouble size;
   GLdouble aspect;

   /* Use the whole window. */
   glViewport(0, 0, w, h);

   /* We are going to do some 2-D orthographic drawing. */
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   size = (GLdouble)((w >= h) ? w : h) / 2.0;
   if (w <= h) {
      aspect = (GLdouble)h/(GLdouble)w;
      glOrtho(-size, size, -size*aspect, size*aspect, -100000.0, 100000.0);
   }
   else {
      aspect = (GLdouble)w/(GLdouble)h;
      glOrtho(-size*aspect, size*aspect, -size, size, -100000.0, 100000.0);
   }

   /* Make the world and window coordinates coincide so that 1.0 in */
   /* model space equals one pixel in window space.                 */
   glScaled(aspect, aspect, 1.0);

   /* Now determine where to draw things. */
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

}

void 
my_handle_key(unsigned char key, int x, int y)
{
   switch (key) {
   case 27:
      exit(1);
      break;
   default:
      break;
   }
}

void
draw_stuff()
{
   /* Set up some strings with the characters to draw. */
   char x1[256];
   char x2[256];
   char x3[256];
   char x4[256];
   char x5[256];
   char x6[256];
   char x7[256];
   char x8[256];
   unsigned int i;
   unsigned int count;

   count = 0;
   for (i=1; i < 32; i++) {  /* Skip 0 - it's the null terminator! */
      x1[count] = i;
      count++;
   }
   x1[count] = '\0';

   count = 0;
   for (i=32; i < 64; i++) {
      x2[count] = i;
      count++;
   }
   x2[count] = '\0';

   count = 0;
   for (i=64; i < 96; i++) {
      x3[count] = i;
      count++;
   }
   x3[count] = '\0';

   count = 0;
   for (i=96; i < 128; i++) {
      x4[count] = i;
      count++;
   }
   x4[count] = '\0';

   count = 0;
   for (i=128; i < 160; i++) {
      x5[count] = i;
      count++;
   }
   x5[count] = '\0';

   count = 0;
   for (i=160; i < 192; i++) {
      x6[count] = i;
      count++;
   }
   x6[count] = '\0';

   count = 0;
   for (i=192; i < 224; i++) {
      x7[count] = i;
      count++;
   }
   x7[count] = '\0';

   count = 0;
   for (i=224; i < 256; i++) {
      x8[count] = i;
      count++;
   }
   x8[count] = '\0';

   glColor4f(0.0, 1.0, 0.0, 0.0);
   glRasterPos2f(-200.0, 120.0);
   print_string(font_base, x1);
   glRasterPos2f(-200.0, 90.0);
   print_string(font_base, x2);
   glRasterPos2f(-200.0, 60.0);
   print_string(font_base, x3);
   glRasterPos2f(-200.0, 30.0);
   print_string(font_base, x4);
   glRasterPos2f(-200.0, 0.0);
   print_string(font_base, x5);
   glRasterPos2f(-200.0, -30.0);
   print_string(font_base, x6);
   glRasterPos2f(-200.0, -60.0);
   print_string(font_base, x7);
   glRasterPos2f(-200.0, -90.0);
   print_string(font_base, x8);
}

void 
my_display(void)
{

   /* Clear the window. */
   glClearColor(0.0, 0.0, 0.0, 0.0);
   glClear(GL_COLOR_BUFFER_BIT);

   /* Draw coordinate axes, centered in the window. */
   glColor4f(1.0, 0.0, 0.0, 0.0);
   glBegin(GL_LINES); {
      glVertex2f(-10.0, 0.0);
      glVertex2f(10.0, 0.0);
      glVertex2f(0.0, -10.0);
      glVertex2f(0.0, 10.0);
   } glEnd();

   draw_stuff();

   glutSwapBuffers();
}

/*int 
main(int argc, char **argv)
{

   char window_title[256];
   char font_name[256];
   char* default_font = "fixed";

   glutInitWindowSize(425, 350);
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE);

   strcpy(window_title, "glxfont: ");
   if (argc > 1) {
      strcpy(font_name, argv[1]);
   }
   else {
      strcpy(font_name, default_font);
   }
   strcat(window_title, font_name);

   glutCreateWindow(window_title);

   my_init(font_name);

   glutDisplayFunc(my_display);
   glutReshapeFunc(my_reshape);
   glutKeyboardFunc(my_handle_key);

   glutMainLoop();

   return 0;
}*/
