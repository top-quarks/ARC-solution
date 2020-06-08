#include "precompiled_stl.hpp"
using namespace std;
#include "utils.hpp"
#include "spec.hpp"
#include "visu.hpp"

vector<function<SpecRec(Image_,Spec_)>> initIFuncs() {
  vector<function<SpecRec(Image_,Spec_)>> ifuncs;
  ifuncs.push_back(iOuterProductSIFirst);
  ifuncs.push_back(iOuterProductISFirst);
  ifuncs.push_back(iOuterProductSISecond);
  ifuncs.push_back(iOuterProductISSecond);

  ifuncs.push_back(iComposeFirst);
  ifuncs.push_back(iComposeSecond);
  ifuncs.push_back(iColShapeFirst);
  ifuncs.push_back(iColShapeSecond);
  ifuncs.push_back(iRepeatSecond);
  ifuncs.push_back(iMirrorSecond);

  for (int id = 0; id < 8; id++) {
    ifuncs.push_back([id](Image_ a, Spec_ b) {
	return iStack(a, b, id);});
  }
  ifuncs.push_back(iEmbed);
  return ifuncs;
}

vector<Piece> recSolve(vector<function<SpecRec(Image_,Spec_)>>&ifuncs, const vector<Piece>&pieces, Spec_ target, int ti, int depth) {
  const int k = pieces[0].imgs.size();

  vector<Piece> ret;
  {
    Piece r;
    r.imgs.assign(k, toImage(target));
    r.node_prob = -depth;
    r.keepi = ti;
    r.knowi = -1;
    ret.push_back(r);
  }

  for (Piece_ p : pieces) {
    if (target.check(p.imgs[ti])) {
      ret.push_back(p);
      ret.back().node_prob = -depth;
    }
  }

  if (depth == 3) return ret;

  for (const auto& f : ifuncs) {
    int added = 0;
    for (Piece_ p : pieces) {
      auto [spec, rec] = f(p.imgs[ti], target);
      if (spec.bad) continue;

      if (++added > 2) continue;

      for (Piece_ q : recSolve(ifuncs, pieces, spec, ti, depth+1)) {
	Piece r;
	r.imgs.resize(k);
	for (int i = 0; i < k; i++) {
	  r.imgs[i] = rec(p.imgs[i], q.imgs[i]);
	}
	assert(spec.check(q.imgs[ti]));
	if (!target.check(r.imgs[ti])) {
	  cout << 1 << endl;
	  rec(p.imgs[ti], q.imgs[ti]);
	  cout << 2 << endl;
	  //cout << (f.target == function<SpecRec(Image_,Spec_)>(iOuterProductSIFirst).target) << endl;
	  cout << "Target check failed" << endl;
	  print(p.imgs[ti]);
	  print(q.imgs[ti]);
	  print(r.imgs[ti]);
	  print(toImage(target));
	  exit(0);
	}
	r.node_prob = q.node_prob;
	r.keepi = ti;
	r.knowi = p.knowi;
	ret.push_back(r);
      }
    }
  }
  return ret;
}





vector<Image> iBrute(const vector<Piece>&pieces, vector<pair<Image, Image>> train) {
  if (pieces.empty()) return {};
  int k = pieces[0].imgs.size();

  auto ifuncs = initIFuncs();

  vector<Image> preds;
  vector<pair<double,int>> scores;
  for (int ti = 0; ti < k-1; ti++) {
    Spec target = fromImage(train[ti].second);
    for (Piece_ p : recSolve(ifuncs, pieces, target, ti, 0)) {
      int goods = 0;
      for (int i = 0; i < train.size(); i++) {
	if (p.imgs[i] == train[i].second) {
	  goods++;
	}
      }
      double score = goods - p.node_prob*1e-3;

      Image answer = p.imgs.back();
      if (answer.w > 30 || answer.h > 30 || answer.w*answer.h == 0) goods = 0;
      for (int i = 0; i < answer.h; i++)
	for (int j = 0; j < answer.w; j++)
	  if (answer(i,j) < 0 || answer(i,j) >= 10) goods = 0;

      preds.push_back(answer);
      scores.emplace_back(-score, scores.size());
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
