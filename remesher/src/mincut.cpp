#include <algorithm>
#include <iostream>
#include <vector>
#include <cassert>

#include "argparser.h"
extern ArgParser *ARGS;

// ORIGINAL CODE FROM WIKIPEDIA, http://en.wikipedia.org/wiki/Maximum_flow_problem
//Edmonds-Karp
//return the largest flow;flow[] will record every edge's flow
//n, the number of nodes in the graph;cap, the capacity 
//O(VE^2) 
/*
#define N 100
#define inf 0x3f3f3f3f
int Edmonds_Karp(int n,int cap[N][N],int source,int sink,int flow[N][N]){
	int pre[N],que[N],d[N],p,q,t,i,j;
	if (source==sink) return inf;
	memset(flow,0,sizeof(flow));
	while (true){
	        memset(pre,-1,sizeof(pre));
		d[source]=inf;p=q=0,que[q++]=source;
		while(p<q&&pre[sink]<0){
			t=que[p++];
			for (i=0;i<n;i++)
				   if (pre[i]<0&&(j=cap[t][i]-flow[t][i]))
				     pre[que[q++]=i]=t,d[i]=std::min(d[t],j);
		}
		if (pre[sink]<0) break;
		for (i=sink;i!=source;i=pre[i])
				flow[pre[i]][i]+=d[sink],flow[i][pre[i]]-=d[sink];
	}
	for (j=i=0;i<n;j+=flow[source][i++]);
        return j;
}
*/

#define inf 0x3f3f3f3f
int Edmonds_Karp(int n,const std::vector<std::vector<int> > &cap,
		 int source,int sink,std::vector<std::vector<int> > &flow) {

  //(*ARGS->output) << "IN EK " << n << std::endl;

  // some simple args check
  assert (n == (int)cap.size());
  for (int i = 0; i < n; i++) {
    assert (n == (int)cap[i].size());
  }
  assert (source != sink);
  if (source==sink) return inf;

  // I'm not sure why these need to be +1, but otherwise valgrind says bad write
  std::vector<int> path(n+1);
  std::vector<int> usage(n+1);
  int t,i;

  // construct a matrix and initialize the flow as zero
  flow = std::vector<std::vector<int> >(n,std::vector<int>(n,0));

  while (true) {
    
    // for each node in the augmenting path, store the previous node
    std::vector<int> prev_links(n,-1);

    // the amount of flow available at each node (source starts out at infinity)
    usage[source] = inf;

    int p = 0;
    // the path starts at the source
    path[0]=source;
    int q = 1;

    // search for an augmenting path
    while (p < q && (prev_links[sink] < 0)){

      t=path[p++];

      for (i=0;i<n;i++) {
	if ((prev_links)[i] != -1) continue;
	int avail = cap[t][i]-flow[t][i];
	assert (avail >= 0);
	if (avail <= 0) continue;
	//(*ARGS->output) << "q = " << q << std::endl;
	path[q] = i;
	q++;
	prev_links[i] = t;
	usage[i] = std::min(usage[t],avail);
      }
    }

    // didn't find a path, quit
    if (prev_links[sink] < 0) break;

    // walk the path backwards, incrementing the flow
    for (int i = sink; i != source; i = prev_links[i]) {
      flow[prev_links[i]][i] += usage[sink];
      flow[i][prev_links[i]] -= usage[sink];
    }
  }

  // sum up the total flow leaving the source (the max flow)
  int answer = 0;
  for (int i = 0; i < n; i++) {
    answer += flow[source][i];
  }
  return answer;
}


std::vector<int> find_min_cut(int n, const std::vector<std::vector<int> > &cap, 
			      const std::vector<std::vector<int> > &flow, int max_flow,
			      int source, int sink) {

    (*ARGS->output) << "max_flow = " << max_flow << std::endl;

  std::vector<std::pair<int,int> > cut_edges;

  std::vector<int> answer(n,-1);

  // start at the source
  answer[source] = 1;
  while(1) {
    int added = 0;
    for (int i = 0; i < n; i++) {
      if (answer[i] == -1) continue;
      for (int j = 0; j < n; j++) {
	if (answer[j] != -1) continue;
	if (cap[i][j] == 0) continue;
	if (flow[i][j] == cap[i][j]) continue;
	assert (flow[i][j] < cap[i][j]);
	answer[j] = 1;
	added++;
      }
    }
    if (added == 0) break;
  }
  assert (answer[sink] == -1);

  int count = 0;
  for (int i = 0; i < n; i++) {
    if (answer[i] == -1) count++;
    else assert (answer[i] == 1);
  }

    (*ARGS->output) << "counts are " << count << " " << n-count << std::endl;

  return answer;
}
