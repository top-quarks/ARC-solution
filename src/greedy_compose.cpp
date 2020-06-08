#include "precompiled_stl.hpp"
using namespace std;
#include "utils.hpp"
//#include "core_functions.hpp"
//#include "image_functions.hpp"

struct bitptr {
  ull*data, bit;
  bool operator=(bool v) {
    if (v)
      *data |= (ull)v<<bit;
    else
      *data &= ~((ull)v<<bit);
  }
};

struct mybitset {
  vector<ull> data;
  mybitset(int n) {
    data.resize((n+63)/64);
  }
  int operator[](int i) {
    return data[i>>6]>>(i&63)&1;
  }
  bitptr operator()(int i) {
    return {&data[i>>6], ull(i&63)};
  }
};

vector<pair<vector<Image>,double>> greedyCompose(const vector<Piece>&piece0, const vector<Image>&target) {
  if (piece0.empty()) return {};

  vector<Piece> piece;
  set<vector<Image>> seen;
  for (auto&p : piece0) {
    if (!seen.count(p.imgs)) {
      piece.emplace_back(p);
      seen.insert(p.imgs);
    }
  }
  /*sort(piece.begin(), piece.end(), [](const Piece&a,
				      const Piece&b) {
				      return a.second > b.second;});*/
  //cout << piece0.size() << ' ' << piece.size() << endl;

  vector<Image> init = piece[0].imgs;
  for (Image& img : init) {
    assert(img.x == 0 && img.y == 0);
    for (char&c : img.mask)
      c = 10;
  }

  vector<pair<vector<Image>,double>> rets;

  vector<int> sz;
  for (int j = 0; j < init.size(); j++)
    sz.push_back(init[j].mask.size());





  int n = piece.size();

  vector<int> a;
  int M = 0;
  for (int j = 0; j < init.size(); j++) {
    a.push_back(j);
    M += sz[j];
  }

  vector<mybitset> good, active;
  for (int i = 0; i < n; i++) {
    mybitset goodi(M), activei(M), blacki(M);
    int x = 0, y = 0, z = 0;
    for (int j : a) {
      const vector<char>&p = piece[i].imgs[j].mask;
      const vector<char>&t = j < target.size() ? target[j].mask : init[j].mask;
      assert(p.size() == sz[j]);
      assert(t.size() == sz[j]);
      for (int k = 0; k < sz[j]; k++) {
	goodi(x++) = (p[k] == t[k]);
	activei(y++) = p[k] != 0;
	blacki (z++) = p[k] == 0;
      }
    }
    good.push_back(goodi);
    active.push_back(activei);
    good.push_back(goodi);
    active.push_back(blacki);
  }


  int maxiters = 10;

  int knowi = piece[0].knowi;
  for (int it0 = 0; it0 < 10; it0++) {
    /*for (int N = 1; N <= target.size(); N++) {
      int caremask = 1<<piece0[0].keepi;
      int mask = caremask;
      vector<int> order;
      for (int i = 0; i < target.size(); i++) order.push_back(i);
      random_shuffle(order.begin(), order.end());
      for (int j = 0; __builtin_popcount(mask) < N; j++) {
	mask |= 1<<order[j];
      }*/
    for (int mask = 1; mask < min(1<<target.size(), 32); mask++) {
      //{
      //int mask = rand()%min(1<<target.size(), 32);
      int pop = __builtin_popcount(mask);//rand()%target.size();
      /*mask = 0;
      for (int i = 0; i < pop && i < target.size(); i++) {
	int r = -1;
	int its = 0;
	while (r == -1 || (mask>>r&1)) {
	  r = rand()%target.size();
	  assert(its++ < 1000000);
	}
	mask |= 1<<r;
	}*/
      //	for (int caremask = mask; caremask; caremask = (caremask-1)%mask) {
      if (mask == 0 || (knowi >= 0 && mask == 1<<knowi)) {// || __builtin_popcount(mask) > 3) {
	//it0--;
	continue;
      }
      int caremask = 0;
      {
	int carei = 10;
	int its = 0;
	while ((mask>>carei&1) == 0 || carei == knowi) {
	  carei = rand()%target.size();
	  assert(its++ < 1000000);
	}
	caremask = 1<<carei;// | 1<<rand()%target.size();
      }
      vector<int> skip(n);
      if (it0 > 5) {
	for (int i = 0; i < n; i++)
	  skip[i] = rand()%3;
      }

    /*for (int N = 1; N <= target.size(); N++) {
      int mask = (1<<target.size())-1;
      int mask = 0;
      {
      vector<int>  = train;
      swap(train2[it], train2[1]);
      swap(train2[0], train2[1]);*/



      mybitset cur(M), careMask(M);
      {
	int base = 0;
	for (int j : a) {
	  if (!(mask>>j&1))
	    for (int k = 0; k < sz[j]; k++)
	      cur(base+k) = 1;
	  if ((caremask>>j&1))
	    for (int k = 0; k < sz[j]; k++)
	      careMask(base+k) = 1;
	  base += sz[j];
	}
      }


      double depth = 0;

      vector<Image> ret = init;
      for (int it = 0; it < maxiters; it++) {
	int bestcnt = 0, besti = -1;
	for (int i = 0; i < n*2; i++) {
	  if (skip[i>>1]) continue;

	  int cnt = 0, ok = 1;
	  for (int j = 0; j < active[i].data.size(); j++) {
	    cnt += __builtin_popcountll(active[i].data[j] & ~cur.data[j] & careMask.data[j]);
	    ok &= !(active[i].data[j] & ~cur.data[j] & ~good[i].data[j]);
	  }
	  if (ok && cnt > bestcnt) {
	    bestcnt = cnt;
	    besti = i;
	  }
	}

	if (besti == -1) break;

	depth++;
	{
	  int i = besti/2, x = 0;
	  for (int l = 0; l < ret.size(); l++) {
	    for (int j = 0; j < sz[l]; j++) {
	      if (active[besti][x++] && ret[l].mask[j] == 10) {
		//assert(cur[x-1] == 0);
		ret[l].mask[j] = piece[i].imgs[l].mask[j];
	      }
	    }
	  }
	  for (int j = 0; j < M; j++) {
	    if (active[besti][j]) cur(j) = 1;
	  }
	}
      }


      for (Image&img : ret)
	for (char&c : img.mask)
	  if (c == 10) c = 0;

      rets.emplace_back(ret, depth);
    }
  }

  return rets;
}
