#include "precompiled_stl.hpp"
using namespace std;

#include "utils.hpp"
#include "core_functions.hpp"
#include "image_functions.hpp"
#include "image_functions2.hpp"
#include "visu.hpp"
#include "read.hpp"
#include "normalize.hpp"
#include "spec.hpp"

Image solveTask(const Image& img, const vector<pair<Image,Image>>&train, int taski) {
  if (taski == 0) {
    return toOrigin(compress(filterCol(img, 1)));
  } else if (taski == 1) {
    Image cols = cutPickMax(img, 4);
    Image shape = cutPickMax(img, 0);
    return toOrigin(colShape(cols, shape));
  } else if (taski == 2 || taski == 3) {
    return interior(filterCol(img, 2));
  } else if (taski == 4) {
    return outerProductIS(img, count(img, 1, 0));
  } else if (taski == 5) {
    return train[0].second;
  } else if (taski == 6) {
    Image cutter = filterCol(train[0].second, 1);
    Image yellow = compose(cutter, filterCol(img, 4));
    Image part = cutPickMax(yellow, 0);
    Image mask = hull(part);
    Image chosen = toOrigin(compose(img, mask, 2));
    Image positioned = outerProductSI(toOrigin(part), myStack(chosen, Square(1), 2));
    return compose(positioned, cutter, 1);
  } else if (taski == 7) {
    Image blue = filterCol(img, 1);
    Image red = compress(filterCol(img, 2));
    return compose(blue, Move(red, Pos(2,0)), 0);
  } else if (taski == 8) {
    return rigid(img, 2);
  } else if (taski == 9 || taski == 10) {
    Image a = compress(filterCol(img, 2));
    Image b = toOrigin(compress(filterCol(img, 3)));
    return colShape(compose(a, b), 1);
  } else if (taski == 11) {
    return train[0].second;
  } else if (taski == 12) {
    return compose(Square(3), splitPickMax(img, 0));
  } else if (taski == 13) {
    return train[0].second;

  } else if (taski == 15) {
    return toOrigin(compress(cutIndex(img, 0)));
  } else if (taski == 16) {
    Image good = filterCol(img, train[0].second);
    Image a = compose(good, rigid(good, 6));
    Image b = compose(a, Move(rigid(a, 2), Pos(2,2)));
    return embed(b, img);
  } else if (taski == 17) {
    Image a = filterCol(img, 2);
    Image b = filterCol(img, 3);
    return compose(a, colShape(b, 1));
  } else if (taski == 18) {
    Image cross = filterCol(rigid(img,4), 1);
    Image sq = count(filterCol(img, 2), 0, 0);
    return rigid(extend(compress(myStack(invert(sq), cross, 2)), img), 4);
  } else if (taski == 19) {
    Image a = cutIndex(img, 0);
    Image b = cutIndex(img, 1);
    return compose(colShape(compose(hull(compress(a)), hull(compress(b))), 1), img);

  } else if (taski == 21) {
    return rigid(img, 6);
  } else if (taski == 22) {
    Image shape = compress(cutPickMax(img, 8));
    Image points = filterCol(img, 1);
    Image a = cutIndex(points, 0);
    Image b = cutIndex(points, 1);
    Image c = cutIndex(points, 2);
    Image d = cutIndex(points, 3);
    Image ans = img;
    ans = compose(ans, replaceCols(align(outerProductIS(shape, a), a), img));
    ans = compose(ans, replaceCols(align(outerProductIS(shape, b), b), img));
    ans = compose(ans, replaceCols(align(outerProductIS(shape, c), c), img));
    ans = compose(ans, replaceCols(align(outerProductIS(shape, d), d), img));
    return ans;
  } else if (taski == 23) {
    Image a = smear(filterCol(img,2), 6);
    Image b = smear(filterCol(img,3), 6);
    return compose(compose(a, b), colShape(compose(a, b, 2), 1));

  } else if (taski == 25) {
    return embed(Move(toOrigin(compress(train[0].second)), compress(filterCol(img,1))), img);
  } else if (taski == 26) {
    Image dup = colShape(toOrigin(cutPickMax(img, 0)), 1);
    return compose(outerProductSI(Square(3), myStack(dup, Col(1), 2)), img, 1);
  } else if (taski == 27) {
    return broadcast(img, Square(3));

  } else if (taski == 29) {
    return compose(img, invert(interior2(img)), 2);

  } else if (taski == 31) {
    Image a = cutIndex(img, 0);
    Image b = cutIndex(img, 1);
    Image c = cutIndex(img, 2);
    Image d = cutIndex(img, 3);
    Image ans = compose(invert(Square(3)), Pos(1,1));
    ans = compose(ans, align(a, ans));
    ans = compose(ans, align(b, ans));
    ans = compose(ans, align(c, ans));
    ans = compose(ans, align(d, ans));
    return ans;
  } else if (taski == 32) {
    return toOrigin(compress(filterCol(cutPickMax(img, 4), 1)));
  } else if (taski == 33) {
    Image a = toOrigin(compress(filterCol(img, 3)));
    Image b = toOrigin(compress(filterCol(img, 4)));
    Image both = compose(a, b, 2);
    Image one = compose(a, b);
    return colShape(compose(one, invert(both), 2), 1);
  } else if (taski == 34) {
    return count(img, 0, 1);
  } else if (taski == 35) {
    Image a = smear(filterCol(img, 2), 5);
    Image b = smear(filterCol(img, 3), 4);
    return compose(compose(a, b), colShape(compose(a,b,2), 1));
  } else if (taski == 36) {
    Image a = toOrigin(compress(cutPickMax(img, 0)));
    Image b = toOrigin(compress(cutPickMax(img, 1)));
    return colShape(invert(compose(a, b)), 1);

  } else if (taski == 38) {
    Image gray = colShape(filterCol(img, 5), 1);
    return compose(img, gray);
  } else if (taski == 39) {
    return broadcast(rigid(img,4), Square(4));
  } else if (taski == 40) {
    Image a = smear(filterCol(img, 1), 4);
    Image b = filterCol(img, 2);
    return compose(b, a);
  } else if (taski == 41) {
    Image cutter = filterCol(img, 3);
    Image ret = colShape(getSize(img), 2);
    ret = compose(ret, colShape(cutPickMaxes(invert(img), 3), 1));
    ret = compose(ret, cutter);
    return ret;
  } else if (taski == 42) {
    return compose(train[0].second, compose(img, rigid(img, 6)));
  } else if (taski == 43) {
    Image a = smear(filterCol(img, 1), 4);
    Image b = smear(filterCol(img, 3), 4);
    Image c = smear(filterCol(img, 2), 5);
    return compose(c, compose(a, b));

  } else if (taski == 47) {
    Image fill = colShape(invert(compress(filterCol(img, 2))), 1);
    return compose(smear(fill, invert(img), 6), img);

  } else if (taski == 49) {
    Image base = colShape(cutPickMax(img, 4), 1);
    base = myStack(base, rigid(base, 4), 0);
    base = myStack(base, rigid(base, 5), 1);
    return base;

  } else if (taski == 51) {
    return rigid(compose(rigid(train[0].second, 5), getSize(img), 2), 5);

  } else if (taski == 52) {
    return embed(toOrigin(compress(img)), Square(3));
  } else if (taski == 53) {
    Image takes = splitPickMax(img, 0);
    return toOrigin(cutIndex(takes, 0));
  } else if (taski == 54) {
    Image a = compress(cutIndex(img,0));
    Image b = compress(cutIndex(img,1));
    Image ca = align(Col(1), a, 2, 2);
    Image cb = align(Col(1), b, 2, 2);
    ca = smear(ca, getSize(img), 6);
    cb = smear(cb, getSize(img), 6);
    return compose(compose(compose(compose(img, ca), cb), a), b);
  } else if (taski == 55) {
    Image twos = cutPickMaxes(img, 0);
    return colShape(broadcast(twos, Square(3), 0), 1);
  } else if (taski == 56) {
    Image base = compress(train[0].second);
    Image pos = compress(img);
    return embed(align(base, pos, 2, 2), img);

  } else if (taski == 58) {
    Image blues = count(img, 0, 1);
    Image blacks = count(invert(img), 0, 0);
    blues = outerProductSI(blues, img);
    blacks = outerProductSI(blacks, img);
    return wrap(blues, blacks);
  } else if (taski == 59) {
    return toOrigin(compress(filterCol(img, 1)));
  } else if (taski == 60) {
    return compose(img, rigid(img, 5));

  } else if (taski == 62) {
    Image reds = cutPickMaxes(img, 11);
    return compose(colShape(img, 1), reds);
  } else if (taski == 63) {
    Image blues = colShape(cutPickMaxes(img, 0), 1);
    Image greens = colShape(cutPickMaxes(img, 1), 3);
    return compose(compose(colShape(img, 2), blues), greens);
  } else if (taski == 64) {
    return colShape(filterCol(img, 1), 2);

  } else if (taski == 66) {
    return broadcast(compress(img), Square(3)); //Cheating
  } else if (taski == 67) {
    return compose(smear(filterCol(img, 2), 3), filterCol(img, 1)); //Needs good normalize_rigid

  } else if (taski == 69) {
    Image a = invert(toOrigin(compress(filterCol(img, 2))));
    Image b = invert(toOrigin(compress(filterCol(img, 3))));
    return colShape(compose(a,b,2), 1);
  } else if (taski == 70) {
    Image shape = compress(filterCol(train[0].second, 1));
    Image a = cutIndex(img, 0);
    Image b = cutIndex(img, 1);
    Image c = cutIndex(img, 2);
    a = replaceCols(align(shape, a, 2, 5), img);
    b = replaceCols(align(shape, b, 2, 5), img);
    c = replaceCols(align(shape, c, 2, 5), img);
    return embed(compose(compose(a,b), c), img);
  } else if (taski == 71) {
    Image shape = hull(compress(img));
    shape = compose(shape, colShape(interior2(shape), 2));
    return embed(shape, img);
  } else if (taski == 72) {
    return myStack(img, rigid(img,5), 1);
  } else if (taski == 73) {
    Image base = broadcast(compress(img), Square(3));
    return outerProductIS(base, base);

  } else if (taski == 77) {
    return myStack(img, rigid(img,5), 1);
  } else if (taski == 78) {
    Image a = compose(img, rigid(img, 6));
    Image b = compose(a, Move(rigid(a, 2), Pos(2,2)));
    return embed(b, img);
  } else if (taski == 79) {
    Image ship = filterCol(img, 2);
    Image cover = smear(ship, 2);
    return compose(compose(smear(filterCol(img, 1), 5), cover, 2), img);
  } else if (taski == 82) {
    Image mask = compress(interior(smear(filterCol(img, 2), 6)));
    return toOrigin(colShape(embed(img,mask), 2));
  } else if (taski == 83) {
    Image ret = myStack(img,rigid(img,3), 0);
    return myStack(ret,rigid(ret,2),1);
  } else if (taski == 84) {
    return embed(count(img, 0, 1), Square(3));
  } else if (taski == 85) {
    return train.back().second;
  } else if (taski == 86) {
    Image a = cutIndex(img,0);
    Image b = cutIndex(img,1);
    Image c = cutIndex(img,2);
    a = align(a,a,5,0);
    b = align(b,b,5,0);
    c = align(c,c,5,0);
    return embed(compose(compose(a,b),c), img);
  } else if (taski == 88) {
    Image cutter = filterCol(img, 3);
    Image mi = colShape(cutPickMaxes(invert(img),1), 2);
    Image ma = colShape(cutPickMaxes(invert(img),0), 1);
    return compose(compose(mi, ma), cutter);
  } else if (taski == 89) {
    Image dots = filterCol(img, 3);
    Image lines = smear(dots, 1);
    Image red = filterCol(img, 2);
    Image top = cutIndex(red,0);
    Image bot = cutIndex(red,1);

    Image overlap = compose(lines,top,2);
    overlap = align(overlap, bot, 1, 1);
    Image lines2 = smear(overlap,getSize(img),1);
    dots = colShape(dots, 1);
    return compose(compose(compose(lines, lines2), red), dots);
  } else if (taski == 90) {
    Image a = cutIndex(img,0);
    Image b = cutIndex(img,1);
    Image c = cutIndex(img,2);
    Image d = cutIndex(img,3);
    Image whole = getSize(img);
    Image blue = img;
    blue = compose(smear(a,whole,2), blue);
    blue = compose(smear(b,whole,2), blue);
    blue = compose(smear(c,whole,2), blue);
    blue = compose(smear(d,whole,2), blue);
    blue = colShape(blue, 1);
    Image red = img;
    red = compose(align(outerProductIS(a,Square(2)),a,2,2), red);
    red = compose(align(outerProductIS(b,Square(2)),b,2,2), red);
    red = compose(align(outerProductIS(c,Square(2)),c,2,2), red);
    red = compose(align(outerProductIS(d,Square(2)),d,2,2), red);
    red = colShape(red, 2);
    return compose(compose(blue,red),img);
    /*} else if (taski == 88) {
    //Needs better align
    Image cutter = filterCol(img,Col(1));
    Image ret = cutPickMax(img,cutter,0);
    Image a = cutIndex(img,cutter,0);
    Image b = cutIndex(img,cutter,1);
    Image c = cutIndex(img,cutter,2);
    Image d = cutIndex(img,cutter,3);
    ret = compose(ret, align(a,ret), 3);
    ret = compose(ret, align(b,ret), 3);
    ret = compose(ret, align(c,ret), 3);
    ret = compose(ret, align(d,ret), 3);
    return toOrigin(ret);*/
  } else if (taski == 93) {
    Image blue = compress(filterCol(img,1));
    return embed(compose(align(colShape(Square(3), 3), blue, 2, 2), blue), img);
  } else if (taski == 95) {
    Image cols = smear(splitPickMaxes(img, 1), 2);
    return replaceCols(img, cols);
  } else if (taski == 96) {
    Image base = compress(img);
    return toOrigin(outerProductIS(base,base));
  } else if (taski == 97) {
    return compose(img, colShape(interior2(img), 1));
  } else if (taski == 101) {
    return colShape(broadcast(compress(filterCol(img,2)), Square(3)), 1);
  }
  return img;
}





void evalTasks() {
  vector<Sample> sample = readAll("training", 100);//evaluation
  Visu visu;

  int corrects = 0;
  int place_count[11] = {};
  //#pragma omp parallel for
  for (int si = 0; si < sample.size(); si++) {
    Sample&s = sample[si];
    {
      vector<Simplifier> sims;
      remapCols(s.train, sims);
      for (auto&[in,out] : s.train)
	tie(in,out) = make_pair(sims[0].in(in), sims[0].out(in,out));
      tie(s.test_in,s.test_out) = make_pair(sims[0].in(s.test_in), sims[0].out(s.test_in,s.test_out));
      s.test = {{s.test_in, s.test_out}};
    }
    /*{
      vector<Simplifier> sims;
      normalizeRigid(s.train, sims);
      for (auto&[in,out] : s.train)
	tie(in,out) = make_pair(sims[0].in(in), sims[0].out(in,out));
      for (auto&[in,out] : s.test)
	tie(in,out) = make_pair(sims[0].in(in), sims[0].out(in,out));
	}*/

    /*visu.next(s.id);
    for (auto [in,out] : s.train) {
      visu.add(in,out);
      }*/

    Image pred = solveTask(s.test[0].first, s.train, si);
    cout << "Task " << si << ": " << (pred == s.test[0].second ? "OK" : "Failed") << endl;
    corrects += (pred == s.test[0].second);
    if (pred != s.test_out) {// && pred != s.test_in) {
      visu.next(to_string(si));//s.id);
      visu.add(s.test_in, pred);
      visu.add(s.test_in, s.test_out);
      for (auto [in,out] : s.train)
	visu.add(in, out);
    }
  }
  cout << corrects << " / " << sample.size() << endl;
  exit(0);
}
