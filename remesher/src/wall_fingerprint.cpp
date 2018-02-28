#include <vector>
#include <list>
#include <map>
#include <set>
#include <iomanip>

#include "utils.h"
#include "wall.h"
#include "wall_chain.h"
#include "walls.h"
#include "mesh.h"
#include "polygon.h"
#include "vectors.h"
#include "wall_fingerprint.h"

#define SAMPLES 1000

using std::set;
using std::vector;
using std::cout;
using std::endl;


extern ArgParser *ARGS;

// ===========================================================================
// ===========================================================================
// ===========================================================================

double EvalChainLink(BasicWall *w1, bool flip_first, BasicWall *w2, bool flip_second) {

  if (w1->IsColumn() || w2->IsColumn()) return false;

  Vec3f a = (w1->front_start()+w1->back_start())*0.5;
  Vec3f b = (w1->front_end()+w1->back_end())*0.5;
  Vec3f c = (w2->front_start()+w2->back_start())*0.5;
  Vec3f d = (w2->front_end()+w2->back_end())*0.5;
  Vec3f s_traj1 = w1->get_start_trajectory();
  Vec3f e_traj1 = w1->get_end_trajectory();
  Vec3f s_traj2 = w2->get_start_trajectory();
  Vec3f e_traj2 = w2->get_end_trajectory();

  Vec3f test_traj1 = a-b;  test_traj1.Normalize();
  Vec3f test_traj2 = c-d;  test_traj2.Normalize();
  double test_dot1 =  -test_traj1.Dot3(e_traj1);
  double test_dot2 =   test_traj2.Dot3(s_traj2);
  if (test_dot1 < 0.3 || test_dot2 < 0.3) {
    (*ARGS->output) << "WARNING: bad dots" << test_dot1 << " " << test_dot2 << std::endl;
  }

  if (flip_first) {
    Vec3f tmp = a; a = b; b = tmp;
    tmp = s_traj1; s_traj1 = e_traj1; e_traj1 = tmp;
  }
  if (flip_second) {
    Vec3f tmp = c; c = d; d = tmp;
    tmp = s_traj2; s_traj2 = e_traj2; e_traj2 = tmp;
  }

  double dot = -(e_traj1.Dot3(s_traj2));
  if (dot < 0) { 
    return -1; }
  if (dot < 0.5) {  
    return -1; }
  
  double dist_ad = DistanceBetweenTwoPoints(a,d);
  double dist_bc = DistanceBetweenTwoPoints(b,c);
  if (dist_ad < dist_bc) { return -1; }

  Vec3f m_traj = b-c;      m_traj.Normalize();
  double dot1 = -m_traj.Dot3(e_traj1);
  double dot2 =  m_traj.Dot3(s_traj2);
  //(*ARGS->output) << "flips " << flip_first << flip_second << "   all dots " << dot << " " << dot1 << " " << dot2 << "   dist_bc " << dist_bc; // << std::endl;

  if (dot1 < 0 || dot2 < 0) { return -1; }

  // TEST??
  if ((dot1 < 0.6 || dot2 < 0.6) && dist_bc > 2.0*INCH_IN_METERS) { return -1; }
  if ((dot1 < 0.8 || dot2 < 0.8) && dist_bc > 1.0*INCH_IN_METERS) { return -1; }

  double answer = dot * dot1 * dot2 / dist_bc;

  //  (*ARGS->output) << " distance " << dist_bc * INCH_IN_METERS << std::endl;
  // (*ARGS->output) << "  dots " << dot << " " << dot1 << " " << dot2 << "   dist_bc " << dist_bc / INCH_IN_METERS 
  //   << "    answer " << answer << std::endl;
 
  

  if (dist_bc > 5.0*INCH_IN_METERS && (dot < 0.8 || dot1 < 0.6 || dot2 < 0.6))     
    return -1;
  //) { (*ARGS->output) << "  very close!" << std::endl;  return answer; }

  //(*ARGS->output) << "dist=" << dist_bc/INCH_IN_METERS << "  dots=" << dot1 <<"," <<dot2 << std::endl;
  /*  if (dist_bc < 1.0*INCH_IN_METERS) { (*ARGS->output) << "  very close!" << std::endl;  return answer; }
  if (dist_bc < 5.0*INCH_IN_METERS && dot1 > 0.6 && dot2 > 0.6) {  (*ARGS->output) << "  medium close!" << std::endl;  return answer; }
  if (dot1 > 0.95 && dot2 > 0.95) { (*ARGS->output) << "  quite parallel!" << std::endl; return answer; }
  (*ARGS->output) << "SOMETHING ELSE " << std::endl;
  */
  return answer;
}

// ================================================================================================
void Walls::ConnectWallsIntoChains() {

  unsigned int num_walls = numWalls();

  std::cout << "Walls::ConnectWallsIntoChains" << num_walls << std::endl;

  std::vector<ChainNeighbor> best_neighbors(num_walls);

  for (unsigned int i = 0; i < num_walls; i++) {
    BasicWall *w1 = walls[i];
    for (unsigned int j = 0; j < num_walls; j++) {
      if (i == j) continue;

      //std::cout << "pair " << i << " " << j << std::endl;

      BasicWall *w2 = walls[j];

      // Try every combination of connecting these two walls into a chain
      double chain_eval_a = EvalChainLink(w1,false,w2,false);
      double chain_eval_b = EvalChainLink(w1,false,w2,true );
      double chain_eval_c = EvalChainLink(w2,false,w1,false);
      double chain_eval_d = EvalChainLink(w2,true ,w1,false);
      int num_matches = (chain_eval_a > 0) + (chain_eval_b > 0) + (chain_eval_c > 0) + (chain_eval_d > 0);
      if (num_matches == 0) continue;

      // store the best link
      if (chain_eval_a >= chain_eval_b && chain_eval_a >= chain_eval_c && chain_eval_a >= chain_eval_d) {
	if (best_neighbors[i].end_neighbor == -1 || best_neighbors[i].end_fit < chain_eval_a) {
	  best_neighbors[i].end_neighbor = j;
	  best_neighbors[i].flip_end_neighbor = false;
	  best_neighbors[i].end_fit = chain_eval_a;
	}
      } 
      else if (chain_eval_b >= chain_eval_a && chain_eval_b >= chain_eval_c && chain_eval_b >= chain_eval_d) {
	if (best_neighbors[i].end_neighbor == -1 || best_neighbors[i].end_fit < chain_eval_b) {
	  best_neighbors[i].end_neighbor = j;
	  best_neighbors[i].flip_end_neighbor = true;
	  best_neighbors[i].end_fit = chain_eval_b;
	}	
      }
      else if (chain_eval_c >= chain_eval_a && chain_eval_c >= chain_eval_b && chain_eval_c >= chain_eval_d) {
	if (best_neighbors[i].start_neighbor == -1 || best_neighbors[i].start_fit < chain_eval_c) {
	  best_neighbors[i].start_neighbor = j;
	  best_neighbors[i].flip_start_neighbor = false;
	  best_neighbors[i].start_fit = chain_eval_c;
	}
      }
      else if (chain_eval_d >= chain_eval_a && chain_eval_d >= chain_eval_b && chain_eval_d >= chain_eval_c) {
	if (best_neighbors[i].start_neighbor == -1 || best_neighbors[i].start_fit < chain_eval_d) {
	  best_neighbors[i].start_neighbor = j;
	  best_neighbors[i].flip_start_neighbor = true;
	  best_neighbors[i].start_fit = chain_eval_d;
	}
      } else {
	(*ARGS->output) << "uh oh" << std::endl;
	exit(0);
      }
    }
  }

  // =================================================================================

  CleanupBestNeighbors(best_neighbors);
  if (args->TWEAK_WALLS == true) {
    TweakWalls(best_neighbors);
  }
  MakeChains(best_neighbors);
  CheckChainsForOverlap();

  std::cout << "done with Walls::ConnectWallsIntoChains" << std::endl;

}

bool WallsAreParallel(ArgParser *args, BasicWall *w1, BasicWall *w2) {
  if (w1->IsCurvedWall()) return false;
  if (w2->IsCurvedWall()) return false;
  if (w1->numConvexQuads() != 1) return false;
  if (w2->numConvexQuads() != 1) return false;
  if (w1->IsColumn()) return false;
  if (w2->IsColumn()) return false;

  Vec3f v1 = w1->get_start_trajectory();
  Vec3f v2 = w2->get_start_trajectory();

  double angle = AngleBetween(v1,v2);
  assert (angle >= 0);
  if (angle < args->PARALLEL_ANGLE_THRESHHOLD) return true;
  assert (angle <= M_PI);
  if (angle > M_PI-args->PARALLEL_ANGLE_THRESHHOLD) return true;

  return false;
}

// ===================================================================================================
// ===================================================================================================
// ===================================================================================================
void Walls::TweakWalls(const std::vector<ChainNeighbor> &best_neighbors) {
  int num_walls = numWalls();

  // -----------------------------------------------------------
  // look for sets of parallel walls
  vector<vector<int> > parallel_sets;
  vector<int> which_parallel_set(num_walls);
  for (int i = 0; i < int(num_walls); i++) {
    bool added = false;
    for (unsigned int j = 0; j < parallel_sets.size(); j++) {
      int sample = parallel_sets[j][0];
      if (WallsAreParallel(args,walls[i],walls[sample])) {
	parallel_sets[j].push_back(i);
	added = true;
	which_parallel_set[i] = j;
	break;
      }
    }
    if (added == false) {
      parallel_sets.push_back(vector<int>(1,i));
      which_parallel_set[i] = parallel_sets.size()-1;
    }
  }
  // compute average trajectories
  vector<Vec3f> trajectories(parallel_sets.size());
  for (unsigned int i = 0; i < parallel_sets.size(); i++) {
    BasicWall *w = walls[parallel_sets[i][0]];
    if (w->IsColumn()) continue;
    Vec3f first = w->get_start_trajectory();
    for (unsigned int j = 0; j < parallel_sets[i].size(); j++) {
      Vec3f traj = walls[parallel_sets[i][j]]->get_start_trajectory();
      double length = walls[parallel_sets[i][j]]->getConvexQuad(0).Length();
      if (traj.Dot3(first) > 0.7) {
	trajectories[i] += traj*length;
      } else {
	assert (traj.Dot3(first) < -0.7);
	trajectories[i] += -traj*length;
      }
    }
    trajectories[i].Normalize();
  }
  // -----------------------------------------------------------
  // look for sets of walls to make perpendicular
  for (unsigned int i = 0; i < parallel_sets.size(); i++) {
    for (unsigned int j = i+1; j < parallel_sets.size(); j++) {
      double angle = AngleBetween(trajectories[i],trajectories[j]);
      assert (angle >= 0 && angle <= M_PI);
      if (fabs(angle-M_PI/2.0) < args->PERPENDICULAR_ANGLE_THRESHHOLD) {
	Vec3f avg_traj = trajectories[i] + trajectories[j];
	avg_traj.Normalize();
	Vec3f a(-avg_traj.z(),0,avg_traj.x());
	Vec3f b(avg_traj.z(),0,-avg_traj.x());
	a += avg_traj;  a.Normalize();
	b += avg_traj;  b.Normalize();
	if (trajectories[i].Dot3(a) > 0.7) {
	  assert (trajectories[j].Dot3(b) > 0.7);
	  trajectories[i] = a;
	  trajectories[j] = b;
	} else {
	  assert (trajectories[i].Dot3(b) > 0.7);
	  assert (trajectories[j].Dot3(a) > 0.7);
	  trajectories[i] = b;
	  trajectories[j] = a;
	}
      }
    }
  }

  // ---------------------------------------------
  // collect the centroids
  vector<Vec3f> centroids(num_walls);
  for (int i = 0; i < num_walls; i++) {
    centroids[i] = walls[i]->getCentroid();
  }
  // look for colinear walls
  for (unsigned int i = 0; i < parallel_sets.size(); i++) {
    vector<vector<int> > colinear_sets;
    Vec3f perp(trajectories[i].z(),0,-trajectories[i].x());
    for (unsigned int j = 0; j < parallel_sets[i].size(); j++) {
      int wall_id = parallel_sets[i][j];
      double d = perp.Dot3(centroids[wall_id]);
      bool found_colinear = false;
      for (unsigned int k = 0; k < colinear_sets.size(); k++) {
	int other_wall_id = colinear_sets[k][0];
	double d2 = perp.Dot3(centroids[other_wall_id]);
	if (fabs(d-d2) < args->ADJUSTABLE_D_OFFSET) {
	  found_colinear = true;
	  colinear_sets[k].push_back(wall_id);
	}
      }
      if (found_colinear == false) {
	colinear_sets.push_back(vector<int>(1,wall_id));
      }
    }
    // adjust the centroids
    for (unsigned int j = 0; j < colinear_sets.size(); j++) {
      double average_d = 0;
      for (unsigned int k = 0; k < colinear_sets[j].size(); k++) {
	int wall_id = colinear_sets[j][k];
	average_d += perp.Dot3(centroids[wall_id]);
      }
      average_d /= double(colinear_sets[j].size());
      for (unsigned int k = 0; k < colinear_sets[j].size(); k++) {
	int wall_id = colinear_sets[j][k];
	Vec3f adjust = (average_d - perp.Dot3(centroids[wall_id]))*perp;
	centroids[wall_id] += adjust;
      }
    }
  }

  // ---------------------------------------------
  // ADJUST THE WALLS 
  for (int i = 0; i < num_walls; i++) {
    BasicWall *w = walls[i];
    if (w->IsColumn()) continue;
    Vec3f traj = trajectories[which_parallel_set[i]];
    Vec3f orig_traj = w->get_start_trajectory();
    if (traj.Dot3(orig_traj) < -0.7) traj = -traj;
    assert (traj.Dot3(orig_traj));
    if (w->numConvexQuads() == 1) {
      const ConvexQuad &q = w->getConvexQuad(0);
      double length = q.Length();
      double width = q.Width();
      Vec3f perp(traj.z(),0,-traj.x());
      width = args->WALL_THICKNESS;
      Vec3f a = centroids[i] + traj*length*0.5 - perp*width*0.5;
      Vec3f b = centroids[i] - traj*length*0.5 - perp*width*0.5;
      Vec3f c = centroids[i] - traj*length*0.5 + perp*width*0.5;
      Vec3f d = centroids[i] + traj*length*0.5 + perp*width*0.5;
      ConvexQuad q2(a,b,c,d);
      w->setConvexQuad(0,q2);
      int num_windows = w->getWindows().size();
      for (int j = 0; j < num_windows; j++) {
	double left,right;
	w->getWindowDimensions(j,left,right);
	Vec3f a = centroids[i] - traj*length*(left-0.5) - perp*width*0.5;  // left
	Vec3f b = centroids[i] - traj*length*(right-0.5) - perp*width*0.5; // right
	Vec3f c = centroids[i] - traj*length*(right-0.5) + perp*width*0.5; // right
	Vec3f d = centroids[i] - traj*length*(left-0.5) + perp*width*0.5;  // left
	ConvexQuad q3(a,b,c,d);
	w->setWindowConvexQuad(j,q3);
      }
    } else {
      // curved wall (or l wall)

    }
  }
}


void BasicWall::getWindowDimensions(int i, double &left, double &right) {
  assert (numConvexQuads() == 1);
  assert (!IsColumn());
  assert (!IsCurvedWall());
  assert (i >= 0 && windows.size());
  ConvexQuad &q = convex_quads[0];
  Vec3f a = (q.verthelper(0) + q.verthelper(3)) * 0.5;
  Vec3f b = (q.verthelper(1) + q.verthelper(2)) * 0.5;
  Window &w = windows[i];
  Vec3f c = (w.verthelper(0) + w.verthelper(3)) * 0.5;
  Vec3f d = (w.verthelper(1) + w.verthelper(2)) * 0.5;
  left = DistanceBetweenTwoPoints(a,c)/DistanceBetweenTwoPoints(a,b);
  right = DistanceBetweenTwoPoints(a,d)/DistanceBetweenTwoPoints(a,b);
  assert (left < right);
  assert (left > 0.0);
  assert (right < 1.0);
}

// ===================================================================================================
// ===================================================================================================
// ===================================================================================================

void Walls::CleanupBestNeighbors(std::vector<ChainNeighbor> &best_neighbors) {
  // If the best wall connection is not mutual, then clear out the partial connection
  unsigned int num_walls = numWalls();
  for (int j = 0; j < int(num_walls); j++) {
    int i = best_neighbors[j].start_neighbor;
    if (i != -1) {
      if (best_neighbors[j].flip_start_neighbor) {
	if (best_neighbors[i].start_neighbor != j) {
	  best_neighbors[j].start_neighbor = -1;
	}
      } else {
	if (best_neighbors[i].end_neighbor != j) {
	  best_neighbors[j].start_neighbor = -1;
	}
      }
    }
    int k = best_neighbors[j].end_neighbor;
    if (k != -1) {
      if (best_neighbors[j].flip_end_neighbor) {
	if (best_neighbors[k].end_neighbor != j) {
	  best_neighbors[j].end_neighbor = -1;
	}
      } else {
	if (best_neighbors[k].start_neighbor != j) {
	  best_neighbors[j].end_neighbor = -1;
	}
      }
    }
  } 
}


// ========================================================================
// ========================================================================
// ========================================================================
void Walls::MakeChains(std::vector<ChainNeighbor> &best_neighbors) {
  unsigned int num_walls = numWalls();
  std::vector<int> counts;

  for (unsigned int i = 0; i < num_walls; i++) {
    int count = (best_neighbors[i].start_neighbor != -1) + (best_neighbors[i].end_neighbor != -1);
    counts.push_back(count);
  }

  // ---------------------------------
  // output all single walls!
  for (unsigned int i = 0; i < num_walls; i++) {
    if (counts[i] == 0) {
      BasicWall *w = walls[i];
      WallChain tmp;
      tmp.sided = false;
      if (!w->IsColumn()) {
	tmp.sided = true;
	Vec3f s_traj = w->get_start_trajectory()*scene_radius*2.0;
	tmp.quads.push_back(ConvexQuad(w->front_start()+s_traj,w->front_start(),w->back_start(),w->back_start()+s_traj));
      }
      unsigned int num_quads = w->numConvexQuads();
      for (unsigned int j = 0; j < num_quads; j++) {
	const ConvexQuad &q = w->getConvexQuad(j);
	tmp.quads.push_back(q);
      }
      if (!w->IsColumn()) {
	Vec3f e_traj = w->get_end_trajectory()*scene_radius*2.0;
	tmp.quads.push_back(ConvexQuad(w->front_end(),w->front_end()+e_traj,w->back_end()+e_traj,w->back_end()));
      }
      wall_chains.push_back(tmp);      
      counts[i] = -1;
    }
  }

  // ---------------------------------
  // output all open chains!
  for (unsigned int i = 0; i < num_walls; i++) {
    if (counts[i] == 1) {
      int current = i;
      WallChain tmp;
      tmp.sided=true;
      bool first = true;
      bool flipped = false;
      while (1) {
	BasicWall *w = walls[current];
	if (counts[current] == -1) { 
	  if (!flipped) {
	    unsigned int num_quads = w->numConvexQuads();
	    for (unsigned int j = 0; j < num_quads; j++) {
	      const ConvexQuad &q = w->getConvexQuad(j);
	      tmp.quads.push_back(q);
	    }
	    Vec3f e_traj = w->get_end_trajectory()*scene_radius*2.0;
	    tmp.quads.push_back(ConvexQuad(w->front_end(),w->front_end()+e_traj,w->back_end()+e_traj,w->back_end()));
	  } else {
	    int num_quads = (int)w->numConvexQuads();
	    for (int j = num_quads-1; j >= 0; j--) {
	      const ConvexQuad &q = w->getConvexQuad(j);
	      tmp.quads.push_back(ConvexQuad(q[2],q[3],q[0],q[1]));
	    }
	    Vec3f s_traj = w->get_start_trajectory()*scene_radius*2.0;
	    tmp.quads.push_back(ConvexQuad(w->back_start(),w->back_start()+s_traj,w->front_start()+s_traj,w->front_start()));
	  }
	  break;
	} 
	assert (counts[current] == 1);
	int next;
	if (best_neighbors[current].start_neighbor != -1) {
	  assert (best_neighbors[current].end_neighbor == -1);
	  if (first) {
	    Vec3f e_traj = w->get_end_trajectory()*scene_radius*2.0;
	    tmp.quads.push_back(ConvexQuad(w->back_end()+e_traj,w->back_end(),w->front_end(),w->front_end()+e_traj));
	    first = false;
	  }
	  int num_quads = (int)w->numConvexQuads();
	  for (int j = num_quads-1; j >= 0; j--) {
	    const ConvexQuad &q = w->getConvexQuad(j);
	    tmp.quads.push_back(ConvexQuad(q[2],q[3],q[0],q[1]));
	  }

	  next = best_neighbors[current].start_neighbor;
	  BasicWall *w2 = walls[next];
	  best_neighbors[current].start_neighbor = -1;
	  if (!best_neighbors[current].flip_start_neighbor) {
	    assert (best_neighbors[next].end_neighbor == current);
	    best_neighbors[next].end_neighbor = -1;
	    tmp.quads.push_back(ConvexQuad(w->back_start(),w2->back_end(),w2->front_end(),w->front_start()));
	    flipped = true;
	  } else {
	    assert (best_neighbors[next].start_neighbor == current);
	    best_neighbors[next].start_neighbor = -1;
	    tmp.quads.push_back(ConvexQuad(w->back_start(),w2->front_start(),w2->back_start(),w->front_start()));
	    flipped = false;
	  }
	} else {
	  assert (best_neighbors[current].end_neighbor != -1);
	  if (first) {
	    Vec3f s_traj = w->get_start_trajectory()*scene_radius*2.0;
	    tmp.quads.push_back(ConvexQuad(w->front_start()+s_traj,w->front_start(),w->back_start(),w->back_start()+s_traj));
	    first = false;
	  }
	  unsigned int num_quads = w->numConvexQuads();
	  for (unsigned int j = 0; j < num_quads; j++) {
	    const ConvexQuad &q = w->getConvexQuad(j);
	    tmp.quads.push_back(q);
	  }

	  next = best_neighbors[current].end_neighbor;
	  BasicWall *w2 = walls[next];
	  best_neighbors[current].end_neighbor = -1;
	  if (!best_neighbors[current].flip_end_neighbor) {
	    assert (best_neighbors[next].start_neighbor == current);
	    best_neighbors[next].start_neighbor = -1;
	    tmp.quads.push_back(ConvexQuad(w->front_end(),w2->front_start(),w2->back_start(),w->back_end()));
	    flipped = false;
	  } else {
	    assert (best_neighbors[next].end_neighbor == current);
	    best_neighbors[next].end_neighbor = -1;
	    tmp.quads.push_back(ConvexQuad(w->front_end(),w2->back_end(),w2->front_end(),w->back_end()));
	    flipped = true;
	  }
	}
	if (counts[next] == 1) { counts[next] = -1; }
	else { assert (counts[next] == 2); counts[next] = 1; }
	assert (counts[current] == 1);
	counts[current] = -1;
	current = next;
      }
      wall_chains.push_back(tmp);      
    }
  }

  // ---------------------------------
  // output all closed chains!
  for (unsigned int i = 0; i < num_walls; i++) {
    if (counts[i] == 2) {
      int current = (int)i;
      WallChain tmp;
      tmp.sided=true;

      while (1) {
	BasicWall *w = walls[current];
	int next;
	if (best_neighbors[current].start_neighbor != -1) {
	  int num_quads = (int)w->numConvexQuads();
	  for (int j = num_quads-1; j >= 0; j--) {
	    const ConvexQuad &q = w->getConvexQuad(j);
	    tmp.quads.push_back(ConvexQuad(q[2],q[3],q[0],q[1]));
	  }

	  next = best_neighbors[current].start_neighbor;
	  BasicWall *w2 = walls[next];
	  best_neighbors[current].start_neighbor = -1;
	  if (!best_neighbors[current].flip_start_neighbor) {
	    assert (best_neighbors[next].end_neighbor == current);
	    best_neighbors[next].end_neighbor = -1;
	    tmp.quads.push_back(ConvexQuad(w->back_start(),w2->back_end(),w2->front_end(),w->front_start()));
	  } else {
	    assert (best_neighbors[next].start_neighbor == current);
	    best_neighbors[next].start_neighbor = -1;
	    tmp.quads.push_back(ConvexQuad(w->back_start(),w2->front_start(),w2->back_start(),w->front_start()));
	  }
	} else {
	  assert (best_neighbors[current].end_neighbor != -1);
	  unsigned int num_quads = w->numConvexQuads();
	  for (unsigned int j = 0; j < num_quads; j++) {
	    const ConvexQuad &q = w->getConvexQuad(j);
	    tmp.quads.push_back(q);
	  }

	  next = best_neighbors[current].end_neighbor;
	  BasicWall *w2 = walls[next];
	  best_neighbors[current].end_neighbor = -1;
	  if (!best_neighbors[current].flip_end_neighbor) {
	    assert (best_neighbors[next].start_neighbor == current);
	    best_neighbors[next].start_neighbor = -1;
	    tmp.quads.push_back(ConvexQuad(w->front_end(),w2->front_start(),w2->back_start(),w->back_end()));
	  } else {
	    assert (best_neighbors[next].end_neighbor == current);
	    best_neighbors[next].end_neighbor = -1;
	    tmp.quads.push_back(ConvexQuad(w->front_end(),w2->back_end(),w2->front_end(),w->back_end()));
	  }
	}
	if (next == (int)i) break;
	assert (counts[next] == 2); 
	counts[next] = 1; 
	counts[current] = -1;
	current = next;
      }
      wall_chains.push_back(tmp);      
    }
  }
}

// ========================================================================
// ========================================================================  

void Walls::CheckChainsForOverlap() {
  //(*ARGS->output) << "CHECK CHAINS FOR OVERLAP" << std::endl;
  while (1) {
    unsigned int num_chains = wall_chains.size();
    for (unsigned int i = 0; i < num_chains; i++) {
      const WallChain &chain = wall_chains[i];
      int num_quads = chain.quads.size();
      bool chain_cut = false;
      bool partial = false;
      int partial_min = -1;
      int partial_max = -1;
      for (int j = 0; j < num_quads; j++) {
	if (chain_cut == true) break;
	for (int k = j+1; k < num_quads; k++) {
	  Vec3f vA,vB,vC,vD;
	  bool A,B,C,D;
	  A = SegmentsIntersect(chain.quads[j][0],chain.quads[j][1],chain.quads[k][0],chain.quads[k][1],vA);
	  B = SegmentsIntersect(chain.quads[j][0],chain.quads[j][1],chain.quads[k][2],chain.quads[k][3],vB);
	  C = SegmentsIntersect(chain.quads[j][2],chain.quads[j][3],chain.quads[k][0],chain.quads[k][1],vC);
	  D = SegmentsIntersect(chain.quads[j][2],chain.quads[j][3],chain.quads[k][2],chain.quads[k][3],vD);
	  if (A && B && C && D) {
	    // COMPLETE OVERLAP OF THESE QUADS (SIMPLE CASE)
	    chain_cut = true;
	    // construct a closed loop chain piece
	    WallChain chain_1;
	    chain_1.sided = true;
	    chain_1.quads.push_back(ConvexQuad(vA,chain.quads[j][1],chain.quads[j][2],vD));
	    for (int q = j+1; q < k; q++) { chain_1.quads.push_back(chain.quads[q]); }
	    chain_1.quads.push_back(ConvexQuad(chain.quads[k][0],vA,vD,chain.quads[k][3]));
	    if (j != 0 || k != num_quads-1) {
	      // need to construct a second chain piece for the other part
	      WallChain chain_2;
	      chain_2.sided = true;
	      for (int q = 0; q < j; q++) { chain_2.quads.push_back(chain.quads[q]); }
	      chain_2.quads.push_back(ConvexQuad(chain.quads[j][0],vA,vD,chain.quads[j][3]));
	      chain_2.quads.push_back(ConvexQuad(vA,chain.quads[k][1],chain.quads[k][2],vD));
	      for (int q = k+1; q < num_quads; q++) { chain_2.quads.push_back(chain.quads[q]); }
	      // add the second chain piece 
	      wall_chains.push_back(chain_2);
	    }
	    // replace the original chain with the first chain piece
	    wall_chains[i] = chain_1;
	    break;
	  } else if (A || B || C || D) {
	    //(*ARGS->output) << "PARTIAL INTERSECTION " << i << " num quads " << num_quads << " " << j << k << " " << A << B << C << D << std::endl;
	    if (partial == false) {
	      partial_min = j+1;
	      partial_max = k-1;
	      partial = true;
	    } else {
	      partial_min = max2(partial_min,j+1);
	      partial_max = max2(partial_max,k-1);
	    }
	  }
	}
      }
      if (partial) {
	if (partial_min >= partial_max) {
	  (*ARGS->output) << " UH OH IN PARTIAL CHAIN INTERSECTION " << std::endl;
	  exit(0);
	}
	int choice = partial_min + ((partial_max-1-partial_min)/2);
	assert (choice >= partial_min);
	assert (choice+1 <= partial_max);
	// construct the first piece
	WallChain chain_1;
	chain_1.sided = true;
	for (int q = 0; q <= choice; q++) { chain_1.quads.push_back(chain.quads[q]); }
	Vec3f va = chain.quads[choice][1]-chain.quads[choice][0]; va.Normalize();
	Vec3f vb = chain.quads[choice][2]-chain.quads[choice][3]; vb.Normalize();
	Vec3f vab_avg = va+vb; vab_avg.Normalize();
	Vec3f A = chain.quads[choice][1] + 2 * scene_radius * vab_avg;
	Vec3f B = chain.quads[choice][2] + 2 * scene_radius * vab_avg;
	chain_1.quads.push_back(ConvexQuad(chain.quads[choice][1],A,B,chain.quads[choice][2]));
	// construct the second piece
	WallChain chain_2;
	chain_2.sided = true;
	va = chain.quads[choice+1][1]-chain.quads[choice+1][0]; va.Normalize();
	vb = chain.quads[choice+1][2]-chain.quads[choice+1][3]; vb.Normalize();
	vab_avg = va+vb; vab_avg.Normalize();
	A = chain.quads[choice+1][0] - 2 * scene_radius * vab_avg;
	B = chain.quads[choice+1][3] - 2 * scene_radius * vab_avg;
	chain_2.quads.push_back(ConvexQuad(A,chain.quads[choice+1][0],chain.quads[choice+1][3],B));
	for (int q = choice+1; q < num_quads; q++) { chain_2.quads.push_back(chain.quads[q]); }
	// replace the original chain with the first chain piece
	wall_chains[i] = chain_1;
	// add the second piece
	wall_chains.push_back(chain_2);
	break;
      }
    }
    if (num_chains == wall_chains.size()) { break; }
    //(*ARGS->output) << "split at least one chain " << num_chains << " -> " << wall_chains.size() << std::endl;
    //(*ARGS->output) << "REPEAT CHAIN CHECK" << std::endl;
  }
}

// ========================================================================
// ========================================================================
// ========================================================================  

std::vector<WALL_FINGERPRINT> Walls::FingerprintPoint(const Vec3f &pt) const {
  std::vector<WALL_FINGERPRINT> answer;
  unsigned int num_chains = wall_chains.size();
  // determine the sidedness of this cell for each wall chain
  for (unsigned int i = 0; i < num_chains; i++) {
    WALL_FINGERPRINT tmp = FINGERPRINT_NONE;
    const WallChain &chain = wall_chains[i];
    if (!chain.sided) {
      tmp = FINGERPRINT_FRONT;
    } else {
      unsigned int num_quads = chain.quads.size();
      double shortest = -1;
      for (unsigned int j = 0; j < num_quads; j++) {
	// check to see if the pt is inside the chain band
	if (chain.quads[j].PointInside(pt)) { tmp = FINGERPRINT_MIDDLE; break; }
	// check to see which side of the chain the pt is closer to
	double da = DistanceToLineSegment(pt,chain.quads[j][0],chain.quads[j][1]);
	double db = DistanceToLineSegment(pt,chain.quads[j][2],chain.quads[j][3]);
	if (shortest < 0 || da < shortest) { shortest = da; tmp = FINGERPRINT_FRONT; }
	if (db < shortest)                 { shortest = db; tmp = FINGERPRINT_BACK; }
      }
    }
    assert (tmp != FINGERPRINT_NONE);
    // add the sidedness to the fingerprint
    answer.push_back(tmp);
  }
  return answer;
}

std::vector<WALL_FINGERPRINT> Walls::FingerprintPoly(Poly *p) const {
  std::vector<WALL_FINGERPRINT> answer;
  assert (p != NULL);
  Vec3f centroid = p->getCentroid();
  return FingerprintPoint(centroid);
}


const BasicWall& Walls::WallFromFingerprint(Poly *p, Poly *p2) {
  //assert(0);
  assert (p != NULL);
  assert (p2 != NULL);
  std::vector<WALL_FINGERPRINT> a = FingerprintPoly(p);
  std::vector<WALL_FINGERPRINT> b = FingerprintPoly(p2);

  int num_diff = 0;
  assert (a.size() == b.size());
  assert (a.size() == wall_chains.size()); // + curvedwalls.size() + lshapedwalls.size());
  
  for (unsigned int i = 0; i < a.size(); i++) {
    if (a[i] != b[i]) {
      num_diff++;
      if (i < walls.size())
	return *walls[i];
      if (b[i] != FINGERPRINT_MIDDLE) {
	(*ARGS->output) << " something fishy probably a curved wall bug " << std::endl;
	continue;
      }
      assert (b[i] == FINGERPRINT_MIDDLE);
      assert (a[i] == FINGERPRINT_FRONT ||
	      a[i] == FINGERPRINT_BACK);
    }
  }
  if (num_diff != 1) { 
      (*ARGS->output) << " num_diff != 1" << std::endl; 
  }  
  //  assert(0);
  //exit(0);
  //assert (answer != NULL);
  //return answer;
  //return NULL;
  throw -1;
}



void printPrint(const std::vector<WALL_FINGERPRINT> &p1) {
    for (unsigned int i = 0; i < p1.size(); i++) {
      if (p1[i] == FINGERPRINT_FRONT) (*ARGS->output) << "front  ";
      else if (p1[i] == FINGERPRINT_BACK) (*ARGS->output) << "BACK   ";
      else if (p1[i] == FINGERPRINT_MIDDLE) (*ARGS->output) << "****** ";
      else assert(0);
    }
    (*ARGS->output) << std::endl;
}

std::ostream& operator<<(std::ostream& ostr, const std::vector<WALL_FINGERPRINT> &p1) {
//void printPrint2(const std::vector<WALL_FINGERPRINT> &p1) {
  for (unsigned int i = 0; i < p1.size(); i++) {
    if (p1[i] == FINGERPRINT_FRONT) ostr << "f";
    else if (p1[i] == FINGERPRINT_BACK) ostr << "B";
    else if (p1[i] == FINGERPRINT_MIDDLE) ostr << "*";
    else assert(0);
  }
  return ostr;
}

bool samePrint(const std::vector<WALL_FINGERPRINT> &p1,const std::vector<WALL_FINGERPRINT> &p2) {
  assert (p1.size() == p2.size());
  for (unsigned int i = 0; i < p1.size(); i++) {
    if (p1[i] != p2[i]) return false;
  }
  return true;
}

bool noMiddle(const std::vector<WALL_FINGERPRINT> &p1) {
  for (unsigned int i = 0; i < p1.size(); i++) {
    if (p1[i] == FINGERPRINT_MIDDLE) return false;
  }
  return true;
}
			
int NumMiddles(const std::vector<WALL_FINGERPRINT> &print) {
  int answer = 0;
  for (unsigned int i = 0; i < print.size(); i++) {
    if (print[i] == FINGERPRINT_MIDDLE) answer++;
  }
  return answer;
}




Vec3f NumMiddlesColor(const std::vector<WALL_FINGERPRINT> &print) {
  int num_mid = NumMiddles(print);
  if (num_mid == 0) 
    return Vec3f(0,0,0);
  if (num_mid == 1)
    return Vec3f(0,0,1);
  if (num_mid == 2)
    return Vec3f(0,1,0);
  if (num_mid == 3)
    return Vec3f(1,1,0);
  if (num_mid == 4)
    return Vec3f(1,0,0);
  return Vec3f(1,1,1);
}


Vec3f FingerprintColor(ArgParser *args, const std::vector<WALL_FINGERPRINT> &print, int which_subgroup) {
  static std::list<Vec3f> available_colors;
  static bool flag = false;
  static std::map<int,Vec3f> colors;
  if (!flag) {
    flag = true;
    available_colors.push_back(Vec3f(1,0,0)); // red 
    available_colors.push_back(Vec3f(0,1,0)); // green
    available_colors.push_back(Vec3f(0,0,1)); // blue

    available_colors.push_back(Vec3f(1,1,0)); // yellow
    available_colors.push_back(Vec3f(1,0,1)); // purple
    available_colors.push_back(Vec3f(0,1,1)); // cyan

    available_colors.push_back(Vec3f(1,0.5,0)); // orange
    available_colors.push_back(Vec3f(1,0.3,0.7)); // light pink purple
    available_colors.push_back(Vec3f(0.3,0,0.8)); // dark purple

    available_colors.push_back(Vec3f(0.8,1,0.5)); // pale green
    available_colors.push_back(Vec3f(0,0.5,0)); // dark green

    available_colors.push_back(Vec3f(0.8,0.8,1)); // pale blue
    available_colors.push_back(Vec3f(0,0,0.6)); // dark blue

    available_colors.push_back(Vec3f(0.5,0.5,0)); // dark brown
    available_colors.push_back(Vec3f(0.5,0,0)); // dark red 
    available_colors.push_back(Vec3f(1,0.8,0.8)); // pale pink

  }
  unsigned int which = which_subgroup;
  for (unsigned int i = 0; i < print.size(); i++) {
    which = which*2;
    if (print[i] == FINGERPRINT_FRONT) which += 1;
    if (print[i] == FINGERPRINT_MIDDLE) {
      return Vec3f(1,1,1);
      //which += 1;
    }
  }
  assert (which >= 0); 
  if (colors.find(which)==colors.end()) {
    Vec3f v;
    if (available_colors.size() > 0) {
      v = available_colors.front();
      available_colors.pop_front();
    } else {
      v = args->RandomColor();
    }
    colors.insert(std::make_pair(which,v));
  }
  return colors[which];
}

double Walls::ComputePercentEnclosed(Poly *p) const {

  Vec3f p_centroid = p->getCentroid();
  return ComputePercentEnclosed(p_centroid);

}


double Walls::ComputePercentEnclosed(const Vec3f &v) const {

  int num_walls = numWalls();

  // return -1 if the point is inside any wall
  for (int i = 0; i < num_walls; i++) {
    if (walls[i]->PointInside(v)) return -1;
  }

  // initialize all the samples to false (not enclosed)
  bool arr[SAMPLES];
  for (int i = 0; i < SAMPLES; i++) { arr[i] = false; }

  //(*ARGS->output) << "\n===========================\nHERE " << SAMPLES << std::endl;

  // loop through all the walls
  for (int i = 0; i < numWalls(); i++) {
    BasicWall *w = walls[i];

    // intersect with the edges of each quad
    int num_quads = w->numConvexQuads();
    for (int j = 0; j < num_quads; j++) {
      const ConvexQuad &q = w->getConvexQuad(j);
      for (int k = 0; k < 4; k++) {
	Vec3f normal;

	// don't compare against edges that face the away from the test point
	computeNormal(q[k],q[(k+1)%4],q[k]+Vec3f(0,1,0),normal);
	if (normal.Dot3(v - q[k]) > 0) 
	  continue;
	
	// 
	Vec3f v1 = Vec3f(1,0,0);
	Vec3f v2 = q[(k+1)%4]-v;
	Vec3f v3 = q[k]-v;
	v2.Normalize();
	v3.Normalize();
	double a2 = AngleBetweenNormalized(v1,v2);
	double a3 = AngleBetweenNormalized(v1,v3);
	if (v2.z() < 0) {
	  a2 = 2*M_PI - a2;
	}
	if (v3.z() < 0) {
	  a3 = 2*M_PI - a3;
	}
	
	if (fabs(a2-a3) < 0.000001) {
	  //(*ARGS->output) << "TOO SIMILAR" << std::endl;
	  continue;
	}

	//(*ARGS->output) << "a " << a2 << " " << a3 << std::endl;
	if (a2 < a3) {    
	  for (int i = 0; i < SAMPLES; i++) {     
	    double val = 2*M_PI*i / double(SAMPLES);    
	    if (val >= a2 && val <= a3) {     
	      //(*ARGS->output) << ".";
	      arr[i] = true;    
	    }     
	  }       
	} else {    
	  for (int i = 0; i < SAMPLES; i++) {     
	    double val = 2*M_PI*i / double(SAMPLES);    
	    if (val >= a2 || val <= a3) {
	      //(*ARGS->output) << "*";
	      arr[i] = true;    
	    }
	  }       
	}  
      }
    }
  }

  int sum = 0;
  for (int i = 0; i < SAMPLES; i++) { if (arr[i]) sum++; }

  double answer = sum / double(SAMPLES);
  if (answer > 0.99) {
    //(*ARGS->output) << answer << std::endl;
    //exit(0);
  }
  return answer;

}


// returns true if any group was split
bool PolygonLabels::SplitMixedEnclosureGroups(double tuned_enclosure_threshold) {
  bool answer = false;
  for (std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::iterator iter=groups.begin(); iter!=groups.end(); iter++) {
    const std::vector<WALL_FINGERPRINT> &print = iter->first;
    if (!(noMiddle(print))) continue;  // THIS SHOULD BE INVESTIGATED FURTHER, images_20091028/image_00053.wall
    PolygonGroupData &data = iter->second;
    int num_subgroups = data.numSubGroups();
    for (int i = 0; i < num_subgroups; i++) {
      std::string is_enclosed;
      data.AnalyzeEnclosureHistogram(i,is_enclosed,tuned_enclosure_threshold);
      if (is_enclosed == "MIXED_INTERIOR" ||
	  is_enclosed == "MIXED_not") {
	bool success = data.SplitSubGroup(i);
	if (success) answer = true;
      }
    }
  }
  return answer;
}

// =====================================
// defined in mincut.cpp
int Edmonds_Karp(int n,const std::vector<std::vector<int> > &cap,
		 int source,int sink,std::vector<std::vector<int> > &flow);
std::vector<int> find_min_cut(int n, const std::vector<std::vector<int> > &cap, 
			      const std::vector<std::vector<int> > &flow, int max_flow,
			      int source, int sink);
// =====================================

double LengthOfSharedEdge(Poly *a, Poly *b) {
  for (int i = 0; i < a->numVertices(); i++) {
    int j = (i+1)%a->numVertices();
    assert (a->HasBothVertices((*a)[i],(*a)[j]));
    if (!b->HasBothVertices((*a)[i],(*a)[j])) continue;
    Vec3f vi = a->getMesh()->getVertex((*a)[i])->get();
    Vec3f vj = a->getMesh()->getVertex((*a)[j])->get();
    return DistanceBetweenTwoPoints(vi,vj);
  }
  return -1;
}


bool PolygonGroupData::SplitSubGroup(int i) {
  assert (i >= 0 && i < numSubGroups());

  // collect all the polygons and put them in a vector so we can build a graph
  const std::set<Poly*> &pset = get(i).polys;
  std::vector<Poly*> allpolys;
  for (std::set<Poly*>::const_iterator itr = pset.begin(); itr != pset.end(); itr++) { allpolys.push_back(*itr); }
  if (allpolys.size() == 1) { 
      (*ARGS->output) << "only one poly, can't split" << std::endl;
    return false; }

  // prepare to solve maxflow/mincut  
  int n = allpolys.size();
  std::vector<std::vector<int> > cap = std::vector<std::vector<int> >(n,std::vector<int>(n,0));
  std::vector<std::vector<int> > flow;
  int source = -1;
  double source_enclosure = 0;
  int sink = -1;
  double sink_enclosure = 0;
  // identify the source & sink and prepare the capacity graph
  for (int j = 0; j < n; j++) {
    double ej = allpolys[j]->getPercentEnclosed();
    if (source == -1 || source_enclosure < ej) { source = j; source_enclosure = ej; }
    if (sink == -1   || sink_enclosure > ej  ) { sink = j;   sink_enclosure = ej; }
  }
  if (source == sink) return false;

  for (int j = 0; j < n; j++) {
    double ej = allpolys[j]->getPercentEnclosed();
    for (int k = 0; k < n; k++) {
      double length = LengthOfSharedEdge(allpolys[j],allpolys[k]);
      if (length < -0.5) continue;
      double ek = allpolys[j]->getPercentEnclosed();
      if (ej > 0.9*source_enclosure && ek > 0.9*source_enclosure) length *=10;
      //if (ej < 0.5 && ek < 0.5) length *=10;
      if (ej <= ek) { 	
	cap[j][k] = std::max(1,int(100*length)); 
      }
    }
  }  


  // solve the maxflow/mincut problem
  int max_flow = Edmonds_Karp(n,cap,source,sink,flow);
  std::vector<int> partition = find_min_cut(n,cap,flow,max_flow,source,sink);
  assert (n == (int)partition.size());

  double area_source_side = 0;
  double area_sink_side = 0;

  for (int j = 0; j < n; j++) {
    if (partition[j] == 1) {
      area_source_side += allpolys[j]->Area();
    } else {
      assert (partition[j] == -1);
      area_sink_side += allpolys[j]->Area();
    }
  }

  double cut_length_inches = max_flow/(100.0*INCH_IN_METERS);
  double source_area_inches = area_source_side / (INCH_IN_METERS*INCH_IN_METERS);
  double sink_area_inches = area_sink_side / (INCH_IN_METERS*INCH_IN_METERS);

    (*ARGS->output) << std::setprecision(6);
    (*ARGS->output) << "cut length:  " << cut_length_inches << " inches"<< std::endl;
    (*ARGS->output) << "SOURCE AREA: " << source_area_inches << std::endl;
    (*ARGS->output) << "SINK AREA:   " << sink_area_inches << std::endl;

  if (source_area_inches < 3.5 ||
      sink_area_inches < 3.5) {
      (*ARGS->output) << "area too small to split" << std::endl;
    return false;
  }

  if (source_area_inches < 8.0 || sink_area_inches < 8.0) {
    if (cut_length_inches > 1.5*sqrt(source_area_inches) ||
	cut_length_inches > 1.5*sqrt(sink_area_inches)) {
	(*ARGS->output) << "cut_length too large to split" << std::endl;
      return false;
    }
  }
      
  // go ahead with the split!
  // add a new group, clear out the old group
  subgroups.push_back(SubGroupData());
  subgroups[i] = SubGroupData();
  for (int k = 0; k < n; k++) {
    int which_group;
    if (partition[k] == 1) {
      which_group = i;
    } else {
      assert (partition[k] == -1);
      which_group = subgroups.size()-1;
    }
    Poly *p = allpolys[k];
    p->which_subgroup = which_group;
    subgroups[which_group].polys.insert(p);
    double area = p->Area();
    double enclosed = p->getPercentEnclosed();
    subgroups[which_group].area_weighted_enclosure += area*enclosed; 
    subgroups[which_group].area_sum += area; 
    assert (!isWallOrWindow(p->getCellType()));
  }
    (*ARGS->output) << "finished split " << std::endl;
  return true;
}

ZoneData PolygonGroupData::AnalyzeZone(const std::vector<WALL_FINGERPRINT> &print, int i) {

  ZoneData answer;
  answer.print = print;
  answer.subgroup = i;
  answer.interior = false;

  AverageEnclosure(i,answer.average_enclosure,answer.area); 
  
  const std::set<Poly*> &polys = get(i).polys;
  for (std::set<Poly*>::iterator itr = polys.begin(); itr != polys.end(); itr++) {    
    Poly *poly = *itr;
    for (int k = 0; k < poly->numVertices(); k++) { 
      std::vector<Element*> neighbors = poly->getNeighbors(k);
      for (unsigned int junk = 0; junk < neighbors.size(); junk++) {	
	assert (neighbors[junk]->isAPolygon());
	Poly *neighbor = (Poly*)neighbors[junk];
	if (neighbor->isWall()) {
	  int id = neighbor->getWallID();	  
	  Mesh *m = poly->getMesh();
	  int va = (*poly)[k];
	  int vb = (*poly)[(k+1)%poly->numVertices()];
	  assert (poly->HasVertex(va));
	  assert (poly->HasVertex(vb));
	  assert (neighbor->HasVertex(va));
	  assert (neighbor->HasVertex(vb));
	  Vec3f a = m->getVertex(va)->get();
	  Vec3f b = m->getVertex(vb)->get();
	  double length = DistanceBetweenTwoPoints(a,b);
	  answer.wall_border[id] += length;
	}
      }
    }
  }
  /*
  (*ARGS->output) << "THIS ZONE TOUCHES: ";
  for (std::map<int,double>::iterator itr = answer.wall_border.begin(); itr != answer.wall_border.end(); itr++) {
    (*ARGS->output) << " " << itr->first;
  }
  (*ARGS->output) << std::endl;
  */

  return answer;
}

std::map<std::pair<std::vector<WALL_FINGERPRINT>,int>,std::string> GLOBAL_FLOOR_PLAN_COLORS;

Vec3f GLOBAL_FLOOR_PLAN_COLOR(const std::vector<WALL_FINGERPRINT> &print, int which_subgroup) {
  std::map<std::pair<std::vector<WALL_FINGERPRINT>,int>,std::string>::iterator itr = 
    GLOBAL_FLOOR_PLAN_COLORS.find(make_pair(print,which_subgroup));
  std::string answer = "nothing";
  if (itr != GLOBAL_FLOOR_PLAN_COLORS.end()) answer = itr->second;
  if (answer == "nothing") {
    return Vec3f(1,0,0);
  } else {
    return Vec3f(0,1,0);
  }
}

double EvaluateZoneUsage(ArgParser *args, int q,
			 const std::vector<ZoneData> &zones, const std::set<int> &zones_to_use, 
			 const std::vector<WallEvidence> &all_wall_evidence, 
			 double best_zones_e) {

  //std::map<std::pair<std::vector<WALL_FINGERPRINT>,int>,std::string> NEW_GLOBAL_FLOOR_PLAN_COLORS;

  double exterior_walls = 0;
  double unused_walls = 0;
  double inferred_walls = 0;
  double interior_walls = 0;

  for (unsigned int i = 0; i < all_wall_evidence.size(); i++) {
    const WallEvidence &w = all_wall_evidence[i];
    int which_zone_a = -1;
    int which_zone_b = -1;
    for (unsigned int j = 0; j < zones.size(); j++) {
      const ZoneData &tmp = zones[j];
      if (tmp.print == w.a && tmp.subgroup == w.which_subgroup_a) { which_zone_a = j; }
      if (tmp.print == w.b && tmp.subgroup == w.which_subgroup_b) { which_zone_b = j; }
    }
    assert (which_zone_a >= 0 && which_zone_b >= 0);

    assert (which_zone_a != which_zone_b);
    if (which_zone_a > which_zone_b) continue;

    bool a_interior = zones_to_use.find(which_zone_a) != zones_to_use.end();
    bool b_interior = zones_to_use.find(which_zone_b) != zones_to_use.end();

    if (a_interior && b_interior) {
      //NEW_GLOBAL_FLOOR_PLAN_COLORS[make_pair(w->
      interior_walls += w.wall_area;   
      if (w.nonwall_area < 2.0*INCH_IN_METERS*args->WALL_THICKNESS) {
	//(*ARGS->output) << "compare " << std::setprecision(6) << w->nonwall_area << " " << 1.0*INCH_IN_METERS*WALL_THICKNESS << std::endl;
	//(*ARGS->output) << "adding some inferred length " << std::endl;
	inferred_walls += w.nonwall_area;
      }
    }
    else if (a_interior != b_interior) {
      exterior_walls += w.wall_area;
      inferred_walls += w.nonwall_area;
    }
    else {
      assert (!a_interior && !b_interior);
      unused_walls += w.wall_area;
    }
  }

  exterior_walls = exterior_walls / args->WALL_THICKNESS / INCH_IN_METERS;
  unused_walls   = unused_walls   / args->WALL_THICKNESS / INCH_IN_METERS;
  inferred_walls = inferred_walls / args->WALL_THICKNESS / INCH_IN_METERS;
  interior_walls = interior_walls / args->WALL_THICKNESS / INCH_IN_METERS;

  double interior_area = 0;
  double exterior_area = 0;
  for (unsigned int i = 0; i < zones.size(); i++) {
    if (zones_to_use.find(i) == zones_to_use.end()) 
      exterior_area += zones[i].area;
    else
      interior_area += zones[i].area;
  }

  double answer = unused_walls + inferred_walls; // + (interior_walls * 0.01) + (interior_area*0.01);

  if (args->non_zero_interior_area && interior_area < 0.00001) {
      (*ARGS->output) << " this is not a good choice" << std::endl;
    return answer;
  }

  if (best_zones_e < 0 || answer < 1.05*best_zones_e) {
    //if (best_zones_e < 0 || answer < best_zones_e) {
      (*ARGS->output) << " EZU  " << q << " " 
		<< " exterior:"         << std::setprecision(1) << std::setw(5) << exterior_walls 
		<< " unused:"           << std::setprecision(1) << std::setw(5) << unused_walls 
		<< " inferred:"         << std::setprecision(1) << std::setw(5) << inferred_walls 
		<< " interior:"         << std::setprecision(1) << std::setw(5) << interior_walls 
		<< " wallsum: "         << std::setprecision(1) << std::setw(5) << exterior_walls+unused_walls+interior_walls 
		<< " evaluate: "        << std::setprecision(1) << std::setw(5) << answer
		<< " interior area: "   << std::setprecision(4) << std::setw(7) << interior_area
		<< " total area: "      << std::setprecision(4) << std::setw(7) << interior_area+exterior_area
		<< std::endl;
  }
  
  return answer;
}

///////////////////////////////////////////////////////////////

void AddWallEvidence(std::vector<WallEvidence> &all_evidence,		     
		     const std::vector<WALL_FINGERPRINT> &a, int which_subgroup_a,
		     const std::vector<WALL_FINGERPRINT> &b, int which_subgroup_b,
		     bool is_real_wall, double area) {
  for (unsigned int i = 0; i < all_evidence.size(); i++) {
    WallEvidence &tmp = all_evidence[i];
    if (tmp.a != a) continue;
    if (tmp.which_subgroup_a != which_subgroup_a) continue;
    if (tmp.b != b) continue;
    if (tmp.which_subgroup_b != which_subgroup_b) continue;
    if (is_real_wall) tmp.wall_area += area; 
    else tmp.nonwall_area += area;
    return;
  }

  // need to add new
  WallEvidence tmp;
  tmp.a = a;
  tmp.which_subgroup_a = which_subgroup_a;
  tmp.b = b;
  tmp.which_subgroup_b = which_subgroup_b;
  if (is_real_wall) { tmp.wall_area = area; tmp.nonwall_area = 0; }
  else              { tmp.wall_area = 0; tmp.nonwall_area = area; }
  all_evidence.push_back(tmp);
}

double LengthOfSharedEdges(const std::set<Poly*> &a, const std::set<Poly*> &b) {
  double answer = 0;
  for (std::set<Poly*>::const_iterator itr = a.begin(); itr != a.end(); itr++) {
    Poly *p = *itr;
    Mesh *m = p->getMesh();
    for (int i = 0; i < p->numVertices(); i++) {
      std::vector<Element*> neighbors = p->getNeighbors(i);
      if (neighbors.size() == 0) continue;
      assert (neighbors.size() == 1);
      assert (neighbors[0]->isAPolygon());
      Poly *p2 = (Poly*)neighbors[0];
      assert (a.find(p) != a.end());
      if (b.find(p2) != b.end()) {
	Vec3f pt0 = m->getVertex((*p)[i])->get();
	Vec3f pt1 = m->getVertex((*p)[(i+1)%p->numVertices()])->get();
	answer += DistanceBetweenTwoPoints(pt0,pt1);
      }      
    }
  }
  return answer;
}

void PolygonLabels::WallAnalysis(ArgParser *args, std::vector<WallEvidence> &all_evidence) {
  for (std::map<std::vector<WALL_FINGERPRINT>, PolygonGroupData>::iterator itr = groups.begin(); itr != groups.end(); itr++) {
    const std::vector<WALL_FINGERPRINT> &print = itr->first;
    if (NumMiddles(print) != 0) continue;
    const PolygonGroupData &data = itr->second;
    for (int i = 0; i < data.numSubGroups(); i++) {

      for (int q = 0; q < data.numSubGroups(); q++) {
	if (q == i) continue;
	//(*ARGS->output) << "should check border between subgroups " << print << " " << i << "&" << q << std::endl;
	const SubGroupData &sgi = data.get(i);
	const SubGroupData &sgq = data.get(q);
	double length = LengthOfSharedEdges(sgi.polys,sgq.polys);	
	//(*ARGS->output) << std::setprecision(9) << "length " << length << std::endl;
	AddWallEvidence(all_evidence, print, i, print, q, false, 2*length*args->WALL_THICKNESS);
	//AddWallEvidence(all_evidence, print, i, print, q, false, 0); //length*WALL_THICKNESS);
      }

      // loop over all the subgroups/zones that are potential interiors
      for (unsigned int j = 0; j < print.size(); j++) {
	std::vector<WALL_FINGERPRINT> print2 = print;
	assert (print2[j] == FINGERPRINT_FRONT || print2[j] == FINGERPRINT_BACK);
	if (print2[j] == FINGERPRINT_FRONT) print2[j] = FINGERPRINT_BACK;
	else print2[j] = FINGERPRINT_FRONT;
	std::map<std::vector<WALL_FINGERPRINT>, PolygonGroupData>::iterator itr2 = groups.find(print2);
	if (itr2 == groups.end()) continue;

	// loop over all subgroups that are potential neighbors
	const PolygonGroupData &data2 = itr2->second;	
	std::vector<WALL_FINGERPRINT> between_print = print;
	between_print[j] = FINGERPRINT_MIDDLE;
	std::map<std::vector<WALL_FINGERPRINT>, PolygonGroupData>::iterator itr_between = groups.find(between_print);
	if (itr_between == groups.end()) { 


	  
	    (*ARGS->output) << "nobetweengroup" << std::endl; 
	  /*assert(0);*/ continue; }
	
	const PolygonGroupData &data_between = itr_between->second;
	std::vector<int> between_groups_neighbors = data.whichSubGroupsAreNeighbors(i,data_between);

	for (unsigned int k = 0; k < between_groups_neighbors.size(); k++) {
	  int w = between_groups_neighbors[k];
	  const SubGroupData &wallsubgroup = data_between.get(w);
	  assert (w >=0 && w < data_between.numSubGroups());
	  std::vector<int> tmp = data_between.whichSubGroupsAreNeighbors(w,data2);
	  if (tmp.size() > 1) { (*ARGS->output) << "CRAP IN WALL EVIDENCE, need to fix area sum used: " << tmp.size() << std::endl; }
	  for (unsigned int m = 0; m < tmp.size(); m++) {
	    AddWallEvidence(all_evidence, print, i, print2, tmp[m], data_between.IsRealWall(w), wallsubgroup.area_sum);
	  }
	}
      }

      // print out the evidence
      /*
      for (unsigned int v = 0; v < all_evidence.size(); v++) {
	WallEvidence tmp = all_evidence[v];
	if (tmp.a == print && tmp.which_subgroup_a == i) {
	  double wl = tmp.wall_area / WALL_THICKNESS / INCH_IN_METERS;
	  double nl = tmp.nonwall_area / WALL_THICKNESS / INCH_IN_METERS;
	  (*ARGS->output) << "  a neighbor " << tmp.b << "" << tmp.which_subgroup_b << "    " 
		    << wl << " /" << nl << std::endl;
	  
	}
      }
      */
    }
  }
  //  exit(0);
}

///////////////////////////////////////////////////////////////

void PolygonLabels::PickInteriorZones(ArgParser *args, std::vector<ZoneData> &zones, double tuned_enclosure_threshold) {
  int numwalls = 0;
  for (unsigned int i = 0; i < zones.size(); i++) {
    for (std::map<int,double>::iterator itr = zones[i].wall_border.begin(); itr != zones[i].wall_border.end(); itr++) {
      numwalls = std::max(numwalls,itr->first+1);
    }
  }



  (*ARGS->output) << "numwalls " << numwalls << endl;

  std::vector<double> total_length(numwalls,0.0);
  std::vector<int> possible_zones;
  for (unsigned int i = 0; i < zones.size(); i++) {
    //(*ARGS->output) << "zone " << i << " " << zones[i].average_enclosure << " " << tuned_enclosure_threshold << std::endl;
    if (zones[i].average_enclosure >= tuned_enclosure_threshold) possible_zones.push_back(i);
    //(*ARGS->output) << "ZONE " << std::setw(3) << i << "  " << std::fixed << std::setw(4) << zones[i].average_enclosure << " TOUCHES: ";
    for (std::map<int,double>::iterator itr = zones[i].wall_border.begin(); itr != zones[i].wall_border.end(); itr++) {
      int w = itr->first;
      double l = itr->second;
      assert (w >= 0 && w < numwalls);
      //assert ((int)used_length.size() == numwalls);
      //(*ARGS->output) << " " << std::setw(3) << w << "(" << std::setprecision(2) << std::setw(4) << std::fixed << l << ")";
      total_length[w]+=l;
    }
    //(*ARGS->output) << std::endl;
  }

  int num_to_check = (int)pow(2.f,(int)possible_zones.size());

  double best_zones_e = -1;
  std::set<int> best_zones_to_use;

  std::vector<WallEvidence> all_wall_evidence;
  WallAnalysis(args,all_wall_evidence);


  (*ARGS->output) << "num possiblezones " << possible_zones.size() << " check " << num_to_check << " possibilities " << endl; 


  if (num_to_check > 5000000) {
    std::cout << "ERROR:  TOO MANY COMBINATIONS TO CHECK" << std::endl;
    return;
    exit(0);
  }
  int chosen_q = -1;

  for (int q = 0; q < num_to_check; q++) {
    //std::cout << "NUM TO CHECK " << q << " " << num_to_check << std::endl;
    std::set<int> zones_to_use;
    for (unsigned int p = 0; p < possible_zones.size(); p++) {
      int flag = (q >> p) % 2 == 1;
      if (flag) zones_to_use.insert(possible_zones[p]);
    }

    double e = EvaluateZoneUsage(args,q,zones,zones_to_use,all_wall_evidence, best_zones_e);

    if (best_zones_e < -0.1 || e < best_zones_e) {
      chosen_q = q;
      best_zones_e = e;
      best_zones_to_use = zones_to_use;
    }
  }


    (*ARGS->output) << "selected zone " << chosen_q << std::endl;


  // ========================
  for (std::set<int>::iterator itr = best_zones_to_use.begin();
       itr != best_zones_to_use.end(); itr++) {

      (*ARGS->output) << "ADD ZONE " << *itr << std::endl;

    SetInterior(zones[*itr].print,zones[*itr].subgroup);
  }
}


void PolygonLabels::TabulateEnclosure(ArgParser *args,double tuned_enclosure_threshold) {

  (*ARGS->output) << "threshold = " << tuned_enclosure_threshold << std::endl;

  std::vector<ZoneData> zones;

  std::vector<WALL_FINGERPRINT> best_print;
  //  double best_enclosure = -1;
  //int best_subgroup = -1;
  for (std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::iterator iter=groups.begin(); iter!=groups.end(); iter++) {
    const std::vector<WALL_FINGERPRINT> &print = iter->first;
    if (!(noMiddle(print))) continue;  // THIS SHOULD BE INVESTIGATED FURTHER, images_20091028/image_00053.wall
    PolygonGroupData &data = iter->second;
    int num_subgroups = data.numSubGroups();
    for (int i = 0; i < num_subgroups; i++) {
      ZoneData zd = data.AnalyzeZone(print,i);
      zones.push_back(zd);
    }
  }

  PickInteriorZones(args,zones,tuned_enclosure_threshold);
}


void PolygonLabels::Print() const {

  (*ARGS->output) << "POLYGON LABELS, " << groups.size() << " different fingerprints" << std::endl;

  for (std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::const_iterator iter=groups.begin(); iter!=groups.end(); iter++) {
    const std::vector<WALL_FINGERPRINT> &print = iter->first;

    (*ARGS->output) << "   "; 

    printPrint(print);
    const PolygonGroupData &data = iter->second;
    data.Print();
  }
}

void PolygonLabels::PrintStats() const {
  int num_prints = groups.size();
  int num_subgroups = 0;
  int num_interiors = 0;
  int num_inferredwalls = 0;
  for (std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::const_iterator iter=groups.begin(); iter!=groups.end(); iter++) {
    const PolygonGroupData &data = iter->second;
    int n = data.numSubGroups();
    num_subgroups += n;
    for (int i = 0; i < n; i++) {
      if (data.IsInterior2(i)) num_interiors++;
      if (data.IsInferredWall(i)) num_inferredwalls++;
    } 
  }
  (*ARGS->output) << "POLYGON LABELS:" << std::endl;
  (*ARGS->output) << "  " << num_prints    << " different fingerprints" << std::endl;
  (*ARGS->output) << "  " << num_subgroups << " subgroups" << std::endl;
  (*ARGS->output) << "  " << num_interiors << " interiors" << std::endl;
  (*ARGS->output) << "  " << num_inferredwalls << " inferredwalls" << std::endl;
}

#define NUM_BUCKETS 10


void PolygonGroupData::AnalyzeEnclosureHistogram(int i, std::string &is_enclosed, double tuned_enclosure_threshold) {

  double average_enclosure;
  assert (i >= 0 && i < numSubGroups());
  is_enclosed = "";

  // go through all the polys and put all the samples in a vector
  std::vector<double> samples;
  const std::set<Poly*> &polys = get(i).polys;
  for (std::set<Poly*>::iterator itr = polys.begin(); itr != polys.end(); itr++) {
    for (unsigned int k = 0; k < (*itr)->enclosure_samples.size(); k++) {
      double e = (*itr)->enclosure_samples[k].second;
      if (e < 0) { (*ARGS->output) << "skip " << e << std::endl; continue; }
      assert (e >= 0 && e <= 1.0);
      samples.push_back(e);
    }
  }

  // in case no point samples were in this polygon group, use the
  // weighted average of the polygon centroid values
  if (samples.size() == 0) {
    (*ARGS->output) << std::endl;
    average_enclosure = getWeightedCentroidEnclosure(i);
    //if (average_enclosure > ENCLOSED_THRESHHOLD) {
    if (average_enclosure > tuned_enclosure_threshold) {// ENCLOSED_THRESHHOLD) {
      is_enclosed = "INTERIOR";
    } else {
      is_enclosed = "not";
    }
    return;
  }

  // compute average enclosure of this subgroup
  average_enclosure = 0;
  for (unsigned int k = 0; k < samples.size(); k++) {
    average_enclosure += samples[k];
  }
  average_enclosure /= double(samples.size());


    (*ARGS->output) << "avg=" << std::fixed << std::setprecision(2) << std::setw(3) << average_enclosure;


  // compute standard deviation of enclosure
  double standard_deviation = 0;
  for (unsigned int k = 0; k < samples.size(); k++) {
    standard_deviation += square(average_enclosure-samples[k]);
  }
  standard_deviation = sqrt(standard_deviation/double(samples.size()));


    (*ARGS->output) << "  stdev=" << std::fixed << std::setprecision(2) << std::setw(3) << standard_deviation;

  // make a histogram of the sample enclosure values
  std::vector<int> buckets(NUM_BUCKETS,0);
  double low = 0;
  double med = 0;
  double high = 0;

  double HIGH_THRESH = (tuned_enclosure_threshold + 1.0) / 2.0;
  double LOW_THRESH = (tuned_enclosure_threshold + 0.0) / 2.0;

  // go through all the samples in all the polys in this group
  for (unsigned int k = 0; k < samples.size(); k++) {
    if (samples[k] < LOW_THRESH) low++;  // LOWER_THRESHHOLD) low++;
    else if (samples[k] < HIGH_THRESH) med++; //ER_THRESHHOLD) med++;
    else high++;
    int w = int(floor(samples[k]*NUM_BUCKETS));
    if (w == NUM_BUCKETS) w = NUM_BUCKETS-1;
    assert (w >= 0 && w < NUM_BUCKETS);	
    buckets[w]++;
  }

  // OUTPUT THE HISTOGRAM
  for (int k = 0; k < NUM_BUCKETS; k++) {

      (*ARGS->output) << " " << std::setw(3) << buckets[k];

  }

    (*ARGS->output) << "   =   " << std::setw(4) << samples.size();

  assert (low + med + high == samples.size());

  low /= double(samples.size());
  med /= double(samples.size());
  high /= double(samples.size());

    (*ARGS->output) << "    " << std::fixed << std::setprecision(2) << std::setw(3) << low;
    (*ARGS->output) << "  " << std::fixed << std::setprecision(2) << std::setw(3) << med;
    (*ARGS->output) << "  " << std::fixed << std::setprecision(2) << std::setw(3) << high;

  if (average_enclosure > tuned_enclosure_threshold && 
      standard_deviation > 0.05 &&
      samples.size() > 50) {
    is_enclosed = "MIXED_INTERIOR";    
  } else if (high > 0.1 &&
	     standard_deviation > 0.05 &&
	     samples.size() > 400) {
    is_enclosed = "MIXED_INTERIOR";
  } else if (average_enclosure > tuned_enclosure_threshold) {
    is_enclosed = "INTERIOR";
  } else {
    is_enclosed = "not";
  }
  /*
  if (average_enclosure > tuned_enclosure_threshold && //ENCLOSED_THRESHHOLD && 
      standard_deviation > 0.05 &&
      samples.size() > 50) {
    // MIXED
    

  }
  */

  /*
  assert (is_enclosed == "");
  if (low < 0.1) is_enclosed = "INTERIOR";
  else if (high < 0.1) is_enclosed = "not";
  else { 
    // mixed histogram
    if (average_enclosure > tuned_enclosure_threshold) { //ENCLOSED_THRESHHOLD) {
      is_enclosed = "MIXED_INTERIOR";
    } else {
      is_enclosed = "MIXED_not";
    }
  }
  */

    (*ARGS->output) << "  " << is_enclosed << std::endl;
}



void PolygonGroupData::AverageEnclosure(int i, double &average_enclosure, double &area) {

  assert (i >= 0 && i < numSubGroups());

  area = 0;

  // go through all the polys and put all the samples in a vector
  std::vector<double> samples;
  const std::set<Poly*> &polys = get(i).polys;
  for (std::set<Poly*>::iterator itr = polys.begin(); itr != polys.end(); itr++) {
    area += (*itr)->Area();
    for (unsigned int k = 0; k < (*itr)->enclosure_samples.size(); k++) {
      double e = (*itr)->enclosure_samples[k].second;
      if (e < 0) {
	continue;
      }
      assert (e >= 0 && e <= 1.0);
      samples.push_back(e);
    }
  }

  // in case no point samples were in this polygon group, use the
  // weighted average of the polygon centroid values
  if (samples.size() == 0) {
    //  (*ARGS->output) << std::endl;
    average_enclosure = getWeightedCentroidEnclosure(i);
    //if (average_enclosure > ENCLOSED_THRESHHOLD) {
    //  is_enclosed = "INTERIOR";
    //} else {
    //  is_enclosed = "not";
    // }
    return;
  }

  // compute average enclosure
  average_enclosure = 0;
  for (unsigned int k = 0; k < samples.size(); k++) {
    average_enclosure += samples[k];
  }
  average_enclosure /= double(samples.size());

}



void PolygonGroupData::Print() const {
  int num_subgroups = numSubGroups();
  for (int i = 0; i < num_subgroups; i++) {
    int interior = IsInterior2(i);
    int inferred_wall = IsInferredWall(i);
    assert (!(interior && inferred_wall));
    (*ARGS->output) << "      " << i << " with " << get(i).polys.size() << " polygons"; 
    if (interior) 
      (*ARGS->output) << "     INTERIOR  ";
    else if (inferred_wall)
      (*ARGS->output) << "     WALL      ";
    else {
      (*ARGS->output) << "     exterior     ";
    }
    (*ARGS->output) << std::endl;
  }
}


void PolygonLabels::LabelAdditionalInteriors() {
  int added;
  do {
    added = 0;
    for (std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::iterator iter=groups.begin(); iter!=groups.end(); iter++) {
      const std::vector<WALL_FINGERPRINT> &print = iter->first;
      if (noMiddle(print)) continue;
      PolygonGroupData &data = iter->second;
      int num_subgroups = data.numSubGroups();
      for (int j = 0; j < num_subgroups; j++) {
	if (data.IsInterior2(j)) continue;
	assert (!data.IsInferredWall(j));
	int num_middles = NumMiddles(print);
	if (num_middles == 0) continue;
	for (unsigned int k = 0; k < print.size(); k++) {
	  if (print[k] == FINGERPRINT_MIDDLE) {
	    std::vector<WALL_FINGERPRINT> a(print);
	    std::vector<WALL_FINGERPRINT> b(print);
	    a[k] = FINGERPRINT_FRONT;
	    b[k] = FINGERPRINT_BACK;
	    bool a_present = false;
	    bool b_present = false;	
	    std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::iterator a_iter = groups.find(a);
	    std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::iterator b_iter = groups.find(b);
	    if (a_iter == groups.end() || b_iter == groups.end()) continue;
	    std::vector<int> a_subgroup = data.whichSubGroupsAreNeighbors(j,a_iter->second);
	    std::vector<int> b_subgroup = data.whichSubGroupsAreNeighbors(j,b_iter->second);
	    for (int q = 0; q < (int)a_subgroup.size(); q++) { if (IsInterior2(a,a_subgroup[q])) a_present = true; }
	    for (int q = 0; q < (int)b_subgroup.size(); q++) { if (IsInterior2(b,b_subgroup[q])) b_present = true; }
	    if (a_present && b_present) { 
	      data.SetInterior(j);
	      added++;
	    }
	  }
	}  
      }
    }
    //(*ARGS->output) << "ADDED " << added << " interior prints" << std::endl;
  } while (added > 0);
}


void PolygonLabels::LabelTinyUnusedWalls() {
  return;

  // this does the edges (1 num middle)
  for (std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::iterator iter=groups.begin(); iter!=groups.end(); iter++) {
    const std::vector<WALL_FINGERPRINT> &print = iter->first;
    PolygonGroupData &data = iter->second;
    int num_subgroups = data.numSubGroups();
    for (int s = 0; s < num_subgroups; s++) {
      if (data.IsInterior2(s)) continue;
      assert (!data.IsTinyUnusedWall(s));
      std::vector<WALL_FINGERPRINT> a,b;//,c,d;
      int tmp = print.size();
      for (int i = 0; i < tmp; i++) {
	if (print[i] != FINGERPRINT_MIDDLE) continue;
	a = print;
	b = print;
	a[i] = FINGERPRINT_FRONT;
	b[i] = FINGERPRINT_BACK;
	bool front_present = false;
	bool back_present = false;
	std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::iterator a_iter = groups.find(a);
	std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::iterator b_iter = groups.find(b);
	if (a_iter == groups.end() || b_iter == groups.end()) continue;
	std::vector<int> a_subgroup = data.whichSubGroupsAreNeighbors(s,a_iter->second);
	std::vector<int> b_subgroup = data.whichSubGroupsAreNeighbors(s,b_iter->second);
	for (int q = 0; q < (int)a_subgroup.size(); q++) { if (IsInterior2(a,a_subgroup[q])) front_present = true; }
	for (int q = 0; q < (int)b_subgroup.size(); q++) { if (IsInterior2(b,b_subgroup[q])) back_present = true; }
	if ((front_present && !back_present) || (!front_present && back_present)) { 
	  data.SetTinyUnusedWall(s); 
	  continue; 
	}
      }      
    }
  }
}




void PolygonLabels::LabelInferredWalls() {
  // this does the edges (1 num middle)
  for (std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::iterator iter=groups.begin(); iter!=groups.end(); iter++) {
    const std::vector<WALL_FINGERPRINT> &print = iter->first;
    PolygonGroupData &data = iter->second;
    int num_subgroups = data.numSubGroups();
    for (int s = 0; s < num_subgroups; s++) {
      if (data.IsInterior2(s)) continue;
      assert (!data.IsInferredWall(s));
      std::vector<WALL_FINGERPRINT> a,b;//,c,d;
      int tmp = print.size();
      for (int i = 0; i < tmp; i++) {
	if (print[i] != FINGERPRINT_MIDDLE) continue;
	a = print;
	b = print;
	a[i] = FINGERPRINT_FRONT;
	b[i] = FINGERPRINT_BACK;
	bool front_present = false;
	bool back_present = false;
	std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::iterator a_iter = groups.find(a);
	std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::iterator b_iter = groups.find(b);
	if (a_iter == groups.end() || b_iter == groups.end()) continue;
	std::vector<int> a_subgroup = data.whichSubGroupsAreNeighbors(s,a_iter->second);
	std::vector<int> b_subgroup = data.whichSubGroupsAreNeighbors(s,b_iter->second);
	for (int q = 0; q < (int)a_subgroup.size(); q++) { if (IsInterior2(a,a_subgroup[q])) front_present = true; }
	for (int q = 0; q < (int)b_subgroup.size(); q++) { if (IsInterior2(b,b_subgroup[q])) back_present = true; }
	if ((front_present && !back_present) || (!front_present && back_present)) { 
	  data.SetInferredWall(s); 
	  continue; 
	}
      }      
    }
  }
  LabelInferredWallsHelper();
}


void PolygonLabels::InsertPolygon(Poly *p) {
  assert (p != NULL); 
  std::pair<std::vector<WALL_FINGERPRINT>,PolygonGroupData> pr = std::make_pair(p->print_X,PolygonGroupData());
  std::pair<std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::iterator,bool> tmp = groups.insert(pr);
  tmp.first->second.AddPolygon(p);
}

void PolygonGroupData::AddPolygon(Poly *p) {
  assert (p != NULL);
  std::pair<std::set<Poly*>::iterator,bool> tmp = allpolys.insert(p);
  assert (tmp.second == true);
}


#define APPROXIMATE_DOOR_WIDTH (2.5*INCH_IN_METERS)
#define APPROXIMATE_DOOR_AREA (0.5*INCH_IN_METERS * 2.5*INCH_IN_METERS)

double PolygonGroupData::getMaxDistance(int i) const {
  const std::set<Poly*> &polys = get(i).polys;
  std::vector<Vec3f> verts;
  for (std::set<Poly*>::const_iterator iter = polys.begin(); iter != polys.end(); iter++) {
    int num_verts = (*iter)->numVertices();
    Mesh *m = (*iter)->getMesh();
    for (int i = 0; i < num_verts; i++) {
      verts.push_back(m->getVertex((*(*iter))[i])->get());
    }
  }
  //int num_polys = polys.size();
  int num_verts = verts.size();
  //(*ARGS->output) << num_polys << " have " << num_verts << " vertices " << std::endl;
  assert (num_verts >= 3);
  double answer = -1;
  for (int i = 0; i < num_verts; i++) {
    for (int j = 0; j < num_verts; j++) {
      double d = DistanceBetweenTwoPoints(verts[i],verts[j]);
      if (answer < d)
	answer = d;
    }
  }
  return answer;
}

void PolygonLabels::SearchForInteriorInferredWalls() {
  int added;
  do {
    added = 0;
    for (std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::iterator iter=groups.begin(); iter!=groups.end(); iter++) {
      const std::vector<WALL_FINGERPRINT> &print = iter->first;
      int num_middles = NumMiddles(print);
      if (num_middles == 0) continue;
      PolygonGroupData &data = iter->second;
      int num_subgroups = data.numSubGroups();
      for (int s = 0; s < num_subgroups; s++) {
	//	if (!data.IsInterior(s)) continue;   // THIS LINE SHOULD BE INVESTIGATED MORE, SITUATIONS WHERE WE SHOULD CHOP OFF REAL WALLS FROM INTENDED DIAGRAM   
	if (data.IsInferredWall(s)) continue;      
	if (data.IsRealWall(s)) { data.SetInferredWall(s); added++; continue; }

	// check to see if it touches an inferred wall...
	std::vector<int> neighbors = data.whichSubGroupsAreNeighbors(s, data);
	bool touches_wall = false;
	for (int i = 0; i < (int)neighbors.size(); i++) {
	  if (data.IsInferredWall(neighbors[i])) touches_wall = true; }
	if (touches_wall == true) {
	  double area = data.getArea(s);
	  double max_distance = data.getMaxDistance(s);
	  if (area < APPROXIMATE_DOOR_AREA && max_distance < APPROXIMATE_DOOR_WIDTH) { 
	    data.SetInferredWall(s); 
	    added++; 
	  }
	}
      }
    }
    added += LabelInferredWallsHelper();
  } while (added > 0);
}


int PolygonLabels::LabelInferredWallsHelper() {
  int answer = 0;
  // this does the joins between 2 wall chains! (2 num middles)
  for (std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::iterator iter=groups.begin(); iter!=groups.end(); iter++) {
    const std::vector<WALL_FINGERPRINT> &print = iter->first;
    PolygonGroupData &data = iter->second;
    std::vector<WALL_FINGERPRINT> a,b,c,d;
    int tmp = print.size();
    if (NumMiddles(print) < 2) continue;
    for (int i = 0; i < tmp; i++) {
      if (print[i] != FINGERPRINT_MIDDLE) continue;
      for (int j = i+1; j < tmp; j++) {
	if (print[j] != FINGERPRINT_MIDDLE) continue;
	a = print;
	b = print;
	c = print;
	d = print;
	a[i] = FINGERPRINT_FRONT;
	b[i] = FINGERPRINT_BACK;
	c[j] = FINGERPRINT_FRONT;
	d[j] = FINGERPRINT_BACK;
	bool front_present = false;
	bool back_present = false;
	bool front_present2 = false;
	bool back_present2 = false;
	int num_subgroups = data.numSubGroups();
	for (int s = 0; s < num_subgroups; s++) {
	  //	  if (!data.IsInterior(s)) continue;
	  if (data.IsInferredWall(s)) continue;
	  //assert (!data.IsInferredWall(s));
	  std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::iterator a_iter = groups.find(a);
	  std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::iterator b_iter = groups.find(b);
	  if (a_iter == groups.end() || b_iter == groups.end()) continue;
	  std::vector<int> a_subgroup = data.whichSubGroupsAreNeighbors(s,a_iter->second);
	  std::vector<int> b_subgroup = data.whichSubGroupsAreNeighbors(s,b_iter->second);
	  for (int q = 0; q < (int)a_subgroup.size(); q++) { if (IsInferredWall(a,a_subgroup[q])) front_present = true; }
	  for (int q = 0; q < (int)b_subgroup.size(); q++) { if (IsInferredWall(b,b_subgroup[q])) back_present = true; }
	  std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::iterator c_iter = groups.find(c);
	  std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::iterator d_iter = groups.find(d);
	  if (c_iter == groups.end() || d_iter == groups.end()) continue;
	  std::vector<int> c_subgroup = data.whichSubGroupsAreNeighbors(s,c_iter->second);
	  std::vector<int> d_subgroup = data.whichSubGroupsAreNeighbors(s,d_iter->second);
	  for (int q = 0; q < (int)c_subgroup.size(); q++) { if (IsInferredWall(c,c_subgroup[q])) front_present2 = true; }
	  for (int q = 0; q < (int)d_subgroup.size(); q++) { if (IsInferredWall(d,d_subgroup[q])) back_present2 = true; }
	}
	int test = front_present + back_present + front_present2 + back_present2;
	if (test >= 2) {
	  for (int s = 0; s < num_subgroups; s++) {
	    data.SetInferredWall(s);
	    answer++;
	  }
	}
      }      
    }
  }
  return answer;
}








void FloodFill(Poly *p, SubGroupData &data, int group_id) {
  assert (p->isMarked() == false);
  p->Mark();
  p->which_subgroup = group_id;
  std::set<Poly*>::iterator itr = data.polys.find(p);
  assert (itr == data.polys.end());
  std::pair<std::set<Poly*>::iterator,bool> tmp = data.polys.insert(p);
  assert (tmp.second == true);
  double area = p->Area();
  double enclosed = p->getPercentEnclosed();
  data.area_weighted_enclosure += area*enclosed; 
  data.area_sum += area; 
  if (isWallOrWindow(p->getCellType())) {
    data.wall_area_sum += area;
  }
  std::vector<WALL_FINGERPRINT> &print = p->print_X;
  bool wow = isWallOrWindow(p->getCellType());
  int num_verts = p->numVertices();
  for (int i = 0; i < num_verts; i++) {
    std::vector<Element*> elements = p->getNeighbors(i);
    if (elements.empty()) continue;
    assert (elements.size() == 1);
    Poly *p2 = (Poly*)elements[0];
    if (p2->isMarked()) continue;
    std::vector<WALL_FINGERPRINT> &print2 = p2->print_X;
    bool wow2 = isWallOrWindow(p2->getCellType());
    if (!samePrint(print,print2)) continue;
    if (wow != wow2) continue;
    FloodFill(p2,data,group_id);
  }
}

void PolygonGroupData::ConnectedGroupAnalysis() {
  subgroups.clear();
  int num_polys = allpolys.size();
  assert (num_polys > 0);
  Markable::NextMark();
  int num_subgroups = 0;
  for (std::set<Poly*>::iterator iter = allpolys.begin(); iter != allpolys.end(); iter++) {
    if (!(*iter)->isMarked()) {
      subgroups.push_back(SubGroupData());
      FloodFill(*iter,subgroups[num_subgroups],num_subgroups);
      num_subgroups++;
    }
  }
  // some checking
  assert (num_subgroups == int(subgroups.size()));
  if (num_subgroups != 1) {
    int sum = 0;
    for (int i = 0; i < num_subgroups; i++) {
      //(*ARGS->output) << "  MARKED " << sub_groups[i].size() << std::endl;
      sum += get(i).polys.size();
    }
    assert (sum == num_polys);
  } else {
    assert (int(get(0).polys.size()) == num_polys);
  }
}



bool PolygonLabels::IsInterior2(const std::vector<WALL_FINGERPRINT> &print, Poly *p) const { 
  std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::const_iterator iter = groups.find(print);
  if (iter == groups.end()) return false;
  return iter->second.IsInterior2(p); 
}

bool PolygonLabels::IsInterior2(const std::vector<WALL_FINGERPRINT> &print, int i) const { 
  std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::const_iterator iter = groups.find(print);
  if (iter == groups.end()) return false;
  return iter->second.IsInterior2(i); 
}

bool PolygonLabels::IsMixed(const std::vector<WALL_FINGERPRINT> &print, Poly *p) const { 
  std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::const_iterator iter = groups.find(print);
  if (iter == groups.end()) return false;
  return iter->second.IsMixed(p); 
}

bool PolygonLabels::IsInferredWall(const std::vector<WALL_FINGERPRINT> &print, Poly *p) const { 
  std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::const_iterator iter = groups.find(print);
  if (iter == groups.end()) return false;
  return iter->second.IsInferredWall(p); 
}

bool PolygonLabels::IsInferredWall(const std::vector<WALL_FINGERPRINT> &print, int i) const { 
  std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::const_iterator iter = groups.find(print);
  if (iter == groups.end()) return false;
  return iter->second.IsInferredWall(i); 
}

bool PolygonLabels::IsTinyUnusedWall(const std::vector<WALL_FINGERPRINT> &print, Poly *p) const { 
  std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::const_iterator iter = groups.find(print);
  if (iter == groups.end()) return false;
  return iter->second.IsTinyUnusedWall(p); 
}

bool PolygonLabels::IsTinyUnusedWall(const std::vector<WALL_FINGERPRINT> &print, int i) const { 
  std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::const_iterator iter = groups.find(print);
  if (iter == groups.end()) return false;
  return iter->second.IsTinyUnusedWall(i); 
}

void PolygonLabels::SetInterior(const std::vector<WALL_FINGERPRINT> &print, int i) {
  std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::iterator iter = groups.find(print);
  assert (iter != groups.end());
  iter->second.SetInterior(i); 
}

void PolygonLabels::SetMixed(const std::vector<WALL_FINGERPRINT> &print, int i) {
  std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::iterator iter = groups.find(print);
  assert (iter != groups.end());
  iter->second.SetMixed(i); 
}

void PolygonLabels::SetInferredWall(const std::vector<WALL_FINGERPRINT> &print, int i) {
  std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::iterator iter = groups.find(print);
  assert (iter != groups.end());
  iter->second.SetInferredWall(i); 
}

void PolygonLabels::SetTinyUnusedWall(const std::vector<WALL_FINGERPRINT> &print, int i) {
  std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::iterator iter = groups.find(print);
  assert (iter != groups.end());
  iter->second.SetTinyUnusedWall(i); 
}


std::vector<int> PolygonGroupData::whichSubGroupsAreNeighbors(int sub, const PolygonGroupData &data2) const {
  assert (sub >= 0 && sub < numSubGroups());
  std::vector<int> answer;
  int num_subgroups = data2.numSubGroups();
  for (int j = 0; j < num_subgroups; j++) {
    if (this == &data2 && j == sub) continue;
    for (std::set<Poly*>::iterator iter = get(sub).polys.begin(); iter != get(sub).polys.end(); iter++) {
      Poly *p = *iter;
      assert (p != NULL);
      int num_verts = p->numVertices();
      for (int i = 0; i < num_verts; i++) {
	const std::vector<Element*> &neighbors = p->getNeighbors(i);
	if (neighbors.empty()) continue;
	assert (neighbors.size() == 1);
	Poly *p2 = (Poly*)neighbors[0];
	assert (p2 != NULL);
	if (data2.get(j).polys.find(p2) != data2.get(j).polys.end()) {
	  if (answer.size() == 0 || answer.back() != j) {
	    answer.push_back(j);
	  }
	}
      }
    }
  }
  return answer;
}
