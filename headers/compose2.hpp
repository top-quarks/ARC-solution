struct Candidate {
  vImage imgs;
  double score;
  int cnt_pieces, sum_depth, max_depth;
  Candidate(vImage_ imgs_, double score_) : imgs(imgs_), score(score_) {
    cnt_pieces = sum_depth = max_depth = -1;
  }
  Candidate(vImage_ imgs_, int cnt_pieces_, int sum_depth_, int max_depth_) :
    imgs(imgs_), cnt_pieces(cnt_pieces_), sum_depth(sum_depth_), max_depth(max_depth_) {
    score = -1;
  }
};

inline bool operator<(const Candidate& a, const Candidate& b) {
  return a.score > b.score;
}

vector<Candidate> composePieces2(Pieces&pieces, vector<pair<Image, Image>> train, vector<point> out_sizes);
vector<Candidate> evaluateCands(const vector<Candidate>&cand, vector<pair<Image,Image>> train);
