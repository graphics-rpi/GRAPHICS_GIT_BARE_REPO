#include <vector>
#include <algorithm>

#include "mesh.h"
#include "argparser.h"
#include "bvh.h"
#include "meshmanager.h"
#include "matrix.h"

static int GLOBAL_iteration_count = 0;

// =================================================================
// =================================================================
// helper function

double EvaluateAssignment(Vec3f &centroid, Vec3f &normal, Vec3f &centroid2, Vec3f &normal2) {

  double distance = DistanceBetweenTwoPoints(centroid,centroid2);
  double dot = normal.Dot3(normal2);
  
  Vec3f avg_normal = normal + normal2;
  avg_normal.Normalize();

  //  double d  = centroid.Dot3(avg_normal);
  //double d2 = centroid2.Dot3(avg_normal);
  //double diff = fabs(d-d2);

  //  return (diff * 10 + distance) * (1.1-dot);
  double scale = 2.01-dot;
  assert (scale >= 1);
  return distance * scale * scale * scale;
}

// =================================================================
// =================================================================

void Mesh::AssignPatchesAndZones(MeshManager *meshes) {
  SeedPatches(meshes); 
  RandomizePatchColors(meshes);
  IteratePatches(meshes,3);
  std::cout << "Mesh::AssignPatchesAndZones, assigned "
            << numPatches() << " patches." << std::endl;
  CheckPatches();
  AssignZones(meshes);
  CheckPatches();
  std::cout << "Mesh::AssignPatchesAndZones, finished, assigned " 
            << numPatches() << " patches." << std::endl;
}


void Mesh::CheckPatches() const {
  for (unsigned int i = 0; i < numPatches(); i++) {
    if (patches[i].getElementsInPatch().size() == 0) {
      std::cout << "WARNING: empty patch " << i << std::endl;
    }
    for (std::set<ElementID>::const_iterator itr = patches[i].getElementsInPatch().begin(); 
	 itr != patches[i].getElementsInPatch().end(); itr++) {
      Element *e = Element::GetElement(*itr);
      assert (e != NULL);
      assert (getAssignedPatchForElement(e->getID()) == (int)i);
    }
  }
  for (unsigned int i = 0; i < numZones(); i++) {
    if (zones[i].getPatchesInZone().size() == 0) {
      std::cout << "WARNING: empty zone " << i << std::endl;
    }
    for (std::set<PatchID>::const_iterator itr = zones[i].getPatchesInZone().begin(); 
	 itr != zones[i].getPatchesInZone().end(); itr++) {
      //const Patch &p = getPatch(*itr);      
      assert (getAssignedZoneForPatch(*itr) == (int)i);
    }
  }

  /*  
  const std::map<std::string, MeshMaterial>& mats = getMaterials(); 
  for (std::map<std::string,MeshMaterial>::const_iterator itr = mats.begin();
       itr != mats.end(); itr++) {
    std::cout << "MATERIAL " << itr->first << "  ";
    int count = 0;
    double area = 0;
    for (elementshashtype::const_iterator itr2 = elements.begin(); 
	 itr2 != elements.end(); itr2++) {
      Element *e = itr2->second;
      assert (e != NULL);
      if (e->getRealMaterialName() == itr->first) {
	count++;
	area += e->Area();
      }
    }
    std::cout << "count = " << count << " area = " << area << std::endl;
  }

  std::cout << "--------------------------------------" << std::endl;
  */


  const std::set<std::string> mats2 = getSimpleMaterialNames(); 
  for (std::set<std::string>::const_iterator itr = mats2.begin();
       itr != mats2.end(); itr++) {
    std::cout << "MATERIAL " << *itr << "  ";

    const MeshMaterial& mm = getMaterialFromSimpleName(*itr);
    std::cout << mm.getName() << " ";

    int count = 0;
    double area = 0;
    for (elementshashtype::const_iterator itr2 = elements.begin(); itr2 != elements.end(); itr2++) {
      Element *e = itr2->second;
      assert (e != NULL);
      if (e->getMaterialPtr()->getSimpleName() == *itr) {
	count++;
	area += e->Area();
      }
    }
    std::cout << "count = " << count << " area = " << area << std::endl;
  }
}


void Mesh::AddZone(const std::string &name, const Vec3f &color) {
  zones.push_back(Zone(name,color));
}

void Mesh::AddPatch(const Vec3f &color) {
  patches.push_back(Patch(numPatches(),0,color));
}

void Mesh::AssignZones(MeshManager *meshes) {

  zones.clear();

  BoundingBox *bbox = getBoundingBox();
  assert (bbox != NULL);
  Vec3f min = bbox->getMin();
  Vec3f max = bbox->getMax();
  Vec3f diff = max-min;
  double maxdim = bbox->maxDim();

  // -------------------------------
  // hardcode zones for 6 directions

  std::vector<Vec3f> directions;
  directions.push_back(Vec3f( 0, 1, 0));
  directions.push_back(Vec3f( 0,-1, 0)); 
  directions.push_back(Vec3f( 1, 0, 0));
  directions.push_back(Vec3f(-1, 0, 0));
  directions.push_back(Vec3f( 0, 0, 1));
  directions.push_back(Vec3f( 0, 0,-1));

  double north_angle = meshes->getWalls()->north_angle;

  Mat m = Mat::MakeYRotation(-north_angle);
  
  for (unsigned int i = 0; i < directions.size(); i++) {
    m.TransformDirection(directions[i]);
    directions[i].Normalize();
  }

  std::vector<double> distance(6,100000000000); // start at ~infinity
  
  for (unsigned int i = 0; i < patches.size(); i++) {
    //int best = -1;
    //double best_dot = -1;
    for (unsigned int j = 0; j < directions.size(); j++) {
      Vec3f centroid = patches[i].getCentroid();
      Vec3f normal = patches[i].getNormal();
      double dot = normal.Dot3(directions[j]);
      if (dot < 0.9) continue;
      double dist = centroid.Dot3(directions[j]);
      if (dist < distance[j]) distance[j] = dist;
    }
  }

  double tolerance = 0.05*maxdim;

  std::vector<PreZone> regions;
  regions.push_back(PreZone("floor"  , Vec3f(0,1,0), Plane(directions[0],distance[0]), tolerance));
  regions.push_back(PreZone("ceiling", Vec3f(1,1,0), Plane(directions[1],distance[1]), tolerance));
  regions.push_back(PreZone("west"   , Vec3f(1,0,1), Plane(directions[2],distance[2]), tolerance));
  regions.push_back(PreZone("east"   , Vec3f(0,1,1), Plane(directions[3],distance[3]), tolerance));
  regions.push_back(PreZone("north"  , Vec3f(1,0,0), Plane(directions[4],distance[4]), tolerance));
  regions.push_back(PreZone("south"  , Vec3f(0,0,1), Plane(directions[5],distance[5]), tolerance));
 
  for (unsigned int i = 0; i < regions.size(); i++) {
    zones.push_back(Zone(regions[i].name,regions[i].color));
  }
  zones.push_back(Zone("other",Vec3f(0.8,0.5,0.3)));

  
  for (unsigned int i = 0; i < patches.size(); i++) {
    int best = -1;
    double best_dot = -1;
    for (unsigned int j = 0; j < regions.size(); j++) {
      Vec3f centroid = patches[i].getCentroid();
      Vec3f normal = patches[i].getNormal();
      double dot = normal.Dot3(regions[j].plane.getNormal());
      if (dot < 0.9) continue;
      if (fabs(regions[j].plane.SignedDistanceToPlane(centroid)) 
	  > regions[j].tolerance) {
	continue;
      }
      if (best == -1 || best_dot < dot) {
	best = j;
	best_dot = dot;
      }
    }
    if (best == -1) {
      best = regions.size();
    }
    patches[i].setZone(best);
    zones[best].addPatchToZone(i); //patches[i]);
  }
  std::cout << "done assigning " << numZones() << " zones." << std::endl;
  RandomizeZoneColors(meshes);
  AnalyzeZones(meshes);
}

void Mesh::AnalyzeZones(MeshManager *meshes) {
  for (unsigned int i = 0; i < zones.size(); i++) {
    std::cout << "ZONE " << zones[i].getName() << " ";
    double sum = 0.0;
    int count = 0;
    for  (std::set<PatchID>::const_iterator itr = zones[i].getPatchesInZone().begin();
	  itr != zones[i].getPatchesInZone().end(); itr++) {
      sum += getPatch(*itr).getArea();
      //std::cout << "   " << getPatch(*itr).getArea() << std::endl;
      count++;
    }
    std::cout << "count = " << count;
    std::cout << "   area = " << sum / double(12*12) << std::endl;
  }
}

void Mesh::RandomizePatchColors(MeshManager *meshes) {
  for (unsigned int i = 0; i < patches.size(); i++) {
    patches[i].setColor(meshes->args->RandomColor());
  }
}

void Mesh::RandomizeZoneColors(MeshManager *meshes) {
  /*
  for (int i = 0; i < zones.size(); i++) {
    zones[i].setColor(meshes->args->RandomColor());
  }
  */
}

// =================================================================
// =================================================================

void Mesh::SeedPatches(MeshManager *meshes) {
  std::cout << "Mesh::SeedPatches, desired # = " 
            << meshes->args->desired_patch_count << std::endl;
  GLOBAL_iteration_count = 0;
  element_to_patch.clear();  
  patches.clear();
  zones.clear();
  int desired_num_patches = meshes->args->desired_patch_count;
  int num_triangles = numTriangles();
  //  std::cout << "seeds: ";
  assert (desired_num_patches > 0);
  assert (num_triangles > 0);

  // FIRST: collect all non "extra" material triangles
  std::vector<int> ids_of_non_extras;
  for (elementshashtype::const_iterator foo = elements.begin();
       foo != elements.end();
       foo++) {
    Element *e = foo->second;
    const MeshMaterial *mm = e->getMaterialPtr();
    if (mm->isExtra()) continue;
    ids_of_non_extras.push_back(e->getID());
  }

  assert (ids_of_non_extras.size() > 0);

  // THEN: RANDOMLY SHUFFLE THIS LIST
  std::random_shuffle(ids_of_non_extras.begin(),ids_of_non_extras.end());

  // CHOOSE FIRST DESIRED # OF THESE ELEMENTS
  for (int i = 0; 
       i < ids_of_non_extras.size() && i < desired_num_patches;
       i++) {
    int id = ids_of_non_extras[i];
    //std::cout << id << " ";
    patches.push_back(Patch(patches.size(),id,meshes->args->RandomColor()));
  }
  assert (patches.size() <= desired_num_patches);
  assert (patches.size() > 0);
  std::cout << "Mesh::SeedPatches, after seeding, #patches = " 
            << numPatches() << std::endl;
}


bool Mesh::ElectNewSeeds(MeshManager *meshes) { 

  std::cout << "ELECT NEW SEEDS" << std::endl;

  int no_seed = 0;
  
  for (unsigned int i = 0; i < numPatches(); i++) {

    Vec3f avg_normal(0,0,0);
    Vec3f avg_centroid(0,0,0);    
    double total_area = 0;
    double total_weight = 0;
    Vec3f normal,centroid;

    if (patches[i].getElementsInPatch().size() == 0) {
      no_seed++;
      continue;
    }

    assert (patches[i].getElementsInPatch().size() > 0);

    for (std::set<ElementID>::const_iterator itr = patches[i].getElementsInPatch().begin(); 
	 itr != patches[i].getElementsInPatch().end(); itr++) {
      Element *e = Element::GetElement(*itr);
      if (e == NULL) continue; 
      e->computeCentroid(centroid);
      e->computeNormal(normal);
      double area = e->Area();
      double weight = area;
      avg_normal += weight*normal;
      avg_centroid += weight*centroid;
      total_area += area;
      total_weight += weight;
    }
    avg_centroid *= 1/(double)total_weight;
    avg_normal.Normalize();
    
    Element *best_seed = NULL;
    double best_value = -1;
    
    for (std::set<ElementID>::const_iterator itr = patches[i].getElementsInPatch().begin(); 
	 itr != patches[i].getElementsInPatch().end(); itr++) {
      Element *e = Element::GetElement(*itr);
      if (e == NULL) continue; 
      e->computeCentroid(centroid);
      e->computeNormal(normal);
      double value = EvaluateAssignment(centroid,normal,avg_centroid,avg_normal);
      if (best_seed == NULL || value < best_value) {
	best_seed = e;
	best_value = value;
      }
    }
    if (best_seed == NULL) {
      patches[i].setSeed(0); //NULL);
      patches[i].setArea(0);
      patches[i].setNormal(Vec3f(0,0,0));
      patches[i].setCentroid(Vec3f(0,0,0));
    } else {
      patches[i].setSeed(best_seed->getID());
      patches[i].setArea(total_area);
      patches[i].setNormal(avg_normal);
      patches[i].setCentroid(avg_centroid);
    }
  }
  if (no_seed > 0) {
  std::cout << "NO SEED FOR " << no_seed << " PATCHES / " << numPatches() << "TOTAL PATCHES." << std::endl;
  }
  //return false;
  return true;//false;
}


void Mesh::AddOrDeleteSeeds(MeshManager *meshes) { 

  std::cout << "ADD OR DELETE SEEDS " << std::endl;

  int desired_num_patches = meshes->args->desired_patch_count;
  int num_patches = numPatches();
  //int change = 0;

  double max_patch_area;
  double min_patch_area;
  int patch_min_area;
  double sum_patch_area = 0;
  int patch_max_area;
  for (unsigned int i = 0; i < numPatches(); i++) {
    double area = patches[i].getArea();
    if (patches[i].getElementsInPatch().size() == 0) {
      std::cout << "EMPTY PATCH" << std::endl;
    }
    assert (patches[i].getElementsInPatch().size() > 0);
    //std::cout << "patch " << i << " " << area << std::endl;
    sum_patch_area += area;
    if (i == 0) {
      max_patch_area = min_patch_area = area;
      patch_min_area = patch_max_area = i;
    } else {
      if (area < min_patch_area) { min_patch_area = area; patch_min_area = i; }
      if (area > max_patch_area) { max_patch_area = area; patch_max_area = i; }
    }
  }
  double avg_patch_area = sum_patch_area / double(num_patches);
  std::cout << "AVG PATCH AREA " << avg_patch_area << std::endl;
  std::cout << "MIN PATCH AREA " << min_patch_area / avg_patch_area << " " << patch_min_area << std::endl;
  std::cout << "MAX PATCH AREA " << max_patch_area / avg_patch_area << " " << patch_max_area << std::endl;

  int unassigned_count = 0;

  std::vector<int> unassigned_elements;
  
  Element *worst = NULL;
  double worst_value;

  for (elementshashtype::const_iterator foo = getElements().begin(); foo != getElements().end(); foo++) {
    Element *e = foo->second;
    ElementID id = e->getID();

    const MeshMaterial *mm = e->getMaterialPtr();
    if (mm->isExtra()) continue;
    
    PatchID pid = getAssignedPatchForElement(id);
    if (pid == -1) {
      unassigned_count++;
      unassigned_elements.push_back(id);
    } else {
      const Patch& patch = getPatch(pid);
      ElementID seedID = patch.getSeed();
      assert (seedID > 0);
      Element *seed = Element::GetElement(seedID);
      assert (seed != NULL);

      if (e == seed) continue;

      Vec3f centroid; e->computeCentroid(centroid);
      Vec3f normal; e->computeNormal(normal);
      Vec3f centroid2; seed->computeCentroid(centroid2);
      Vec3f normal2; seed->computeNormal(normal2);

      double value = EvaluateAssignment(centroid,normal,centroid2,normal2);
      if (worst == NULL || worst_value < value) {
	worst = e;
	worst_value = value;
	//std::cout << "update worst " << worst_value << " " << worst->getID() << std::endl;
      }
    }
  }  


  if (unassigned_count > 0) {

    std::cout << "CREATING PATCH FOR ONE UNASSIGNED ID" << std::endl;

    assert (unassigned_elements.size() > 0);

    ElementID worstID = unassigned_elements[0]; //worst->getID();
    //PatchID worstPatchID = getAssignedPatchForElement(worstID);
    //assert (worstPatchID != -1);
    //Patch& patch = patches[worstPatchID];
    //patch.removeElementFromPatch(worstID);
    //patch.recomputeArea();
    patches.push_back(Patch(num_patches,worstID,meshes->args->RandomColor()));
    element_to_patch[worstID] = num_patches;

  } else if (desired_num_patches > num_patches && worst != NULL) {
    std::cout << "ADDING!a" << std::endl;
    ElementID worstID = worst->getID();
    std::cout << "ADDING!b" << std::endl;
    PatchID worstPatchID = getAssignedPatchForElement(worstID);
    std::cout << "ADDING!c" << std::endl;
    assert (worstPatchID != -1);
    std::cout << "ADDING!d" << std::endl;
    Patch& patch = patches[worstPatchID];
    std::cout << "ADDING!e" << std::endl;
    patch.removeElementFromPatch(worstID);
    patch.recomputeArea();
    std::cout << "ADDING!f" << std::endl;    
    patches.push_back(Patch(num_patches,worst->getID(),meshes->args->RandomColor()));
    std::cout << "ADDING!g" << std::endl;
    element_to_patch[worstID] = num_patches;
  }

  //  CheckPatches();
  
  std::cout << "unassigned_count " << unassigned_count << std::endl;

}


int FindBestSeed(Element *e, BVH& bvh, double max_dim, std::vector<Patch>& patches, std::map<Element*,int>& mapper) {

  int which_seed = -1;

  // ignore certain materials
  const MeshMaterial *mm = e->getMaterialPtr();
  if (mm->isExtra()) return -2;  // -2 means extra material (ok not to be assigned!)

  Element *best_seed = NULL;

  Vec3f centroid; e->computeCentroid(centroid);
  Vec3f normal; e->computeNormal(normal);
  double best_value = -1;
      
  int count = 0;
  double radius = max_dim / 100.0;      
  while (best_seed == NULL) {
    
    //while (1) {
    count ++;
    if (count > 30) break;
    fflush(stdout);
    
    BoundingBox querybox(centroid - radius*Vec3f(1,1,1), centroid + radius*Vec3f(1,1,1));
    
    std::set<Element*> neighbors;
    bvh.query(querybox,neighbors);
    
    for (std::set<Element*>::iterator itr = neighbors.begin(); itr != neighbors.end(); itr++) {
      Element *seed = *itr;
      assert (seed != NULL);
      const MeshMaterial *seed_mm = seed->getMaterialPtr();
      if ((seed_mm != mm) &&
          seed_mm->getSimpleName() != mm->getSimpleName()) continue;
      Vec3f centroid2 = patches[mapper[seed]].getCentroid();
      Vec3f normal2 = patches[mapper[seed]].getNormal();
      double value = EvaluateAssignment(centroid, normal,centroid2,normal2);
      if (value < radius && (best_seed == NULL || value < best_value)) {
        best_value = value;
        best_seed = seed;
        which_seed = mapper[seed]; 
        assert (which_seed >= 0 && which_seed < (int)patches.size()); //numPatches());
      }
    }
    
    //if (best_seed != NULL) break;
    radius *= 2;
  }

  return which_seed;
}


void Mesh::IteratePatches(MeshManager *meshes, int num_requested_iterations) { 

  for (int iter = 0; iter < num_requested_iterations; iter++) {

    std::cout << "ITERATE PATCHES " << iter+1 << "/" << num_requested_iterations << std::endl;
    GLOBAL_iteration_count++;
    
    std::cout << "GLOBAL ITER COUNT " << GLOBAL_iteration_count << std::endl;
    if (GLOBAL_iteration_count > 1) {
      AddOrDeleteSeeds(meshes);
      bool change = ElectNewSeeds(meshes);
      if (!change) break;
    }    

    // =============================================
    // clear the old patches
    element_to_patch.clear();
    assert (numPatches() > 0);

    // =============================================
    // The seeds 
    std::set<Element*> elems;
    std::map<Element*,int> mapper;
    for (unsigned int i = 0; i < numPatches(); i++) {
      int seedid = patches[i].getSeed();
      assert (seedid > 0);
      Element* e = Element::GetElement(seedid);
      assert (e != NULL);
      patches[i].clearElementsInPatch();
      assert (elems.find(e) == elems.end());
      elems.insert(e);
      assert (mapper.find(e) == mapper.end());
      mapper[e] = i;
    }    
    assert (elems.size() > 0);
    assert (mapper.size() == elems.size());
    assert (elems.size() == numPatches());


    // =============================================
    // put all of the seeds in a BVH bounding volume hierarchy structure
    std::cout << "create bvh "; fflush (stdout);
    BVH bvh(elems);
    std::cout << "done " << elems.size() << std::endl;
    assert (getBoundingBox() != NULL);
    double max_dim = getBoundingBox()->maxDim();
    

    // =============================================
    // loop over all of the elements, find the seed that best matches this element
    int unassigned = 0;
    //int new_assignments = 0;
    
    int foototal = getElements().size();
    int foocounter = 0;
    int fooprogress = 0;
    for (elementshashtype::const_iterator foo = getElements().begin(); foo != getElements().end(); foo++) {

      // progress bar
      if (foocounter > fooprogress*foototal / 50.0) {
	std::cout << "."; fflush(stdout);
	fooprogress++;
      }
      foocounter++;

      Element *e = foo->second;

      // find a patch for this element!
      ElementID id = e->getID();

      // see if this element is a seed / already assigned!
      //Element *best_seed = NULL;
      //int which_seed = -1;


      int which_seed;

      if (mapper.find(e) != mapper.end()) {
        //best_seed = e;
        which_seed = mapper[e];
      } else {
        which_seed = FindBestSeed(e,bvh,max_dim,patches,mapper);
      }


      if (which_seed == -2) {
        // -2 means extra material (ok not to be assigned!)
      } else if (which_seed == -1) {
        // oops, we really did want this to be assigned!
	unassigned ++;
      } else {
	patches[which_seed].addElementToPatch(id);
	element_to_patch[id] = which_seed; 
      }

    }
    std::cout << "UNASSIGNED COUNT = " << unassigned << std::endl;
  }
}

// =================================================================
// =================================================================

PatchID Mesh::getAssignedPatchForElement(ElementID id) const {
  std::map<ElementID,PatchID>::const_iterator itr = element_to_patch.find(id);
  if (itr == element_to_patch.end()) {
    return -1;
  }
  return itr->second;
}


ZoneID Mesh::getAssignedZoneForPatch(PatchID pid) const {
  if (pid == -1) return -1;
  assert (pid >= 0 && pid < (int)numPatches());
  return patches[pid].getZone();
}



ZoneID Mesh::getAssignedZoneForElement(ElementID id) const {
  PatchID pid = getAssignedPatchForElement(id);
  return getAssignedZoneForPatch(pid);
}


