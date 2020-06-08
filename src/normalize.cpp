#include "precompiled_stl.hpp"
/*#include <cassert>
#include <functional>
#include <vector>
#include <iostream>
#include <map>
#include <cmath>*/
using namespace std;

#include "utils.hpp"
#include "core_functions.hpp"
#include "image_functions.hpp"
#include "visu.hpp"
#include "read.hpp"
#include "normalize.hpp"

vector<double> shapeFeatures(Image_ img, int col) {
  double fill_cnt = core::count(img);
  double comp_cnt = core::countComponents(img);
  double int_cnt = core::count(interior(img));
  vector<double> r =  {fill_cnt, 1./(fill_cnt+1e-3),
		       comp_cnt, 1./(comp_cnt+1e-3),
		       int_cnt, 1./(int_cnt+1e-3)};
  for (int c = 0; c < 10; c++)
    r.push_back((c == col)+2);

  point center2 = img.p*2+img.sz-point{1,1};
  for (int a = 0; a < 2; a++) {
    for (int b = 0; b < 2; b++) {
      for (int c = 0; c < 2; c++) {
	point dir;
	if (c) dir = point{a*2-1, b*2-1}*2;
	else dir = point{(a*2-1)*b, (a*2-1)*!b}*3;
	int ma = -50;
	for (int i = 0; i < img.h; i++)
	  for (int j = 0; j < img.w; j++)
	    if (img(i,j)) ma = max(ma, (point{i*2,j*2}-center2)*dir);
	r.push_back(ma+1000);
      }
    }
  }
  return r;
}

struct UniquePicker {
  vector<int> feature_dim;
  int save;
  UniquePicker(const vector<Image>&ins, int save_) {
    save = save_ | 1;

    int nfeats = -1;
    vector<vector<vector<double>>> feat; // inp, col, feat
    for (Image_ in : ins) {
      vector<vector<double>> features;
      vector<pair<Image, int>> split = core::splitCols(in);
      for (auto&[sh,col] : split) {
	features.push_back(shapeFeatures(sh,col));
	nfeats = features.back().size();
      }
      feat.push_back(features);
    }

    int nins = ins.size();
    vector<vector<int>> done(nins); // inp, cole
    vector<int> cols_left(nins);
    for (int i = 0; i < nins; i++) {
      done[i].assign(feat[i].size(), 0);
      cols_left[i] = feat[i].size();
    }

    if (nfeats == -1) return;

    while (1) {
      vector<double> score(nfeats, 1e3); //feat
      vector<vector<int>> picki(nins); //inp, feat -> col

      int found = 0;
      for (int inpi = 0; inpi < nins; inpi++) {
	if (!cols_left[inpi]) continue;
	found = 1;

	auto&f = feat[inpi];
	int cols = f.size();
	for (int fi = 0; fi < nfeats; fi++) {
	  pair<double,int> best = {-1,-1};
	  for (int i = 0; i < cols; i++) {
	    if (done[inpi][i]) continue;
	    double worst = 1e3;
	    for (int j = 0; j < cols; j++) {
	      if (done[inpi][j] || i == j) continue;
	      worst = min(worst, diff(f[i][fi], f[j][fi]));
	    }
	    best = max(best, make_pair(worst, i));
	  }
	  assert(best.second != -1);
	  score[fi] = min(score[fi], best.first);
	  picki[inpi].push_back(best.second);
	}
      }
      if (!found) break;

      pair<double,int> best = {-1,-1};
      for (int fi = 0; fi < nfeats; fi++) {
	best = max(best, make_pair(score[fi], fi));
	//cout << score[fi] << '-' << fi << "  ";
      }
      //cout << endl;
      //cout << best.first << '-' << best.second << endl;
      int pickf = best.second;
      assert(pickf != -1);
      feature_dim.push_back(pickf);
      for (int inpi = 0; inpi < nins; inpi++) {
	if (!cols_left[inpi]) continue;
	auto&col = picki[inpi];
	done[inpi][col[pickf]] = 1;
	cols_left[inpi]--;
      }
    }

    //for (int i : feature_dim) cout << i << endl;
    //cout << endl;
    //exit(0);
  }

  double diff(double a, double b) const {
    return a/(b+1e-5);
  }

  int getUnique(const vector<vector<double>>&features, const vector<int>&done, int fi) const {
    pair<double,int> best = {-1,-1};
    int n = features.size();
    for (int i = 0; i < n; i++) {
      if (done[i]) continue;
      double score = 0;
      for (int j = 0; j < features[i].size(); j++) {
	if (fi != -1) j = fi;
	double fscore = 1e3;
	for (int k = 0; k < n; k++) {
	  if (k == i || done[k]) continue;
	  fscore = min(fscore, diff(features[i][j], features[k][j]));
	}
	score = max(score, fscore);
	if (fi != -1) break;
      }
      best = max(best, make_pair(score, i));
      //cout << score << ' ';
    }
    //cout << endl;
    //exit(0);
    assert(best.first != -1);
    return best.second;
  }

  void getMap(Image_ in, int cols[10]) const {
    int j = 0, done = 0;
    for (int i = 0; i < 10; i++)
      if (save>>i&1) {
	done |= 1<<i;
	cols[i] = j++;
      }

    vector<vector<double>> features;
    vector<pair<Image, int>> split = core::splitCols(in);
    vector<int> splitcol;
    for (auto&[sh,col] : split) {
      if (save>>col&1) continue;
      features.push_back(shapeFeatures(sh, col));
      splitcol.push_back(col);
    }

    vector<int> order, fdone(features.size());
    for (int it = 0; it < features.size(); it++) {
      int fi = it < feature_dim.size() ? feature_dim[it] : -1;
      int i = getUnique(features, fdone, fi);
      fdone[i] = 1;
      order.push_back(i);
    }
    for (int&i : order) i = splitcol[i];

    for (int i : order) {
      if (done>>i&1) continue;
      cols[i] = j++;
      done |= 1<<i;
    }
    for (int i = 0; i < 10; i++) {
      if ((done>>i&1) == 0) {
	cols[i] = j++;
	done |= 1<<i;
      }
    }
    assert(done == (1<<10)-1);
  }
};






Image remapCols(Image_ img, int cols[10]) {
  Image r = img;
  for (int i = 0; i < r.h; i++)
    for (int j = 0; j < r.w; j++)
      r(i,j) = cols[img(i,j)];
  return r;
}



void remapCols(const vector<pair<Image,Image>>&train, vector<Simplifier>&sims) {
  int orin = 0, orout = 0, andin = ~0, andout = ~0;
  for (auto [in,out] : train) {
    int maskin = core::colMask(in);
    int maskout = core::colMask(out);
    orin |= maskin;
    andin &= maskin;
    orout |= maskout;
    andout &= maskout;
  }

  int save = andout & ~orin;
  if (orout == andout) save = andout;
  save |= andin&~orout;

  vector<Image> train_ins;
  for (auto&[in,out] : train) train_ins.push_back(in);
  UniquePicker up(train_ins, save | (orout&~orin));

  Simplifier ret;

  ret.in = [up](Image_ in) {
    int cols[10];
    up.getMap(in, cols);
    return remapCols(in, cols);
  };
  ret.out = [up](Image_ in, Image_ out) {
    int cols[10];
    up.getMap(in, cols);
    return remapCols(out, cols);
  };
  ret.rec = [up](Image_ in, Image_ out) {
    int cols[10];
    up.getMap(in, cols);
    int icols[10];
    for (int i = 0; i < 10; i++)
      icols[cols[i]] = i;
    return remapCols(out, icols);
  };

  sims.push_back(ret);
}


Image listCols(Image_ img, int extra) {
  int mask = core::colMask(img);
  int w = __builtin_popcount(mask);
  Image ret = core::full({w,2}, 9);
  int j = 0;
  for (int i = 0; i < 10; i++)
    if ((mask>>i&1) && !(extra>>i&1))
      ret(0,j++) = i;
  j = 0;
  for (int i = 0; i < 10; i++)
    if (extra>>i&1)
      ret(1,j++) = i;
  return ret;
}



void normalizeCols(vector<Sample>&sample) {
  Visu visu;

  int count = 0;
  for (Sample&s : sample) {
    auto train = s.train;
    vector<Simplifier> sims;
    remapCols(train, sims);
    if (sims.size()) {
      Simplifier&sim = sims[0];
      for (auto&[in,out] : train) {
	Image cp = out;
	out = sim.out(in, out);
	assert(cp == sim.rec(in, out));
	in = sim.in(in);
      }

      count++;
      visu.next(s.id);
      for (auto [in,out] : s.train)
	visu.add(in,out);
      visu.next(s.id);
      for (auto [in,out] : train) {
	visu.add(in,out);
      }
    }
    continue;
    vector<pair<int,int>> masks;
    int orin = 0, orout = 0, andin = ~0, andout = ~0;
    for (auto [in,out] : s.train) {
      masks.emplace_back(core::colMask(in), core::colMask(out));
      orin |= masks.back().first;
      andin &= masks.back().first;
      orout |= masks.back().second;
      andout &= masks.back().second;
    }
    int eq = core::countCols(s.train[0].first);

    int meaningCols = checkAll(s.train, [&](pair<Image,Image> p) {
	return core::countCols(p.first) == eq;});
    //if (__builtin_popcount(eq&~1) != 1) continue;

    if (meaningCols) {
      count++;
      visu.next(s.id);
      for (auto [in,out] : s.train)
	visu.add(in,out);
      visu.next(s.id);
      for (auto [in,out] : s.train) {
	//visu.add(in,core::embed(core::filterCol(out,10), out.sz));
	visu.add(listCols(in, andin&~orout),listCols(out, andout&~orin));
      }
    }
  }
  cout << count << " tasks" << endl;
}












//Needs rigid:
//0, 2, 12, 24, 52, (63), 66, 76, (91), 96, 130, (134), 138, 176, (182)

struct OrientationPicker {
  int feature_dim;
  OrientationPicker(const vector<Image>&ins) {
    feature_dim = 0;
    double best_score = 1.5;

    {
      double score = 1e3;
      for (Image_ in : ins) {
	score = min(score, max(in.w*1./in.h, in.h*1./in.w));
      }
      if (score > best_score) {
	feature_dim = 1;
	best_score = score;
      }
    }

    for (int c = 0; c < 10; c++) {
      double score = 1e3;
      for (Image_ in : ins) {
	auto [p,q] = inertia(in, c);
	double s = max(p*1./(q+1.), q*1./(p+1.));
	score = min(score, sqrt(s));
      }
      if (score > best_score) {
	feature_dim = 2+c;
	best_score = score;
      }
    }

    //TODO: improve heuristics for when to use
    {
      double scard = 1e3, sdiag = 1e3;
      for (Image_ in : ins) {
	double x[10] = {}, y[10] = {}, cnt[10] = {};
	colorMeans(in, x, y, cnt);

	double sx = 1e3, sy = 1e3, sxy = 1e3, syx = 1e3;
	int found = 0;
	for (int a = 1; a < 10; a++) {
	  for (int b = 1; b < a; b++) {
	    if (cnt[a] && cnt[b]) {
	      double dx = x[a]-x[b];
	      double dy = y[a]-y[b];
	      sx = min(sx, abs(dx)/(abs(dy)+1e-1));
	      sy = min(sy, abs(dy)/(abs(dx)+1e-1));
	      sxy = min(sxy, abs(dx+dy)/(abs(dx-dy)+1e-1));
	      syx = min(syx, abs(dx-dy)/(abs(dx+dy)+1e-1));
	      found = 1;
	      goto outside;
	    }
	  }
	}
      outside:
	if (!found) scard = sdiag = -1;
	scard = min(scard, max(sx, sy));
	sdiag = min(sdiag, max(sxy, syx));
      }
      scard /= 5., sdiag /= 5.;
      if (scard > best_score) {
	best_score = scard;
	feature_dim = 12;
      }
      if (sdiag > best_score) {
	best_score = sdiag;
	feature_dim = 13;
      }
    }
  }



  pair<double,double> inertia(Image_ img, char c) const {
    double x = 0, y = 0, xx = 0, yy = 0, cnt = 0;
    for (int i = 0; i < img.h; i++)
      for (int j = 0; j < img.w; j++)
	if (img(i,j) == c) {
	  x += j, xx += j*j;
	  y += i, yy += i*i;
	  cnt++;
	}
    if (cnt < 4) return {0.,0.};
    return {(xx*cnt-x*x), (yy*cnt-y*y)};
  }

  void colorMeans(Image_ in, double*x, double*y, double*cnt) const {
    for (int i = 0; i < in.h; i++)
      for (int j = 0; j < in.w; j++) {
	char c = in(i,j);
	x[c] += j;
	y[c] += i;
	cnt[c]++;
      }

    for (int c = 0; c < 10; c++) {
      if (cnt[c]) {
	x[c] /= cnt[c];
	y[c] /= cnt[c];
      }
    }
  }

  int getRigid(Image_ in) const {
    if (feature_dim == 0) return 0;
    else if (feature_dim == 1) return in.h > in.w ? 6 : 0;
    else if (feature_dim >= 2 && feature_dim < 12) {
      auto [p,q] = inertia(in, feature_dim-2);
      return p > q ? 6 : 0;
    } else if (feature_dim == 12 || feature_dim == 13) {

      double x[10] = {}, y[10] = {}, cnt[10] = {};
      colorMeans(in, x, y, cnt);

      for (int a = 1; a < 10; a++)
	for (int b = 1; b < a; b++) {
	  if (cnt[a] && cnt[b]) {
	    double dx = x[a]-x[b];
	    double dy = y[a]-y[b];
	    if (feature_dim == 12) {
	      if (abs(dx) > abs(dy))
		return dx < 0 ? 4 : 0;
	      else
		return dy < 0 ? 7 : 6;
	    } else {
	      if (abs(dx+dy) > abs(dx-dy))
		return dx+dy < 0 ? 7 : 0;
	      else
		return dx-dy < 0 ? 4 : 5;
	    }
	  }
	}
      return 0;
    }
    else assert(0);


    //Criteria:
    //Size should be horizontal
    //Colored lines should be vertical
    //2 colors should be top-left and bottom-right diagonally
    //2 colors should be left and right horizontally

    //Preserve:
    //Things touching one side / corner
    //Colored lines in output

  }
};

void normalizeRigid(const vector<pair<Image,Image>>&train, vector<Simplifier>&sims) {
  Simplifier ret;

  vector<Image> train_ins;
  for (auto&[in,out] : train) train_ins.push_back(in);
  OrientationPicker up(train_ins);

  ret.in = [up](Image_ in) {
    int rid = up.getRigid(in);
    return toOrigin(rigid(in, rid));
  };
  ret.out = [up](Image_ in, Image_ out) {
    int rid = up.getRigid(in);
    return toOrigin(rigid(out, rid));
  };
  ret.rec = [up](Image_ in, Image_ out) {
    int rid = up.getRigid(in);
    int inv[] = {0,3,2,1,4,5,6,7};
    return toOrigin(rigid(out, inv[rid]));
  };

  sims.push_back(ret);
}



void evalNormalizeRigid() {
  vector<Sample> sample = readAll("training", 100);//evaluation
  Visu visu;

  int place_count[11] = {};
  //#pragma omp parallel for
  for (int si = 0; si < sample.size(); si++) {
    Sample&s = sample[si];

    {
      vector<Simplifier> sims;
      remapCols(s.train, sims);
      for (auto&[in,out] : s.train)
	tie(in,out) = make_pair(sims[0].in(in), sims[0].out(in,out));
      for (auto&[in,out] : s.test)
	tie(in,out) = make_pair(sims[0].in(in), sims[0].out(in,out));
    }

    if (1) {
      visu.next(s.id);
      for (auto [in,out] : s.train) {
	visu.add(in,out);
      }
      vector<Simplifier> sims;
      normalizeRigid(s.train, sims);
      for (auto&[in,out] : s.train) {
	tie(in,out) = make_pair(sims[0].in(in), sims[0].out(in,out));
      }
      for (auto&[in,out] : s.test) {
	tie(in,out) = make_pair(sims[0].in(in), sims[0].out(in,out));
      }

      visu.next(s.id);
      for (auto [in,out] : s.train) {
	visu.add(in,out);
      }
      continue;
    }
  }
}



pair<Image,Image> Simplifier::operator()(Image_ a, Image_ b) {
  return {in(a), out(a,b)};
}

Simplifier normalizeCols(const vector<pair<Image,Image>>&train) {
  vector<Simplifier> sims;
  remapCols(train, sims);
  return sims[0];
}

Simplifier normalizeDummy(const vector<pair<Image,Image>>&train) {
  Simplifier ret;
  ret.in = [](Image_ in) { return in; };
  ret.out = [](Image_ in, Image_ out) { return out; };
  ret.rec = [](Image_ in, Image_ out) { return out; };
  return ret;
}
