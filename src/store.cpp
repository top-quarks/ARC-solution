#include "precompiled_stl.hpp"
using namespace std;
#include "utils.hpp"


void storePieces(vector<Piece>&vp, string id, int id_ind) {
  assert(sizeof(Piece) == sizeof(vector<Image>) + sizeof(double) + sizeof(int)*2);
  char fn[200];
  sprintf(fn, "store/piece/%s_%d", id.c_str(), id_ind);
  FILE*fp = fopen(fn, "wb");
  auto put = [fp](auto n) {
    fwrite(&n, sizeof(n), 1, fp);
  };
  put((int)vp.size());
  for (Piece&p : vp) {
    put((int)p.imgs.size());
    for (Image_ img : p.imgs) {
      put(img.w), put(img.h), put(img.x), put(img.y);
      fwrite(img.mask.data(), sizeof(img.mask[0]), img.w*img.h, fp);
    }
    put(p.node_prob);
    put(p.keepi);
    put(p.knowi);
  }
  fclose(fp);
}

vector<Piece> loadPieces(string id, int id_ind) {
  assert(sizeof(Piece) == sizeof(vector<Image>) + sizeof(double) + sizeof(int)*2);
  char fn[200];
  sprintf(fn, "store/piece/%s_%d", id.c_str(), id_ind);
  FILE*fp = fopen(fn, "rb");
  auto readint = [fp]() {
    int n;
    assert(fread(&n, sizeof(n), 1, fp) == 1);
    return n;
  };
  auto read = [fp](auto&n) {
    assert(fread(&n, sizeof(n), 1, fp) == 1);
  };
  vector<Piece> vp(readint());
  for (Piece&p : vp) {
    p.imgs.resize(readint());
    for (Image& img : p.imgs) {
      read(img.w);
      read(img.h);
      read(img.x);
      read(img.y);
      img.mask.resize(img.w*img.h);
      assert(fread(img.mask.data(), sizeof(img.mask[0]), img.w*img.h, fp) == img.w*img.h);
    }
    read(p.node_prob);
    read(p.keepi);
    read(p.knowi);
  }
  fclose(fp);
  return vp;
}


void storePieces(vector<vector<Piece>>&pieces) {
  assert(sizeof(Piece) == sizeof(vector<Image>) + sizeof(double) + sizeof(int)*2);
  FILE*fp = fopen("store/pieces", "wb");
  auto put = [fp](auto n) {
    fwrite(&n, sizeof(n), 1, fp);
  };
  put((int)pieces.size());
  for (auto&vp : pieces) {
    put((int)vp.size());
    for (Piece&p : vp) {
      put((int)p.imgs.size());
      for (Image_ img : p.imgs) {
	put(img.w), put(img.h), put(img.x), put(img.y);
	fwrite(img.mask.data(), sizeof(img.mask[0]), img.w*img.h, fp);
      }
      put(p.node_prob);
      put(p.keepi);
      put(p.knowi);
    }
  }
  fclose(fp);
}

vector<vector<Piece>> loadPieces() {
  assert(sizeof(Piece) == sizeof(vector<Image>) + sizeof(double) + sizeof(int)*2);
  FILE*fp = fopen("store/pieces", "rb");
  auto readint = [fp]() {
    int n;
    assert(fread(&n, sizeof(n), 1, fp) == 1);
    return n;
  };
  auto read = [fp](auto&n) {
    assert(fread(&n, sizeof(n), 1, fp) == 1);
  };
  vector<vector<Piece>> pieces(readint());
  for (auto&vp : pieces) {
    vp.resize(readint());
    for (Piece&p : vp) {
      p.imgs.resize(readint());
      for (Image& img : p.imgs) {
	read(img.w);
	read(img.h);
	read(img.x);
	read(img.y);
	img.mask.resize(img.w*img.h);
	assert(fread(img.mask.data(), sizeof(img.mask[0]), img.w*img.h, fp) == img.w*img.h);
      }
      read(p.node_prob);
      read(p.keepi);
      read(p.knowi);
    }
  }
  fclose(fp);
  return pieces;
}


void storeCands(vector<vector<Image>>&cands) {
  FILE*fp = fopen("store/cands", "wb");
  auto put = [fp](auto n) {
    fwrite(&n, sizeof(n), 1, fp);
  };
  put((int)cands.size());
  for (auto&vi : cands) {
    put((int)vi.size());
    for (Image&img : vi) {
      put(img.w), put(img.h), put(img.x), put(img.y);
      fwrite(img.mask.data(), sizeof(img.mask[0]), img.w*img.h, fp);
    }
  }
  fclose(fp);
}

vector<vector<Image>> loadCands() {
  FILE*fp = fopen("store/cands", "rb");
  auto readint = [fp]() {
    int n;
    assert(fread(&n, sizeof(n), 1, fp) == 1);
    return n;
  };
  auto read = [fp](auto&n) {
    assert(fread(&n, sizeof(n), 1, fp) == 1);
  };
  vector<vector<Image>> cands(readint());
  for (auto&vi : cands) {
    vi.resize(readint());
    for (Image& img : vi) {
      read(img.w);
      read(img.h);
      read(img.x);
      read(img.y);
      img.mask.resize(img.w*img.h);
      assert(fread(img.mask.data(), sizeof(img.mask[0]), img.w*img.h, fp) == img.w*img.h);
    }
  }
  fclose(fp);
  return cands;
}
