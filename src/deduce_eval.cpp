#include "precompiled_stl.hpp"
using namespace std;
#include "utils.hpp"
#include "read.hpp"
#include "visu.hpp"
#include "core_functions.hpp"
#include "image_functions.hpp"
#include "image_functions2.hpp"
#include "normalize.hpp"
#include "spec.hpp"


#include <execinfo.h>
string track(void*funptr) {
  string r(*backtrace_symbols(&funptr, 1));
  regex re("_Z[0-9]{1,3}([a-zA-Z[0-9]+?)RK5");
  smatch match;
  if (regex_search(r, match, re) && match.size() > 1) {
    return match.str(1);
  }
  return r;
}

void print(Spec s) {
  cout << endl;
  cout << "Spec" << endl;
  if (s.bad) {
    cout << "Bad!" << endl;
    cout << endl;
    return;
  }

  if (s.anypos)
    cout << "Pos: * (" << s.x << ", " << s.y << ")" << endl;
  else
    cout << "Pos: " << s.x << ", " << s.y << endl;

  cout << "Size: " << s.w << " x " << s.h << endl;
  for (int i = 0; i < s.h; i++) {
    for (int j = 0; j < s.w; j++) {
      int m = s(i,j);
      if (m == 0)
	cout << 0;
      if (m == allCols)
	cout << "*";
      else if (m == (allCols&~1))
	cout << "@";
      else if ((m&(m-1)) == 0) {
	for (int c = 0; c < 10; c++)
	  if (m == 1<<c)
	    cout << c;
      } else {
	cout << "?";
      }
    }
    cout << endl;
  }
}

struct Rec {
  SpecRec data;
  Spec target;
  Image given;
  Rec*par;
  string fname;
  Rec(SpecRec(*f)(Image_, Spec_), Image_ given_, Spec_ target_) {
    fname = track((void*)f);
    par = NULL;
    given = given_;
    target = target_;
    if (target.bad) {
      cerr << "Bad target given to " << fname << endl;
      exit(1);
    }
    data = f(given, target);
    if (data.first.bad) {
      cerr << "Target can't be satisfied by " << fname << endl;
      print(given);
      print(target);
      exit(1);
    }
  }
  Rec(SpecRec(*f)(Image_, Spec_), Image_ given, Rec& target) : Rec(f,given,(Spec_)target) {
    par = &target;
  }
  operator Spec() const { return data.first; }
  Image operator()(Image_ found) {
    if (!data.first.check(found)) {
      cerr << "Argument to " << fname << " doesn't satisfy spec" << endl;
      print(found);
      print(data.first);
      exit(1);
    }
    Image ret = data.second(given, found);
    if (!target.check(ret)) {
      cerr << "Reconstruction from " << fname << " doesn't satisfy spec" << endl;
      print(ret);
      print(target);
      exit(1);
    }
    cout << "YAY" << endl;
    if (par == NULL) return ret;
    else return (*par)(ret);
  }
};


Image deduceEval(Image in, vector<pair<Image,Image>> train, int taski, Image target_img) {
  Simplifier sim = normalizeCols(train);
  Image in0 = in;

  Spec target = fromImage(target_img);

  if (taski == 5) {
    in = sim.in(in);

    target = fromImage(sim.out(in0,target_img));

    Image dot = compress(filterCol(in, 1));
    Image part = myStack(dot,Col(0),2);
    Image front = filterCol(in, 2);

    Rec grid(iComposeFirst, front, target);
    Rec sz(iRepeatSecond, part, grid);
    return sim.rec(in0, sz(in));

    /*Image out = repeat(dot, in, 1);
    out = compose(out, front);
    return sim.rec(in0, out);*/

  } else if (taski == 7) {
    Image out = outerProductSI(Square(3), in);
    Image lines = colShape(smear(out, 4), 1);
    Image x = colShape(Move(out,Pos( 1, 1)), 3);
    Image y = colShape(Move(out,Pos(-1,-1)), 3);

    Rec a(iComposeFirst, out, target);
    Rec b(iComposeFirst, lines, a);
    Rec c(iCompose2First, x, b);
    Rec d(iCompose2First, y, c);
    return d(out);
    /*Image green = colShape(out, 3);
    Image blue = colShape(lines, 1);
    Image a = Move(green, Pos(1,1));
    Image b = Move(green, Pos(1,-1));
    return compose(blue, compose(out, compose(a, b), 3));*/

  } else if (taski == 8) {
    Image squares = interior(filterCol(in,1));
    Image light = compose(pickNotMaxes(cut(in), 11),0);
    Image border = makeBorder(filterCol(in,1), 2);

    //return compose({in, border, colShape(light,8), colShape(squares,6)}, 0);
  } else if (taski == 10) {
    Image sz = outerProductIS(in,count(Square(5),4,1));

    //return mirror(in, sz);

  } else if (taski == 21) {
    Image parts = connect(in, 0);
    parts = smear(parts, 3);
    Image bottom = embed(pickMax(splitRows(in),6),in);
    bottom = smear(bottom,3);

    //parts = compose(parts,bottom,2);
    //return compose(in, compose(bottom, parts));

  } else if (taski == 22) {
    Image mask = hull(compress(filterCol(in,4)));

    /*Image marked = compose(in, mask, 2);
    Image need = eraseCol(marked, 4);
    return replaceTemplate(in, need, marked);*/
  } else if (taski == 3) {
    Image a = eraseCol(cutIndex(in,0), 2);
    Image b = cutIndex(in,1);
    Image c = cutIndex(in,2);

    /*Image out = broadcast(b,c);
    out = replaceTemplate(out,hull(a),a);
    return compose(c, out);*/

  } else if (taski == 4) {
    Image part = rigid(cutPickMax(in, 4), 4);

    //return replaceTemplate(in, invert(part), part);
  }

  return badImg;
}

void deduceEvals() {
  vector<Sample> sample = readAll("evaluation", -1);
  int samples = sample.size();
  sample = vector<Sample>(sample.begin()+samples-100,sample.end());

  sample[7].train.erase(sample[7].train.begin()+1);

  Visu visu;

  int corrects = 0;
  for (int si = 0; si < sample.size(); si++) {
    Sample&s = sample[si];
    visu.next(to_string(si));
    int ok = 1;
    for (auto [in,out] : s.train) {
      Image pred = toOrigin(deduceEval(in, s.train, si, out));
      ok &= (pred == out);
      visu.add(in, out);
      visu.add(in, pred);
    }
    corrects += ok;
    cout << "Task " << si << ": " << (ok ? "OK" : "Failed") << endl;
  }
  cout << corrects << " / " << sample.size() << endl;
  exit(0);
}
