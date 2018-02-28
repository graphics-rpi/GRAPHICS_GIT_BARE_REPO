#ifndef _COMMAND_H
#define _COMMAND_H

#include <cassert>

// ===================================================================

enum COMMAND_TYPE { 
  COMMAND_NULL,
  COMMAND_QUIT,
  COMMAND_LOAD,
  COMMAND_CUT_THROUGH_PLANES,
  COMMAND_CHEAT_FAKE_MATERIALS,

  COMMAND_BAD_TRIANGLES_STATUS,
  COMMAND_ELIMINATE_BAD_TRIANGLES,
  COMMAND_SUBDIVIDE,
  COMMAND_SPLIT_EDGES,
  COMMAND_FLIP_EDGES,
  COMMAND_MOVE_VERTICES,
  COMMAND_COLLAPSE_EDGES,
  COMMAND_FIX_SEAMS,
  COMMAND_TRIANGULATE,
  COMMAND_CUT_EDGES,
  COMMAND_MOVE_VERTICES_RANDOMLY,
  COMMAND_COMPRESS_VERTICES,
  COMMAND_SEED_PATCHES,
  COMMAND_ITERATE_PATCHES,
  COMMAND_RANDOMIZE_PATCH_COLORS,
  COMMAND_RANDOMIZE_ZONE_COLORS,
  COMMAND_ASSIGN_ZONES,

  COMMAND_EVALUATE,

  COMMAND_SAVE_REMESH,

  COMMAND_PAINT_DENSITY,
  COMMAND_SAVE_DENSITY,
  
  COMMAND_SEED,
  COMMAND_CLUSTER,
  COMMAND_SEAM,
  COMMAND_REPROJECT_SEAMED,
  COMMAND_CLEAR_NEIGHBORS,
  COMMAND_FIND_NEIGHBORS,
  COMMAND_FIX_NEIGHBORS,
  COMMAND_ADJUST_PLANE_BOUNDARIES,
  COMMAND_ITERATE_ALL,
  COMMAND_ITERATE_PROBLEMS,
  COMMAND_TILT,
  COMMAND_CHECK_VORONOI,
  COMMAND_SAVE_VORONOI_SURFACE,
  COMMAND_SAVE_FULL_VORONOI,
  COMMAND_SAVE_COMPRESSED_VORONOI,

  COMMAND_HANG_TRIANGLE_MESH_RESET,
  COMMAND_HANG_VORONOI_MESH_RESET,
  COMMAND_HANG_TRIANGLE_MESH,
  COMMAND_HANG_VORONOI_MESH,
  COMMAND_FLIP_GAUDI_TRIANGLE_MESH_EDGES,
  COMMAND_SAVE_GAUDI_TRIANGLES,
  COMMAND_SAVE_GAUDI_VORONOI,

  COMMAND_CREATE_TILES,
  COMMAND_UNROLL_TILES,
  COMMAND_SAVE_TILES
};

// ===================================================================
// ===================================================================

class Command {
  
public:

  Command() { type = COMMAND_NULL, timer = 0; }
  ~Command() { }
  
  void Set(enum COMMAND_TYPE t, int time) { type = t; timer = time; }
  void Clear() { 
    assert (timer == 0);
    assert (type != COMMAND_NULL);
    type = COMMAND_NULL;
  }

  void Print() const {

    if (type == COMMAND_NULL) { printf ("is NULL COMMAND\n"); return; }

    assert (type != COMMAND_NULL);
    switch (type) {
    case COMMAND_QUIT: printf ("QUIT  "); break;
    case COMMAND_LOAD: printf ("LOAD  "); break;
    case COMMAND_CUT_THROUGH_PLANES: printf ("CUT_THROUGH_PLANES  "); break;
    case COMMAND_CHEAT_FAKE_MATERIALS: printf ("CHEAT_FAKE_MATERIALS  "); break;

    case COMMAND_BAD_TRIANGLES_STATUS: printf("BAD_TRIANGLES_STATUS  "); break;
    case COMMAND_ELIMINATE_BAD_TRIANGLES: printf("ELIMINATE_BAD_TRIANGLES  "); break;
    case COMMAND_SUBDIVIDE: printf("SUBDIVIDE  "); break;
    case COMMAND_SPLIT_EDGES: printf("SPLIT_EDGES  "); break;
    case COMMAND_FLIP_EDGES: printf("FLIP_EDGES  "); break;
    case COMMAND_MOVE_VERTICES: printf("MOVE_VERTICES  "); break;
    case COMMAND_COLLAPSE_EDGES: printf("COLLAPSE_EDGES  "); break;
    case COMMAND_SAVE_REMESH: printf("SAVE_REMESH  "); break;
        
    case COMMAND_PAINT_DENSITY: printf("PAINT_DENSITY  "); break;
    case COMMAND_SAVE_DENSITY: printf("SAVE_DENSITY  "); break;
        
    case COMMAND_SEED: printf("SEED  "); break;
    case COMMAND_CLUSTER: printf("CLUSTER  "); break;
        
    case COMMAND_FIND_NEIGHBORS: printf("FIND_NEIGHBORS  "); break;
    case COMMAND_FIX_NEIGHBORS: printf("FIX_NEIGHBORS  "); break;
    case COMMAND_SAVE_VORONOI_SURFACE: printf("SAVE_VORONOI_SURFACE  "); break;
    case COMMAND_SAVE_FULL_VORONOI: printf("SAVE_FULL_VORONOI  "); break;
        
    case COMMAND_HANG_TRIANGLE_MESH_RESET: printf("HANG_TRIANGLE_MESH_RESET  "); break;
    case COMMAND_HANG_VORONOI_MESH_RESET: printf("HANG_VORONOI_MESH_RESET  "); break;
    case COMMAND_HANG_TRIANGLE_MESH: printf("HANG_TRIANGLE_MESH  "); break;
    case COMMAND_HANG_VORONOI_MESH: printf("HANG_VORONOI_MESH  "); break;
    case COMMAND_FLIP_GAUDI_TRIANGLE_MESH_EDGES: printf("FLIP_GAUDI_TRIANGLE_MESH_EDGES  "); break;
    case COMMAND_SAVE_GAUDI_TRIANGLES: printf("SAVE_GAUDI_TRIANGLES  "); break;
    case COMMAND_SAVE_GAUDI_VORONOI: printf("SAVE_GAUDI_VORONOI  "); break;

    case COMMAND_CREATE_TILES: printf ("CREATE_TILES  "); break;
    case COMMAND_UNROLL_TILES: printf ("UNROLL_TILES  "); break;
    case COMMAND_SAVE_TILES: printf ("SAVE_TILES  "); break;

    default: printf ("UNKNOWN COMMAND  "); assert (0); break;
    }
    printf ("%d\n",timer);
  }

  enum COMMAND_TYPE getType() const { return type; }
  int getTimer() const { assert (timer >= 0); return timer; }
  void setTimer(int t) { assert (t >= 0); timer = t; }
  void decrTimer() { timer--; assert (timer >= 0); }

private:

  Command(const Command &c) { assert (0); }
  //Command& operator=(const Command &c) { assert (0); }

  // REPRESENATION
  enum COMMAND_TYPE type;
  int timer;

};

// ===================================================================
// ===================================================================

#define MAX_COMMANDS 2000

class CommandQueue {

public:

  // ========================
  // CONSTRUCTOR & DESTRUCTOR
  CommandQueue() {
    count = 0;
    next = 0;
  }
  ~CommandQueue() {}

  // ==========
  // ACCESSSORS
  bool Empty() const { return (count == 0); }
  int Count() const { return count; }
  
  Command* Next() {
    assert (count > 0);
    assert (commands[next].getType() != COMMAND_NULL);
    return &commands[next];
  }

  // =========
  // MODIFIERS
  void Add(enum COMMAND_TYPE t, int time = 1) {
    if (count >= MAX_COMMANDS) {
      printf ("WARNING:  COMMAND QUEUE FULL... IGNORING THIS COMMAND\n");
    } else {
      int x = (next+count) % MAX_COMMANDS;
      assert (commands[x].getType() == COMMAND_NULL);
      commands[x].Set(t,time);
      count++;
    }
  }
  
  void Remove() {
    assert (count > 0);
    commands[next].Clear();
    next = (next+1) % MAX_COMMANDS;
    count--;
  }

  // PRINTING
  void Print() const {
      printf ("%d commands\n", count);
    for (int i = 0; i < count; i++) {
      printf ("COMMAND %d: ", i);
      commands[(next+i)%MAX_COMMANDS].Print();
    }
  }

private:
  
  // ==============
  // REPRESENTATION
  Command commands[MAX_COMMANDS];
  int count;
  int next;
};

// ===================================================================

#endif


