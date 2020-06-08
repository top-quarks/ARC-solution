#include "precompiled_stl.hpp"
using namespace std;
#include "utils.hpp"
#include "read.hpp"
#include "visu.hpp"
#include "core_functions.hpp"
#include "image_functions.hpp"
#include "normalize.hpp"
#include "spec.hpp"
#include "image_functions2.hpp"



int rcDiff(Image_ img) {
  vector<int> row(img.h,1), col(img.w,1);
  for (int i = 0; i < img.h; i++)
    for (int j = 0; j < img.w; j++)
      if (img(i,j) == 0) row[i] = col[j] = 0;
  int ans = 0;
  for (int i = 0; i < img.h; i++)
    ans += row[i];
  for (int j = 0; j < img.w; j++)
    ans -= col[j];
  return ans;
}


Simplifier normalizeOrient() {
  Simplifier sim;
  sim.in = [](Image_ in) {
    return rcDiff(in) >= 0 ? in : rigid(in,6);
  };
  sim.out = [](Image_ in, Image_ out) {
    return rcDiff(in) >= 0 ? out : rigid(out,6);
  };
  sim.rec = [](Image_ in, Image_ out) {
    return rcDiff(in) >= 0 ? out : rigid(out,6);
  };
  return sim;
}


#define VECIFY(func)				\
  vImage func(vImage_ vimg) {			\
    vImage ret(vimg.size());			\
    for (int i = 0; i < vimg.size(); i++) 	\
      ret[i] = func(vimg[i]);			\
    return ret;					\
  }
VECIFY(hull);

#define FOREACH(content)			\
  vImage ret(vi.size());			\
  for (int i = 0; i < vi.size(); i++)		\
    ret[i] = content;				\
  return ret;

vImage count(vImage_ vi, int a, int b) {
  FOREACH(count(vi[i],a,b));
}





Image solveEval(Image in, vector<pair<Image,Image>> train, int taski) {
  //Skipped  23,40,53,58,90
  //Error in train  7, 27, 44/45, 77
  auto sim = normalizeCols(train);
  auto in0 = in;

  if (taski == 0) {
    vImage out = gravity(in, 0);
    return embed(composeGrowing(out),in);

  } else if (taski == 1) {
    in = sim.in(in);

    vImage inside = insideMarked(in);
    Image out = pickMax(inside,0);
    return sim.rec(in0,out);

  } else if (taski == 2) {
    in = eraseCol(in, 2);
    return greedyFillBlack(in);

  } else if (taski == 3) {
    Image a = eraseCol(cutIndex(in,0), 2);
    Image b = cutIndex(in,1);
    Image c = cutIndex(in,2);
    Image out = broadcast(b,c);
    out = replaceTemplate(out,hull(a),a);
    return compose(c, out);

  } else if (taski == 4) {
    Image part = rigid(cutPickMax(in, 4), 4);
    return replaceTemplate(in, invert(part), part);

  } else if (taski == 5) {
    in = sim.in(in);

    Image dot = compress(filterCol(in, 1));
    Image out = repeat(dot, in, 1);
    return sim.rec(in0, compose(out, filterCol(in, 2)));

  } else if (taski == 6) {
    Simplifier orient = normalizeOrient();
    in = orient.in(in);
    Image base = colShape(interior(filterCol(in, 3)),1);
    base = smear(base, 5);
    Image cross = compose(filterCol(in,2), base, 2);
    cross = makeBorder(cross);
    Image out = compose(compose(compose(base, in), cross), filterCol(in,3));
    return orient.rec(in0, out);

  } else if (taski == 7) {
    Image out = repeat(in, core::full({in.w*3,in.h*3}));
    Image green = colShape(out, 3);
    Image blue = colShape(smear(out, 4), 1);
    Image a = Move(green, Pos(1,1));
    Image b = Move(green, Pos(-1,-1));
    return compose(blue, compose(out, compose(a, b), 3));

  } else if (taski == 8) {
    in = sim.in(in);
    Image squares = interior(filterCol(in,1));
    Image light = composeGrowing(pickNotMaxes(cut(in), 11));
    Image border = makeBorder(filterCol(in,1));
    Image out = composeGrowing({in, colShape(border,2), colShape(light,5), colShape(squares,4)});
    return sim.rec(in0,out);

  } else if (taski == 9) {
    Image out = in;
    for (Image_ img : cut(in)) {
      Image dot = embed(img, out);
      out = compose(smear(dot, img.x < in.w-img.x ? 1 : 0), out);
      out = compose(smear(dot, img.y < in.h-img.y ? 3 : 2), out);
    }
    return out;

  } else if (taski == 10) {
    Image sz = train[0].second;//core::full(in.w*5,in.h);
    return mirror(in, sz);

  } else if (taski == 11) {
    return spreadCols(in,1);

  } else if (taski == 12) {
    in = sim.in(in);

    Image swapcol = compose(colShape(filterCol(in,1),3),
			    colShape(filterCol(in,3),1));

    Image mirror = align(rigid(in, 8), in);
    Image out = compose(swapcol, mirror, 3);
    return sim.rec(in0, out);

  } else if (taski == 13) {
    return greedyFillBlack(in);

  } else if (taski == 14) {
    Image out = badImg;
    for (Image img : splitCols(in)) {
      img = compress(img);
      Image add = border(count(img,5,0));
      out = compose(out, align(add, out, 2, 2));
    }
    return out;

  } else if (taski == 15) {
    in = eraseCol(in, 8);
    return compress3(compress2(in));

  } else if (taski == 16) {
    Image out = in;
    Image pat = cutIndex(train[0].second,0);
    pat = spreadCols(eraseCol(pat,4));
    for (Image_ img : cut(in)) {
      Image base = broadcast(pat,img);
      base = compose(base, colShape(interior2(interior2(img)), 4));
      out = compose(out, base);
    }
    return out;

  } else if (taski == 17) {
    in = sim.in(in);
    point ins = compress(interior(filterCol(in, 2))).sz;
    int borders = ins.x == ins.y ? ins.x : 2;
    for (int i = 0; i < borders; i++)
      in = compose(in,makeBorder(in, 1));
    return sim.rec(in0, in);

  } else if (taski == 18) {
    return compress(compose(rigid(in,2),in,4));

  } else if (taski == 19 || taski == 20) {
    in = sim.in(in);
    Image base = sim.out(train[0].first, train[0].second);
    Image out = Move(base, outerProductIS(Move(compress(in), Pos(-1,-1)),Square(4)));
    out = extend2(out, base);
    return sim.rec(in0,out);

  } else if (taski == 21) {
    Image parts = smear(connect(in, 0), 3);
    Image bottom = smear(embed(pickMax(splitRows(in),6),in), 3);
    parts = compose(parts,bottom,2);
    return compose(in, compose(bottom, parts));

  } else if (taski == 22) {
    Image marked = compose(in, hull(compress(filterCol(in,4))), 2);
    Image need = eraseCol(marked, 4);
    return replaceTemplate(in, need, marked);

  } else if (taski == 23) { //Wrong
    in = eraseCol(in, 5);
    Image ret = invert(hull(in));
    vImage s = splitAll(in);
    for (Image_ p : s) {
      for (Image_ q : s) {
	if (p.sz == q.sz && !core::count(p) && core::isRectangle(q)) {
	  ret = compose(ret, align(q,p,2,2));
	}
      }
      if (p.sz != in.sz && core::count(interior(p)))
	ret = compose(ret, p);
    }
    return ret;

  } else if (taski == 24) {
    Image a = myStack(in,rigid(in,1),0);
    Image b = myStack(rigid(in,2),rigid(in,3),0);
    return myStack(a,b,1);

  } else if (taski == 25) {
    in = sim.in(in);
    in = eraseCol(in,1);
    vImage cols = splitCols(in);
    vImage lens = count(cols, 2,1);
    Image out = myStack(lens, 1);
    return sim.rec(in0,out);

  } else if (taski == 26) {
    in = sim.in(in);
    Image out = eraseCol(eraseCol(in,1),2);
    out = connect(out,2);
    out = compose(out,in);
    return sim.rec(in0, out);

  } else if (taski == 27) {
    in = sim.in(in);
    Image out = compose(in, filterCol(rigid(in,4),1));
    return sim.rec(in0, out);

  } else if (taski == 28) {
    return compress3(in);

  } else if (taski == 29) {
    in = sim.in(in);

    int blue = core::majorityCol(in);
    int red = 3-blue;
    Image out = eraseCol(in,blue);
    vImage to = cut(filterCol(in,3));
    for (Image img : to) {
      img = compress(img);
      out = replaceTemplate(out,colShape(img,red),img,2);
    }
    out = compose(out,filterCol(in,blue));
    return sim.rec(in0,out);

  } else if (taski == 30) {
    return sim.rec(in0,Col(1));

  } else if (taski == 31) {
    in = sim.in(in);
    in = filterCol(in,2);
    Image out = compose(colShape(smear(in,2),1), in);
    return sim.rec(in0,out);

  } else if (taski == 32) {
    Image gray = cutPickMax(in,0);
    Image cols = eraseCol(gray,5);

    Image shapes = compress(compose(in, gray, 4));

    cols = spreadCols(broadcast(cols,shapes));
    return embed(compose(replaceCols(shapes,cols), gray), in);

  } else if (taski == 33) {
    return compose(border(colShape(compose(smear(in,4), smear(in,5), 2), 8)), in);

  } else if (taski == 34) {
    Image blue = filterCol(in,8);
    Image squares = Fill(blue);

    Image dots = eraseCol(in,8);
    Image out = blue;
    for (Image_ img : splitCols(dots)) {
      Image part = compress(compose(hull(compress(img)), squares, 2));
      part = makeBorder2(part, 1);
      out = compose(part, out);
    }
    return out;

  } else if (taski == 35) {
    return compose(colShape(hull(in),5),Move(in,Pos(-1,0)),3);

  } else if (taski == 36) {
    in = eraseCol(in,3);
    in = compose(in,rigid(in,2));
    in = compose(in,rigid(in,4));
    return in;

  } else if (taski == 37) {
    in = eraseCol(in,3);

    Image out = compose(in,rigid(in,2));
    out = compose(out,rigid(out,4));
    return compress(compose(out,invert(in),2));

  } else if (taski == 38) {
    return outerProductIS(in,count(in,1,0));

  } else if (taski == 39) {
    Image out = in;
    for (Image_ img : cut(colShape(in,1))) {
      Image part = compose(in, makeBorder2(img,0), 2);
      part = eraseCol(part, 5);
      out = compose(broadcast(majCol(part), img), out);
    }
    return out;

  } else if (taski == 40) { //Wrong

    return in;

  } else if (taski == 41) {
    return outerProductSI(hull(in),in);

  } else if (taski == 42) {
    Image marked = compress(filterCol(in,8));
    Image out = replaceTemplate(in, colShape(marked,1), marked);
    Image marks = filterCol(out,8);
    Image orange = compose(colShape(connect(marks,2),7), in, 2);
    return compose(out, compose(orange, marks));

  } else if (taski == 43) {
    in = sim.in(in);
    Image bg = compress(filterCol(in,1));
    Image out = bg;
    for (Image_ img : cut(in)) {
      Image need = makeBorder2(colShape(invert(img),1),0);
      out = replaceTemplate(out, need, makeBorder2(img,0));
    }
    out = compose(out,bg);
    return sim.rec(in0, out);

  } else if (taski == 44 || taski == 45) {
    Image shape = compress(filterCol(in,8));
    Image room = invert(Square(3));
    shape = embed(align(shape,room,1,3), room);
    return outerProductSI(count(filterCol(in,4),2,1),shape);

  } else if (taski == 46) {
    Image squares = colShape(repeat(hull(cutPickMax(in,0)), in, 1),2);
    return compose(squares,in,4);

  } else if (taski == 47 || taski == 48) {
    Image a = compress(filterCol(in,6));
    Image b = toOrigin(compress(filterCol(in,5)));
    return colShape(invert(compose(a,b)),4);

  } else if (taski == 49) {
    vImage cols = splitCols(in);
    vImage lens = count(cols, 2,1);
    Image out = myStack(lens,1);
    return rigid(out,2);

  } else if (taski == 50) {
    Image shapes = stackLine(cut(filterCol(in,6)));
    Image cols = stackLine(cut(eraseCol(in,6)));
    return compose(broadcast(cols, shapes), shapes, 2);

  } else if (taski == 51) {
    Image out = replaceTemplate(in, invert(Square(3)), Square(3));
    out = replaceTemplate(out, invert(Square(2)), Square(2));
    return out;

  } else if (taski == 52) {
    Image green = filterCol(in,3);
    vImage hulls = hull( cut(green) );
    Image filled = embed(compose(hulls,0),in);
    Image specials = compose(filterCol(in,2), filled, 2);
    return compose(in, makeBorder(specials));

  } else if (taski == 53) { //Wrong
    Image center = interior(in);
    Image base = train[1].second;
    base = compose(base, interior(base));
    base = embed(align(base,center),in);
    return greedyFillBlack(base);

  } else if (taski == 54) {
    Image out = toOrigin(cutPickMax(in,0));
    for (Image_ pa : cut(cutPickMaxes(in,1))) {
      Image a = half(pa,0);
      Image b = half(pa,1);
      out = swapTemplate(out,a,b);
    }
    return out;

  } else if (taski == 55 || taski == 56) {
    Image a = half(in, 2);
    Image b = toOrigin(half(in, 3));
    Image or_ = compose(a,b);
    Image and_= compose(a,b,2);
    return colShape(compose(or_,and_,4),6);

  } else if (taski == 57) {
    Image out = toOrigin(cutPickMaxes(in,0));
    for (Image_ img : pickNotMaxes(cut(in),0)) {
      out = compose(out,align(img,out));
    }
    return out;

  } else if (taski == 58) { //Wrong


  } else if (taski == 59) {
    return compose(smear(eraseCol(in,5),0),in,2);

  } else if (taski == 60) {
    in = eraseCol(in,6);
    in = compose(in,rigid(in,6));
    in = greedyFillBlack(in);
    return in;

  } else if (taski == 61) {
    in = sim.in(in);
    Image a = filterCol(in,2);
    Image b = filterCol(in,1);

    Image out = Square(3);
    a = compress(wrap(count(interior(a),0,1),out));
    b = compress(wrap(count(interior(b),0,1),out));
    out = embed(toOrigin(myStack(a, b, 1)), out);
    return sim.rec(in0,out);

  } else if (taski == 62) {
    Image out = in;
    int col = 1;
    for (Image_ img : pickNotMaxes(splitColumns(in),0)) {
      out = compose(colShape(hull(img), col++), out);
    }
    return out;

  } else if (taski == 63) {
    Image b = half(compress(in), 3);
    return compose(in, colShape(b,2));

  } else if (taski == 64) {
    in = sim.in(in);

    int flip = 0;
    if (core::count(half(in,2)) < core::count(half(in,3))) flip = 1;
    if (flip) in = rigid(in,5);

    Image both = compose(half(in,2),rigid(in,5),2);
    Image out = smear(filterCol(both,2),2);
    out = compose(colShape(out,1), in);

    if (flip) out = rigid(out,5);
    return sim.rec(in0, out);

  } else if (taski == 65) {
    Image out = in;
    for (Image img : cut(in)) {
      Image piece = img;
      img = eraseCol(img, 4);
      piece = makeBorder2(piece, count(img,0,0));
      out = compose(out, piece);
    }
    return out;

  } else if (taski == 66) {
    Image marked = compress(cutPickMax(in, filterCol(in,5), 1));
    Image out = replaceTemplate(in, marked, Col(0));
    return compose(marked, out);

  } else if (taski == 67) {
    Image marked = compose(pickNotMaxes(splitAll(invert(in)),1),0);
    return compose(colShape(marked,8),in);

  } else if (taski == 68 || taski == 69) {
    Image center = interior2(in);
    return compose(spreadCols(in), smear(center,14), 2);

  } else if (taski == 70) {
    vImage outs = cut(in);
    Image out = stackLine(outs);
    return out;

  } else if (taski == 71) {
    Image to = colShape(Square(4),4);
    for (int c = 1; c < 10; c++) {
      in = replaceTemplate(in,colShape(to,c),to, 1);
    }
    return in;

  } else if (taski == 72) {
    Image a = compress(in);
    int other_col = 6+3-core::majorityCol(a);
    Image b = toOrigin(colShape(rigid(a,6),other_col));
    Image c = myStack(a,b,2);
    c = myStack(myStack(c,c,2),c,2);

    a = colShape(a,1);
    c = compose(c,a);
    c = compose(c,align(rigid(c,4),c));
    c = compose(c,align(rigid(c,5),c));
    return embed(compose(c,in),in);

  } else if (taski == 73) {
    in = rigid(in,2);
    return mirror(in,outerProductIS(in,Square(2)));

  } else if (taski == 74) {
    in = sim.in(in);
    Image shapes = repeat(pickMax(cut(in),0),in,1);
    Image grid = filterCol(in,1);
    Image cols = spreadCols(eraseCol(eraseCol(in,1),2));
    Image out = compose(compose(cols, shapes, 2), grid);
    return sim.rec(in0,out);

  } else if (taski == 75) {
    Image out = in;
    for (Image_ img : cut(in)) {
      int ccs = core::countComponents(invert(img));
      int cols[] = {0,1,2,3,7};
      out = compose(out, colShape(img,cols[ccs]));
    }
    return out;

  } else if (taski == 76) {
    Image inside = cutPickMax(in,filterCol(in,5),1);
    Image out = in;
    for (Image img : splitCols(inside)) {
      img = compress(img);
      Image green = colShape(img,3);
      out = replaceTemplate(out,green,img,2, 1);
    }
    return out;

  } else if (taski == 77) {
    Image a = compress(in);
    for (int i = 0; i < a.h; i++) {
      for (int j = 0; j < a.w; j++) {
	int d = min({i,j,a.h-1-i,a.w-1-j});
	a(i,j) = d%2 == 0 ? 5 : d%4 == 1 ? 2 : 0;
      }
    }
    return embed(a,in);

  } else if (taski == 78) {
    int W = cutIndex(in,0).w;

    Image out = in;
    for (Image_ img : cut(in)) {
      map<Image,int> cnt;
      for (int d = 0; d < 4; d++) {
	int nx = img.x+(W+1)*((d==0)-(d==1));
	int ny = img.y+(W+1)*((d==2)-(d==3));

	if (nx >= 0 && ny >= 0 && nx+4 <= in.w && ny+4 <= in.h) {
	  Image r = core::subImage(in,{nx,ny},{W,W});
	  if (core::count(r) > 0) {
	    int rs[] = {4,4,5,5};
	    Image here = rigid(r,rs[d]);
	    here.p = img.p;
	    ++cnt[here];
	  }
	}
      }
      for (auto [here,c] : cnt) {
	if (c == 2)
	  out = compose(out, here);
      }
    }
    return out;

  } else if (taski == 79) {
    Image out = in;
    for (Image_ img : cut(in)) {
      out = compose(out, repeat(compress(img),img));
    }
    return out;

  } else if (taski == 80) {
    Image pat = compress(compose(pickNotMaxes(splitAll(in),0),0));
    return extend2(Move(pat,Pos(-1,0)),in);

  } else if (taski == 81) {
    Image dots = cutPickMaxes(in,1);
    Image out = in;
    for (Image_ dot : cut(dots)) {
      for (Image_ img : cut(in)) {
	if (img.w > 1) {
	  out = compose(out, align(img, dot), 3);
	}
      }
    }
    return out;

  } else if (taski == 82) {
    return outerProductSI(in,invert(in));

  } else if (taski == 83) {
    Image out = hull0(in);
    for (Image_ img : splitCols(in)) {
      Image top = cutPickMax(img,5);
      Image side = cutPickMax(img,6);
      Image add;
      if (top.p == side.p)
	add = smear(embed(top,in),2);
      else {
	add = compose(smear(embed(top,in),2),
		      smear(embed(side,in),4));
	add = compose(add, hull(compress(img)),2);
      }
      out = compose(add, out);
    }
    return out;

  } else if (taski == 84) {
    Image out = in;
    for (Image_ img : cut(in)) {
      out = compose(out, colShape(majCol(eraseCol(img,1)), interior2(img)));
    }
    return out;

  } else if (taski == 85) {
    Image cols = rigid(cutPickMax(in,1), 3);
    Image shape = compress(filterCol(in,8));
    return compose(in,compose(broadcast(cols,shape), shape, 2));

  } else if (taski == 86) {
    Image out = hull0(in);
    for (Image_ img : cut(in)) {
      Image yellow = filterCol(img,4);
      Image line = cutPickMax(yellow,invert(yellow),0);
      vImage splitted = cut(compose(img,invert(embed(line,img)),2));
      Image c = compose(pickUnique(splitted,0), line);
      out = compose(out, line);
      out = compose(out,c);
      out = compose(out,mirror2(c,line));
    }
    return out;

  } else if (taski == 87) {
    return outerProductSI(filterCol(in,5),in);

  } else if (taski == 88) {
    Image from = Square(2);
    in = replaceTemplate(in,from,invert(from),1);
    return greedyFillBlack(in);

  } else if (taski == 89) {
    Image black = badImg;
    Image out = badImg;
    for (Image_ img : pickMaxes(splitAll(in),0)) {
      Image plus = smear(embed(interior2(img),in),6);
      Image both = compose(plus,out,2);
      black = compose(black,both);
      out = compose(out,plus);
    }
    return compose(compose(in,out),black,4);

  } else if (taski == 90) { //Wrong

  } else if (taski == 91 || taski == 92) {
    Image shape = compress(filterCol(in,1));
    return outerProductIS(compress2(eraseCol(in,1)),shape);

  } else if (taski == 93 || taski == 94) {
    int col = core::majorityCol(in);
    if (col == 1) return train[0].second;
    else if (col == 2) return train[1].second;
    else if (col == 3) return train[2].second;
    else return badImg;

  } else if (taski == 95) {
    Image cols = eraseCol(in,5);
    Image gray = filterCol(in,5);
    cols = compose(cols,rigid(cols,2));
    cols = spreadCols(cols);
    return compose(colShape(cols, Fill(in)),gray);

  } else if (taski == 96) {
    Image side = cutPickMax(in,0);
    Image out = in;
    Image red = colShape(Square(5),2);
    for (Image_ img : cut(side)) {
      Image a = colShape(majCol(img),red);
      Image b = compose(red,align(colShape(img,1),red,2,2));
      out = swapTemplate(out,a,b,1);
    }
    return out;

  } else if (taski == 97) {
    Image a = half(in,2);
    Image b = toOrigin(half(in,3));
    return colShape(compose(a,b),4);

  } else if (taski == 98) {
    Image out = hull0(in);
    for (Image img : splitCols(in)) {
      img = compress(img);
      int col = core::majorityCol(img);
      if (col == 8)
	img.x = 1;
      else if (col == 2)
	img.x = 2;
      else if (col == 4)
	img.x = 3;
      else if (col == 3)
	img.x = 4;
      out = compose(out,img);
    }
    return out;

  } else if (taski == 99) {
    in = sim.in(in);
    Image out = filterCol(in,2);
    for (Image_ img : insideMarked(in)) {
      Image a = compose(rigid(img,4),img);
      Image b = compose(rigid(img,5),img);
      out = compose(out, img.w > img.h ? a : b);
    }
    return sim.rec(in0,out);
  }

  return in;
}


void evalEvals(int print = 1) {
  vector<Sample> sample = readAll("evaluation", -1);
  int samples = sample.size();
  sample = vector<Sample>(sample.begin()+samples-100,sample.end());

  Visu visu;

  int corrects = 0;
  for (int si = 0; si < sample.size(); si++) {
    Sample&s = sample[si];
    visu.next(s.id);
    int ok = 1;
    for (auto [in,out] : s.train) {
      Image pred = toOrigin(solveEval(in, s.train, si));
      ok &= (pred == out);
      auto sim = normalizeCols(s.train);
      auto in0 = in;
      tie(in,out) = sim(in,out);
      visu.add(in, out);
      visu.add(in, sim(in0,pred).second);
    }
    corrects += ok;
    if (print)
      cout << "Task " << si << ": " << (ok ? "OK" : "Failed") << endl;
  }

  if (print) {
    cout << corrects << " / " << sample.size() << endl;
    exit(0);
  }
  if (!print)
    assert(corrects == 95);
}
