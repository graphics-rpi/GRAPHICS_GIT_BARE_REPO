#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "vectors.h"
#include <string>
#include <cassert>

class MeshMaterial {
  
public:
  
  MeshMaterial() { SetDefaults(); }
  // Note: default assignment operator & copy constructor will work just ok!

  void SetDefaults() {
    name = "default_material";
    lambertian = true;
    glass = prismatic = tabular = invisible = false;
    extra = fillin = false;
    exterior = sensor = emitter = false;
    diffuse_reflectance = Vec3f(0.9,0.9,0.9);
    diffuse_transmittance = specular_transmittance = specular_reflectance = Vec3f(0,0,0);
    physical_diffuse_reflectance = Vec3f(0.9,0.9,0.9);
    emittance = Vec3f(0,0,0);
    prismatic_angles = Vec2f(0,0);
    prismatic_normal = Vec3f(1,0,0);
    prismatic_up = Vec3f(0,1,0);
    tabular_material_directory_name = "bad_directory_name";
    texture_filename = "";
    texture_offset_s = 0;
    texture_offset_t = 0;
  }
  
  // MODIFIERS
  void setName(const std::string &n) { SetDefaults(); name = n; }
  void setLambertian() { lambertian = true; glass = prismatic = tabular = invisible = false; }
  void setGlass()      { glass = true; lambertian = prismatic = tabular = invisible = false; }
  void setPrismatic()  { prismatic = true; lambertian = glass = tabular = invisible = false; }
  void setTabular()    { tabular = true; lambertian = glass = prismatic = invisible = false; }
  void setInvisible()  { invisible = true; lambertian = glass = prismatic = tabular = false; }
  void setExtra() { extra = true; fillin = false; }
  void setFillin() { fillin = true; extra = false; }
  void setExterior() { exterior = true; }
  void setEmitter() { emitter = true; }
  void setSensor() { sensor = true; }
  void setDiffuseTransmittance(const Vec3f &v) { assert (glass); diffuse_transmittance = v; }
  void setSpecularTransmittance(const Vec3f &v) { assert(glass || prismatic); specular_transmittance = v; }
  void setDiffuseReflectance(const Vec3f &v) { 
    //assert (lambertian || glass); 
    diffuse_reflectance = v; }
  void setSpecularReflectance(const Vec3f &v) { assert (glass); specular_reflectance = v; }
  void setPhysicalDiffuseReflectance(const Vec3f &v) { physical_diffuse_reflectance = v; }
  void setEmittance(const Vec3f &v) { assert (emitter); emittance = v; }
  void setPrismaticAngles(double angle1, double angle2) { assert (prismatic); prismatic_angles = Vec2f(angle1,angle2); }
  void setPrismaticUp(const Vec3f &v) { assert (prismatic); prismatic_up = v; }
  void setPrismaticNormal(const Vec3f &v) { assert (prismatic); prismatic_normal = v; }
  void setTabularDirectoryName(const std::string &s) { assert (tabular); tabular_material_directory_name = s; }
  void setTextureFilename(const std::string &s, double offset_s=0, double offset_t=0) { texture_filename = s; texture_offset_s = offset_s; texture_offset_t = offset_t; }

  // ACCESSORS
  const std::string& getName() const { return name; }

  std::string getSimpleName() const { 
    if (name.substr(0,7) == "FILLIN_") {
      return name.substr(7,name.size()-7);
    } else {
      return name; 
    }
  }

  bool isLambertian() const { return lambertian; }
  bool isGlass() const { return glass; }
  bool isPrismatic() const { return prismatic; }
  bool isTabular() const { return tabular; }
  bool isInvisible() const { if (invisible) assert (sensor); return invisible; }
  // added for Yu Sheng for subsurface scattering...
  bool isSSS() const { if (name.find("SSS") != std::string::npos) return true; return false; }
  bool isExtra() const { return extra; }
  bool isFillin() const { return fillin; }
  bool isExterior() const { return exterior; }
  bool isEmitter() const { return emitter; }
  bool isSensor() const { return sensor; }
  const Vec3f& getDiffuseTransmittance() const { assert (glass); return diffuse_transmittance; }
  const Vec3f& getSpecularTransmittance() const { assert (glass || prismatic); return specular_transmittance; }
  const Vec3f& getDiffuseReflectance() const { assert (lambertian || glass); return diffuse_reflectance; }
  const Vec3f& getSpecularReflectance() const { assert (glass); return specular_reflectance; }
  const Vec3f& getPhysicalDiffuseReflectance() const { return physical_diffuse_reflectance; }
  const Vec3f& getEmittance() const { assert (emitter); return emittance; }
  const Vec2f& getPrismaticAngles() const { assert (prismatic); return prismatic_angles; }
  const Vec3f& getPrismaticUp() const { assert (prismatic); return prismatic_up; }
  const Vec3f& getPrismaticNormal() const { assert (prismatic); return prismatic_normal; }
  const std::string& getTabularDirectoryName() const { assert (tabular); return tabular_material_directory_name; }  
  bool isInvisibleSensor() const { return (invisible && sensor); }
  bool isOpaqueSensor() const { return (!invisible && sensor); }
  bool isProjection() const { return (!fillin && !extra); }
  const std::string& getTextureFilename() const { return texture_filename; }
  double getTextureOffsetS() const { return texture_offset_s; }
  double getTextureOffsetT() const { return texture_offset_t; }

private:

  // REPRESENTATION
  std::string name;

  bool lambertian;
  bool glass;
  bool prismatic;
  bool tabular;
  bool invisible;

  bool extra;
  bool fillin;
  
  bool exterior;
  bool sensor;
  bool emitter;

  Vec3f diffuse_transmittance;
  Vec3f specular_transmittance;
  Vec3f diffuse_reflectance;
  Vec3f specular_reflectance;

  Vec3f physical_diffuse_reflectance;

  Vec3f emittance;
  
  Vec2f prismatic_angles;
  Vec3f prismatic_up;
  Vec3f prismatic_normal;

  std::string tabular_material_directory_name;

  std::string texture_filename;
  double texture_offset_s;
  double texture_offset_t;
};

#endif
