
struct Spec {
  bool bad = 0, anypos = 0;
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
  vector<int> mask;
  inline int& operator()(int i, int j) {
#if defined CHECK_BOUNDS
    assert(i >= 0 && i < h && j >= 0 && j < w);
#endif
    return mask[i*w+j];
  }
  inline int operator()(int i, int j) const {
#if defined CHECK_BOUNDS
    assert(i >= 0 && i < h && j >= 0 && j < w);
#endif
    return mask[i*w+j];
  }
  bool check(Image_ img) const {
    if (bad) return 0;
    if (img.sz != sz || (!anypos && img.p != p)) return 0;
    for (int i = 0; i < mask.size(); i++)
      if ((mask[i]>>img.mask[i]&1) == 0) return 0;
    return 1;
  }
};

typedef const Spec& Spec_;
const int allCols = (1<<10)-1;

using SpecRec = pair<Spec,function<Image(Image_,Image_)>>;
const SpecRec badSpec = {{1}, NULL};

double entropy(Spec_ spec);

Spec fromImage(Image_ img);
Image toImage(Spec_ spec);
SpecRec iComposeFirst(Image_ img, Spec_ target);
SpecRec iComposeSecond(Image_ img, Spec_ target);
SpecRec iColShapeFirst(Image_ img, Spec_ target);
SpecRec iColShapeSecond(Image_ img, Spec_ target);
SpecRec iRepeatSecond(Image_ img, Spec_ target);
SpecRec iMirrorSecond(Image_ img, Spec_ target);
SpecRec iOuterProductSIFirst(Image_ img, Spec_ target);
SpecRec iOuterProductISFirst(Image_ img, Spec_ target);
SpecRec iOuterProductSISecond(Image_ img, Spec_ target);
SpecRec iOuterProductISSecond(Image_ img, Spec_ target);
SpecRec iStack(Image_ img, Spec_ target, int id);
SpecRec iEmbed(Image_ img, Spec_ target);

SpecRec iCompose2First(Image_ img, Spec_ target);
