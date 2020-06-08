struct Functions {
  vector<function<Image()>> nullary;
  vector<function<Image(Image_)>> unary;
  vector<function<Image(Image_, Image_)>> binary;
  vector<int> sz;
  void add(function<Image()> f) {
    nullary.push_back(f);
  }
  void add(function<Image(Image_)> f) {
    unary.push_back(f);
  }
  void add(function<Image(Image_,Image_)> f) {
    binary.push_back(f);
  }
};

Functions initFuncs();

vector<Piece> brutePieces(Image_ test_in, const vector<pair<Image,Image>>&train);
