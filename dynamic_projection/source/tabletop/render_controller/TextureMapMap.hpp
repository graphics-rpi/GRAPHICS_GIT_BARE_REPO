#ifndef TEXTURE_MAP_MAP_INCLUDED_
#define TEXTURE_MAP_MAP_INCLUDED_
#include <unordered_map>
#include <vector>
#include <string>
#include <cstring>
#include <cstdio>

// CHANGED THIS
#define IMAGEDIR "/ramdisk/images"
//#define IMAGEDIR /home/greg/ramdisk/

class TextureMapMap {
public:
  TextureMapMap(){}
  void loadFile(const char *filename){
    FILE *fp = fopen(filename, "rt");
    if (NULL == fp){
      fprintf(stderr, "unable to open %s\n", filename);
      exit(-1);
    }
    load(fp);
    fclose(fp);
  }
  void load(FILE *fp){
    while(!feof(fp)){
      char textline[1024];
      char *material_name;
      char *texture_file;
      fgets(textline, 1024, fp);
      if (!strcmp(textline, "END TEXTURE_MAP_MAP\n")){
        break;
      } else if (material_name = strtok(textline, "|")){
        if (texture_file = strtok(NULL, " \t\n")){
          texture_filenames.push_back(texture_file);
          material_map[material_name] = texture_filenames.size()-1;
        }
      } 
    }
  }
  void addToMap(char* name, char* file)
  {
       char str[256];
       // CHANGED THIS
       //sprintf(str, "/home/greg/ramdisk/images/surface_camera_%s",file);
       sprintf(str, IMAGEDIR "/surface_camera_%s",file);
       texture_filenames.push_back(str);
       material_map[name] = texture_filenames.size()-1;
  }
  void addToMultiMap(char* name, char* file)
  {
       char str[256];
       sprintf(str, "/home/grfx/app_data/%s",file);
       int index=0;
       for( ; index<texture_filenames.size();index++)
          if(!strcmp(texture_filenames[index].c_str(),str))
            break;
       if(index==texture_filenames.size()){
         texture_filenames.push_back(str);
         material_map[name] = texture_filenames.size()-1;
       }
  }


  void addToPuzzleMap(char* name, char* file)
  {
       char str[256];
       sprintf(str, "%s",file);
       texture_filenames.push_back(str);
       material_map[name] = texture_filenames.size()-1;
  }

  int getNumTextures(){
    return texture_filenames.size();
  }
  std::string getTextureFilename(int idx){
    return texture_filenames.at(idx);
  }
  int getTextureID(std::string material_name){
    if (material_map.find(material_name) != material_map.end()){
      return material_map[material_name];
    } else {
      return -1;
    }
  }
private:
  std::vector<std::string> texture_filenames;
  std::unordered_map<std::string, int> material_map;

};

#endif // #ifndef TEXTURE_MAP_MAP_INCLUDED_
