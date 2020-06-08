/*struct Piece2 {
  vector<int> ind;
  int depth;
  };*/
struct Piece3 {
  int memi, depth;
};

struct Pieces {
  vector<DAG> dag;
  vector<Piece3> piece;
  //vector<vector<int>> seen;
  vector<int> mem;
};

Pieces makePieces2(vector<DAG>&dag, vector<pair<Image,Image>> train, vector<point> out_sizes);
