#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include <Image.h>
#include "Vector3.h"
using std::cout;
using std::endl;
using std::string;


class ScreenReader
{
    public:
    ScreenReader(string image_loc_param)
    {
        printf("ScreenReader loading %s \n", image_loc_param.c_str());
        Image<Vector3<byte> > im = Image<Vector3<byte> >(image_loc_param.c_str());

        uchar3* data= (uchar3*)im.getData();
        width=im.getCols();
        height=im.getRows();
        image_buf=new char[width*height];
        for( int row=0; row<height; row++)
        {
           for( int col=0; col<width; col++)
           {
              if(data[im.linear_index(row,col)].x<30)
                image_buf[row*width+col]='X';
              else
                image_buf[row*width+col]='O';
              //printf("%c", image_buf[row*width+col]);
           }
           //printf("\n");
        }
        //sleep(10);
        /*
        image_loc=image_loc_param;
        std::ifstream image_if(image_loc.c_str());
        string line;
        image_if>>width>>height;
        image_buf=new char[width*height];
        printf("width %d height %d \n", width, height);

        getline(image_if, line);

        for(int i=0; i< height; i++)
        {
            getline(image_if, line);
            assert(line.size()>=width);
            for(int j=0; j< width; j++)
            {
                char curChar=line[j];
                image_buf[i*width+j]=curChar;
                //cout<<curChar;
            }
            //cout<<endl;
        }
        image_if.close();*/
    }

    ~ScreenReader()
    {
        //printf("deconstructor\n");
        delete image_buf;
    }

    char* getImageBuf()
    {
        return image_buf;
    }

    int getWidth()
    {
        return width;
    }
    int getHeight()
    {
        return height;
    }



    private:
        string image_loc;
        char* image_buf;
        int width;
        int height;
};
