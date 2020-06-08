#include "precompiled_stl.hpp"
using namespace std;
#include "utils.hpp"
#include "core_functions.hpp"
#include "image_functions.hpp"
#include "greedy_compose.hpp"
#include "visu.hpp"

bool operator<(const Piece& a, const Piece& b) {
  for (int i = 0; i < a.imgs.size(); i++)
    if (a.imgs[i] != b.imgs[i])
      return a.imgs[i] < b.imgs[i];
  return
    tie(a.node_prob, a.keepi, a.knowi) <
    tie(b.node_prob, b.keepi, b.knowi);
}

vector<Piece> embedPieces(const vector<Piece>&pieces, vector<pair<Image, Image>> train) {
  int inputs = train.size()+1;

  int maxgoods = 0, szi = 0;
  for (int i = 0; i < pieces.size(); i++) {
    const vector<Image>&outs = pieces[i].imgs;
    assert(outs.size() == inputs);
    int goods = 0;
    for (int i = 0; i < train.size(); i++) {
      if (outs[i].sz == train[i].second.sz) goods++;
    }
    if (goods > maxgoods) {
      maxgoods = goods;
      szi = i;
    }
  }

  //cout << maxgoods << " / " << train.size() << endl;

  vector<Image> ret;
  for (int i = 0; i < inputs; i++) {
    point sz = pieces[szi].imgs[i].sz;
    if (i < train.size()) {
      sz = train[i].second.sz;
    } else {
      sz.x = min(sz.x, 30);
      sz.y = min(sz.y, 30);
    }
    ret.push_back(core::full({0,0}, sz, 10));
  }

  //set<pair<int,int>> kk;

  set<Piece> seen;
  vector<Piece> embed_pieces;
  for (auto&piece : pieces) {
    //kk.emplace(piece.keepi, piece.knowi);
    if (piece.imgs.back().w == 0) continue;

    Piece newpiece = piece;
    newpiece.imgs.resize(inputs);
    auto&toadd = newpiece.imgs;
    for (int k : {0,1}) {
      for (int j = 0; j < inputs; j++) {
	if (!k)
	  toadd[j] = embed(piece.imgs[j], ret[j]);
	else
	  toadd[j] = embed(toOrigin(piece.imgs[j]), ret[j]);
      }
      if (!seen.count(newpiece)) {
	embed_pieces.push_back(newpiece); //TODO think about (k ? prob*0.5 : prob)
	seen.insert(newpiece);
      }
      if (piece.imgs[0].x == 0 && piece.imgs[0].y == 0) break;
    }
  }

  /*
  for (auto [keepi, knowi] : kk) {
    for (int c = 0; c < 10; c++) {
      Piece p;
      p.node_prob = 0;
      p.keepi = keepi;
      p.knowi = knowi;
      p.imgs.resize(inputs);
      for (int i = 0; i < inputs; i++) {
	p.imgs[i] = core::full({0,0}, ret[i].sz, c);
      }
      embed_pieces.push_back(p);
    }
  }
  */

  return embed_pieces;
}


vector<Image> composePieces(const vector<Piece>&pieces, vector<pair<Image, Image>> train) {

  vector<Image> preds;
  vector<pair<double,int>> scores;

  auto addPred = [&](const pair<vector<Image>,double>&outs_, int knowi) {
    auto [outs, prior] = outs_;
    double goods = 0, nontrivial = 1;
    //goods = 1, nontrivial = 1;

    for (int i = 0; i < train.size(); i++) {
      if (outs[i] == train[i].second) { //TODO: match scoreEqual
	goods += (i == knowi ? 0.8 : 1);
	nontrivial += (i != knowi); //TODO
      }
    }
    if (nontrivial) {
      double score = goods-prior*0.01;
      //assert(goods);
      Image answer = outs.back();
      if (answer.w > 30 || answer.h > 30 || answer.w*answer.h == 0) goods = 0;
      for (int i = 0; i < answer.h; i++)
	for (int j = 0; j < answer.w; j++)
	  if (answer(i,j) < 0 || answer(i,j) >= 10) goods = 0;

      if (goods) {
	preds.push_back(answer);
	scores.emplace_back(-score, scores.size());
      }
    }
  };


  {
    vector<Image> outs;
    for (auto [in,out] : train) outs.push_back(out);
    map<pair<int,int>, vector<Piece>> m;
    for (auto&p : pieces)
      m[{p.keepi, p.knowi}].push_back(p);
    for (auto&[pa,v] : m) {
      for (auto&piece : v)
	addPred({piece.imgs, 1}, piece.knowi);
      for (auto&ret : greedyCompose(v, outs)) {
	addPred(ret, pa.second);
      }
    }
  }

  sort(scores.begin(), scores.end());
  vector<Image> preds_;
  set<ull> seen;
  for (auto [score,i] : scores) {
    ull h = hashImage(preds[i]);
    if (!seen.count(h)) {
      preds_.push_back(preds[i]);
      seen.insert(h);
    }
  }

  return preds_;
}
