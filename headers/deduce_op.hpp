
struct deduceOuterProduct {
  vector<pair<Image,Image>> train_targets;
  int rec_funci;
  deduceOuterProduct(vector<pair<Image,Image>> train);
  Image reconstruct(Image_ a, Image_ b);
};
void addDeduceOuterProduct(Pieces&pieces, vector<pair<Image,Image>> train, vector<Candidate>&cands);
