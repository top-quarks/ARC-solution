#include "precompiled_stl.hpp"
#include <chrono>
using namespace std;
#include "utils.hpp"
#include "read.hpp"
#include "normalize.hpp"
#include "core_functions.hpp"
#include "image_functions.hpp"

#include "brute.hpp"

Functions initFuncs() {
  Functions funcs;

  //Nullary
  //for (int i = 1; i <= 5; i++)
  //  funcs.add([i]() {return Square(i);});
  /*for (int dx = -3; dx <= 3; dx++)
    for (int dy = -3; dy <= 3; dy++)
      if ((dx || dy) && abs(dx)+abs(dy) <= 3)
      funcs.add([dx,dy](){return Pos(dx+dy*10);});*/

  //Pos, Col, Line?

  //Unary
  for (int c = 0; c < 10; c++)
    funcs.add([c](Image_ img) {return filterCol(img, c);});
  funcs.add(getPos);
  funcs.add(getSize);
  funcs.add(hull);
  funcs.add(toOrigin);
  funcs.add(compress);
  funcs.add(Fill);
  funcs.add(interior);
  funcs.add(interior2);
  funcs.add(border);
  funcs.add(center);
  funcs.add(heuristicCut);

  for (int i : {0,1,4})
    funcs.add([i](Image_ img) {return cutPickMax(img, i);});
  for (int i : {0,1})
    funcs.add([i](Image_ img) {return cutPickMaxes(img, i);});
  for (int i : {0,1,7})
    funcs.add([i](Image_ img) {return splitPickMax(img, i);});
  for (int i : {0,1})
  funcs.add([i](Image_ img) {return splitPickMaxes(img, i);});

  //invert is filterCol(img, 0)
  for (int c = 0; c < 10; c++)
    funcs.add([c](Image_ img) {return colShape(img, c);});
  for (int i = 1; i < 8; i++)
    funcs.add([i](Image_ img) {return rigid(img, i);});
  for (int a = 0; a < 3; a++)
    for (int b = 0; b < 3; b++)
      funcs.add([a,b](Image_ img) {return count(img, a, b);});
  for (int i = 0; i < 7; i++)
    funcs.add([i](Image_ img) {return smear(img, i);});

  //for (int i = 0; i < 14; i++){}

  /*for (int i = 0; i < 4; i++) {
    funcs.add([i](Image_ img) {return cutIndex(img, i);});
    }*/


  //Binary

  funcs.add(embed);
  funcs.add(outerProductIS);
  funcs.add(outerProductSI);
  for (int i = 0; i < 4; i++)
    funcs.add([i](Image_ a, Image_ b) {return compose(a, b, i);});

  funcs.add(Move);
  funcs.add(extend);
  funcs.add(wrap);

  funcs.add([](Image_ a, Image_ b) {return filterCol(a,b);});
  funcs.add([](Image_ a, Image_ b) {return broadcast(a,b);});
  funcs.add([](Image_ a, Image_ b) {return colShape(a,b);});
  funcs.add([](Image_ a, Image_ b) {return smear(a,b,6);});
  funcs.add([](Image_ a, Image_ b) {return align(a,b);});
  funcs.add(replaceCols);
  for (int i = 0; i < 4; i++)
    funcs.add([i](Image_ a, Image_ b) {return myStack(a, b, i);});

  funcs.sz = {(int)funcs.nullary.size(),
	      (int)funcs.unary.size(),
	      (int)funcs.binary.size()};

  static int said = 1;
  if (!said) {
    cout << "Count of functions (nullary, unary, binary): ";
    cout << funcs.nullary.size() << ' ' << funcs.unary.size() << ' ' << funcs.binary.size() << endl;
    said = 1;
  }
  return funcs;
}


struct Node {
  vector<Image> imgs;
  double prob;
  int depth;

  int fi; //function index
  vector<int> args; //node indices of arguments
  ull hash() const {
    ull r = 0;
    for (const Image&img : imgs) {
      r = r*19247121+hashImage(img);
    }
    return r;
  }
};

double now() { return chrono::steady_clock::now().time_since_epoch().count()*1e-9; }

struct DAG {
  vector<Node> tree;
  vector<double> acc;
  int nins;

  Functions funcs;

  mt19937 mrand;
  DAG(int nins_, const Functions&funcs_) {
    mrand.seed(now()*1e9);
    acc = {0};
    nins = nins_;
    funcs = funcs_;
  }

  int dups = 0, tot = 0;
  set<ull> seen;
  int add(const Node&n) {
    ull h = n.hash();
    tot++;
    if (seen.count(h)) {// && 0) {
      dups++;
      return 0;
    }
    seen.insert(h);
    tree.push_back(n);
    acc.push_back(acc.back()+n.prob);
    return 1;
  }

  void calc(Node&node) {
    int ary = node.args.size();
    for (int i = 0; i < nins; i++) {
      Image img;
      if (ary == 0)
	img = funcs.nullary[node.fi]();
      else if (ary == 1)
	img = funcs.unary[node.fi](tree[node.args[0]].imgs[i]);
      else if (ary == 2)
	img = funcs.binary[node.fi](tree[node.args[0]].imgs[i], tree[node.args[1]].imgs[i]);
      else assert(0);
      for (int c : img.mask) assert(c >= 0 && c < 10); //DEBUG
      node.imgs.push_back( move(img) );
    }
  }

  Image recCalcImage(Node&node, Image_ img) {
    map<int,Image> mem;
    mem[0] = img;
    tree.push_back(node);
    function<Image(int ni)> rec = [&](int ni) {
      if (!mem.count(ni)) {
	Node&n = tree[ni];
	int ary = n.args.size();
	if (ary == 0)
	  return mem[ni] = n.imgs[0];
	else if (ary == 1)
	  return mem[ni] = funcs.unary[n.fi](rec(n.args[0]));
	else if (ary == 2)
	  return mem[ni] = funcs.binary[n.fi](rec(n.args[0]),
					      rec(n.args[1]));
	else assert(0);
      }
      return mem[ni];
    };
    Image ans = rec(tree.size()-1);
    tree.pop_back();
    return ans;
  }

  vector<pair<vector<Image>,double>> calcAll(const vector<Image>&base) {
    int N = base.size();

    vector<pair<vector<Image>,double>> ret;
    ret.emplace_back(move(base), tree[0].prob);
    for (int ni = 1; ni < tree.size(); ni++) {
      Node&n = tree[ni];
      int ary = n.args.size();
      ret.emplace_back(vector<Image>(N), n.prob);
      for (int i = 0; i < N; i++) {
	auto&cur = ret.back().first;
	if (ary == 0)
	  cur[i] = n.imgs[0];
	else if (ary == 1)
	  cur[i] = funcs.unary[n.fi](ret[n.args[0]].first[i]);
	else if (ary == 2)
	  cur[i] = funcs.binary[n.fi](ret[n.args[0]].first[i],
					     ret[n.args[1]].first[i]);
	else assert(0);
      }
    }
    return move(ret);
  }

  vector<int> sampleArgs(int ary) {
    vector<int> args;
    for (int i = 0; i < ary; i++) {
      double unif = mrand()* acc.back()/mrand.max();
      int p = upper_bound(acc.begin(), acc.end(), unif)-acc.begin()-1;
      args.push_back(p);
    }
    return args;
  }
  int sampleFI(int ary) {
    int fis = funcs.nullary.size();
    if (ary == 1) fis = funcs.unary.size();
    else if (ary == 2) fis = funcs.binary.size();
    return mrand()%fis;
  }
};






void generateDijkstra(Image test_in, vector<pair<Image,Image>> train, int keepi, int knowi, const Functions&funcs, double maxt, vector<Piece>&pieces) {

  double W[3] = {1./funcs.nullary.size(),
		 1./funcs.unary.size(),
		 0.5/funcs.binary.size()};
  double knowW = .1;

  double start_time = now();

  //assert(train.size() == 2);

  vector<Image> targets = {train[keepi].second};//, train[1].second};
  Image given = knowi >= 0 ? train[knowi].second : badImg;


  const int nins = 1;
  DAG dag(nins, funcs);

  priority_queue<pair<double,vector<int>>> pq;
  pq.emplace(W[0], vector<int>{});

  auto add = [&](Node n) {
    assert(n.imgs.size() == nins);

    int ni = (int)dag.tree.size();
    if (!dag.add(n)) return;

    pq.emplace(W[1] * n.prob, vector<int>{ni});
    pq.emplace(W[2] * n.prob * dag.tree[0].prob, vector<int>{0, ni});
  };


  auto tryAdd = [&](int fi, const vector<int>&args) {
    int ary = args.size();
    assert(ary <= 2);

    Node node;
    node.fi = fi;
    node.args = args;
    dag.calc(node);

    int bad = 0;
    for (Image_ img : node.imgs) {
      if (img.w > MAXSIDE || img.h > MAXSIDE || img.w*img.h > MAXAREA || img.w*img.h == 0) {
	bad = 1;
      }
    }
    if (bad) return;

    if (ary == 0) {
      node.prob = W[0];
      node.depth = 1;
    } else if (ary == 1) {
      node.prob = dag.tree[args[0]].prob * W[1];
      node.depth = dag.tree[args[0]].depth+1;
    } else if (ary == 2) {
      node.prob = dag.tree[args[0]].prob * dag.tree[args[1]].prob * W[2];
      node.depth = max(dag.tree[args[0]].depth, dag.tree[args[1]].depth)+1;
    }

    //if (node.depth < 4)
    add(node);
  };


  add(Node{{train[keepi].first}, 1, 0,  -1,{}});
  add(Node{{given}, knowW, 1,  -1,{}});

  if (core::countCols(train[keepi].first) < 5) {
    for (int c = 0; c < 10; c++) {
      Node node;
      node.depth = 0;
      node.prob = .2;
      node.fi = c;
      node.args = {0};
      dag.calc(node);
      add(node);
    }
  }

  int iters = 0;
  while (now() - start_time < maxt && dag.tree.size() < 1000000) {
    if (pq.empty()) break;
    //assert(pq.size());
    auto [prob, args] = pq.top();
    pq.pop();

    if (args.size() == 2 && args[0] != args[1]) {
      int a = args[0]+1, b = args[1];
      if (a <= b)
	pq.emplace(dag.tree[a].prob * dag.tree[b].prob * W[2], vector<int>{a, b});
    }

    int ary = args.size();
    for (int fi = 0; fi < funcs.sz[ary]; fi++) {
      for (int flip = 0; flip < 2; flip++) {

	tryAdd(fi, args);
	iters++;

	if (args.size() < 2 || args[0] == args[1]) break;
	swap(args[0], args[1]);
      }
    }
  }


  {
    vector<Image> inputs;
    for (auto&[in,out] : train)
      inputs.push_back(in);
    inputs.push_back(test_in);

    for (auto&[v,p] : dag.calcAll(inputs)) {
      pieces.push_back({v,p,keepi,knowi});
    }
  }

  //cout << dag.tree.size() << " / " << iters << endl;
}

extern double BRUTE_TIME;

vector<Piece> brutePieces(Image_ test_in, const vector<pair<Image,Image>>&train) {
  Functions funcs = initFuncs();

  vector<Piece> pieces;
  //#warning change tott
  double tott = BRUTE_TIME;
  double maint = tott/2.;
  assert(train.size() >= 2);
  double restt = (tott-maint)/(train.size()+1-1);
  for (int it = 0; it <= train.size(); it++) {
    double maxt = it == 1 ? maint : restt;
    //vector<pair<Image,Image>> train2 = train;
    //swap(train2[it], train2[0]);
    //swap(train2[0], train2[1]);
    //train.resize(2);
    int keepi = it%train.size(), knowi = (it+1)%train.size();
    if (it >= train.size()) knowi = -1;
    generateDijkstra(test_in, train, keepi, knowi, funcs, maxt, pieces);
  }
  return pieces;
}
