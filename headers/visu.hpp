struct Visu {
  FILE*fp;
  Visu();
  ~Visu();
  void next(string s);
  void add(Image in, Image out);
};

void plot(const vector<vector<int>>&inp, const char*filename = "out.ppm");
void print(Image img);
