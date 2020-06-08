#include "precompiled_stl.hpp"
using namespace std;
#include "utils.hpp"
#include "core_functions.hpp"
#include "image_functions.hpp"
#include "spec.hpp"
#include "read.hpp"
#include "normalize.hpp"


vImage splitAll(Image_ img) {
  vector<Image> ret;
  Image done = core::empty(img.p,img.sz);
  for (int i = 0; i < img.h; i++) {
    for (int j = 0; j < img.w; j++) {
      if (!done(i,j)) {
	Image toadd = core::empty(img.p,img.sz);
	function<void(int,int,int)> dfs = [&](int r, int c, int col) {
	  if (r < 0 || r >= img.h || c < 0 || c >= img.w || img(r,c) != col || done(r,c)) return;
	  toadd(r,c) = img(r,c)+1;
	  done(r,c) = 1;
	  for (int d = 0; d < 4; d++)
	    dfs(r+(d==0)-(d==1),c+(d==2)-(d==3),col);
	};
	dfs(i,j,img(i,j));
	toadd = compress(toadd);
	for (int i = 0; i < toadd.h; i++) {
	  for (int j = 0; j < toadd.w; j++) {
	    toadd(i,j) = max(0, toadd(i,j)-1);
	  }
	}
	//Image i = interior(toadd);
	//if (!core::count(compose(img,i,2)))
	if (core::count(toadd))
	  ret.push_back(toadd);
      }
    }
  }
  return ret;
}


Image eraseCol(Image img, int col) {
  for (int i = 0; i < img.h; i++)
    for (int j = 0; j < img.w; j++)
      if (img(i,j) == col) img(i,j) = 0;
  return img;
}


// Looks for 4 corners
vImage insideMarked(Image_ in) {
  vector<Image> ret;
  for (int i = 0; i+1 < in.h; i++) {
    for (int j = 0; j+1 < in.w; j++) {
      for (int h = 1; i+h+1 < in.h; h++) {
	for (int w = 1; j+w+1 < in.w; w++) {
	  char col = in(i,j);
	  if (!col) continue;
	  int ok = 1;
	  for (int k = 0; k < 4; k++) {
	    int x = j+k%2*w, y = i+k/2*h;
	    for (int d = 0; d < 4; d++) {
	      if ((d != 3-k) == (in(y+d/2,x+d%2) != col)) {
		ok = 0;
		goto fail;
	      }
	    }
	  }
	fail:
	  if (ok) {
	    Image inside = invert(core::full(point{j+1,i+1}, point{w,h}));
	    ret.push_back(compose(inside,in,3));
	  }
	}
      }
    }
  }
  return ret;
}

Image makeBorder(Image_ img, int bcol = 1) {
  Image ret = hull0(img);
  for (int i = 0; i < ret.h; i++) {
    for (int j = 0; j < ret.w; j++) {
      if (img(i,j) == 0) {
	int ok = 0;
	for (int ni : {i-1,i,i+1}) {
	  for (int nj : {j-1,j,j+1}) {
	    if (img.safe(ni,nj)) {
	      ok = 1;
	      break;
	    }
	  }
	}
	if (ok) {
	  ret(i,j) = bcol;
	}
      }
    }
  }
  return ret;
}


Image makeBorder2(Image_ img, int usemaj = 1) {
  int bcol = 1;
  if (usemaj) bcol = core::majorityCol(img);

  point rsz = img.sz+point{2,2};
  if (max(rsz.x,rsz.y) > MAXSIDE || rsz.x*rsz.y > MAXAREA) return badImg;
  Image ret = core::full(img.p-point{1,1}, rsz, bcol);
  for (int i = 0; i < img.h; i++)
    for (int j = 0; j < img.w; j++)
      ret(i+1,j+1) = img(i,j);
  return ret;
}

Image makeBorder2(Image_ img, Image_ bord) {
  int bcol = core::majorityCol(bord);
  point rsz = img.sz+bord.sz+bord.sz;
  if (max(rsz.x,rsz.y) > MAXSIDE || rsz.x*rsz.y > MAXAREA) return badImg;
  Image ret = core::full(img.p-bord.sz, rsz, bcol);
  for (int i = 0; i < img.h; i++)
    for (int j = 0; j < img.w; j++)
      ret(i+bord.h,j+bord.w) = img(i,j);
  return ret;
}


//Delete black rows / cols
Image compress2(Image_ img) {
  vector<int> row(img.h), col(img.w);
  for (int i = 0; i < img.h; i++)
    for (int j = 0; j < img.w; j++)
      if (img(i,j)) row[i] = col[j] = 1;
  vector<int> rows, cols;
  for (int i = 0; i < img.h; i++) if (row[i]) rows.push_back(i);
  for (int j = 0; j < img.w; j++) if (col[j]) cols.push_back(j);
  Image ret = core::empty(point{(int)cols.size(), (int)rows.size()});
  for (int i = 0; i < ret.h; i++)
    for (int j = 0; j < ret.w; j++)
      ret(i,j) = img(rows[i],cols[j]);
  return ret;
}


//Group single color rectangles
Image compress3(Image_ img) {
  if (img.w*img.h <= 0) return badImg;
  vector<int> row(img.h), col(img.w);
  row[0] = col[0] = 1;
  for (int i = 0; i < img.h; i++) {
    for (int j = 0; j < img.w; j++) {
      if (i && img(i,j) != img(i-1,j)) row[i] = 1;
      if (j && img(i,j) != img(i,j-1)) col[j] = 1;
    }
  }
  vector<int> rows, cols;
  for (int i = 0; i < img.h; i++) if (row[i]) rows.push_back(i);
  for (int j = 0; j < img.w; j++) if (col[j]) cols.push_back(j);
  Image ret = core::empty(point{(int)cols.size(), (int)rows.size()});
  for (int i = 0; i < ret.h; i++)
    for (int j = 0; j < ret.w; j++)
      ret(i,j) = img(rows[i],cols[j]);
  return ret;
}


//TODO: return badImg if fail

Image greedyFill(Image& ret, vector<pair<int,vector<int>>>&piece, Spec&done, int bw, int bh, int&donew) {
  sort(piece.rbegin(), piece.rend());

  const int dw = ret.w-bw+1, dh = ret.h-bh+1;
  if (dw < 1 || dh < 1) return badImg;

  vector<int> dones(dw*dh, -1);
  priority_queue<tuple<int,int,int>> pq;
  auto recalc = [&](int i, int j) {
    int cnt = 0;
    for (int y = 0; y < bh; y++)
      for (int x = 0; x < bw; x++)
	cnt += done(i+y,j+x);
    if (cnt != dones[i*dw+j]) {
      dones[i*dw+j] = cnt;
      pq.emplace(cnt,j,i);
    }
  };
  for (int i = 0; i+bh <= ret.h; i++)
    for (int j = 0; j+bw <= ret.w; j++)
      recalc(i,j);

  while (pq.size()) {
    auto [ds,j,i] = pq.top();
    pq.pop();
    if (ds != dones[i*dw+j]) continue;
    int found = 0;
    for (auto [cnt,mask] : piece) {
      int ok = 1;
      for (int y = 0; y < bh; y++)
	for (int x = 0; x < bw; x++)
	  if (done(i+y,j+x) && ret(i+y,j+x) != mask[y*bw+x])
	    ok = 0;
      if (ok) {
	for (int y = 0; y < bh; y++) {
	  for (int x = 0; x < bw; x++) {
	    if (!done(i+y,j+x)) {
	      done(i+y,j+x) = donew;
	      if (donew > 1) donew--;
	      ret(i+y,j+x) = mask[y*bw+x];
	    }
	  }
	}
	for (int y = max(i-bh+1,0); y < min(i+bh, dh); y++)
	  for (int x = max(j-bw+1,0); x < min(j+bw, dw); x++)
	    recalc(y,x);
	found = 1;
	break;
      }
    }
    if (!found)
      return badImg;//ret;
  }
  return ret;
}




Image greedyFillBlack(Image_ img, int N = 3) {
  Image ret = core::empty(img.p, img.sz);
  Spec done;
  done.sz = img.sz;
  done.mask.assign(done.w*done.h,0);

  int donew = 1e6;
  for (int i = 0; i < ret.h; i++) {
    for (int j = 0; j < ret.w; j++) {
      if (img(i,j)) {
	ret(i,j) = img(i,j);
	done(i,j) = donew;
      }
    }
  }

  map<vector<int>,int> piece_cnt;
  vector<int> mask;
  const int bw = N, bh = N;
  for (int r = 0; r < 8; r++) {
    Image rot = rigid(img,r);
    for (int i = 0; i+bh <= rot.h; i++) {
      for (int j = 0; j+bw <= rot.w; j++) {
	mask.reserve(bw*bh);
	mask.resize(0);
	int ok = 1;
	for (int y = 0; y < bh; y++)
	  for (int x = 0; x < bw; x++) {
	    char c = rot(i+y,j+x);
	    mask.push_back(c);
	    if (!c) ok = 0;
	  }
	if (ok)
	  ++piece_cnt[mask];
      }
    }
  }
  vector<pair<int,vector<int>>> piece;
  for (auto&[p,c] : piece_cnt)
    piece.emplace_back(c,p);

  return greedyFill(ret, piece, done, bw, bh, donew);
}

Image greedyFillBlack2(Image_ img, int N = 3) {
  Image filled = greedyFillBlack(img, N);
  return compose(filled, img, 4);
}

Image extend2(Image_ img, Image_ room) {
  Image ret = core::empty(room.p, room.sz);
  Spec done;
  done.sz = room.sz;
  done.mask.assign(done.w*done.h,0);

  point d = room.p-img.p;
  int donew = 1e6;
  for (int i = 0; i < ret.h; i++) {
    for (int j = 0; j < ret.w; j++) {
      int x = j+d.x, y = i+d.y;
      if (x >= 0 && y >= 0 && x < img.w && y < img.h) {
	ret(i,j) = img(y,x);
	done(i,j) = donew;
      }
    }
  }

  map<vector<int>,int> piece_cnt;
  vector<int> mask;
  const int bw = 3, bh = 3;
  for (int r = 0; r < 8; r++) {
    Image rot = rigid(img,r);
    for (int i = 0; i+bh <= rot.h; i++) {
      for (int j = 0; j+bw <= rot.w; j++) {
	mask.reserve(bw*bh);
	mask.resize(0);
	for (int y = 0; y < bh; y++)
	  for (int x = 0; x < bw; x++)
	    mask.push_back(rot(i+y,j+x));
	++piece_cnt[mask];
      }
    }
  }
  vector<pair<int,vector<int>>> piece;
  for (auto&[p,c] : piece_cnt)
    piece.emplace_back(c,p);

  return greedyFill(ret, piece, done, bw, bh, donew);
}



Image connect(Image_ img, int id) {
  assert(id >= 0 && id < 3);
  Image ret = core::empty(img.p, img.sz);

  //Horizontal
  if (id == 0 || id == 2) {
    for (int i = 0; i < img.h; i++) {
      int last = -1, lastc = -1;
      for (int j = 0; j < img.w; j++) {
	if (img(i,j)) {
	  if (img(i,j) == lastc) {
	    for (int k = last+1; k < j; k++)
	      ret(i,k) = lastc;
	  }
	  lastc = img(i,j);
	  last = j;
	}
      }
    }
  }

  //Vertical
  if (id == 1 || id == 2) {
    for (int j = 0; j < img.w; j++) {
      int last = -1, lastc = -1;
      for (int i = 0; i < img.h; i++) {
	if (img(i,j)) {
	  if (img(i,j) == lastc) {
	    for (int k = last+1; k < i; k++)
	      ret(k,j) = lastc;
	  }
	  lastc = img(i,j);
	  last = i;
	}
      }
    }
  }

  return ret;
}

Image replaceTemplate(Image_ in, Image_ need_, Image_ marked_, int overlapping = 0, int rigids = 0) {
  if (marked_.sz != need_.sz) return badImg;
  if (need_.w*need_.h <= 0) return in;

  const int rots = rigids ? 8 : 1;
  vector<Image> needr(rots), markedr(rots);
  for (int r = 0; r < rots; r++) {
    needr[r] = rigid(need_,r);
    markedr[r] = rigid(marked_,r);
  }

  Image ret = in;
  for (int r = 0; r < rots; r++) {
    Image_ need = needr[r];
    Image_ marked = markedr[r];

    for (int i = 0; i+need.h <= ret.h; i++) {
      for (int j = 0; j+need.w <= ret.w; j++) {
	int ok = 1;
	for (int y = 0; y < need.h; y++)
	  for (int x = 0; x < need.w; x++)
	    if ((overlapping ? in : ret)(i+y,j+x) != need(y,x)) ok = 0;

	if (overlapping == 2) {
	  for (int y = -1; y <= need.h; y++) {
	    for (int x = -1; x <= need.w; x++) {
	      if (x >= 0 && y >= 0 && x < need.w && y < need.h) continue;

	      char nn = need(clamp(y,0,need.h-1),
			     clamp(x,0,need.w-1));
	      if (nn && nn == in.safe(i+y,j+x)) ok = 0;
	    }
	  }
	}

	if (ok) {
	  for (int y = 0; y < need.h; y++)
	    for (int x = 0; x < need.w; x++)
	      ret(i+y,j+x) = marked(y,x);
	}
      }
    }
  }
  return ret;
}



Image swapTemplate(Image_ in, Image_ a, Image_ b, int rigids = 0) {
  if (a.sz != b.sz) return badImg;
  if (a.w*a.h <= 0) return in;

  const int rots = rigids ? 8 : 1;
  vector<Image> ar(rots), br(rots);
  for (int r = 0; r < rots; r++) {
    ar[r] = rigid(a,r);
    br[r] = rigid(b,r);
  }
  Image done = hull0(in), ret = in;
  for (int k : {0,1}) {
    for (int r = 0; r < rots; r++) {
      Image_ need = k ? ar[r] : br[r];
      Image_ to   = k ? br[r] : ar[r];

      for (int i = 0; i+need.h <= ret.h; i++) {
	for (int j = 0; j+need.w <= ret.w; j++) {

	  int ok = 1;
	  for (int y = 0; y < need.h; y++)
	    for (int x = 0; x < need.w; x++)
	      if (done(i+y,j+x) || ret(i+y,j+x) != need(y,x)) ok = 0;
	  if (ok) {
	    for (int y = 0; y < need.h; y++) {
	      for (int x = 0; x < need.w; x++) {
		ret(i+y,j+x) = to(y,x);
		done(i+y,j+x) = 1;
	      }
	    }
	  }
	}

      }
    }
  }
  return ret;
}


Image spreadCols(Image img, int skipmaj = 0) {
  int skipcol = -1;
  if (skipmaj)
    skipcol = core::majorityCol(img);

  Image done = hull0(img);
  queue<tuple<int,int,int>> q;
  for (int i = 0; i < img.h; i++) {
    for (int j = 0; j < img.w; j++) {
      if (img(i,j)) {
	if (img(i,j) != skipcol)
	  q.emplace(j,i,img(i,j));
	done(i,j) = 1;
      }
    }
  }
  while (q.size()) {
    auto [j,i,c] = q.front();
    q.pop();
    for (int d = 0; d < 4; d++) {
      int ni = i+(d==0)-(d==1);
      int nj = j+(d==2)-(d==3);
      if (ni >= 0 && nj >= 0 && ni < img.h && nj < img.w && !done(ni,nj)) {
	img(ni,nj) = c;
	done(ni,nj) = 1;
	q.emplace(nj,ni,c);
      }
    }
  }
  return img;
}


vImage splitColumns(Image_ img) {
  if (img.w*img.h <= 0) return {};
  vector<Image> ret(img.w);
  for (int j = 0; j < img.w; j++) {
    ret[j].p = {j,0};
    ret[j].sz = {1,img.h};
    ret[j].mask.resize(img.h);
    for (int i = 0; i < img.h; i++)
      ret[j].mask[i] = img(i,j);
  }
  return ret;
}
vImage splitRows(Image_ img) {
  if (img.w*img.h <= 0) return {};
  vector<Image> ret(img.h);
  for (int i = 0; i < img.h; i++) {
    ret[i].p = {0,i};
    ret[i].sz = {img.w,1};
    ret[i].mask.resize(img.w);
    for (int j = 0; j < img.w; j++)
      ret[i].mask[j] = img(i,j);
  }
  return ret;
}


Image half(Image_ img, int id) {
  assert(id >= 0 && id < 4);
  if (id == 0) {
    return core::subImage(img, {0,0}, {img.w/2, img.h});
  } else if (id == 1) {
    return core::subImage(img, {img.w-img.w/2,0}, {img.w/2, img.h});
  } else if (id == 2) {
    return core::subImage(img, {0,0}, {img.w, img.h/2});
  } else if (id == 3) {
    return core::subImage(img, {0,img.h-img.h/2}, {img.w, img.h/2});
  }
  return badImg;
}


Image smear(Image_ img, int id) {
  assert(id >= 0 && id < 15);
  const pair<int,int> R = {1,0}, L = {-1,0}, D = {0,1}, U = {0,-1};
  const pair<int,int> X = {1,1}, Y = {-1,-1}, Z = {1,-1}, W = {-1,1};
  const vector<pair<int,int>> d[15] = {{R},{L},{D},{U},
				       {R,L},{D,U},
				       {R,L,D,U},
				       {X},{Y},{Z},{W},
				       {X,Y},{Z,W},
				       {X,Y,Z,W},
				       {R,L,D,U,X,Y,Z,W}};
  const int w = img.w;
  Image ret = img;


  for (auto [dx,dy] : d[id]) {
    int di = dy*w+dx;

    for (int i = 0; i < ret.h; i++) {
      int step = i == 0 || i == ret.h-1 ? 1 : max(ret.w-1,1);
      for (int j = 0; j < ret.w; j += step) {
	if (i-dy < 0 || j-dx < 0 || i-dy >= img.h || j-dx >= img.w) {
	  int steps = MAXSIDE;
	  if (dx ==-1) steps = min(steps, j+1);
	  if (dx == 1) steps = min(steps, img.w-j);
	  if (dy ==-1) steps = min(steps, i+1);
	  if (dy == 1) steps = min(steps, img.h-i);

	  int ind = i*w+j;
	  int end_ind = ind+steps*di;
	  int c = 0;
	  for (; ind != end_ind; ind += di) {
	    if (img.mask[ind]) c = img.mask[ind];
	    if (c) ret.mask[ind] = c;
	  }
	}
      }
    }
  }

  return ret;
}


Image mirror2(Image_ a, Image_ line) {
  Image ret;
  if (line.w > line.h) {
    ret = rigid(a,5);
    ret.x = a.x;
    ret.y = line.y*2+line.h-a.y-a.h;
  } else {
    ret = rigid(a,4);
    ret.y = a.y;
    ret.x = line.x*2+line.w-a.x-a.w;
  }
  return ret;
}

vImage gravity(Image_ in, int d) {
  vImage pieces = splitAll(in);
  Image room = hull0(in);
  int dx = (d==0)-(d==1);
  int dy = (d==2)-(d==3);

  vImage ret;
  Image out = room;
  sort(pieces.begin(), pieces.end(), [dx,dy](Image_ a, Image_ b) {
      return (a.x-b.x)*dx+(a.y-b.y)*dy > 0;});
  for (Image p : pieces) {
    while (1) {
      p.x += dx;
      p.y += dy;
      for (int i = 0; i < p.h; i++) {
	for (int j = 0; j < p.w; j++) {
	  if (p(i,j) == 0) continue;
	  int x = j+p.x-out.x;
	  int y = i+p.y-out.y;
	  if (x < 0 || y < 0 || x >= out.w || y >= out.h || out(y,x)) {
	    p.x -= dx;
	    p.y -= dy;
	    goto done;
	  }
	}
      }
    }
  done:
    ret.push_back(p);
    out = compose(out, p, 3);
  }
  return ret;
}



Image myStack(vImage_ lens, int id) {
  int n = lens.size();
  if (!n) return badImg;
  vector<pair<int,int>> order(n);
  for (int i = 0; i < n; i++) {
    order[i] = {lens[i].w*lens[i].h,i};
  }
  sort(order.begin(), order.end());

  Image out = lens[order[0].second];
  for (int i = 1; i < n; i++)
    out = myStack(out,lens[order[i].second],id);
  return out;
}

Image stackLine(vImage_ shapes) {
  int n = shapes.size();
  if (!n) return badImg;
  else if (n == 1) return shapes[0];
  vector<int> xs(n), ys(n);
  for (int i = 0; i < n; i++) {
    xs[i] = shapes[i].x;
    ys[i] = shapes[i].y;
  }
  sort(xs.begin(), xs.end());
  sort(ys.begin(), ys.end());
  int xmin = 1e9, ymin = 1e9;
  for (int i = 1; i < n; i++) {
    xmin = min(xmin, xs[i]-xs[i-1]);
    ymin = min(ymin, ys[i]-ys[i-1]);
  }
  int dx = 1, dy = 0;
  if (xmin < ymin) dx = 0, dy = 1;

  vector<pair<int,int>> order(n);
  for (int i = 0; i < shapes.size(); i++) {
    order[i] = {shapes[i].x*dx+shapes[i].y*dy,i};
  }
  sort(order.begin(), order.end());

  Image out = shapes[order[0].second];
  for (int i = 1; i < n; i++)
    out = myStack(out,shapes[order[i].second],dy);
  return out;
}


Image composeGrowingSlow(vImage_ imgs) {
  int n = imgs.size();
  if (!n) return badImg;

  vector<pair<int,int>> order(n);
  for (int i = 0; i < n; i++) {
    order[i] = {core::count(imgs[i]), i};
  }
  sort(order.rbegin(), order.rend());

  Image ret = imgs[order[0].second];
  for (int i = 1; i < n; i++)
    ret = compose(ret, imgs[order[i].second], 0);
  return ret;
}


Image composeGrowing(vImage_ imgs) {
  int n = imgs.size();
  if (!n) return badImg;
  if (n == 1) return imgs[0];

  int minx = 1e9, miny = 1e9, maxx = -1e9, maxy = -1e9;
  for (Image_ img : imgs) {
    minx = min(minx, img.x);
    miny = min(miny, img.y);
    maxx = max(maxx, img.x+img.w);
    maxy = max(maxy, img.y+img.h);
  }

  point rsz = {maxx-minx, maxy-miny};
  if (max(rsz.x, rsz.y) > MAXSIDE || rsz.x*rsz.y > MAXAREA || rsz.x <= 0 || rsz.y <= 0)
    return badImg;

  vector<pair<int,int>> order(n);
  for (int i = 0; i < n; i++) {
    order[i] = {core::count(imgs[i]), i};
  }
  sort(order.rbegin(), order.rend());

  Image ret = core::empty(point{minx, miny}, rsz);
  for (auto [cnt,imgi] : order) {
    Image_ img = imgs[imgi];
    int dx = img.x-ret.x, dy = img.y-ret.y;
    for (int i = 0; i < img.h; i++) {
      for (int j = 0; j < img.w; j++) {
	if (img(i,j))
	  ret(i+dy,j+dx) = img(i,j);
      }
    }
  }
  //assert(ret == composeGrowingSlow(imgs));
  return ret;
}


Image pickUnique(vImage_ imgs, int id) {
  assert(id == 0);

  int n = imgs.size();
  if (!n) return badImg;

  //Pick the one with the unique color
  vector<int> mask(n);
  vector<int> cnt(10);
  for (int i = 0; i < n; i++) {
    mask[i] = core::colMask(imgs[i]);
    for (int c = 0; c < 10; c++) {
      if (mask[i]>>c&1) cnt[c]++;
    }
  }
  int reti = -1;
  for (int i = 0; i < n; i++) {
    for (int c = 0; c < 10; c++) {
      if (mask[i]>>c&1) {
	if (cnt[c] == 1) {
	  if (reti == -1) reti = i;
	  else return badImg;
	}
      }
    }
  }
  if (reti == -1) return badImg;
  return imgs[reti];
}
