#include "precompiled_stl.hpp"
#include <chrono>
using namespace std;
#include "utils.hpp"
#include "read.hpp"
#include "normalize.hpp"
#include "core_functions.hpp"
#include "image_functions.hpp"
#include "image_functions2.hpp"

#include "visu.hpp"

#include "brute2.hpp"
#include "pieces.hpp"

#include "timer.hpp"

extern int MAXDEPTH, print_nodes;

//double build_f_time = 0, apply_f_time = 0;
//double real_f_time = 0;

double now() {
  ll t = chrono::high_resolution_clock::now().time_since_epoch().count();
  static ll time0 = 0;
  if (time0 == 0) time0 = t;
  return (t-time0)*1e-9;
}
//double now() { return chrono::steady_clock::now().time_since_epoch().count()*1e-9;}

Timer build_f_time, apply_f_time, real_f_time, add_time, find_child_time, add_child_time, hash_time, map_time, total_time;
Timer state_time;

void Functions3::add(const string& name, int cost_, const function<bool(const State&,State&)>&func, int list) {
  //if (cost_ != 10) cout << name << endl;
  //assert(cost_ == 10);
  if (list) listed.push_back(names.size());
  names.push_back(name);
  f_list.push_back(func);
  cost.push_back(cost_);
}

void Functions3::add(string name, int cost, const function<Image(Image_)>&f, int list) { //list = 1
  auto func = [f](const State& cur, State& nxt) {

    //if (cur.isvec) return false;

    nxt.vimg.resize(cur.vimg.size());
    nxt.isvec = cur.isvec;

    int area = 0;
    for (int i = 0; i < cur.vimg.size(); i++) {
      real_f_time.start();
      nxt.vimg[i] = f(cur.vimg[i]);
      real_f_time.stop();

      area += nxt.vimg[i].w*nxt.vimg[i].h;
      if (area > MAXPIXELS) return false;
    }
    return true;
  };
  add(name, cost, func, list);
}

void Functions3::add(string name, int cost, const function<vImage(Image_)>&f, int list) { //list = 1
  const int buffer = 5;
  auto func = [f,cost](const State& cur, State& nxt) {
    if (cur.isvec || cur.depth+cost+buffer > MAXDEPTH) return false;
    real_f_time.start();
    nxt.vimg = f(cur.vimg[0]);
    real_f_time.stop();
    nxt.isvec = true;
    return true;
  };
  add(name, cost, func, list);
}

void Functions3::add(string name, int cost, const function<Image(vImage_)>&f, int list) { //list = 1
  auto func = [f](const State& cur, State& nxt) {
    if (!cur.isvec) return false;
    nxt.vimg.resize(1);
    real_f_time.start();
    nxt.vimg[0] = f(cur.vimg);
    real_f_time.stop();
    nxt.isvec = false;
    return true;
  };
  add(name, cost, func, list);
}

void Functions3::add(string name, int cost, const function<vImage(vImage_)>&f, int list) { //list = 1
  auto func = [f](const State& cur, State& nxt) {
    if (!cur.isvec) return false;
    real_f_time.start();
    nxt.vimg = f(cur.vimg);
    real_f_time.stop();
    nxt.isvec = true;
    return true;
  };
  add(name, cost, func, list);
}

void Functions3::add(const vector<point>&sizes, string name, int cost, const function<Image(Image_,Image_)>&f, int list) { //list = 1
  int szi = 0;
  for (point sz : sizes) {
    Image arg2 = core::empty(sz);
    auto func = [f,arg2](const State& cur, State& nxt) {

      if (cur.isvec) return false;

      nxt.vimg.resize(cur.vimg.size());

      int area = 0;
      for (int i = 0; i < cur.vimg.size(); i++) {
	real_f_time.start();
	nxt.vimg[i] = f(cur.vimg[i], arg2);
	real_f_time.stop();

	area += nxt.vimg[i].w*nxt.vimg[i].h;
	if (area > MAXPIXELS) return false;
      }
      nxt.isvec = cur.isvec;
      return true;
    };
    add(name+" "+to_string(szi++), cost, func, list);
  }
}

string Functions3::getName(int fi) {
  assert(fi >= 0 && fi < names.size());
  return names[fi];
}
int Functions3::findfi(string name) {
  int fi = find(names.begin(), names.end(), name)-names.begin();
  if (fi == names.size()) {
    cerr << name << " is not a known function" << endl;
    assert(0);
  }
  return fi;
}


Functions3 initFuncs3(const vector<point>&sizes) {
  Functions3 funcs;

  // Unary

  //invert is filterCol(img, 0)
  for (int c = 0; c < 10; c++)
    funcs.add("filterCol "+to_string(c), 10, [c](Image_ img) {return filterCol(img, c);});
  for (int c = 1; c < 10; c++)
    funcs.add("eraseCol "+to_string(c), 10,
	      [c](Image_ img) {return eraseCol(img, c);});

  for (int c = 1; c < 10; c++)
    funcs.add("colShape "+to_string(c), 10,
	      [c](Image_ img) {return colShape(img, c);}, 0);

  funcs.add("compress", 10, [](Image_ img) {return compress(img);});
  funcs.add("getPos", 10, getPos);
  funcs.add("getSize0", 10, getSize0);
  funcs.add("getSize", 10, getSize);
  funcs.add("hull0", 10, hull0);
  funcs.add("hull", 10, hull);
  funcs.add("toOrigin", 10, toOrigin);
  funcs.add("Fill", 10, Fill);
  funcs.add("interior", 10, interior);
  funcs.add("interior2", 10, interior2);
  funcs.add("border", 10, border);
  funcs.add("center", 10, center);
  funcs.add("majCol", 10, majCol);

  //funcs.add("greedyFillBlack", 10, [](Image_ img) {return greedyFillBlack(img);});
  //funcs.add("greedyFillBlack2", 10, [](Image_ img) {return greedyFillBlack2(img);});

  for (int i = 1; i < 9; i++)
    funcs.add("rigid "+to_string(i), 10,
	      [i](Image_ img) {return rigid(img, i);});
  for (int a = 0; a < 3; a++)
    for (int b = 0; b < 3; b++)
      funcs.add("count "+to_string(a)+" "+to_string(b), 10,
		[a,b](Image_ img) {return count(img, a, b);});
  for (int i = 0; i < 15; i++)
    funcs.add("smear "+to_string(i), 10,
	      [i](Image_ img) {return smear(img, i);});


  funcs.add("makeBorder", 10,
	    [](Image_ img) {return makeBorder(img, 1);});

  for (int id : {0,1})
    funcs.add("makeBorder2 "+to_string(id), 10,
	      [id](Image_ img) {return makeBorder2(img, id);});
  funcs.add("compress2", 10, compress2);
  funcs.add("compress3", 10, compress3);

  for (int id = 0; id < 3; id++)
    funcs.add("connect "+to_string(id), 10,
	      [id](Image_ img) {return connect(img,id);});

  for (int id : {0,1})
    funcs.add("spreadCols "+to_string(id), 10,
	      [id](Image_ img) {return spreadCols(img, id);});

  for (int id = 0; id < 4; id++)
    funcs.add("half "+to_string(id), 10,
	      [id](Image_ img) {return half(img, id);});


  for (int dy = -2; dy <= 2; dy++) {
    for (int dx = -2; dx <= 2; dx++) {
      funcs.add("Move "+to_string(dx)+" "+to_string(dy), 10,
		[dx,dy](Image_ img) {return Move(img, Pos(dx,dy));}, 0);
    }
  }

  // Binary
  funcs.add(sizes, "embed", 10, embed);
  funcs.add(sizes, "wrap", 10, wrap);
  funcs.add(sizes, "broadcast", 10, [](Image_ a, Image_ b) {return broadcast(a,b);});
  funcs.add(sizes, "repeat 0",  10, [](Image_ a, Image_ b) {return repeat(a,b);});
  funcs.add(sizes, "repeat 1",  10, [](Image_ a, Image_ b) {return repeat(a,b,1);});
  funcs.add(sizes, "mirror 0",  10, [](Image_ a, Image_ b) {return mirror(a,b);});
  funcs.add(sizes, "mirror 1",  10, [](Image_ a, Image_ b) {return mirror(a,b,1);});


  //Split
  funcs.add("cut",       10, [](Image_ img) {return cut(img);});
  funcs.add("splitCols", 10, [](Image_ img) {return splitCols(img);});
  funcs.add("splitAll",     10, splitAll);
  funcs.add("splitColumns", 10, splitColumns);
  funcs.add("splitRows",    10, splitRows);
  funcs.add("insideMarked", 10, insideMarked);
  for (int id = 0; id < 4; id++)
    funcs.add("gravity "+to_string(id), 10,
	      [id](Image_ img) {return gravity(img,id);});


  //Join
  for (int id = 0; id < 14; id++)
    funcs.add("pickMax "+to_string(id), 10,
	      [id](vImage_ v) {return pickMax(v,id);});
  for (int id = 0; id < 1; id++)
    funcs.add("pickUnique "+to_string(id), 10,
	      [id](vImage_ v) {return pickUnique(v,id);});

  funcs.add("composeGrowing", 10, composeGrowing);
  funcs.add("stackLine", 10, stackLine);
  for (int id = 0; id < 2; id++) //consider going to 4
    funcs.add("myStack "+to_string(id), 10,
	      [id](vImage_ v) {return myStack(v,id);}); //


  //Vector
  for (int id = 0; id < 14; id++)
    funcs.add("pickMaxes "+to_string(id), 10,
	      [id](vImage_ v) {return pickMaxes(v,id);});
  for (int id = 0; id < 14; id++)
    funcs.add("pickNotMaxes "+to_string(id), 10,
	      [id](vImage_ v) {return pickNotMaxes(v,id);});


  static int said = 0;
  if (!said) {
    cout << "Function count: " << funcs.listed.size() << endl;
    said = 1;
  }


  //funcs.add("smear",    [](Image_ a, Image_ b) {return smear(a,b,6);});

  //funcs.add(insideMarked); //only do once at depth 0

  // Smear diagonals?

  // outerProducts

  //for (int id = 0; id < 4; id++)
  //  funcs.add([id](Image_ img) {return gravity(img, id);});

  // Image makeBorder(Image_ img, int bcol = 1);
  // Image makeBorder2(Image_ img, Image_ bord);
  // Image greedyFillBlack(Image_ img, int N = 3);
  // Image extend2(Image_ img, Image_ room);
  // Image replaceTemplate(Image_ in, Image_ need_, Image_ marked_, int overlapping = 0, int rigids = 0);
  // Image swapTemplate(Image_ in, Image_ a, Image_ b, int rigids = 0);

  // funcs.add("heuristicCut", heuristicCut);

  return funcs;
}

Image DAG::getImg(int ni) {
  return tiny_node.getImg(ni);
  //assert(tiny_node.getImg(ni) == node[ni].vimg[0]);
  //return node[ni].state.vimg[0];
  /*
  //cout << nodei << endl;
  assert(nodei >= 0 && nodei < (int)node.size());
  assert(!node[nodei].isvec);
  if (node[nodei].pfi == embed1fi) {
    assert(funcs.f_list[embed1fi](node[nodei], tmp_node));
    //assert(tmp_node.vimg == node[nodei].vimg);
    return tmp_node.vimg[0];
  } else {
    assert(node[nodei].vimg.size());
    return node[nodei].vimg[0];
    }*/
}


int DAG::add(const State&nxt, bool force) { //force = false
  hash_time.start();
  ull h = nxt.hash();
  hash_time.stop();
  int nodes = tiny_node.size();
  map_time.start();
  auto [nodei,inserted] = hashi.insert(h,nodes);
  //auto [it,inserted] = hashi.insert({h,nodes}); int nodei = it->second;
  //assert(inserted == tiny_inserted);
  //assert(nodei == tiny_nodei);

  map_time.stop();

  add_time.start();
  if (inserted || force) {
    bool ispiece = !nxt.isvec;
    if (!nxt.isvec && target_size != point{-1,-1})
      ispiece &= (nxt.vimg[0].p == point{0,0} && nxt.vimg[0].sz == target_size);
    /*{
      Node n;
      n.state = nxt;
      n.ispiece = ispiece;
      n.freed = false;
      node.push_back(n);
      }*/
    tiny_node.append(nxt, ispiece);
  }
  add_time.stop();
  return nodei;
}


void DAG::build() {
  build_f_time.start();

  for (int curi = 0; curi < tiny_node.size(); curi++) {
    int depth = tiny_node[curi].depth;
    if (depth+1 > MAXDEPTH) continue;

    //vector<pair<int,int>> child;
    State nxt;
    state_time.start();
    State cur_state = tiny_node.getState(curi);
    state_time.stop();
    for (int fi : funcs.listed) {
      nxt.depth = depth+funcs.cost[fi];
      if (nxt.depth > MAXDEPTH) continue;
      if (funcs.f_list[fi](cur_state, nxt)) {
	int newi = add(nxt);
	//child.emplace_back(fi, newi);
	tiny_node.addChild(curi, fi, newi);
      } else {
	tiny_node.addChild(curi, fi, -1);
	//child.emplace_back(fi, -1);
      }

    }
    //node[curi].child = child;
  }

  build_f_time.stop();
}

void DAG::initial(Image_ test_in, const vector<pair<Image,Image>>&train, vector<point> sizes, int ti) {
  if (sizes.size() > 1)
    target_size = sizes[1];
  else
    target_size = point{-1,-1};

  Image in = ti < train.size() ? train[ti].first : test_in;

  add(State({in}, false, 0), true);

  //Output sizes
  for (point sz : sizes)
    add(State({core::empty(sz)}, false, 10), true);

  // Outputs of other trains
  for (int tj = 0; tj < train.size(); tj++)
    add(State({ti != tj ? train[tj].second : core::empty(train[tj].second.sz)}, false, 10), true);

  //add(State({greedyFillBlack2(in)}, false, 10), true);

  //filterCol?

  givens = tiny_node.size();
}

//time each function
void DAG::benchmark() {
  vector<pair<double,int>> v;
  for (int fi : funcs.listed) {
    double start_time = now();
    State nxt;
    for (int i = 0; i < tiny_node.size(); i++) {
      funcs.f_list[fi](tiny_node.getState(i), nxt);
    }
    double elapsed = now()-start_time;
    v.emplace_back(elapsed, fi);
  }
  sort(v.begin(), v.end());
  for (auto [t,fi] : v)
    printf("%.1f ms - %s\n", t*1e3, funcs.getName(fi).c_str());
}

int DAG::applyFunc(int curi, int fi, const State&state) {

  find_child_time.start();
  //auto it = lower_bound(node[curi].child.begin(), node[curi].child.end(), make_pair(fi,-1));
  int it2 = tiny_node.getChild(curi, fi);
  find_child_time.stop();
  if (it2 != TinyChildren::None) {//it != node[curi].child.end() && it->first == fi) {
    //if (it2 != it->second) cout << it2 << ' ' << it->second << endl;
    //assert(it2 == it->second);
    return it2;//it->second;
  }
  //assert(it2 == -2);


  State nxt;
  nxt.depth = tiny_node[curi].depth+funcs.cost[fi];
  //nxt.par = curi;

  int newi = -1;

  apply_f_time.start();
  bool ok = funcs.f_list[fi](state, nxt);
  apply_f_time.stop();

  if (ok) {
    //nxt.pfi = fi;
    newi = add(nxt);
  }

  add_child_time.start();
  tiny_node.addChild(curi, fi, newi);
  //node[curi].child.emplace_back(fi, newi);
  //sort(node[curi].child.begin(), node[curi].child.end());
  add_child_time.stop();
  return newi;
}

int DAG::applyFunc(int curi, int fi) {
  state_time.start();
  State state = tiny_node.getState(curi);
  state_time.stop();
  return applyFunc(curi, fi, state);
}

void DAG::applyFunc(string name, bool vec) {
  int fi = funcs.findfi(name);

  int start_nodes = tiny_node.size();
  for (int curi = 0; curi < start_nodes; curi++) {
    if (tiny_node[curi].isvec == vec) applyFunc(curi, fi);
  }
}

void DAG::applyFuncs(vector<pair<string,int>> names, bool vec) {
  vector<pair<int,int>> fis;
  for (auto [name,id] : names) {
    fis.emplace_back(funcs.findfi(name), id);
  }

  vector<int> parid(tiny_node.size(),-1);
  for (int curi = 0; curi < tiny_node.size(); curi++) {
    if (tiny_node[curi].isvec != vec) continue;
    state_time.start();
    State state = tiny_node.getState(curi);
    state_time.stop();

    for (int i = 0; i < fis.size(); i++) {
      auto [fi,id] = fis[i];
      if (id <= parid[curi]) continue;

      if (applyFunc(curi, fi, state) == parid.size()) {
	parid.push_back(id);
	assert(parid.size() == tiny_node.size());
      }
    }
  }
}





void DAG::buildBinary() {
  int fis = *max_element(funcs.listed.begin(), funcs.listed.end())+1;
  binary.assign(fis*fis, -1);
  vector<State> state(fis);
  vector<int> active(fis), memi(fis);
  for (int fi : funcs.listed) {
    int curi = tiny_node.getChild(0, fi);
    if (curi >= 0) {
      active[fi] = 1;
      state[fi] = tiny_node.getState(curi);
      memi[fi] = curi;
    }
  }
  for (int fa : funcs.listed) {
    if (!active[fa]) continue;
    for (int fb : funcs.listed) {
      if (!active[fb]) continue;

      if (state[fa].isvec || state[fb].isvec) continue;
      State nxt;
      nxt.vimg = {align(state[fa].vimg[0], state[fb].vimg[0])};
      nxt.depth = 2; //TODO
      nxt.isvec = false;
      binary[fa*fis+fb] = add(nxt);
      /*if (binary[fa*fis+fb] != memi[fa]) {
	cout << binary[fa*fis+fb] << ' ' << memi[fa] << ' ' << tiny_node.size() << endl;
      }
      assert(binary[fa*fis+fb] == memi[fa]);*/
    }
  }
}



vector<DAG> brutePieces2(Image_ test_in, const vector<pair<Image,Image>>&train, vector<point> out_sizes) {
  int print = 1;

  vector<DAG> dag(train.size()+1);

  int all_train_out_mask = 0, and_train_out_mask = ~0;
  for (int ti = 0; ti < train.size(); ti++)
    and_train_out_mask &= core::colMask(train[ti].second);

  for (int ti = 0; ti <= train.size(); ti++) {
    vector<point> sizes;
    if (ti < train.size())
      sizes.push_back(train[ti].first.sz);
    else
      sizes.push_back(test_in.sz);

    if (out_sizes.size())
      sizes.push_back(out_sizes[ti]);

    dag[ti].funcs = initFuncs3(sizes);

    dag[ti].initial(test_in, train, sizes, ti);

    total_time.start();

    double start_time = now();
    dag[ti].build();
    if (print) cout << now()-start_time << endl;
    //dag[ti].buildBinary();
    //if (print) cout << now()-start_time << endl;
    dag[ti].applyFunc("composeGrowing", 1);
    if (print) cout << now()-start_time << endl;

    if (sizes.size() > 1) {
      vector<pair<string,int>> toapply;
      toapply.emplace_back("toOrigin",0);
      for (int c = 1; c <= 5; c++)
	if (and_train_out_mask>>c&1)
	  toapply.emplace_back("colShape "+to_string(c),1);
      toapply.emplace_back("embed 1",2);
      dag[ti].applyFuncs(toapply, 0);
      /*
      dag[ti].applyFunc("toOrigin", 0);
      if (print) cout << now()-start_time << endl;

      int mask;
      if (ti < train.size()) {
	mask = core::colMask(train[ti].second);
	all_train_out_mask |= mask;
      } else {
	mask = all_train_out_mask;
      }

      for (int c = 1; c <= 5; c++)
	if (and_train_out_mask>>c&1)
	dag[ti].applyFunc("colShape "+to_string(c), 0);
      if (print) cout << now()-start_time << endl;

      if (ti < train.size())
	deducePositions(dag[ti], train[ti].second);
      if (print) cout << now()-start_time << endl;

      dag[ti].applyFunc("embed 1", 0);
      */
      if (print) cout << now()-start_time << endl;

      /*if (ti < train.size())
	deducePositions(dag[ti], train[ti].second);

	if (print) cout << now()-start_time << endl;*/

      total_time.stop();
      total_time.print("Total time");
      build_f_time.print("Build f time");
      apply_f_time.print("Apply f time");
      real_f_time .print("Real f time ");

      add_time.print("Add time");
      find_child_time.print("Find child");
      add_child_time.print("Add child");
      hash_time.print("Hash");
      map_time.print("Map");

      state_time.print("getState");
      //exit(0);
      /*FILE*fp = fopen("images.txt", "w");
      for (Node&n : dag[ti].node) {
	for (Image_ img : n.vimg) {
	  fprintf(fp, "%d %d %d %d\n", img.x, img.y, img.w, img.h);
	  for (char c : img.mask)
	    fprintf(fp, "%c", '0'+c);
	  fprintf(fp, "\n");
	}
      }
      exit(0);*/
      //dag[ti].freeAll();
    } else total_time.stop();
    //dag[ti].benchmark();
    //exit(0);

    /*
    for (Node&n : dag[ti].node) {
      if (!n.isvec && n.pfi == dag[ti].embed1fi) {
	n.vimg.clear();
	n.vimg.shrink_to_fit();
      }
    }
    */

    /*if (ti < train.size()) {
      for (Node&n : dag[ti].node) {
	if (n.par > -1 && (n.isvec || n.img[0].w > 30 || n.img[0].h > 30)) {// || n.img[0].p != point{0,0} || n.img[0].sz != given_sizes[ti][1])) {
	  n.img.clear();
	  n.img.shrink_to_fit();
	}
      }
      dag[ti].hashi.clear();
      }*/
  }


  if (out_sizes.size() && print_nodes) {
    cout << "Dag sizes: ";
    for (int i = 0; i < dag.size(); i++)
      cout << dag[i].tiny_node.size() << " ";
    cout << endl;
  }

  return dag;
}
