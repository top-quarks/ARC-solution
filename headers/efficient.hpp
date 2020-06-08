typedef unsigned long long ull;
struct hashEntry {
  ull key;
  int value, next;
};
struct TinyHashMap {
  static constexpr double sparse_factor = 1.5, resize_step = 2;
  static constexpr int minsz = 1<<20;
  vector<hashEntry> data;
  vector<int> table;
  ull mask;
  pair<int,bool> insert(ull key, int value);
  unsigned int size() {
    return data.size();
  }
  void clear() {
    data.clear();
    table.clear();
  }
};





struct TinyChildren {
  static constexpr int dense_thres = 10;
  static constexpr int None = -2;
  union {
    int*dense;
    pair<int,int>*sparse;
  };
  short sz = 0, cap = 0;
  TinyChildren() {
    dense = NULL;
  }
  void add(int fi, int to);
  int get(int fi);
  ~TinyChildren() {
    delete[]dense;
    dense = NULL;
  }
  TinyChildren(const TinyChildren&) = delete;
  TinyChildren(TinyChildren&&o) {
    dense = o.dense;
    sz = o.sz, cap = o.cap;
    o.dense = NULL;
  }

  void legacy(vector<pair<int,int>>&child);
  void clear() {
    delete[]dense;
    dense = NULL;
    sz = cap = 0;
  }
};




struct TinyBank {
  vector<unsigned int> mem;
  ll curi = 0;
  void alloc() {
    if (curi/32+2000 >= mem.size())
      mem.resize(curi/32+2000);
  }
  inline void set(ll bi, unsigned int v) {
    int i = bi>>5, j = bi&31;
    if (i >= mem.size()) mem.resize(max(i+1,1024));
    mem[i] |= v<<j;
    //if (v) mem[i] |= 1u<<j;
    //else mem[i] &= ~(1u<<j);
  }
  inline void set(ll bi, unsigned int v, int len) {
    int i = bi>>5, j = bi&31;

    mem[i] |= v << j;
    if (j+len > 32)
      mem[i+1] |= v >> 32-j;
  }
  inline int get(ll bi) {
    return mem[bi>>5]>>(bi&31)&1;
  }
};

struct TinyImage {
  static constexpr int align = 16;
  uint32_t memi;
  short sz;
  char x, y, w, h;
  uint8_t tree[9];
  TinyImage(Image_ img, TinyBank&bank);
  Image decompress(TinyBank&bank);
};

struct TinyNode {
  int*vimg = NULL;
  bool isvec, ispiece;
  short imgs; //TODO change to unsigned char
  char depth;
  TinyChildren child;
  TinyNode() {}
  TinyNode(TinyNode&&o) = default;
};

struct TinyNodeBank {
  TinyBank bank;
  vector<TinyImage> imgs;
  vector<TinyNode> node;
  Image read(int i) {
    assert(i >= 0 && i < imgs.size());
    return imgs[i].decompress(bank);
  }
  Image getImg(int ni) {
    return imgs[node[ni].vimg[0]].decompress(bank);
  }
  State getState(int ni) {
    State ret;
    ret.vimg.resize(node[ni].imgs);
    for (int i = 0; i < node[ni].imgs; i++)
      ret.vimg[i] = imgs[node[ni].vimg[i]].decompress(bank);
    ret.depth = node[ni].depth;
    ret.isvec = node[ni].isvec;
    return ret;
  }
  void append(const State&state, bool ispiece) {
    assert(state.depth >= 0 && state.depth < 128);
    TinyNode v;
    //assert(state.vimg.size() < 1<<16); //TODO: change to unsigned char

    v.imgs = min((int)state.vimg.size(),100000);
    v.vimg = new int[v.imgs];
    for (int i = 0; i < v.imgs; i++) {
      v.vimg[i] = imgs.size();
      imgs.emplace_back(state.vimg[i], bank);
    }
    v.isvec = state.isvec;
    v.ispiece = ispiece;
    v.depth = state.depth;
    node.push_back(move(v));
  }
  void addChild(int ni, int fi, int to) {
    node[ni].child.add(fi, to);
  }
  int getChild(int ni, int fi) {
    return node[ni].child.get(fi);
  }
  ~TinyNodeBank() {
    for (TinyNode&n : node)
      delete[]n.vimg;
  }
  TinyNode& operator[](int i) {
    return node[i];
  }
  int size() { return node.size(); }
};
