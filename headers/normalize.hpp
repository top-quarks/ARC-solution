struct Simplifier {
  function<Image(Image_)> in;
  function<Image(Image_,Image_)> out, rec;
  pair<Image,Image> operator()(Image_ a, Image_ b);
};

Simplifier normalizeCols(const vector<pair<Image,Image>>&train);
Simplifier normalizeDummy(const vector<pair<Image,Image>>&train);

//inline vector<double> shapeFeatures(Image_ img, int col);
//inline Image remapCols(Image_ img, int cols[10]);
void remapCols(const vector<pair<Image,Image>>&train, vector<Simplifier>&sims);
//inline Image listCols(Image_ img, int extra);
void normalizeCols(vector<Sample>&sample);

void normalizeRigid(const vector<pair<Image,Image>>&train, vector<Simplifier>&sims);
void evalNormalizeRigid();
