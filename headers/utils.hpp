//const int MAXSIDE = 100, MAXAREA = 40*40, MAXPIXELS = 40*40*5;
extern int MAXSIDE, MAXAREA, MAXPIXELS;

//#define CHECK_BOUNDS

struct point {
  int x, y;
};
inline point operator+(const point& a, const point& b) {
  return {a.x+b.x, a.y+b.y};
}
inline point operator-(const point& a, const point& b) {
  return {a.x-b.x, a.y-b.y};
}
inline point operator*(const point& a, int f) {
  return {a.x*f, a.y*f};
}
inline point operator/(const point& a, int f) {
  assert(a.x%f == 0 && a.y%f == 0);
  return {a.x/f, a.y/f};
}
inline int operator*(const point& a, const point& b) {
  return a.x*b.x+a.y*b.y;
}
inline int operator%(const point& a, const point& b) {
  return a.x*b.y-a.y*b.x;
}
inline bool operator==(const point& a, const point& b) {
  return a.x == b.x && a.y == b.y;
}
inline bool operator!=(const point& a, const point& b) {
  return a.x != b.x || a.y != b.y;
}



struct Image {
  union {
    point p;
    struct {
      int x, y;
    };
  };
  union {
    point sz;
    struct {
      int w, h;
    };
  };
  vector<char> mask;
  inline char& operator()(int i, int j) {
#if defined CHECK_BOUNDS
    assert(i >= 0 && i < h && j >= 0 && j < w);
#endif
    return mask[i*w+j];
  }
  inline char operator()(int i, int j) const {
#if defined CHECK_BOUNDS
    assert(i >= 0 && i < h && j >= 0 && j < w);
#endif
    return mask[i*w+j];
  }
  inline char safe(int i, int j) const {
    return (i<0 || j<0 || i>=h || j>=w ? (char)0 : mask[i*w+j]);
  }
};

using Image_ = const Image&;
using vImage = vector<Image>;
using vImage_ = const vector<Image>&;

inline bool operator==(Image_ a, Image_ b) {
  return tie(a.p,a.sz,a.mask) == tie(b.p,b.sz,b.mask);
}
inline bool operator!=(Image_ a, Image_ b) {
  return tie(a.p,a.sz,a.mask) != tie(b.p,b.sz,b.mask);
}
inline bool operator<(Image_ a, Image_ b) {
  if (a.sz != b.sz) return tie(a.w,a.h) < tie(b.w,b.h);
  return a.mask < b.mask;
}

using ll = long long;
using ull = unsigned long long;
inline ull hashImage(Image_ img) {
  const ull base = 137;
  ull r = 1543;
  r = r*base+img.w;
  r = r*base+img.h;
  r = r*base+img.x;
  r = r*base+img.y;
  for(char c : img.mask) {
    //assert(c >= 0 && c < 10);
    r = r*base+c;
  }
  return r;
}

struct Piece {
  vector<Image> imgs;
  double node_prob;
  int keepi, knowi;
};

const Image badImg = {{0,0},{0,0},{}};
const Image dummyImg = {{0,0},{1,1},{0}};

using Piece_ = const Piece&;

template<class T, class F>
int checkAll(const vector<T>&v, F f) {
  int all = 1;
  for (const T& it : v)
    all &= f(it);
  return all;
}
template<class T, class F>
int allEqual(const vector<T>&v, F f) {
  int need = f(v[0]);
  for (const T& it : v) {
    if (f(it) != need) return 0;
  }
  return 1;
}
