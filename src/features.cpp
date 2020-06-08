/*#include <cassert>
#include <functional>
#include <vector>*/
#include "precompiled_stl.hpp"
using namespace std;
#include "utils.hpp"
#include "core_functions.hpp"
#include "image_functions.hpp"

int symmetric(Image_ img, int id) {
  assert(id >= 0 && id < 5);
  int flipx = 0, flipy = 0, swapxy = 0;
  if (id == 0) {
    //Flip x
    flipx = 1;
  } else if (id == 1) {
    //Flip y
    flipy = 1;
  } else if (id == 2) {
    //Rot 180
    flipx = 1;
    flipy = 1;
  } else if (id == 3) {
    //Swap xy
    swapxy = 1;
  } else if (id == 4) {
    //Rot 90
    flipx = 1;
    swapxy = 1;
  } else assert(0);

  if (swapxy && img.w != img.h) return 0;
  for (int i = 0; i < img.h; i++) {
    for (int j = 0; j < img.w; j++) {
      int x = flipx ? img.w-1-j : j;
      int y = flipy ? img.h-1-i : i;
      if (swapxy) swap(x,y);
      if (img(i,j) != img(y,x)) return 0;
    }
  }
  return 1;
}

int countFeat(Image_ img, int id) {
  assert(id >= 0 && id < 5);
  if (id == 0) return core::colMask(img);
  else if (id == 1) return core::countCols(img);
  else if (id == 2) return core::count(img);
  else if (id == 3) return core::countComponents(img);
  else if (id == 4) return core::majorityCol(img);
  else assert(0);
}

void addAlignCols(Image_ in, Image_ out, vector<double>&ret) {
  int has[10][10] = {};
  int has2[2][2] = {};
  int allin = 1, allout = 1;
  for (int i = 0; i < min(in.h,out.h); i++) {
    for (int j = 0; j < min(in.w,out.w); j++) {
      has[in(i,j)][out(i,j)] = 1;
      has2[!in(i,j)][!out(i,j)] = 1;
      allin &= in(i,j) == out(i,j) || in(i,j) == 0;
      allout &= in(i,j) == out(i,j) || out(i,j) == 0;
    }
  }
  ret.push_back(allin);
  ret.push_back(allout);
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      ret.push_back(has[i][j]/20.);
      ret.push_back(!has[i][j]/20.);
    }
  }
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {
      ret.push_back(has2[i][j]);
      ret.push_back(!has2[i][j]);
    }
  }
}

void addNbrCols(Image_ out, vector<double>&ret) {
  int has[10][10] = {};
  for (int i = 0; i < out.h; i++) {
    for (int j = 0; j < out.w; j++) {
      if (i) {
	has[out(i-1,j)][out(i,j)] = 1;
	has[out(i,j)][out(i-1,j)] = 1;
      }
      if (j) {
	has[out(i,j-1)][out(i,j)] = 1;
	has[out(i,j)][out(i,j-1)] = 1;
      }
    }
  }
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      ret.push_back(has[i][j]/20.);
      ret.push_back(!has[i][j]/20.);
    }
  }
}

void calcColCounts(Image_ img, int*cnt) {
  for (int i = 0; i < img.h; i++)
    for (int j = 0; j < img.w; j++)
      cnt[img(i,j)]++;

}
void addColCounts(Image_ in, Image_ out, vector<double>&ret) {
  int inc[10] = {}, outc[10] = {};
  calcColCounts(in,inc);
  calcColCounts(out,outc);
  for (int c = 0; c < 10; c++)
    ret.push_back((inc[c] && inc[c] == outc[c])/5.);
}

//TODO: actually verify
using ull = unsigned long long;
bool containsExact(Image_ big, Image_ small) {
  if (small.w > big.w || small.h > big.h) return 0;

  static ull
    hash_small[MAXSIDE+1][MAXSIDE+1] = {},
    hash_big[MAXSIDE+1][MAXSIDE+1] = {},
    tmp[MAXSIDE+1][MAXSIDE+1] = {};
  static ull basex_pow[MAXSIDE+1],
    basey_pow[MAXSIDE+1], basex = 37, basey = 23;
  static int inited = 0;
  if (!inited) {
    basex_pow[0] = basey_pow[0] = 1;
    for (int i = 1; i <= MAXSIDE; i++) {
      basex_pow[i] = basex_pow[i-1]*basex;
      basey_pow[i] = basey_pow[i-1]*basey;
    }
    inited = 1;
  }
  auto hashImg = [](Image_ img, ull hash[MAXSIDE+1][MAXSIDE+1]) {
    for (int i = 0; i < img.h; i++)
      for (int j = 0; j < img.w; j++)
	tmp[i+1][j] = tmp[i][j]*basey+img(i,j);
    for (int i = 0; i <= img.h; i++)
      for (int j = 0; j < img.w; j++)
	hash[i][j+1] = hash[i][j]*basex+tmp[i][j];
  };
  hashImg(small, hash_small);
  hashImg(big, hash_big);
  ull target = hash_small[small.h][small.w];
  for (int i = 0; i+small.h <= big.h; i++) {
    for (int j = 0; j+small.w <= big.w; j++) {
      if (hash_big[i+small.h][j+small.w]-
	  hash_big[i+small.h][j] * basex_pow[small.w]-
	  hash_big[i][j+small.w] * basey_pow[small.h]+
	  hash_big[i][j] * basex_pow[small.w] * basey_pow[small.h]
	  == target) return 1;
    }
  }
  return 0;
}

void addContains(Image_ in, Image_ out, vector<double>&ret) {
  ret.push_back(containsExact(in,compress(out)));
  int mask = core::colMask(out);
  auto add = [&](int c, double w) {
    if (mask>>c&1) {
      Image inc = filterCol(in,c);
      Image outc = compress(filterCol(out,c));
      //if (outc.w*outc.h >= 9)
      int contained = containsExact(inc,outc);
      ret.push_back(contained * w);
    } else ret.push_back(0);
  };
  for (int c = 1; c < 10; c++) {
    add(c, 1./3);
  }
  add(core::majorityCol(in), 1);
  add(core::majorityCol(out), 1);
}

void addEqualCol(Image_ in, Image_ out, vector<double>&ret) {
  ret.push_back(compress(in) == compress(out));
  int mask = core::colMask(out);
  for (int c = 1; c < 10; c++) {
    if (mask>>c&1) {
      Image inc = compress(filterCol(in,c));
      Image outc = compress(filterCol(out,c));
      //if (inc.w*inc.h >= 9)
      ret.push_back(inc == outc);
    } else ret.push_back(0);
  }
}

void addShapeImplications(Image_ img, vector<double>&ret) {
  vector<vector<point>> need, then;
  //x.x -> .x.
  need.push_back({{0,0},{2,0}});
  then.push_back({{1,0}});

  //x    .
  //. -> x
  //x    .
  need.push_back({{0,0},{0,2}});
  then.push_back({{0,1}});

  //.x.    ...
  //x.x -> .x.
  //.x.    ...
  need.push_back({{1,0},{0,1},{1,2},{2,1}});
  then.push_back({{1,1}});

  //x..    ...
  //... -> .x.
  //..x    ...
  need.push_back({{0,0},{2,2}});
  then.push_back({{1,1}});

  //..x    ...
  //... -> .x.
  //x..    ...
  need.push_back({{0,2},{2,0}});
  then.push_back({{1,1}});

  //x.x    ...
  //... -> .x.
  //x.x    ...
  need.push_back({{0,0},{0,2},{2,0},{2,2}});
  then.push_back({{1,1}});

  //xxx    ...
  //x.x -> .x.
  //xxx    ...
  need.push_back({{0,0},{0,1},{0,2},{1,2},{2,2},{2,1},{2,0},{1,0}});
  then.push_back({{1,1}});

  for (int ni = 0; ni < need.size(); ni++) {
    vector<point>&needi = need[ni], &theni = then[ni];

    int has = 0, all = 1;
    for (int i = 0; i < img.h; i++) {
      for (int j = 0; j < img.w; j++) {
	int ok = 1;
	for (point p : needi) {
	  if (!img.safe(i+p.y, j+p.x)) {
	    ok = 0;
	    break;
	  }
	}
	has |= ok;
	if (ok) {
	  for (point p : theni) {
	    if (!img.safe(i+p.y, j+p.x)) {
	      ok = 0;
	      break;
	    }
	  }
	  all &= ok;
	}
      }
    }
    ret.push_back(has && all);
  }
}

void inConstShapes(Image_ img, vector<double>&ret) {
  static int col[MAXSIDE], row[MAXSIDE];
  int mask = core::colMask(img);
  for (char c = 0; c < 10; c++) {
    if (mask>>c&1) {
      //In few rows / cols
      //Is rect
      for (int i = 0; i < max(img.h,img.w); i++)
	col[i] = row[i] = 0;
      int minx = img.w, miny = img.h, maxx = -1, maxy = -1;
      int cnt = 0;
      for (int i = 0; i < img.h; i++) {
	for (int j = 0; j < img.w; j++) {
	  if (img(i,j) == c) {
	    row[i]++, col[j]++;
	    maxx = max(maxx, j);
	    maxy = max(maxy, i);
	    minx = min(minx, j);
	    miny = min(miny, i);
	    cnt++;
	  }
	}
      }
      ret.push_back((maxx-minx+1)*(maxy-miny+1) == cnt);
      auto top = [](int*a, int n) {
	pair<int,int> ret = {-1,-1};
	for (int i = 0; i < n; i++) {
	  if (ret.first == -1 || a[i] > a[ret.first]) ret = {i, ret.first};
	  else if (ret.second == -1 || a[i] > a[ret.second]) ret.second = i;
	}
	return ret;
      };
      pair<int,int> col2 = top(col, img.w);
      pair<int,int> row2 = top(row, img.h);
      int in1 = 1, in2 = 1;
      for (int i = 0; i < img.h; i++) {
	for (int j = 0; j < img.w; j++) {
	  if (img(i,j) == c) {
	    if (!(i == row2.first || j == col2.first)) {
	      in1 = 0;
	      if (!(i == row2.second || j == col2.second)) {
		in2 = 0;
	      }
	    }
	  }
	}
      }
      int cols = 0, rows = 0;
      for (int i = 0; i < img.w; i++) cols += col[i] > 0;
      for (int i = 0; i < img.h; i++) rows += row[i] > 0;
      ret.push_back(cols == 1);
      ret.push_back(rows == 1);

      ret.push_back(in2);
      ret.push_back(in1);
      //cout << c << ' ' << in1 << ' ' << in2 << endl;

    } else {
      ret.push_back(0);
      ret.push_back(0);
      ret.push_back(0);
      ret.push_back(0);
      ret.push_back(0);
    }
  }
}

vector<double> singleFeatures(Image_ in, Image_ out, Image_ given) {
  static int features = -1;
  vector<double> ret;
  for (int id = 0; id < 5; id++)
    ret.push_back(symmetric(out,id));
  for (int id = 0; id < 5; id++)
    ret.push_back(countFeat(in,id) == countFeat(out,id));
  for (int id = 0; id < 5; id++)
    ret.push_back(countFeat(out,id) == countFeat(given,id));
  ret.push_back(out.sz == in.sz);
  ret.push_back(out.sz == given.sz);
  ret.push_back(broadcast(in,out) == out);
  ret.push_back(broadcast(out,in) == in);

  addAlignCols(in,out,ret);
  addAlignCols(given,out,ret);
  addNbrCols(out,ret);
  addColCounts(in,out,ret);
  addColCounts(given,out,ret);
  addContains(in,out,ret);
  addContains(given,out,ret);
  addContains(out,in,ret);
  addEqualCol(in,out,ret);

  addShapeImplications(out,ret);
  inConstShapes(out,ret);

  if (features == -1) features = ret.size();
  else assert(ret.size() == features);
  return ret;
}


//TODO:
// - row/column-wise features
// - component (color and connected)-wise features


vector<double> featureScores(vector<pair<Image,Image>>&train, Image_ test_in, vector<Image>&imgs) {
  vector<vector<double>> train_features, test_features;
  Image_ given = train[0].second;
  for (auto&[in,out] : train) {
    train_features.push_back(singleFeatures(in,out,given));
  }
  for (auto&img : imgs) {
    test_features.push_back(singleFeatures(test_in,img,given));
  }

  vector<double> scores(imgs.size());
  for (int fi = 0; fi < train_features[0].size(); fi++) {
    int all = 1;
    for (vector<double>&feat : train_features) {
      all &= !!feat[fi];
    }
    if (all) {
      for (int i = 0; i < imgs.size(); i++) {
	scores[i] += test_features[i][fi];
	//if (i == 21 && test_features[i][fi]) cout << fi << ' ';
      }
    }
  }
  //cout << endl;
  //cout << scores[0] << ' ' << scores[17] << endl;

  return scores;
}
