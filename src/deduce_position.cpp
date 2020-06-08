#include "precompiled_stl.hpp"
using namespace std;
#include "utils.hpp"
#include "core_functions.hpp"
#include "image_functions.hpp"

#include "visu.hpp"
#include "brute2.hpp"
#include "deduce_position.hpp"

void deducePositions(DAG&dag, Image_ target) {
  int added = 0;

  int start_sz = dag.tiny_node.size();
  for (int ni = 0; ni < start_sz; ni++) {
    if (dag.tiny_node[ni].isvec) continue;
    Image_ base = dag.getImg(ni);
    Image cp = base;

    const int MOVEW = 2;

    int ok[5][5] = {};
    int total = core::count(cp);
    for (int dy = -MOVEW; dy <= MOVEW; dy++) {
      for (int dx = -MOVEW; dx <= MOVEW; dx++) {
	cp.x = base.x+dx;
	cp.y = base.y+dy;
	int sx = max(cp.x,0), sy = max(cp.y,0);
	int ex = min(cp.x+cp.w,target.w), ey = min(cp.y+cp.h,target.h);
	if (sx >= ex || sy >= ey) continue;
	int cnt = 0;
	for (int i = sy; i < ey; i++) {
	  for (int j = sx; j < ex; j++) {
	    char c = cp(i-cp.y,j-cp.x);
	    if (c) {
	      if (target(i,j) != c) goto fail;
	      cnt++;
	    }
	  }
	}
	if (cnt*2 > total && cp.sz == target.sz) {
	  ok[dy+MOVEW][dx+MOVEW] = 1;
	}
      fail:;
      }
    }

    for (int dy = -MOVEW; dy <= MOVEW; dy++) {
      for (int dx = -MOVEW; dx <= MOVEW; dx++) {
	for (int d = 0; d < 4; d++) {
	  int nx = dx+(d==0)-(d==1);
	  int ny = dy+(d==2)-(d==3);
	  if (nx >=-MOVEW && ny >=-MOVEW &&
	      nx <= MOVEW && ny <= MOVEW &&
	      ok[ny+MOVEW][nx+MOVEW]) {
	    ok[dy+MOVEW][dx+MOVEW] = 0;
	  }
	}
	if ((dx || dy) && ok[dy+MOVEW][dx+MOVEW]) {
	  added++;
	  /*if (core::colMask(cp) == (1<<2 | 1) && core::count(cp) >= 9) {
	    cout << di << ": " << dx << ' ' << dy << endl;
	    //print(cp);
	    }*/
	  int fi = dag.funcs.findfi("Move "+to_string(dx)+" "+to_string(dy));
	  int sz = dag.tiny_node.size();
	  if (dag.applyFunc(ni, fi) == sz) {
	    dag.applyFunc(sz, dag.funcs.findfi("embed 1"));
	  }
	  continue;
	}
      }
    }
  }
  //cout << "Added through Move: " << added << endl;
}
