#include "precompiled_stl.hpp"
using namespace std;
#include "utils.hpp"
#include "core_functions.hpp"
#include "image_functions.hpp"
#include "greedy_compose.hpp"
#include "compose.hpp"

#include "brute2.hpp"
#include "pieces.hpp"
#include "compose2.hpp"


int scorePiecesOld(const vector<Piece>&piece, Image_ test_in, Image_ test_out, const vector<pair<Image,Image>>&train) {
  int maxits = 10;

  vector<Image> target;
  for (auto [in,out] : train) {
    target.push_back(out);
  }
  target.push_back(test_out);

  int k = piece[0].imgs.size();
  assert(k == train.size()+1);
  if (piece.empty()) return 0;

  vector<int> sz;
  for (int j = 0; j < k; j++) {
    sz.push_back(piece[0].imgs[j].mask.size());
    if (piece[0].imgs[j].sz != test_out.sz) return 0;
  }


  for (int keepi = 0; keepi < 1; keepi++) { //k-1
    vector<int> a = {k-1};//{keepi, k-1};
    int M = 0;
    for (int j : a) M += sz[j];

    vector<vector<bool>> good, active;
    for (int i = 0; i < piece.size(); i++) {
      vector<bool> goodi(M), activei(M), blacki(M);
      int x = 0, y = 0, z = 0;
      for (int j : a) {
	const vector<char>&p = piece[i].imgs[j].mask;
	const vector<char>&t = target[j].mask;
	for (int k = 0; k < sz[j]; k++) {
	  goodi  [x++] = (p[k] == t[k]);
	  activei[y++] = p[k] > 0;
	  blacki [z++] = p[k] == 0;
	}
      }
      good.push_back(goodi);
      active.push_back(activei);
      good.push_back(goodi);
      active.push_back(blacki);
    }

    vector<bool> cur(M);
    for (int it = 0; it < maxits; it++) {
      int bestcnt = 0, besti = -1;
      for (int i = 0; i < active.size(); i++) {
	int ok = 1, cnt = 0;
	for (int j = 0; j < M; j++) {
	  if (active[i][j] && !cur[j]) {
	    if (!good[i][j]) {
	      ok = 0;
	      goto fail;
	    } else cnt += 1;//j < m[0];
	  }
	}
      fail:
	if (ok && cnt > bestcnt) {
	  bestcnt = cnt;
	  besti = i;
	}
      }
      if (besti == -1) break;

      {
	int i = besti;
	for (int j = 0; j < M; j++) {
	  if (active[i][j]) cur[j] = 1;
	}
      }
    }

    if (1) {
      char col = -1, ok = 1;
      int x = 0;
      for (int j : a) {
	for (int l = 0; l < sz[j]; l++) {
	  if (cur[x++] == 0) {
	    char c = target[j].mask[l];
	    if (col == -1) col = c;
	    if (col != c) ok = 0;
	  }
	}
      }
      if (ok) return 1;
    }

//if (count(cur.begin(), cur.end(), 0) == 0) return 1;
  }

  return 0;
}

int scoreCands(const vector<Candidate>&cands, Image_ test_in, Image_ test_out) {
  for (const Candidate&cand : cands)
    if (cand.imgs.back() == test_out) return 1;
  return 0;
}

int scoreAnswers(vImage_ answers, Image_ test_in, Image_ test_out) {
  assert(answers.size() <= 3);
  for (Image_ answer : answers)
    if (answer.sz == test_out.sz && answer.mask == test_out.mask) return 1;
  return 0;
}



















#include <unordered_set>

template<int nax>
int scorePiecesCore(const vector<Piece>&piece, const vector<pair<Image,Image>>&train, Image_ test_out, int A, int B, int ti) {
  int M = A+B;
  assert(M <= nax);

  vector<pair<bitset<nax>,bitset<nax>>> good, filtered;
  for (int i = 0; i < piece.size(); i++) {
    bitset<nax> goodi, activei, blacki;
    {
      const vector<char>&p = piece[i].imgs.back().mask;
      const vector<char>&t = test_out.mask;
      for (int k = 0; k < A; k++) {
	goodi  [k] = (p[k] == t[k]);
	activei[k] = p[k] > 0;
	blacki [k] = p[k] == 0;
      }
    }
    {
      const vector<char>&p = piece[i].imgs[ti].mask;
      const vector<char>&t = train[ti].second.mask;
      for (int k = 0; k < B; k++) {
	goodi  [A+k] = (p[k] == t[k]);
	activei[A+k] = p[k] > 0;
	blacki [A+k] = p[k] == 0;
      }
    }
    good.emplace_back(goodi & activei, ~goodi & activei);
    good.emplace_back(goodi & blacki, ~goodi & blacki);
  }

  sort(good.begin(), good.end(), [](const pair<bitset<nax>,bitset<nax>>&a,
				    const pair<bitset<nax>,bitset<nax>>&b) {
	 return
	   make_pair(a.first.count(), -a.second.count()) >
	   make_pair(b.first.count(), -b.second.count());
       });


  set<pair<size_t, size_t>> seen0;
  hash<bitset<nax>> hash_fn;

  for (int i = 0; i < good.size(); i++) {
    auto&[ga,ba] = good[i];
    pair<size_t,size_t> h = make_pair(hash_fn(ga), hash_fn(ba));
    if (seen0.count(h)) continue;
    seen0.insert(h);
    int covered = 0, tries = 0;
    for (auto&[gb,bb] : filtered) {
      int gooda = (ga&~gb).any();
      int badb = (~ba&bb).any();
      if (gooda == 0 && badb == 0) {
	covered = 1;
	break;
      }
      if ((++tries)*filtered.size() > 1e7) break;
    }
    if (!covered) {
      filtered.push_back(good[i]);
    }
  }
  good = filtered;
  //cout << piece.size()*2 << ' ' << good.size() << endl;

  int maxits = 5;

  unordered_set<bitset<nax>> seen;
  vector<vector<bitset<nax>>> q(maxits);
  q[0].push_back(bitset<nax>());
  seen.insert(q[0][0]);

  int tries = 0;
  for (int d = 0; d+1 < maxits; d++) {
    for (bitset<nax>&b : q[d]) {
      for (auto&[ga,ba] : good) {
	if (tries++ > 1e7) return 0;
	if ((ba&~b).any()) continue;
	bitset<nax> toadd = ga|b;
	if (toadd.count() == M) return 1;

	if (seen.count(toadd)) continue;
	q[d+1].push_back(toadd);
	seen.insert(toadd);
      }
    }
  }
  return 0;
}


int scorePieces(const vector<Piece>&piece, Image_ test_in, Image_ test_out, const vector<pair<Image,Image>>&train) {

  if (piece.empty() || piece[0].imgs.back().sz != test_out.sz) return 0;

  for (int ti = 0; ti < train.size(); ti++) {
    int A = test_out.mask.size();
    int B = piece[0].imgs[ti].mask.size();
    int M = A+B;

#define scoreMacro(n) if (M <= n) { if (scorePiecesCore<n>(piece, train, test_out, A, B, ti)) return 1; continue; }
    scoreMacro(64);
    scoreMacro(128);
    scoreMacro(256);
    scoreMacro(512);
    scoreMacro(1024);
    scoreMacro(1800);
    assert(0);
  }
  return 0;
}
