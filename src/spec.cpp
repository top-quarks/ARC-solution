#include "precompiled_stl.hpp"
using namespace std;
#include "utils.hpp"
#include "image_functions.hpp"
#include "core_functions.hpp"
#include "spec.hpp"

double entropy(Spec_ spec) {
  if (spec.bad) return 1e9;
  double sum = log(spec.w)+log(spec.h);
  if (!spec.anypos)
    sum += log(spec.x)+log(spec.y);
  double prec[11] = {-1e9};
  for (int i = 2; i < 11; i++) prec[i] = log(i);
  for (int m : spec.mask) {
    sum += prec[10]-prec[__builtin_popcount(m)];
  }
  return sum;
}

Spec fromImage(Image_ img) {
  Spec ret;
  ret.anypos = 0;
  ret.p = img.p;
  ret.sz = img.sz;
  ret.mask.resize(img.mask.size());
  for (int i = 0; i < img.mask.size(); i++)
    ret.mask[i] = 1<<img.mask[i];
  return ret;
}

Image toImage(Spec_ spec) {
  Image ret;
  ret.p = spec.p;
  ret.sz = spec.sz;
  ret.mask.resize(spec.mask.size());
  for (int i = 0; i < spec.mask.size(); i++) {
    ret.mask[i] = __builtin_ctz(spec.mask[i]);
  }
  return ret;
}


//deduce first argument of compose(?, img, 0);
SpecRec iComposeFirst(Image_ img, Spec_ target) {
  if (img.x < target.x ||
      img.y < target.y ||
      img.x+img.w > target.x+target.w ||
      img.y+img.h > target.y+target.h) {
    return badSpec;
  }
  point d = img.p-target.p;
  for (int i = 0; i < img.h; i++)
    for (int j = 0; j < img.w; j++)
      if (img(i,j) && (target(i+d.y,j+d.x)>>img(i,j)&1) == 0) {
	return badSpec;
      }
  Spec ret;
  ret.anypos = 0;
  ret.p = target.p;
  ret.sz = target.sz;
  ret.mask.assign(target.w*target.h, allCols);
  for (int i = 0; i < target.h; i++)
    for (int j = 0; j < target.w; j++)
      if (!img.safe(i-d.y,j-d.x))
	ret(i,j) = target(i,j);
  auto rec = [](Image_ img, Image_ first) {
    return compose(first, img, 0);
  };
  return {ret, rec};
}


//deduce second argument of compose(img, ?, 0);
SpecRec iComposeSecond(Image_ img, Spec_ target) {
  if (img.x < target.x ||
      img.y < target.y ||
      img.x+img.w > target.x+target.w ||
      img.y+img.h > target.y+target.h) return badSpec;

  point d = img.p-target.p;
  for (int i = 0; i < img.h; i++)
    for (int j = 0; j < img.w; j++)
      if (img(i,j) && target(i+d.y,j+d.x) == 1) {
	return badSpec;
      }

  Spec ret;
  ret.anypos = 0;
  ret.p = target.p;
  ret.sz = target.sz;
  ret.mask.assign(target.w*target.h, allCols);
  for (int i = 0; i < target.h; i++) {
    for (int j = 0; j < target.w; j++) {
      int back = img.safe(i-d.y,j-d.x);
      ret(i,j) = target(i,j) & ~1;
      if (target(i,j)>>back&1) ret(i,j) |= 1;
    }
  }
  auto rec = [](Image_ img, Image_ second) {
    return compose(img, second, 0);
  };
  return {ret, rec};
}


// compose(?, img, 3)
SpecRec iCompose2First(Image_ img, Spec_ target) {
  point d = img.p-target.p;
  Spec ret;
  ret.anypos = 0;
  ret.p = target.p;
  ret.sz = target.sz;
  ret.mask.assign(target.w*target.h, allCols);
  for (int i = 0; i < target.h; i++)
    for (int j = 0; j < target.w; j++) {
      char c = img.safe(i-d.y,j-d.x);
      if (!c)
	ret(i,j) = target(i,j);
      else if ((target(i,j)>>c&1) == 0)
	return badSpec;
    }
  auto rec = [](Image_ img, Image_ first) {
    return compose(first, img, 3);
  };
  return {ret, rec};
}





//deduce color argument of colShape
SpecRec iColShapeFirst(Image_ img, Spec_ target) {
  if ((!target.anypos && img.p != target.p) ||
      img.sz != target.sz) return badSpec;
  Spec ret;
  ret.anypos = 1;
  ret.p = {0,0};
  ret.sz = target.sz;
  ret.mask.resize(target.mask.size());
  for (int i = 0; i < target.mask.size(); i++) {
    if (img.mask[i]) {
      ret.mask[i] = target.mask[i];
    } else {
      if ((target.mask[i]&1) == 0) return badSpec;
      ret.mask[i] = allCols;
    }
  }
  auto rec = [](Image_ img, Image_ first) {
    return colShape(first, img);
  };
  return {ret, rec};
}

//deduce shape argument of colShape
SpecRec iColShapeSecond(Image_ img, Spec_ target) {
  Image big = broadcast(img, core::full(target.sz));
  Spec ret;
  ret.anypos = target.anypos;
  ret.p = target.p;
  ret.sz = target.sz;
  ret.mask.resize(target.mask.size(), 0);
  for (int i = 0; i < target.mask.size(); i++) {
    if (target.mask[i]&1)
      ret.mask[i] |= 1;
    if (target.mask[i]>>big.mask[i]&1) {
      ret.mask[i] |= allCols & ~1;
    }
    if (ret.mask[i] == 0) return badSpec;
  }
  auto rec = [](Image_ img, Image_ second) {
    return colShape(img, second);
  };
  return {ret, rec};
}


SpecRec iRepeatSecond(Image_ img, Spec_ target) {
  auto mod = [](int a, int m) {
    a %= m;
    if (a < 0) a += m;
    return a;
  };
  int ai  = mod(target.y-img.y, img.h);
  int aj0 = mod(target.x-img.x, img.w);
  for (int i = 0; i < target.h; i++) {
    int aj = aj0;
    for (int j = 0; j < target.w; j++) {
      if ((target(i,j)>>img(ai,aj)&1) == 0) return badSpec;
      if (++aj == img.w) aj = 0;
    }
    if (++ai == img.h) ai = 0;
  }

  Spec ret;
  ret.anypos = 0;
  ret.p = target.p;
  ret.sz = target.sz;
  ret.mask.assign(ret.w*ret.h, allCols);
  auto rec = [](Image_ img, Image_ second) {
    return repeat(img, second);
  };
  return {ret, rec};
}


SpecRec iMirrorSecond(Image_ img, Spec_ target) {
  auto mod = [](int a, int m) {
    a %= m;
    if (a < 0) a += m;
    return a;
  };
  int ai  = mod(target.y-img.y, img.h*2);
  int aj0 = mod(target.x-img.x, img.w*2);
  for (int i = 0; i < target.h; i++) {
    int aj = aj0;
    for (int j = 0; j < target.w; j++) {
      if ((target(i,j)>>img(ai >= img.h ? img.h*2-1-ai : ai,
			    aj >= img.w ? img.w*2-1-aj : aj)&1) == 0) return badSpec;
      if (++aj == img.w*2) aj = 0;
    }
    if (++ai == img.h*2) ai = 0;
  }

  Spec ret;
  ret.anypos = 0;
  ret.p = target.p;
  ret.sz = target.sz;
  ret.mask.assign(ret.w*ret.h, allCols);
  auto rec = [](Image_ img, Image_ second) {
    return mirror(img, second);
  };
  return {ret, rec};
}



SpecRec iOuterProductSIFirst(Image_ img, Spec_ target) {
  if (target.w%img.w || target.h%img.h ||
      !target.anypos &&
      ((target.x-img.x)%img.w || (target.y-img.y)%img.h))
    return badSpec;

  Spec ret;
  ret.anypos = target.anypos;
  if (target.anypos)
    ret.p = {0,0}; //TODO: set position here also?
  else {
    ret.x = (target.x-img.x)/img.w;
    ret.y = (target.y-img.y)/img.h;
  }
  ret.w = target.w/img.w;
  ret.h = target.h/img.h;
  ret.mask.resize(ret.w*ret.h);
  for (int ii = 0; ii < ret.h; ii++) {
    for (int jj = 0; jj < ret.w; jj++) {
      int all0 = 1, allok = 1;
      for (int i = 0; i < img.h; i++) {
	for (int j = 0; j < img.w; j++) {
	  int m = target(ii*img.h+i,jj*img.w+j);
	  if ((m>>img(i,j)&1) == 0) allok = 0;
	  if ((m&1) == 0) all0 = 0;
	}
      }
      if (!all0 && !allok) return badSpec;
      ret(ii,jj) = all0 + (allCols & ~1) * allok;
    }
  }
  auto rec = [](Image_ img, Image_ first) {
    return outerProductSI(first, img);
  };
  return {ret, rec};
}


SpecRec iOuterProductISFirst(Image_ img, Spec_ target) {
  if (target.w%img.w || target.h%img.h ||
      !target.anypos &&
      ((target.x-img.x)%img.w || (target.y-img.y)%img.h))
    return badSpec;

  Spec ret;
  ret.anypos = target.anypos;
  if (target.anypos)
    ret.p = {0,0}; //TODO: set position here also?
  else {
    ret.x = (target.x-img.x)/img.w;
    ret.y = (target.y-img.y)/img.h;
  }
  ret.w = target.w/img.w;
  ret.h = target.h/img.h;
  ret.mask.resize(ret.w*ret.h);
  for (int ii = 0; ii < ret.h; ii++) {
    for (int jj = 0; jj < ret.w; jj++) {
      int all = allCols;
      for (int i = 0; i < img.h; i++) {
	for (int j = 0; j < img.w; j++) {
	  int in = img(i,j) > 0;
	  int m = target(ii*img.h+i,jj*img.w+j);
	  if (in)
	    all &= m;
	  else if ((m&1) == 0) return badSpec;
	}
      }
      if (all == 0) return badSpec;
      ret(ii,jj) = all;
    }
  }
  auto rec = [](Image_ img, Image_ first) {
    return outerProductIS(first, img);
  };
  return {ret, rec};
}


SpecRec iOuterProductSISecond(Image_ img, Spec_ target) {
  if (target.w%img.w || target.h%img.h)
    return badSpec;

  Spec ret;
  ret.w = target.w/img.w;
  ret.h = target.h/img.h;

  ret.anypos = target.anypos;
  if (target.anypos)
    ret.p = {0,0}; //TODO: set position here also?
  else {
    ret.x = target.x-img.x*ret.w;
    ret.y = target.y-img.y*ret.h;
  }
  ret.mask.resize(ret.w*ret.h);
  for (int i = 0; i < ret.h; i++) {
    for (int j = 0; j < ret.w; j++) {
      int all = allCols;
      for (int ii = 0; ii < img.h; ii++) {
	for (int jj = 0; jj < img.w; jj++) {
	  int in = img(ii,jj) > 0;
	  int m = target(ii*ret.h+i,jj*ret.w+j);
	  if (in)
	    all &= m;
	  else if ((m&1) == 0) return badSpec;
	}
      }
      if (all == 0) return badSpec;
      ret(i,j) = all;
    }
  }
  auto rec = [](Image_ img, Image_ second) {
    return outerProductSI(img, second);
  };
  return {ret, rec};
}


SpecRec iOuterProductISSecond(Image_ img, Spec_ target) {
  if (target.w%img.w || target.h%img.h)
    return badSpec;

  Spec ret;
  ret.w = target.w/img.w;
  ret.h = target.h/img.h;

  ret.anypos = target.anypos;
  if (target.anypos)
    ret.p = {0,0}; //TODO: set position here also?
  else {
    ret.x = target.x-img.x*ret.w;
    ret.y = target.y-img.y*ret.h;
  }
  ret.mask.resize(ret.w*ret.h);
  for (int i = 0; i < ret.h; i++) {
    for (int j = 0; j < ret.w; j++) {

      int all0 = 1, allok = 1;
      for (int ii = 0; ii < img.h; ii++) {
	for (int jj = 0; jj < img.w; jj++) {
	  int m = target(ii*ret.h+i,jj*ret.w+j);
	  if ((m>>img(ii,jj)&1) == 0) allok = 0;
	  if ((m&1) == 0) all0 = 0;
	}
      }
      if (!all0 && !allok) return badSpec;
      ret(i,j) = all0 + (allCols & ~1) * allok;
    }
  }
  auto rec = [](Image_ img, Image_ second) {
    return outerProductIS(img, second);
  };
  return {ret, rec};
}


//Probably buggy
SpecRec iStack(Image_ img, Spec_ target, int id) {
  Spec ret;
  if (id < 4) {
    ret.anypos = 1;
    ret.p = {0,0};
  } else {
    ret.anypos = target.anypos;
    ret.p = target.p;
  }
  int ix, iy, rx, ry;
  if (id == 0) {
    ix = 0, iy = 0, rx = img.w, ry = 0;
    ret.sz = {target.w-img.w, target.h};
  } else if (id == 1) {
    ix = 0, iy = 0, rx = 0, ry = img.h;
    ret.sz = {target.w, target.h-img.h};
  } else if (id == 2) {
    ix = 0, iy = 0, rx = img.w, ry = img.h;
    ret.sz = target.sz-img.sz;
  } else if (id == 3) {
    ix = 0, iy = target.h-img.h, rx = img.w, ry = 0;
    ret.sz = target.sz-img.sz;
  } else if (id == 4) {
    ix = target.w-img.w, iy = 0, rx = 0, ry = 0;
    ret.sz = {ix, target.h};
  } else if (id == 5) {
    ix = 0, iy = target.h-img.h, rx = 0, ry = 0;
    ret.sz = {target.w, iy};
  } else if (id == 6) {
    ix = target.w-img.w, iy = target.h-img.h, rx = 0, ry = 0;
    ret.sz = target.sz-img.sz;
  } else if (id == 7) {
    ix = target.w-img.w, iy = 0, rx = 0, ry = img.h;
    ret.sz = target.sz-img.sz;
  } else assert(id >= 0 && id < 8);

  if (ix < 0 || iy < 0 ||
      ix+img.w > target.w || iy+img.h > target.h ||
      ret.w <= 0 || ret.h <= 0)
    return badSpec;

  for (int i = 0; i < img.h; i++)
    for (int j = 0; j < img.w; j++)
      if ((target(i+iy, j+ix)>>img(i,j)&1) == 0) return badSpec;

  for (int i = 0; i < target.h; i++)
    for (int j = 0; j < target.w; j++)
      if (!(i >= iy && i < iy+img.h && j >= ix && j < ix+img.w ||
	    i >= ry && i < ry+ret.h && j >= rx && j < rx+ret.w) &&
	  (target(i,j)&1) == 0) return badSpec;

  ret.mask.resize(ret.w*ret.h);
  for (int i = 0; i < ret.h; i++)
    for (int j = 0; j < ret.w; j++)
      ret(i,j) = target(i+ry, j+rx);

  auto rec = [id](Image_ img, Image_ other) {
    if (id < 4)
      return myStack(img, other, id);
    else
      return myStack(other, img, id-4);
  };
  return {ret, rec};
}


SpecRec iEmbed(Image_ img, Spec_ target) {
  point d = target.p-img.p;
  for (int i = 0; i < target.h; i++)
    for (int j = 0; j < target.w; j++)
      if ((target(i,j)>>img.safe(i+d.y, j+d.x)&1) == 0) return badSpec;

  Spec ret;
  ret.anypos = target.anypos;
  ret.p = target.p;
  ret.sz = target.sz;
  ret.mask.assign(ret.w*ret.h, allCols);
  auto rec = [](Image_ img, Image_ room) {
    return embed(img, room);
  };
  return {ret, rec};
}


//Very difficult, but maybe possible
//SpecRec iSmear(Image_ base, Spec_ target, int id) {}

//SpecRec iWrap(Image_ img, Spec_ target) {}

/*vector<SpecRec> iTakePos(Image_ img, Spec_ target) {
  Spec ret = target;
  ret.anypos = 1;
  vector<SpecRec> v = {{ret, toOrigin}};
  if (ret.p != point{0,0}) {
    ret.p = {0,0};
    v.emplace_back(ret, toOrigin);
  }
  return v;
}
*/






/*

//deduce both arguments of colShape(?, ?);
pair<pair<Spec,Spec>,function<Image(Image_, Image_)>> iColShape(Spec_ target) {
  Spec a, b;
  a.anypos = b.anypos = 1;
  a.p = b.p = target.p;
  a.sz = b.sz = target.sz;
  int n = target.mask.size();
  a.mask.resize(n, 0);
  b.mask.resize(n, 0);
  for (int i = 0; i < n; i++) {
    int c = target.mask[i];
    if (c & 1) {
      a.mask[i] = allCols;
      b.mask[i] = 1;
    } else {
      a.mask[i] = c;
      b.mask[i] = allCols & ~1;
    }
  }
  auto f = [](Image_ a, Image_ b) {
    return colShape(a,b);
  };
  return {{a,b}, f};
}


pair<vector<pair<Spec,Spec>>,function<Image(Image_, Image_)>> iRepeat(Spec_ target) {
  vector<pair<Spec,Spec>> ret;
  for (int h = 1; h < target.h; h++) {
    for (int w = 1; w < target.w; w++) {

    }
  }
  return {ret, repeat};
}


pair<vector<pair<Spec,Spec>>,function<Image(Image_, Image_)>> iMirror(Spec_ target) {
  vector<pair<Spec,Spec>> ret;
  for (int h = 1; h < target.h; h++) {
    for (int w = 1; w < target.w; w++) {

    }
  }
  return {ret, mirror};
}

pair<pair<Spec,Spec>,function<Image(Image_,Image_)>> iOuterProductIS(Spec_ target) {
  Spec a, b;

  return {{a, b}, outerProductIS};
}


pair<pair<Spec,Spec>, function<Image(Image_,Image_)>> iWrap(Spec_ target) {

}
*/
