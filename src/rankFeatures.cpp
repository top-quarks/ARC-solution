#include "precompiled_stl.hpp"

using namespace std;

#include "utils.hpp"
#include "core_functions.hpp"
#include "image_functions.hpp"
#include "visu.hpp"
#include "read.hpp"
#include "features.hpp"
#include "normalize.hpp"



void genFakes(Image_ truth, vector<Image>&cands) {
  for (int id = 1; id < 8; id++) {
    cands.push_back(rigid(truth,id));
  }
  cands.push_back(compress(truth));
  cands.push_back(embed(truth, core::full(truth.sz+point{1,1}, 1)));
  for (int it = 0; it < 10; it++) {
    Image cp = truth;
    int r = rand()%cp.h;
    int c = rand()%cp.w;
    cp(r,c) = !cp(r,c);
    cands.push_back(cp);
  }
  if (truth.w >= 2 && truth.h >= 2) {
    int its = 0;
    for (int it = 0; it < 10; it++) {
      Image cp = truth;
      while (its < 1000) {
	its++;
	int r = rand()%cp.h;
	int c = rand()%cp.w;
	int d = rand()%2;
	if (r+d < cp.h && c+!d < cp.w &&
	    cp(r,c) != cp(r+d,c+!d)) {
	  swap(cp(r,c),cp(r+d,c+!d));
	  cands.push_back(cp);
	  break;
	}
      }
    }
  }

  int mask_ = core::colMask(truth);
  vector<int> mask;
  for (int c = 0; c < 10; c++)
    if (mask_>>c&1) mask.push_back(c);
  if (mask.size() > 1) {
    for (int it = 0; it < 10; it++) {
      int a = rand()%mask.size();
      int b = rand()%(mask.size()-1);
      if (b >= a) b++;
      a = mask[a];
      b = mask[b];

      Image cp = truth;
      for (int i = 0; i < cp.h; i++) {
	for (int j = 0; j < cp.w; j++) {
	  char&c = cp(i,j);
	  if (c == a) c = b;
	  else if (c == b) c = a;
	}
      }
      cands.push_back(cp);
    }
  }
}

void rankFeatures() {
  vector<Sample> sample = readAll("training", 100);//evaluation
  //sample = {sample[16]};
  if (0) {
    //Dump all tasks to visu.py
    Visu visu;
    for (Sample&s : sample) {
      visu.next(s.id);
      for (auto [in,out] : s.train) {
	visu.add(in,out);
      }
    }
    return;
  }

  /*for (int i = 0; i < sample[0].train.size(); i++) {
    Image in = sample[0].train[i].first;
    Image out = sample[0].train[i].second;
    assert(containsExact(in,in));
    assert(containsExact(out,out));
    cout << i << endl;
    assert(containsExact(in,out));
  }
  return;*/

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


    //for (Sample& s : sample) {
    //swap(s.test[0], s.train[rand()%s.train.size()]);

    auto [test_in, truth] = s.test[0];
    vector<Image> cands = {truth};

    //Fake
    for (Sample& s : sample) {
      for (auto&[in,out] : s.train) {
	cands.push_back(out);
      }
      for (auto&[in,out] : s.test) {
	cands.push_back(out);
      }
    }
    genFakes(truth, cands);


    {
      set<vector<char>> seen;
      vector<Image> filtered = {truth};
      for (Image_ img : cands)
	if (!(img == truth) && !seen.count(img.mask)) {
	  filtered.push_back(img);
	  seen.insert(img.mask);
	}
      swap(cands, filtered);
    }

    vector<double> scores = featureScores(s.train, test_in, cands);
    cout << "Task " << si << endl;
    cout << "Truth score: " << scores[0] << endl;
    int bads = 0;
    for (int i = 1; i < scores.size(); i++)
      bads += scores[i] >= scores[0];

    place_count[min(bads,10)]++;

    {
      visu.next(s.id);
      vector<tuple<double,Image,int>> top;
      for (int i = 0; i < cands.size(); i++)
	top.emplace_back(scores[i], cands[i], i);
      reverse(top.begin(), top.end());
      stable_sort(top.begin(), top.end(), [](const tuple<double,Image,int>&a, const tuple<double,Image,int>&b) {
	  return get<0>(a) > get<0>(b);
	});
      //cout << top[0].first << endl;
      cout << "Top 5: ";
      visu.add(s.train[0].second, truth);
      for (int i = 0; i < 5 && i < top.size(); i++) {
	printf("%d (%.1f) ", get<2>(top[i]), get<0>(top[i]));
	if (get<1>(top[i]) == truth)
	  get<1>(top[i]) = Col(3);
	visu.add(test_in, get<1>(top[i]));
      }
      cout << endl;
    }
  }
  cout << "Place counts: ";
  for (int cnt : place_count) cout << cnt << ' ';
  cout << endl;
}
