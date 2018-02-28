#define NOMINMAX
//#define OMITEXTRA
#define OMITSENSOR
#define HACK
//#define OMITPATCHES
#include "MeshLoader.hpp"

// Constructor junk
MeshLoader::MeshLoader(
  optix::Context context,
  optix::GeometryGroup geomGroup,
  vector<float3>& centroids,
  vector<float3>& normals,
  vector<float>& patchAreas,
  vector<int>& patchStart,
  vector<int>& patchSizes,
  vector<float>& triAreas,
  vector<int>& neighbors,
  vector<float>& neighborWeights,
  const Mesh* meshIn,
  double& northParam,
  double& longitude,
  double& latitude
)
  :
    numberOfTriangles(0),
    mContext(context),
    mGeometryGroup(geomGroup),
    mVertexBuffer(0),
    mNormalBuffer(0),
    mTexcoordBuffer(0),
    mMaterial(0),
    mCentroids(centroids),
    mNormals(normals),
    mPatchAreas(patchAreas),
    mPatchStartIndexes(patchStart),
    mPatchSizes(patchSizes),
    mTriAreas(triAreas),
    mNeighbors(neighbors),
    mNeighborWeights(neighborWeights),
    north(northParam),
    longitude(longitude),
    latitude(latitude),
    mesh(meshIn),
    mAABB() { }

void
MeshLoader::load(  optix::Material&       material) {
  mMaterial=material;
  //Load from mesh here"
  loadVertexData(mesh);

  //Specifies the mesh and the triangle intersection programs to be used
  std::string path("triangleMesh.cu.ptx");
  Program meshIntersect = mContext->createProgramFromPTXFile( path, "meshIntersect" );
  Program meshBbox      = mContext->createProgramFromPTXFile( path, "meshBounds" );

  // Create a GeometryInstance and Geometry for each obj group
  createGeometryInstances(mesh, meshIntersect, meshBbox);
  
  printf("size:CGI %d size:lVD %d \n", size_in_createGeometryInstances, size_in_loadVertexData);
  assert(size_in_createGeometryInstances==size_in_loadVertexData);

}

void
MeshLoader::loadVertexData(const Mesh* mesh) {



  //Gets all the elements (triangles) in the mesh
  const elementshashtype& elements=mesh->getElements();
  north=mesh->getNorthAngleCopy();
  std::cout << "Meshloader::loadVertexData north is now set to " << north << std::endl;

  longitude = mesh->getLongitudeCopy();
  latitude  = mesh->getLatitudeCopy();
  std::cout << "Meshloader::loadVertexData longitude is now set to " << longitude << std::endl;
  std::cout << "Meshloader::loadVertexData latitude  is now set to " << latitude  << std::endl;

  const int numTriangles=mesh->numTriangles();
  //Currently have 3 vertices for each triangle in the mesh (no shared vertices)
  unsigned int numVertices= 3*elements.size();

  //Each vertex has it's own normal
  unsigned int numNormals   = numVertices;

  //Not yet supporting textures
  unsigned int numTexcoords = 0;

  // Create vertex buffer
  mVertexBuffer = mContext->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, numVertices);
  float3* vertexBufferData = static_cast<float3*>(mVertexBuffer->map());

  // Create normal buffer
  mNormalBuffer = mContext->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, numNormals);
  float3* normalBufferData = static_cast<float3*>(mNormalBuffer->map());

  // Create texcoord buffer
  mTexcoordBuffer = mContext->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT2, numTexcoords);
  float2* texcoordBufferData = static_cast<float2*>(mTexcoordBuffer->map());



  int triNum=0;
#ifdef OMITPATCHES
  int numPatches=0;
  mPatchStartIndexes.resize(0);
  mPatchSizes.resize(0);
#else
  int numPatches=mesh->numPatches();
  mPatchStartIndexes.resize(numPatches);
  mPatchSizes.resize(numPatches);

#endif
  int elIndex=0;

  vector<std::string> matNames;


  const std::set<std::string> mm=mesh->getSimpleMaterialNames();
  std::set<std::string>::const_iterator mmIt=mm.begin();
  
  //Populate the material Names array which we use later to add geometry groups based on the material they are
  while(mmIt!=mm.end())
  {

    matNames.push_back(*mmIt);
    mmIt++;

  }
  int numMaterials=matNames.size();

    for(int patchNum=0; patchNum<numPatches;patchNum++)
    {
          mPatchStartIndexes[patchNum]=0;
      mPatchSizes[patchNum]=0;
    }

  //Triangles are ordered first by materials
  for(int mat=0; mat<numMaterials; mat++)
  {
    std::string materialName=matNames[mat];
    const MeshMaterial curMat = mesh->getMaterial(materialName);
    //printf("material name %s \n", materialName.c_str());
    //NEWSTUFF
#ifdef OMITEXTRA
    if(curMat.isExtra())
       continue;
#endif
#ifdef OMITSENSOR
    if(curMat.isSensor())
      continue;
#endif
    for(int patchNum=0; patchNum<numPatches;patchNum++)
    {
      bool patch_activated=false;
      const std::set<ElementID>& elementsInPatch=mesh->getElementsInPatch(patchNum);

      int elInPatch=elementsInPatch.size();
      //Make sure there is at least 1 patch
      if(elInPatch==0){
        std::cout << "patches bug( material: " << materialName << ")" << std::endl;
      }
      assert(elInPatch!=0);

//      printf("elements in patch

      for(std::set<ElementID>::const_iterator it =elementsInPatch.begin();
          it!=elementsInPatch.end();
          it++)
      {
        
        assert(triNum<numTriangles);

        Element* el=Element::GetElement(*it);
        //First need the vertex indiceds
	      std::string meshMatName=el->getMaterialPtr()->getSimpleName();
        if(matNames[mat]==meshMatName)
        {
          if(! patch_activated)
          {
            patch_activated=true;
            
            mPatchStartIndexes[patchNum]=elIndex;
            mPatchSizes[patchNum]=elInPatch;

          }
          float area=el->Area();
          mTriAreas.push_back(area);
          //assert(area>.001);

          int vertIndex1  = (*el)[0];
          int vertIndex2  = (*el)[1];
          int vertIndex3  = (*el)[2];
          int ID=el->getID();
          assert(ID>=0);
          
          //Make sure that the element hasn't already been used
          std::map<int, int>::iterator it = remesh_to_optix_translation.find(ID);
          assert(it==remesh_to_optix_translation.end());
          
          remesh_to_optix_translation[ID]=elIndex;
          assert(elIndex>=0);
          optix_to_remesh_translation.push_back(ID);
          std::vector<std::pair<ElementID,double> > neighbors0=el->getNeighborsForInterpolation(0,NUM_NEIGHBORS);
          std::vector<std::pair<ElementID,double> > neighbors1=el->getNeighborsForInterpolation(1,NUM_NEIGHBORS);
          std::vector<std::pair<ElementID,double> > neighbors2=el->getNeighborsForInterpolation(2,NUM_NEIGHBORS);
          neighbors_with_weights.push_back(neighbors0);
          neighbors_with_weights.push_back(neighbors1);
          neighbors_with_weights.push_back(neighbors2);

          //Get Vec3f from vertices
          Vec3f vert1 = mesh->getVertex(vertIndex1)->get();
          Vec3f vert2 = mesh->getVertex(vertIndex2)->get();
          Vec3f vert3 = mesh->getVertex(vertIndex3)->get();

          const MeshMaterial* meshMat=el->getMaterialPtr();
          bool isSensor =meshMat->isGlass();

          Vec3f normal;

          //Have the element compute it's normal
          el->computeNormal(normal);

          //Have to convert to float3 and put vertex and normal in buffers
          vertexBufferData[triNum*3+0]=make_float3(vert1.x(), vert1.y(), vert1.z());
          vertexBufferData[triNum*3+1]=make_float3(vert2.x(), vert2.y(), vert2.z());
          vertexBufferData[triNum*3+2]=make_float3(vert3.x(), vert3.y(), vert3.z());
          normalBufferData[triNum*3+0]=make_float3(normal.x(),normal.y(),normal.z());
          normalBufferData[triNum*3+1]=make_float3(normal.x(),normal.y(),normal.z());
          normalBufferData[triNum*3+2]=make_float3(normal.x(),normal.y(),normal.z());

          assert(triNum*3+2<numVertices);
          //Also must add each vertex to the bounding box
          mAABB.include(make_float3(vert1.x(), vert1.y(), vert1.z()));
          mAABB.include(make_float3(vert2.x(), vert2.y(), vert2.z()));
          mAABB.include(make_float3(vert3.x(), vert3.y(), vert3.z()));

          triNum++;
          elIndex++;
        }
        //else assert(0);
      }
    }
  }
  for(int i=0; i< neighbors_with_weights.size(); i++)
  {
    std::vector<std::pair<ElementID,double> > neighbors=neighbors_with_weights[i];
    int size=neighbors.size();

  }
  size_in_loadVertexData=elIndex;




  int nww_size=neighbors_with_weights.size();
  for(int i=0; i< nww_size; i++)
  {
    std::vector<std::pair<ElementID,double> > neighbors=neighbors_with_weights[i];
    int size=neighbors.size();
    assert(size>0);
    int remesh_neighbor_zero=neighbors[0].first;
    for(int j=0; j<NUM_NEIGHBORS; j++)
    {

        int remesh_neighbor;
        float weight;
        if(j < size)
        {
            std::pair<ElementID,double> neighbor = neighbors[j];
            remesh_neighbor=neighbor.first;
            weight=neighbor.second;
        }
        else
        {
            remesh_neighbor=remesh_neighbor_zero;
            weight=0;
        }
        int lsvo_neighbor=remesh_to_optix_translation[remesh_neighbor];

        //printf("%d's %dth neighbor: %d \n", i,j, remesh_neighbor);
        assert(lsvo_neighbor<elements.size());
        mNeighbors.push_back( lsvo_neighbor);
        mNeighborWeights.push_back( weight);
    }
  }

//  for(int i=0; i<3*elements.size();i++)
//  {
//    printf("vertex %d: %d \n", i,  vertexBufferData[i]);
//  }
//  sleep(30);

  //Buffers must be unmapped before tracing scene
  mVertexBuffer->unmap();
  mNormalBuffer->unmap();
  mTexcoordBuffer->unmap();


  mPatchAreaBuffer = mContext->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT, numPatches);
  float* patchBufferData = static_cast<float*>(mPatchAreaBuffer->map());
  for(int i=0; i<numPatches; i++)
  {
    float patchArea=mesh->getPatchArea(i);
    patchBufferData[i]=static_cast<float>(patchArea);

    mPatchAreas.push_back(patchArea);
  }
  mPatchAreaBuffer->unmap();
}

// Sees if a vector has a string in it.  If so returns vertex, otherwise -1
int findIndex(vector <std::string>& vec, std::string& str)
{
   int size=vec.size();
   for(int i=0; i<size; i++)
   {
      if(vec[i]==str)
        return i;
   }
   return -1;
}

void
MeshLoader::createGeometryInstances(
  const Mesh* inputMesh,
  Program meshIntersect,
  Program meshBBox
) {

  //The number of triangles is obtained from the remesher mesh
  unsigned int numTriangles = inputMesh->numTriangles();

  //A vector of (OptiX) geometry instances
  std::vector<GeometryInstance> instances;

  // Loop over all groups -- grab the triangles and material props from each group
  const elementshashtype& elements=inputMesh->getElements();

  // A vector of material names, will be stored in the order they appear in the
  //     map of materials from remesher
  std::vector<std::string> matNames;

  //Counts of each material (used to put them into geometry groups in optix)
  std::vector<int> matCounts;

  //This loop copies the material names
  const std::set<std::string> mm=mesh->getSimpleMaterialNames();
  std::set<std::string>::const_iterator mmIt=mm.begin();

  int numPatches=inputMesh->numPatches();

  while(mmIt!=mm.end())
  {

    matNames.push_back(*mmIt);
    matCounts.push_back(0);
    mmIt++;

  }


  //This loop counts how many elements of each material there are
  for(elementshashtype::const_iterator it =elements.begin();
        it!=elements.end();
        it++)
  {
        std::vector<float3> tri;
      Element* el = it->second;
      std::string name = it->second->getMaterialPtr()->getSimpleName();
      int index=findIndex(matNames, name);
      if(index==-1)
      {
        //printf("material name %s \n", name.c_str());

      }
      else
        ;//matCounts[index]= matCounts[index]+1;

  }

  int numMaterials=matCounts.size();

  //We also count the total number of elements in all materials
  int totalCount=0;
  for(int i=0; i< numMaterials; i++)
  {
    std::string materialName=matNames[i];
    const MeshMaterial curMat = inputMesh->getMaterialFromSimpleName(materialName);
     
    for(int patchNum=0; patchNum<numPatches;patchNum++)
    {

      const std::set<ElementID>& elementsInPatch=inputMesh->getElementsInPatch(patchNum);

      int size=0;
      for(std::set<ElementID>::const_iterator it =elementsInPatch.begin();
        it!=elementsInPatch.end();
        it++, size++)
      {
        std::vector<float3> tri;
        Element* el = Element::GetElement(*it);
        assert(el!=NULL);
        const MeshMaterial* meshMat=el->getMaterialPtr();
        assert(meshMat!=NULL);
        bool isGlass =meshMat->isGlass();
        //If the material of the current element is the material we are
        //  currently interested in
        if(
          materialName==meshMat->getSimpleName()
          )
        {
          matCounts[i]++;
        }
      }
    }
     
#ifdef OMITEXTRA

     if(curMat.isExtra())
       matCounts[i]=0;
#endif
#ifdef OMITSENSOR
     if(curMat.isSensor())
       matCounts[i]=0;
#endif
     totalCount+=matCounts[i];
   
  }

    //unsigned int numVertices= 3*elements.size();
//  assert(totalCount==elements.size());
  numTriangles=totalCount;

    int elIndex=0;


  //For each material we pull out the corresponding triangles and put them in a geometry group
  for(int mat=0; mat<numMaterials; mat++)
  {
    // Create vertex index buffers
    int index=0;
    std::string materialName=matNames[mat];
    const MeshMaterial curMat = inputMesh->getMaterialFromSimpleName(materialName);
    //printf("material name %s \n", materialName.c_str());
    //NEWSTUFF
#ifdef OMITEXTRA
    if(curMat.isExtra())
       continue;
#endif
#ifdef OMITSENSOR
    if(curMat.isSensor())
       continue;
#endif
   
    //Notice this size changes for each group
    int sizeOfTriBuf=matCounts[mat];

    //printf("num patches %i \n", numPatches);

    //These buffers are required for each geometry group
    Buffer vertexIndexBuffer      = mContext->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_INT3,
                                                          sizeOfTriBuf);
    int3* vertexIndexBufferData   = static_cast<int3*>(vertexIndexBuffer->map());

    Buffer texcoordIndexBuffer    = mContext->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_INT3,
                                                          sizeOfTriBuf);
    int3* texcoordIndexBufferData = static_cast<int3*>(texcoordIndexBuffer->map());

    Buffer normalIndexBuffer      = mContext->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_INT3,
                                                          sizeOfTriBuf);
    int3* normalIndexBufferData   = static_cast<int3*>(normalIndexBuffer->map());

    Buffer patchIndexBuffer      = mContext->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_INT,
                                                          sizeOfTriBuf);
    int* patchIndexBufferData   = static_cast<int*>(patchIndexBuffer->map());

    Buffer centroidIndexBuffer      = mContext->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_INT,
                                                          sizeOfTriBuf);
    int* centroidIndexBufferData   = static_cast<int*>(centroidIndexBuffer->map());

    Buffer materialBuffer = mContext->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_UNSIGNED_INT,
                                                    sizeOfTriBuf);
    optix::uint* materialBufferData = static_cast<optix::uint*>(materialBuffer->map());


    Buffer patchStartIndexBuffer      = mContext->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_INT,
                                                          numPatches);
    int* patchStartIndexBufferData   = static_cast<int*>(patchStartIndexBuffer->map());

    Buffer patchSizeBuffer      = mContext->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_INT,
                                                          numPatches);
    int* patchSizeBufferData   = static_cast<int*>(patchSizeBuffer->map());


    // Create the mesh object
    Geometry mesh = mContext->createGeometry();
    mesh->setPrimitiveCount(sizeOfTriBuf);
    mesh->setIntersectionProgram(meshIntersect);
    mesh->setBoundingBoxProgram(meshBBox);
    mesh[ "vertexBuffer"         ]->setBuffer(mVertexBuffer);
    mesh[ "patchAreaBuffer"      ]->setBuffer(mPatchAreaBuffer);
    mesh[ "normalBuffer"         ]->setBuffer(mNormalBuffer);
    mesh[ "texcoordBuffer"       ]->setBuffer(mTexcoordBuffer);
    mesh[ "vertexIndexBuffer"    ]->setBuffer(vertexIndexBuffer);
    mesh[ "texcoordIndexBuffer"  ]->setBuffer(texcoordIndexBuffer);
    mesh[ "normalIndexBuffer"    ]->setBuffer(normalIndexBuffer);
    mesh[ "materialBuffer"       ]->setBuffer(materialBuffer);
    mesh[ "centroidIndexBuffer"  ]->setBuffer(centroidIndexBuffer);
    mesh[ "patchIndexBuffer"     ]->setBuffer(patchIndexBuffer);
    mesh[ "patchStartIndexBuffer"]->setBuffer(patchStartIndexBuffer);
    mesh[ "patchSizeBuffer"      ]->setBuffer(patchSizeBuffer);

    // Create the geom instance to hold mesh and material params
    GeometryInstance instance = mContext->createGeometryInstance(mesh, &mMaterial, &mMaterial + 1);

    //Material is set based on whether it is glass or not.
    Vec3f KD,KS,TD,TS,KE;
    float3 blackcolor =make_float3 (0);
        float3 whitecolor =make_float3 (1);
    if(curMat.isGlass())
    {
      KS=curMat.getSpecularReflectance();
      TD=curMat.getDiffuseTransmittance();
      TS=curMat.getSpecularTransmittance();
      instance[ "Ks" ]->setFloat(blackcolor);//KS.x(),KS.y(),KS.z()));
      instance[ "Kd" ]->setFloat(blackcolor);//KS.x(),KS.y(),KS.z()));
      instance[ "Td" ]->setFloat(make_float3(0));
      instance[ "Ts" ]->setFloat(make_float3(.71));
      printf("Ts %f %f %f \n", TS.x(),TS.y(),TS.z());
    }
    else if(curMat.isSensor()||curMat.isExtra())
    {
       instance[ "Ts" ]->setFloat(whitecolor);
       instance[ "Kd" ]->setFloat(blackcolor);
       instance[ "Ks" ]->setFloat(blackcolor);
       instance[ "Td" ]->setFloat(blackcolor);
    }
    else
    {
      instance[ "Ks" ]->setFloat(blackcolor);
      instance[ "Td" ]->setFloat(blackcolor);
      instance[ "Ts" ]->setFloat(blackcolor);
      Vec3f KD=curMat.getDiffuseReflectance();
      instance[ "Kd" ]->setFloat(make_float3(KD.x(),KD.y(),KD.z()));
    }
    //All materials can have Kd

    //Not using emitters yet
    instance[ "Ke" ]->setFloat(blackcolor);

    instances.push_back(instance);
    int i=0;

    int elementsWritten=0;

 
    for(int patchNum=0; patchNum<numPatches;patchNum++)
    {

      const std::set<ElementID>& elementsInPatch=inputMesh->getElementsInPatch(patchNum);

      patchStartIndexBufferData[patchNum]=elIndex;
      int size=0;
      for(std::set<ElementID>::const_iterator it =elementsInPatch.begin();
        it!=elementsInPatch.end();
        it++, size++)
      {
        std::vector<float3> tri;
        Element* el = Element::GetElement(*it);
        assert(el!=NULL);
        const MeshMaterial* meshMat=el->getMaterialPtr();
        assert(meshMat!=NULL);
        bool isGlass =meshMat->isGlass();
        //If the material of the current element is the material we are
        //  currently interested in
        if(
          materialName==meshMat->getSimpleName()
          )
        {
          //assert(elIndex<numTriangles);
          assert(index<sizeOfTriBuf);
          int3 vindices;
          numberOfTriangles++;
          #ifdef HACK
          vindices.x = elIndex*3+0;
          vindices.y = elIndex*3+1;
          vindices.z = elIndex*3+2;
          #else
          vindices.x = elIndex*3+0;
          vindices.y = elIndex*3+1;
          vindices.z = elIndex*3+2;

          #endif
          elementsWritten++;
          assert(vindices.z <size_in_loadVertexData*3);
          //Sets the indices of the vertices and the normal
          //  (currently the same because we are duplicating all info)
          vertexIndexBufferData[ index ]   = vindices;
          normalIndexBufferData[ index ]   = vindices;

          Vec3f pos1,pos2,pos3;

          pos1=inputMesh->getVertex((*el)[0])->get();
          pos2=inputMesh->getVertex((*el)[1])->get();
          pos3=inputMesh->getVertex((*el)[2])->get();

          //Adds the centroid to the centroid array
          mCentroids.push_back(make_float3(
                                            ( pos1.x()+pos2.x()+pos3.x() ) /3,
                                            ( pos1.y()+pos2.y()+pos3.y() ) /3,
                                            ( pos1.z()+pos2.z()+pos3.z() ) /3
                                            ));  //FIXM
          Vec3f normal;
          el->computeNormal(normal);
          mNormals.push_back(make_float3( normal.x(), normal.y(), normal.z()));  //FIXME

          //The centroid index is in terms of all geometry, no just the current
          //   material
          centroidIndexBufferData [index]  = mCentroids.size()-1;
          int patchNumber = inputMesh->getAssignedPatchForElement(el->getID());
          //printf("patch number %d\n", patchNumber);
          patchIndexBufferData [index] = patchNumber;
          //Material functionality needs to be added (currently just uses
          //  the 5 variables above)
          materialBufferData[ index++ ] = 0; // See above TODO

          //If the current element is glass it gets added to the universal
          //   glass triangle array
          //matCounts[mat]++;
          elIndex++;
          if(isGlass)
          {

            tri.push_back(make_float3(
                           pos1.x(),
                           pos1.y(),
                           pos1.z()
                         ));
            tri.push_back(make_float3(
                            pos2.x(),
                          pos2.y(),
                          pos2.z()
                        ));
            tri.push_back(make_float3(
                          pos3.x(),
                          pos3.y(),
                          pos3.z()
                        ));
            mClearTriangles.push_back(tri);
          }//endif isGlass
        }//endif materialName==meshMat->getSimpleName()
        patchSizeBufferData[patchNum]=size;
        assert(patchNum<numPatches);
        assert(patchNum!=-1);
        assert(tri.size()<=matCounts[mat]);
        //assert(size>0);
      }//end for int patchNum=0; patchNum<numPatches;patchNum++





    }//endfor for(int patchNum=0; patchNum<numPatches;patchNum++)
    printf("material: %s mat     count %d \n", matNames[mat].c_str(), matCounts[mat] );
    printf("material: %s written count %d \n", materialName.c_str(), elementsWritten);
    //All mapped buffers must be unmapped
    vertexIndexBuffer->unmap();
    texcoordIndexBuffer->unmap();
    centroidIndexBuffer->unmap();
    patchIndexBuffer->unmap();
    normalIndexBuffer->unmap();
    materialBuffer->unmap();
    patchStartIndexBuffer->unmap();
    patchSizeBuffer->unmap();
    assert(elIndex<elements.size());

  }//endfor
  // Set up group
  mGeometryGroup->setChildCount(static_cast<unsigned int>(instances.size()));

  //Now to create one more group for the missing triangles

  //Pick an acceleration structure
  Acceleration acceleration = mContext->createAcceleration("Sbvh","Bvh");

  //Setting the vertex buffer and index buffer for the current mesh
  acceleration->setProperty("vertex_buffer_name", "vertexBuffer");
  acceleration->setProperty("index_buffer_name", "vertexIndexBuffer");
  mGeometryGroup->setAcceleration(acceleration);

  //Make sure the acc structure is rebuilt
  acceleration->markDirty();

  //Add all geometry to geometry group
  for(unsigned int i = 0; i < instances.size(); ++i)
  {
    mGeometryGroup->setChild(i, instances[i]);
  }

  size_in_createGeometryInstances=elIndex;
}


//Not currently used
/*void
MeshLoader::loadMaterialParams(
  GeometryInstance gi,
  unsigned int index
) {
  // If no materials were given in mesh use reasonable defaults
  if(mMaterialParams.empty()) {
    std::cerr << " MeshLoader not setup to use material override yet!" << std::endl;
    return;
  }

  // Load params from this material into the GI
  if(index < mMaterialParams.size()) {
    MatParams& mp = mMaterialParams[index];
    gi[ "Kd" ]->setFloat(mp.Kd);
    gi[ "Ks" ]->setFloat(mp.Ks);
    gi[ "Td" ]->setFloat(mp.Td);
    gi[ "Ts" ]->setFloat(mp.Ts);
    gi[ "Ke" ]->setFloat(mp.Ke);
    return;
  }

  // Should never reach this point
  std::cerr << "WARNING -- MeshLoader::loadMaterialParams given index out of range: "
            << index << std::endl;
 }*/


