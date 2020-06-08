#include "precompiled_stl.hpp"
/*#include <vector>
#include <cassert>
#include <queue>
#include <tuple>
#include <functional>*/
using namespace std;

#include "utils.hpp"
#include "core_functions.hpp"
#include "image_functions.hpp"

Image Col(int id) {
  assert(id >= 0 && id < 10);
  return core::full({0,0}, {1,1}, id);
}
Image Pos(int dx, int dy) {
  return core::full({dx,dy},{1,1});
}
Image Square(int id) {
  assert(id >= 1);
  return core::full({0,0}, {id,id});
}
Image Line(int orient, int id) {
  assert(id >= 1);
  int w = id, h = 1;
  if (orient) swap(w,h);
  return core::full({0,0}, {w,h});
}

Image getPos(Image_ img) {
  return core::full(img.p, {1,1}, core::majorityCol(img));
}
Image getSize(Image_ img) {
  return core::full({0,0}, img.sz, core::majorityCol(img));
}
Image hull(Image_ img) {
  return core::full(img.p, img.sz, core::majorityCol(img));
}
Image toOrigin(Image img) {
  img.p = {0,0};
  return img;
}

Image getW(Image_ img, int id) {
  return core::full({0,0}, {img.w,id ? img.w : 1}, core::majorityCol(img));
}
Image getH(Image_ img, int id) {
  return core::full({0,0}, {id ? img.h : 1, img.h}, core::majorityCol(img));
}

Image hull0(Image_ img) {
  return core::full(img.p, img.sz, 0);
}
Image getSize0(Image_ img) {
  return core::full({0,0}, img.sz, 0);
}



Image Move(Image img, Image_ p) {
  img.x += p.x;
  img.y += p.y;
  return img;
}

Image filterCol(Image_ img, Image_ palette) {
  Image ret = img;
  int palMask = core::colMask(palette);
  for (int i = 0; i < img.h; i++)
    for (int j = 0; j < img.w; j++)
      if ((palMask>>img(i,j)&1) == 0)
	ret(i,j) = 0;
  return ret;
}

Image filterCol(Image_ img, int id) {
  assert(id >= 0 && id < 10);
  if (id == 0) return invert(img);
  else return filterCol(img, Col(id));
}



Image broadcast(Image_ col, Image_ shape, int include0) { //include0 = 1
  if (col.w*col.h == 0 || shape.w*shape.h == 0) return badImg;

  if (shape.w%col.w == 0 && shape.h%col.h == 0) {
    Image ret = shape;
    int dh = shape.h/col.h, dw = shape.w/col.w;
    for (int ii = 0; ii < col.h; ii++) {
      for (int jj = 0; jj < col.w; jj++) {
	int c = col(ii,jj);
	for (int i = ii*dh; i < ii*dh+dh; i++)
	  for (int j = jj*dw; j < jj*dw+dw; j++)
	    ret(i,j) = c;
      }
    }
    return ret;
  }

  Image ret = shape;
  double fh = col.h*1./shape.h, fw = col.w*1./shape.w;

  const double eps = 1e-9;
  double w0[10] = {};
  for (int c : col.mask) w0[c] += 1e-6;

  double tot = fh*fw;
  double weight[10];
  for (int i = 0; i < shape.h; i++) {
    for (int j = 0; j < shape.w; j++) {
      copy_n(w0, 10, weight);

      double r0 = i*fh+eps, r1 = (i+1)*fh-eps;
      double c0 = j*fw+eps, c1 = (j+1)*fw-eps;

      int guess = !include0;
      for (int y = r0; y < r1; y++) {
	double wy = min((double)y+1,r1)-max((double)y,r0);
	for (int x = c0; x < c1; x++) {
	  double wx = min((double)x+1,c1)-max((double)x,c0);
	  char c = col(y,x);
	  weight[c] += wx*wy;
	  guess = c;
	}
      }

      if (weight[guess]*2 > tot) {
	ret(i,j) = guess;
	continue;
      }

      int maj = !include0;
      double w = weight[maj];
      for (int c = 1; c < 10; c++) {
	if (weight[c] > w) maj = c, w = weight[c];
      }
      ret(i,j) = maj;
      //point sz = {max(c1-c0, 1), max(r1-r0, 1)};
      //ret(i,j) = core::majorityCol(core::subImage(col, {c0,r0}, sz), include0);
    }
  }
  return ret;
}

Image colShape(Image_ col, Image_ shape) {
  if (shape.w*shape.h == 0 || col.w*col.h == 0) return badImg;
  Image ret = broadcast(col, getSize(shape));
  ret.p = shape.p;
  for (int i = 0; i < ret.h; i++)
    for (int j = 0; j < ret.w; j++)
      if (!shape(i,j)) ret(i,j) = 0;
  return ret;
}
Image colShape(Image_ shape, int id) {
  assert(id >= 0 && id < 10);
  Image ret = shape;
  for (char&c : ret.mask)
    c = c ? id : 0;
  return ret;
}



Image compress(Image_ img, Image_ bg) { // bg = Col(0)
  int bgmask = core::colMask(bg);

  int xmi = 1e9, xma = 0, ymi = 1e9, yma = 0;
  for (int i = 0; i < img.h; i++) {
    for (int j = 0; j < img.w; j++) {
      if ((bgmask>>img(i,j)&1) == 0) {
	xmi = min(xmi, j);
	xma = max(xma, j);
	ymi = min(ymi, i);
	yma = max(yma, i);
      }
    }
  }
  Image ret;
  if (xmi == 1e9) {
    ret.p = {0,0};
    ret.sz = {0,0};
    return ret;
  }
  ret.p = img.p + point{xmi, ymi};
  ret.sz = {xma-xmi+1, yma-ymi+1};
  ret.mask.resize(ret.h*ret.w);
  for (int i = ymi; i <= yma; i++) {
    for (int j = xmi; j <= xma; j++) {
      ret(i-ymi,j-xmi) = img(i,j);
    }
  }
  return ret;
}



Image embedSlow(Image_ img, Image_ shape) {
  Image ret = core::empty(shape.p, shape.sz);
  point d = shape.p-img.p;
  for (int i = 0; i < ret.h; i++)
    for (int j = 0; j < ret.w; j++)
      ret(i,j) = img.safe(i+d.y, j+d.x);
  return ret;
}

Image embed(Image_ img, Image_ shape) {
  Image ret = core::empty(shape.p, shape.sz);
  point d = shape.p-img.p;
  int sx = max(0,-d.x);
  int sy = max(0,-d.y);
  int ex = min(ret.w,img.w-d.x);
  int ey = min(ret.h,img.h-d.y);

  int retw = ret.w, imgw = img.w, off = d.y*img.w+d.x;
  for (int i = sy; i < ey; i++)
    for (int j = sx; j < ex; j++)
      ret.mask[i*retw+j] = img.mask[i*imgw+j+off];
  //assert(ret == embedSlow(img,shape));
  return ret;
}


Image compose(Image_ a, Image_ b, const function<int(int,int)>&f, int overlap_only) {
  Image ret;
  if (overlap_only == 1) {
    ret.p = {max(a.p.x,b.p.x),
	     max(a.p.y,b.p.y)};
    point ra = a.p+a.sz, rb = b.p+b.sz;
    ret.sz = {min(ra.x,rb.x),min(ra.y,rb.y)};
    ret.sz = ret.sz-ret.p;
    if (ret.w <= 0 || ret.h <= 0) return badImg;
  } else if (overlap_only == 0) {
    ret.p = {min(a.p.x,b.p.x),
	     min(a.p.y,b.p.y)};
    point ra = a.p+a.sz, rb = b.p+b.sz;
    ret.sz = {max(ra.x,rb.x),max(ra.y,rb.y)};
    ret.sz = ret.sz-ret.p;
  } else if (overlap_only == 2) {
    ret.p = a.p;
    ret.sz = a.sz;
  } else assert(0);
  if (ret.w > MAXSIDE || ret.h > MAXSIDE || ret.w*ret.h > MAXAREA) return badImg;
  ret.mask.assign(ret.w*ret.h, 0);
  point da = ret.p-a.p;
  point db = ret.p-b.p;
  for (int i = 0; i < ret.h; i++) {
    for (int j = 0; j < ret.w; j++) {
      int ca = a.safe(i+da.y, j+da.x);
      int cb = b.safe(i+db.y, j+db.x);
      ret(i,j) = f(ca,cb);
    }
  }
  return ret;
}


Image compose(Image_ a, Image_ b, int id) { //id = 0
  if (id == 0) {
    return compose(a, b, [](int a, int b) {return b ? b : a;}, 0); //a then b, inside either
  } else if (id == 1) {
    return compose(a, b, [](int a, int b) {return b ? b : a;}, 1); //a then b, inside both
  } else if (id == 2) {
    return compose(a, b, [](int a, int b) {return b ? a : 0;}, 1); //a masked by b
  } else if (id == 3) {
    return compose(a, b, [](int a, int b) {return b ? b : a;}, 2); //a then b, inside of a
  } else if (id == 4) {
    return compose(a, b, [](int a, int b) {return b ? 0 : a;}, 2); //a masked by inverse of b, inside of a
  } else assert(id >= 0 && id < 5);
  return badImg;
}

Image outerProductIS(Image_ a, Image_ b) {
  if (a.w*b.w > MAXSIDE || a.h*b.h > MAXSIDE || a.w*b.w*a.h*b.h > MAXAREA) return badImg;
  point rpos = {a.p.x*b.w+b.p.x,
		a.p.y*b.h+b.p.y};
  Image ret = core::empty(rpos, {a.w*b.w, a.h*b.h});
  for (int i = 0; i < a.h; i++)
    for (int j = 0; j < a.w; j++)
      for (int k = 0; k < b.h; k++)
	for (int l = 0; l < b.w; l++)
	  ret(i*b.h+k, j*b.w+l) = a(i,j) * !!b(k,l);
  return ret;
}
Image outerProductSI(Image_ a, Image_ b) {
  if (a.w*b.w > MAXSIDE || a.h*b.h > MAXSIDE || a.w*b.w*a.h*b.h > MAXAREA) return badImg;
  point rpos = {a.p.x*b.w+b.p.x,
		a.p.y*b.h+b.p.y};
  Image ret = core::empty(rpos, {a.w*b.w, a.h*b.h});
  for (int i = 0; i < a.h; i++)
    for (int j = 0; j < a.w; j++)
      for (int k = 0; k < b.h; k++)
	for (int l = 0; l < b.w; l++)
	  ret(i*b.h+k, j*b.w+l) = (a(i,j)>0) * b(k,l);
  return ret;
}



Image Fill(Image_ a) {
  Image ret = core::full(a.p, a.sz, core::majorityCol(a));
  vector<pair<int,int>> q;
  for (int i = 0; i < a.h; i++)
    for (int j = 0; j < a.w; j++)
      if ((i == 0 || j == 0 || i == a.h-1 || j == a.w-1) && !a(i,j)) {
	q.emplace_back(i,j);
	ret(i,j) = 0;
      }
  while (q.size()) {
    auto [r,c] = q.back();
    q.pop_back();
    for (int d = 0; d < 4; d++) {
      int nr = r+(d==2)-(d==3);
      int nc = c+(d==0)-(d==1);
      if (nr >= 0 && nr < a.h && nc >= 0 && nc < a.w && !a(nr,nc) && ret(nr,nc)) {
	q.emplace_back(nr,nc);
	ret(nr,nc) = 0;
      }
    }
  }
  return ret;
}

Image interior(Image_ a) {
  return compose(Fill(a), a, [](int x, int y) {return y ? 0 : x;}, 0);
}

Image border(Image_ a) {
  Image ret = core::empty(a.p, a.sz);
  vector<pair<int,int>> q;
  for (int i = 0; i < a.h; i++)
    for (int j = 0; j < a.w; j++)
      if (i == 0 || j == 0 || i == a.h-1 || j == a.w-1) {
	if (!a(i,j))
	  q.emplace_back(i,j);
	ret(i,j) = 1;
      }
  while (q.size()) {
    auto [r,c] = q.back();
    q.pop_back();
#define DO(nr,nc) {if (!ret(nr,nc)) { ret(nr,nc) = 1; if (!a(nr,nc)) q.emplace_back(nr,nc); }}
    if (r > 0) {
      if (c > 0) DO(r-1,c-1);
      DO(r-1,c);
      if (c+1 < a.w) DO(r-1,c+1);
    }
    if (r+1 < a.h) {
      if (c > 0) DO(r+1,c-1);
      DO(r+1,c);
      if (c+1 < a.w) DO(r+1,c+1);
    }

    if (c > 0) DO(r,c-1);
    if (c+1 < a.w) DO(r,c+1);
#undef DO
    /*for (int nr : {r-1,r,r+1}) {
      for (int nc : {c-1,c,c+1}) {
	if (nr >= 0 && nr < a.h && nc >= 0 && nc < a.w && !ret(nr,nc)) {
    if (!a(nr,nc))
	  q.emplace_back(nr,nc);
	  ret(nr,nc) = 1;
	}
      }
      }*/
  }
    for (int i = 0; i < a.mask.size(); i++)
      ret.mask[i] = ret.mask[i]*a.mask[i];
  return ret;
}


Image alignx(Image_ a, Image_ b, int id) {
  assert(id >= 0 && id < 5);
  Image ret = a;
  if (id == 0)      ret.x = b.x-a.w;
  else if (id == 1) ret.x = b.x;
  else if (id == 2) ret.x = b.x+(b.w-a.w)/2;
  else if (id == 3) ret.x = b.x+b.w-a.w;
  else if (id == 4) ret.x = b.x+b.w;
  return ret;
}
Image aligny(Image_ a, Image_ b, int id) {
  assert(id >= 0 && id < 5);
  Image ret = a;
  if (id == 0)      ret.y = b.y-a.h;
  else if (id == 1) ret.y = b.y;
  else if (id == 2) ret.y = b.y+(b.h-a.h)/2;
  else if (id == 3) ret.y = b.y+b.h-a.h;
  else if (id == 4) ret.y = b.y+b.h;
  return ret;
}
Image align(Image_ a, Image_ b, int idx, int idy) {
  assert(idx >= 0 && idx < 6);
  assert(idy >= 0 && idy < 6);
  Image ret = a;
  if      (idx == 0) ret.x = b.x-a.w;
  else if (idx == 1) ret.x = b.x;
  else if (idx == 2) ret.x = b.x+(b.w-a.w)/2;
  else if (idx == 3) ret.x = b.x+b.w-a.w;
  else if (idx == 4) ret.x = b.x+b.w;

  if      (idy == 0) ret.y = b.y-a.h;
  else if (idy == 1) ret.y = b.y;
  else if (idy == 2) ret.y = b.y+(b.h-a.h)/2;
  else if (idy == 3) ret.y = b.y+b.h-a.h;
  else if (idy == 4) ret.y = b.y+b.h;
  return ret;
}



Image align(Image_ a, Image_ b) {
  //Find most matching color and align a to b using it
  Image ret = a;
  int match_size = 0;
  for (int c = 1; c < 10; c++) {
    Image ca = compress(filterCol(a, c));
    Image cb = compress(filterCol(b, c));
    if (ca.mask == cb.mask) {
      int cnt = core::count(ca);
      if (cnt > match_size) {
	match_size = cnt;
	ret.p = a.p+cb.p-ca.p;
      }
    }
  }
  if (match_size == 0) return badImg;
  return ret;
}

Image replaceCols(Image_ base, Image_ cols) {
  Image ret = base;
  Image done = core::empty(base.p,base.sz);
  point d = base.p-cols.p;
  for (int i = 0; i < base.h; i++) {
    for (int j = 0; j < base.w; j++) {
      if (!done(i,j) && base(i,j)) {
	int acol = base(i,j);
	int cnt[10] = {};
	vector<pair<int,int>> path;
	function<void(int,int)> dfs = [&](int r, int c) {
	  if (r < 0 || r >= base.h || c < 0 || c >= base.w || base(r,c) != acol || done(r,c)) return;
	  cnt[cols.safe(r+d.y,c+d.x)]++;
	  path.emplace_back(r,c);
	  done(r,c) = 1;
	  for (int nr : {r-1,r,r+1})
	    for (int nc : {c-1,c,c+1})
	      dfs(nr,nc);
	};
	dfs(i,j);
	pair<int,int> maj = {0,0};
	for (int c = 1; c < 10; c++) {
	  maj = max(maj, make_pair(cnt[c], -c));
	}
	for (auto [r,c] : path)
	  ret(r,c) = -maj.second;
      }
    }
  }
  return ret;
}

//Exploit symmetries?
Image center(Image_ img) {
  point sz = {(img.w+1)%2+1,
	      (img.h+1)%2+1};
  return core::full(img.p+(img.sz-sz)/2, sz);
}

Image transform(Image_ img, int A00, int A01, int A10, int A11) {
  if (img.w*img.h == 0) return img;
  Image c = center(img);
  point off = point{1-c.w,1-c.h}+(img.p-c.p)*2;
  auto t = [&](point p) {
    p = p*2+off;
    p = {A00*p.x+A01*p.y,
	 A10*p.x+A11*p.y};
    p = p-off;
    p.x >>= 1;
    p.y >>= 1;
    return p;
  };
  point corner[4] = {t({0,0}),t({img.w-1,0}),t({0,img.h-1}),t({img.w-1,img.h-1})};
  point a = corner[0], b = corner[0];
  for (int i = 1; i < 4; i++) {
    a.x = min(a.x, corner[i].x);
    a.y = min(a.y, corner[i].y);
    b.x = max(b.x, corner[i].x);
    b.y = max(b.y, corner[i].y);
  }
  Image ret = core::empty(img.p, b-a+point{1,1});
  for (int i = 0; i < img.h; i++) {
    for (int j = 0; j < img.w; j++) {
      point go = t({j,i})-a;
      ret(go.y,go.x) = img(i,j);
    }
  }
  return ret;
}

int mirrorHeuristic(Image_ img) {
  //Meant to be used for mirroring, flip either x or y, depending on center of gravity
  int cnt = 0, sumx = 0, sumy = 0;
  for (int i = 0; i < img.h; i++) {
    for (int j = 0; j < img.w; j++) {
      if (img(i,j)) {
	cnt++;
	sumx += j;
	sumy += i;
      }
    }
  }
  return abs(sumx*2-(img.w-1)*cnt) < abs(sumy*2-(img.h-1)*cnt);
}

Image rigid(Image_ img, int id) {
  if (id == 0) return img;
  else if (id == 1) return transform(img, 0, 1,-1, 0); //CCW
  else if (id == 2) return transform(img,-1, 0, 0,-1); //180
  else if (id == 3) return transform(img, 0,-1, 1, 0); //CW
  else if (id == 4) return transform(img,-1, 0 ,0, 1); //flip x
  else if (id == 5) return transform(img, 1, 0, 0,-1); //flip y
  else if (id == 6) return transform(img, 0, 1, 1, 0); //swap xy
  else if (id == 7) return transform(img, 0,-1,-1, 0); //swap other diagonal
  else if (id == 8) return rigid(img, 4+mirrorHeuristic(img));
  else assert(id >= 0 && id < 9);
  return badImg;
}

Image invert(Image img) {
  if (img.w*img.h == 0) return img;
  int mask = core::colMask(img);
  int col = 1;
  while (col < 10 && (mask>>col&1) == 0) col++;
  if (col == 10) col = 1;

  for (int i = 0; i < img.h; i++)
    for (int j = 0; j < img.w; j++)
      img(i,j) = img(i,j) ? 0 : col;
  return img;
}


Image interior2(Image_ a) {
  return compose(a, invert(border(a)), 2);
}



Image count(Image_ img, int id, int outType) {
  assert(id >= 0 && id < 7);
  assert(outType >= 0 && outType < 3);
  int num;
  if (id == 0)      num = core::count(img);
  else if (id == 1) num = core::countCols(img);
  else if (id == 2) num = core::countComponents(img);
  else if (id == 3) num = img.w;
  else if (id == 4) num = img.h;
  else if (id == 5) num = max(img.w,img.h);
  else if (id == 6) num = min(img.w,img.h);
  else assert(0);

  point sz;
  if (outType == 0) sz = {num,num};
  else if (outType == 1) sz = {num,1};
  else if (outType == 2) sz = {1,num};
  else assert(0);

  if (max(sz.x,sz.y) > MAXSIDE || sz.x*sz.y > MAXAREA) return badImg;
  return core::full(sz, core::majorityCol(img));
}


Image myStack(Image_ a, Image b, int orient) {
  assert(orient >= 0 && orient <= 3);
  b.p = a.p;
  if (orient == 0) { //Horizontal
    b.x += a.w;
  } else if (orient == 1) { //Vertical
    b.y += a.h;
  } else if (orient == 2) { //Diagonal
    b.x += a.w;
    b.y += a.h;
  } else { //Other diagonal, bottom-left / top-right
    Image c = a;
    c.y += b.h;
    b.x += a.w;
    return compose(c,b);
  }
  return compose(a,b);
}


Image wrap(Image_ line, Image_ area) {
  if (line.w*line.h == 0 || area.w*area.h == 0) return badImg;
  Image ans = core::empty(area.sz);
  for (int i = 0; i < line.h; i++) {
    for (int j = 0; j < line.w; j++) {
      int x = j, y = i;
      x += y/area.h * line.w;
      y %= area.h;

      y += x/area.w * line.h;
      x %= area.w;

      if (x >= 0 && y >= 0 && x < ans.w && y < ans.h)
	ans(y,x) = line(i,j);
    }
  }
  return ans;
}

Image smear(Image_ base, Image_ room, int id) {
  assert(id >= 0 && id < 7);
  const int arr[] = {1,2,4,8,3,12,15};
  int mask = arr[id];

  point d = room.p-base.p;

  Image ret = embed(base, hull(room));
  if (mask&1) {
    for (int i = 0; i < ret.h; i++) {
      char c = 0;
      for (int j = 0; j < ret.w; j++) {
	if (!room(i,j)) c = 0;
	else if (base.safe(i+d.y,j+d.x)) c = base(i+d.y,j+d.x);
	if (c) ret(i,j) = c;
      }
    }
  }

  if (mask>>1&1) {
    for (int i = 0; i < ret.h; i++) {
      char c = 0;
      for (int j = ret.w-1; j >= 0; j--) {
	if (!room(i,j)) c = 0;
	else if (base.safe(i+d.y,j+d.x)) c = base(i+d.y,j+d.x);
	if (c) ret(i,j) = c;
      }
    }
  }

  if (mask>>2&1) {
    for (int j = 0; j < ret.w; j++) {
      char c = 0;
      for (int i = 0; i < ret.h; i++) {
	if (!room(i,j)) c = 0;
	else if (base.safe(i+d.y,j+d.x)) c = base(i+d.y,j+d.x);
	if (c) ret(i,j) = c;
      }
    }
  }

  if (mask>>3&1) {
    for (int j = 0; j < ret.w; j++) {
      char c = 0;
      for (int i = ret.h-1; i >= 0; i--) {
	if (!room(i,j)) c = 0;
	else if (base.safe(i+d.y,j+d.x)) c = base(i+d.y,j+d.x);
	if (c) ret(i,j) = c;
      }
    }
  }

  return ret;
}

/*
Image smear(Image_ base, int id) {
  return smear(base, hull(base), id);
}
*/

Image extend(Image_ img, Image_ room) {
  if (img.w*img.h == 0) return badImg;
  Image ret = room;
  for (int i = 0; i < ret.h; i++) {
    for (int j = 0; j < ret.w; j++) {
      point p = point{j,i}+room.p-img.p;
      p.x = clamp(p.x, 0, img.w-1);
      p.y = clamp(p.y, 0, img.h-1);
      ret(i,j) = img(p.y,p.x);
    }
  }
  return ret;
}








Image pickMax(vImage_ v, const function<int(Image_)>& f) {
  if (v.empty()) return badImg;
  int ma = f(v[0]), maxi = 0;
  for (int i = 1; i < v.size(); i++) {
    int score = f(v[i]);
    if (score > ma) {
      ma = score;
      maxi = i;
    }
  }
  return v[maxi];
}

int maxCriterion(Image_ img, int id) {
  assert(id >= 0 && id < 14);
  switch (id) {
  case 0: return  core::count(img);
  case 1: return -core::count(img);
  case 2: return  img.w*img.h;
  case 3: return -img.w*img.h;
  case 4: return core::countCols(img);
  case 5: return -img.p.y;
  case 6: return  img.p.y;
  case 7: return  core::countComponents(img);
  case 8:
    {
      Image comp = compress(img);
      return comp.w*comp.h-core::count(comp);
    }
  case 9:
    {
      Image comp = compress(img);
      return -(comp.w*comp.h-core::count(comp));
    }
  case 10: return  core::count(interior(img));
  case 11: return -core::count(interior(img));
  case 12: return -img.p.x;
  case 13: return  img.p.x;
  }
  return -1;
}


Image pickMax(vImage_ v, int id) {
  return pickMax(v, [id](Image_ img) {return maxCriterion(img,id);});
}

vImage cut(Image_ img, Image_ a) {
  vector<Image> ret;
  Image done = core::empty(img.p,img.sz);
  point d = img.p-a.p;
  for (int i = 0; i < img.h; i++) {
    for (int j = 0; j < img.w; j++) {
      if (!done(i,j) && !a.safe(i+d.y,j+d.x)) {
	Image toadd = core::empty(img.p,img.sz);
	function<void(int,int)> dfs = [&](int r, int c) {
	  if (r < 0 || r >= img.h || c < 0 || c >= img.w || a.safe(r+d.y,c+d.x) || done(r,c)) return;
	  toadd(r,c) = img(r,c)+1;
	  done(r,c) = 1;
	  for (int nr : {r-1,r,r+1})
	    for (int nc : {c-1,c,c+1})
	      dfs(nr,nc);
	};
	dfs(i,j);
	toadd = compress(toadd);
	for (int i = 0; i < toadd.h; i++) {
	  for (int j = 0; j < toadd.w; j++) {
	    toadd(i,j) = max(0, toadd(i,j)-1);
	  }
	}
	ret.push_back(toadd);
      }
    }
  }
  return ret;
}


vImage splitCols(Image_ img, int include0) { //include0 = 0
  vector<Image> ret;
  int mask = core::colMask(img);
  for (int c = !include0; c < 10; c++) {
    if (mask>>c&1) {
      Image s = img;
      for (int i = 0; i < s.h; i++)
	for (int j = 0; j < s.w; j++)
	  s(i,j) = (s(i,j) == c ? c : 0);
      ret.push_back(s);
    }
  }
  return ret;
}

Image compose(vImage_ imgs, int id) {
  if (imgs.empty()) return badImg;
  Image ret = imgs[0];
  for (int i = 1; i < imgs.size(); i++)
    ret = compose(ret, imgs[i], id);
  return ret;
}

void getRegular(vector<int>&col) {
  int colw = col.size();

  for (int w = 1; w < colw; w++) {
    int s = -1;
    if (colw%(w+1) == w) { //No outer border
      s = w;
    } else if (colw%(w+1) == 1) { //Outer border
      s = 0;
    }
    if (s != -1) {
      int ok = 1;
      for (int i = 0; i < colw; i++) {
	if (col[i] != (i%(w+1) == s)) {
	  ok = 0;
	  break;
	}
      }
      if (ok) {
	return;
      }
    }
  }
  fill(col.begin(), col.end(), 0);
}

Image getRegular(Image_ img) {
  //Look for regular grid division in single color
  Image ret = img;
  vector<int> col(img.w,1), row(img.h,1);
  for (int i = 0; i < img.h; i++) {
    for (int j = 0; j < img.w; j++) {
      if (img(i,j) != img(i,0)) row[i] = 0;
      if (img(i,j) != img(0,j)) col[j] = 0;
    }
  }
  getRegular(col);
  getRegular(row);
  for (int i = 0; i < img.h; i++) {
    for (int j = 0; j < img.w; j++) {
      ret(i,j) = row[i] || col[j];
    }
  }
  return ret;
}


Image cutPickMax(Image_ a, Image_ b, int id) {
  return pickMax(cut(a,b), id);
}
Image regularCutPickMax(Image_ a, int id) {
  Image b = getRegular(a);
  return pickMax(cut(a,b), id);
}
Image splitPickMax(Image_ a, int id, int include0) { //include0 = 0
  return pickMax(splitCols(a, include0), id);
}

Image cutCompose(Image_ a, Image_ b, int id) {
  auto v = cut(a,b);
  for (Image& img : v) img = toOrigin(img);
  return compose(v, id);
}
Image regularCutCompose(Image_ a, int id) {
  Image b = getRegular(a);
  auto v = cut(a,b);
  for (Image& img : v) img = toOrigin(img);
  return compose(v, id);
}
Image splitCompose(Image_ a, int id, int include0) { //include0 = 0
  auto v = splitCols(a, include0);
  for (Image& img : v) img = toOrigin(compress(img));
  return compose(v, id);
}

Image cutIndex(Image_ a, Image_ b, int ind) {
  auto v = cut(a,b);
  if (ind < 0 || ind >= (int)v.size()) return badImg;
  return v[ind];
}


vImage pickMaxes(vImage_ v, function<int(Image_)> f, int invert = 0) {
  int n = v.size();
  if (!n) return {};
  vector<int> score(n);
  int ma = -1e9;
  for (int i = 0; i < n; i++) {
    score[i] = f(v[i]);
    if (!i || score[i] > ma)
      ma = score[i];
  }
  vector<Image> ret_imgs;
  for (int i = 0; i < n; i++)
    if ((score[i] == ma) ^ invert)
      ret_imgs.push_back(v[i]);
  return ret_imgs;
}

vImage pickMaxes(vImage_ v, int id) {
  return pickMaxes(v, [id](Image_ img){return maxCriterion(img, id);}, 0);
}
vImage pickNotMaxes(vImage_ v, int id) {
  return pickMaxes(v, [id](Image_ img){return maxCriterion(img, id);}, 1);
}

Image cutPickMaxes(Image_ a, Image_ b, int id) {
  return compose(pickMaxes(cut(a,b), id), 0);
}
Image splitPickMaxes(Image_ a, int id) {
  return compose(pickMaxes(splitCols(a), id), 0);
}

//Return a single color
//Cut into at least 2 pieces
//No nested pieces
//Must touch at least 2 opposite sides
//Smallest piece should be as big as possible

Image heuristicCut(Image_ img) {
  int ret = core::majorityCol(img, 1);
  int ret_score = -1;

  int mask = core::colMask(img);
  Image done = core::empty(img.p,img.sz);
  for (int col = 0; col < 10; col++) {
    if ((mask>>col&1) == 0) continue;
    fill(done.mask.begin(), done.mask.end(), 0);
    function<void(int,int)> edgy = [&](int r, int c) {
      if (r < 0 || r >= img.h || c < 0 || c >= img.w || img(r,c) != col || done(r,c)) return;
      done(r,c) = 1;
      for (int nr : {r-1,r,r+1})
	for (int nc : {c-1,c,c+1})
	  edgy(nr,nc);
    };
    int top = 0, bot = 0, left = 0, right = 0;
    for (int i = 0; i < img.h; i++) {
      for (int j = 0; j < img.w; j++) {
	if (img(i,j) == col) {
	  if (i == 0) top = 1;
	  if (j == 0) left = 1;
	  if (i == img.h-1) bot = 1;
	  if (j == img.w-1) right = 1;
	}
	if ((i == 0 || j == 0 || i == img.h-1 || j == img.w-1) && img(i,j) == col && !done(i,j)) {
	  edgy(i,j);
	}
      }
    }

    if (!(top && bot || left && right)) continue;

    int score = 1e9, components = 0, nocontained = 1;
    for (int i = 0; i < img.h; i++) {
      for (int j = 0; j < img.w; j++) {
	int cnt = 0, contained = 1;
	if (!done(i,j) && img(i,j) != col) {
	  function<void(int,int)> dfs = [&](int r, int c) {
	    if (r < 0 || r >= img.h || c < 0 || c >= img.w) return;
	    if (img(r,c) == col) {
	      if (done(r,c)) contained = 0;
	      return;
	    }
	    if (done(r,c)) return;
	    cnt++;
	    done(r,c) = 1;
	    for (int nr : {r-1,r,r+1})
	      for (int nc : {c-1,c,c+1})
		dfs(nr,nc);
	  };
	  dfs(i,j);
	  components++;
	  score = min(score, cnt);
	  if (contained) nocontained = 0;
	}
      }
    }
    if (components >= 2 && nocontained && score > ret_score) {
      ret_score = score;
      ret = col;
    }
  }
  return filterCol(img,ret);
}

vImage cut(Image_ img) {
  return cut(img, heuristicCut(img));
}

Image cutPickMax(Image_ a, int id) {
  return cutPickMax(a, heuristicCut(a), id);
}
Image cutIndex(Image_ a, int ind) {
  return cutIndex(a, heuristicCut(a), ind);
}
Image cutPickMaxes(Image_ a, int id) {
  return cutPickMaxes(a, heuristicCut(a), id);
}



Image repeat(Image_ a, Image_ b, int pad) { //pad = 0
  if (a.w*a.h <= 0 || b.w*b.h <= 0) return badImg;
  Image ret;
  ret.p = b.p;
  ret.sz = b.sz;
  ret.mask.resize(ret.w*ret.h,0);

  const int W = a.w+pad, H = a.h+pad;
  int ai  = ((b.y-a.y)%H+H)%H;
  int aj0 = ((b.x-a.x)%W+W)%W;
  for (int i = 0; i < ret.h; i++) {
    int aj = aj0;
    for (int j = 0; j < ret.w; j++) {
      if (ai < a.h && aj < a.w)
	ret(i,j) = a(ai,aj);
      if (++aj == W) aj = 0;
    }
    if (++ai == H) ai = 0;
  }
  return ret;
}

Image mirror(Image_ a, Image_ b, int pad) { //pad = 0
  if (a.w*a.h <= 0 || b.w*b.h <= 0) return badImg;
  Image ret;
  ret.p = b.p;
  ret.sz = b.sz;
  ret.mask.resize(ret.w*ret.h);
  const int W = a.w+pad, H = a.h+pad;
  const int W2 = W*2, H2 = H*2;
  int ai  = ((b.y-a.y)%H2+H2)%H2;
  int aj0 = ((b.x-a.x)%W2+W2)%W2;
  for (int i = 0; i < ret.h; i++) {
    int aj = aj0;
    for (int j = 0; j < ret.w; j++) {
      int x = -1, y = -1;
      if (aj < a.w) x = aj;
      else if (aj >= W && aj < W+a.w) x = W+a.w-1-aj;
      if (ai < a.h) y = ai;
      else if (ai >= H && ai < H+a.h) y = H+a.h-1-ai;
      if (x != -1 && y != -1)
	ret(i,j) = a(y,x);
      if (++aj == W2) aj = 0;
    }
    if (++ai == H2) ai = 0;
  }
  return ret;
}

Image majCol(Image_ img) {
  return Col(core::majorityCol(img));
}
