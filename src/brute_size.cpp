#include "precompiled_stl.hpp"
using namespace std;
#include "utils.hpp"
#include "core_functions.hpp"
#include "image_functions.hpp"
#include "image_functions2.hpp"
#include "visu.hpp"

#include "brute2.hpp"
#include "pieces.hpp"
#include "compose2.hpp"

extern int MAXDEPTH;

bool operator<(const point a, const point b) {
  if (a.x != b.x) return a.x < b.x;
  return a.y < b.y;
}

pair<vector<int>,double> solveSingle(vector<vector<int>>&seeds, const vector<int>& target) {
  int n = target.size()+1;
  vector<int> ans(n,1);
  pair<int,double> best = {-1,1e9};

  auto add = [&](const vector<int>&szs, double loss) {
    int oks = 0;
    for (int ti = 0; ti < target.size(); ti++)
      oks += (szs[ti] == target[ti]);
    pair<int,double> cand = {oks, -loss-10};
    int sz = szs.back();
    if (sz >= 1 && sz <= 30 &&
	cand > best) {
      best = cand;
      ans = szs;
    }
  };

  for (int w = 1; w <= 30; w++) {
    add(vector<int>(n,w), w);
  }

  vector<int> szs(n);
  for (int i = 0; i < seeds.size(); i++) {
    double a = i+1;
    for (int w = 1; w < 6; w++) {
      for (int x = -3; x <= 3; x++) {
	for (int k = 0; k < n; k++)
	  szs[k] = seeds[i][k]*w+x;
	add(szs, a*w*(abs(x)+1));
      }
    }
  }

  return {ans,best.second};
}

point solveSize(vector<vector<point>>&seeds, const vector<point>& target) {
  point ans = {1,1};
  pair<int,double> best = {-1,1e9};

  auto add = [&](const vector<point>&szs, double loss) {
    int oks = 0;
    for (int ti = 0; ti < target.size(); ti++)
      oks += (szs[ti] == target[ti]);
    pair<int,double> cand = {oks, -loss};
    point sz = szs.back();
    if (sz.x >= 1 && sz.x <= 30 &&
	sz.y >= 1 && sz.y <= 30 &&
	cand > best) {
      best = cand;
      ans = sz;
    }
  };

  int n = target.size()+1;
  vector<point> szs(n);

  for (int i = 0; i < seeds.size(); i++) {
    double a = i+1;
    for (int h = 1; h < 6; h++) {
      for (int w = 1; w < 6; w++) {
	for (int y = -3; y <= 3; y++) {
	  for (int x = -3; x <= 3; x++) {
	    for (int k = 0; k < n; k++)
	      szs[k] = {seeds[i][k].x*w+x, seeds[i][k].y*h+y};
	    add(szs, a*w*h*(abs(x)+1)*(abs(y)+1));
	  }
	}
      }
    }
  }

  if (1) {//best.first < target.size()) {
    for (int i = 0; i < seeds.size(); i++) {
      double a = i+1;
      for (int j = 0; j < i; j++) {
	double b = j+1;
	for (int d = 0; d < 3; d++) {
	  for (int k = 0; k < n; k++) {
	    szs[k] = seeds[i][k];
	    if (d == 0 || d == 2) szs[k].x = szs[k].x+seeds[j][k].x;
	    if (d == 1 || d == 2) szs[k].y = szs[k].y+seeds[j][k].y;
	  }
	  add(szs, a*b);
	}
      }
    }
  }

  {
    vector<vector<int>> seedsx, seedsy;
    for (vector<point>&seed : seeds) {
      vector<int> seedx, seedy;
      for (point p : seed) {
	seedx.push_back(p.x);
	seedy.push_back(p.y);
      }
      seedsx.push_back(seedx);
      seedsy.push_back(seedy);
    }
    vector<int> targetx, targety;
    for (point p : target) {
      targetx.push_back(p.x);
      targety.push_back(p.y);
    }
    auto [bestx, scorex] = solveSingle(seedsx, targetx);
    auto [besty, scorey] = solveSingle(seedsy, targety);

    vector<point> combined;
    for (int i = 0; i < n; i++) {
      combined.push_back({bestx[i], besty[i]});
    }
    double combloss = scorex*scorey;
    add(combined, combloss);
  }

  //cout << best.first << ' ' << target.size() << ' ' << best.second << endl;
  //assert(best.first == target.size());
  return ans;
}

vector<point> bruteSize(Image_ test_in, vector<pair<Image,Image>> train) {
  vector<point> out_sizes;
  for (auto [in,out] : train) {
    out_sizes.push_back(out.sz);
  }
  int cp = MAXDEPTH;
  MAXDEPTH = min(MAXDEPTH, 30);
  Pieces pieces;
  {
    vector<DAG> dags = brutePieces2(test_in, train, {});
    pieces = makePieces2(dags, train, {});
  }
  int dags = pieces.dag.size();

  MAXDEPTH = cp;

  vector<point> target;
  for (auto [in,out] : train) target.push_back(out.sz);

  //cout << pieces.piece.size() << endl;
  //cout << pieces.seen.size() << endl;
  vector<vector<point>> seeds;
  set<vector<point>> seen;
  for (Piece3&p : pieces.piece) {
    vector<point> sz;
    int ok = 1;
    int*ind = &pieces.mem[p.memi];
    for (int ti = 0; ti <= train.size(); ti++) {
      if (pieces.dag[ti].tiny_node[ind[ti]].isvec) ok = 0;
      else {
	sz.push_back(pieces.dag[ti].getImg(ind[ti]).sz);
      }
    }
    if (ok) {
      if (seen.insert(sz).second)
	seeds.push_back(sz);
    }
  }

  point ans = solveSize(seeds, target);
  out_sizes.push_back(ans);
  return out_sizes;
}

vector<point> cheatSize(Image_ test_out, vector<pair<Image,Image>> train) {
  vector<point> out_sizes;
  for (auto [in,out] : train) {
    out_sizes.push_back(out.sz);
  }
  out_sizes.push_back(test_out.sz);
  return out_sizes;
}
