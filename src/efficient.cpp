#include "precompiled_stl.hpp"
using namespace std;
#include "utils.hpp"
#include "brute2.hpp"
#include "visu.hpp"

pair<int,bool> TinyHashMap::insert(ull key, int value) {
  if (table.size() <= data.size()*sparse_factor) {
    table.resize(max((int)(table.size() * resize_step), minsz));
    assert((table.size()&(table.size()-1)) == 0);
    mask = table.size()-1;

    fill(table.begin(), table.end(), -1);
    for (int i = 0; i < data.size(); i++) {
      int&head = table[data[i].key&mask];
      data[i].next = head;
      head = i;
    }
  }
  int&head = table[key&mask];
  int previ = head;
  while (1) {
    if (previ == -1) {
      data.push_back({key, value, head});
      head = data.size()-1;
      return {value, true};
    } else if (data[previ].key == key) {
      return {data[previ].value, false};
    }
    previ = data[previ].next;
  }
}




void TinyChildren::add(int fi, int to) {
  assert(fi >= 0);
  if (sz+1 == dense_thres) {
    //Convert to dense
    cap = max(sparse[sz-1].first,fi)+1; //Max fi
    pair<int,int>*old = sparse;
    dense = new int[cap];
    fill_n(dense, cap, None);
    dense[fi] = to;
    for (int i = 0; i < sz; i++) {
      auto [fi, to] = old[i];
      assert(fi >= 0);
      assert(fi < cap);
      dense[fi] = to;
    }
    assert(old);
    delete[]old;
    sz = dense_thres;
  }

  if (sz < dense_thres) {
    //Sparse
    if (sz+1 > cap) {
      cap = max((cap+1)*3/2,4);
      pair<int,int>*old = sparse;
      sparse = new pair<int,int>[cap];
      copy_n(old, sz, sparse);
      if (old) delete[]old;
    }
    {
      int p = sz++;
      while (p && sparse[p-1].first > fi) {
	sparse[p] = sparse[p-1];
	p--;
      }
      sparse[p] = {fi, to};
    }
  } else {
    //Dense
    if (cap <= fi) {
      int oldcap = cap;
      int*old = dense;
      cap = (fi+1)*3/2;
      dense = new int[cap];
      fill_n(dense+oldcap, cap-oldcap, None);
      copy_n(old, oldcap, dense);
      assert(old);
      delete[]old;
    }
    dense[fi] = to;
  }
}
int TinyChildren::get(int fi) {
  assert(fi >= 0);
  if (sz < dense_thres) {
    int low = 0, high = sz-1;
    while (low <= high) {
      int mid = (low+high)>>1;
      int cfi = sparse[mid].first;
      if (cfi == fi) return sparse[mid].second;
      else if (cfi > fi) high = mid-1;
      else low = mid+1;
    }
    return None;
  } else {
    if (fi >= cap) return None;
    return dense[fi];
  }
}
void TinyChildren::legacy(vector<pair<int,int>>&ret) {
  if (sz < dense_thres) {
    //Sparse
    ret.resize(sz);
    for (int i = 0; i < sz; i++)
      ret[i] = sparse[i];
  } else {
    //Dense
    ret.resize(0);
    for (int i = 0; i < cap; i++) {
      if (dense[i] != None)
	ret.emplace_back(i, dense[i]);
    }
  }
}







TinyImage::TinyImage(Image_ img, TinyBank&bank) {
  for (int c : {img.x, img.y, img.w, img.h}) {
    assert(c >= -128 && c < 128);
  }
  x = img.x, y = img.y, w = img.w, h = img.h;

  int freq[10] = {};
  for (char c : img.mask) {
    assert(c >= 0 && c < 10);
    freq[c]++;
  }

  priority_queue<pair<int,int>> pq;
  for (int d = 0; d < 10; d++) {
    if (freq[d]) {
      pq.emplace(-freq[d], -d);
      //cout << d << ": " << freq[d] << endl;
    }
  }
  while (pq.size() < 2) pq.emplace(0,0);

  int nodes = pq.size()-1;
  int pos = nodes-1;
  auto convert = [](int a, int p) {
    if (a <= 0) return -a;
    else {
      assert(9+a-p >= 10 && 9+a-p < 16);
      return 9+a-p;
    }
  };
  while (pq.size() > 1) {
    auto [mfa,a] = pq.top(); pq.pop();
    auto [mfb,b] = pq.top(); pq.pop();
    tree[pos] = convert(a,pos) | convert(b,pos) << 4;
    pq.emplace(mfa+mfb, pos--);
  }
  int code[10] = {}, codelen[10] = {};
  int path[10] = {}, pathlen[10] = {};
  for (int p = 0; p < nodes; p++) {
    for (int k : {0,1}) {
      int child = tree[p]>>k*4&15;
      int newpath = path[p] | k<<pathlen[p];
      if (child < 10) {
	code[child] = newpath;
	codelen[child] = pathlen[p]+1;
      } else {
	child += p-9;
	path[child] = newpath;
	pathlen[child] = pathlen[p]+1;
      }
    }
    //cout << p << ": " << (int)tree[p][0] << ' ' << (int)tree[p][1] << endl;
  }
  /*for (int d = 0; d < 10; d++) {
    cout << d << ": " << bitset<4>(code[d]) << ' ' << codelen[d] << endl;
    }*/

  assert((bank.curi+align-1)/align < 1ll<<32);
  memi = (bank.curi+align-1)/align;
  sz = 0;
  ll memstart = (ll)memi*align;
  bank.alloc();
  for (char c : img.mask) {
    bank.set(memstart+ sz, code[c], codelen[c]);
    sz += codelen[c];
    //for (int i = 0; i < codelen[c]; i++)
    //bank.set(memstart+ sz++, code[c]>>i&1);
  }
  /*for (int it = 0; it < 10; it++)
    cout << (int)img.mask[it] << ' ';
    cout << endl;
    for (int it = 0; it < 20; it++)
    cout << bank.get(memi+it);
    cout << endl;*/
  bank.curi = memstart+sz;
}


Image TinyImage::decompress(TinyBank&bank) {
  Image ret;
  ret.x = x, ret.y = y, ret.w = w, ret.h = h;
  ret.mask.resize(ret.w*ret.h);
  int treep = 0, maski = 0;
  ll memstart = (ll)memi*align;
  for (ll i = memstart; i < memstart+sz; i++) {
    int bit = bank.get(i);
    int child = tree[treep]>>bit*4&15;
    if (child < 10) {
      ret.mask[maski++] = child;
      treep = 0;
    } else
      treep += child-9;
  }
  //cout << maski << ' ' << ret.w*ret.h << endl;
  assert(maski == ret.w*ret.h);
  assert(treep == 0);
  return ret;
}
