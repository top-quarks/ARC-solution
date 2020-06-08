struct Candidate;
int scorePieces(const vector<Piece>&piece, Image_ test_in, Image_ test_out, const vector<pair<Image,Image>>&train);
int scoreCands(const vector<Candidate>&cands, Image_ test_in, Image_ test_out);
int scoreAnswers(vImage_ answers, Image_ test_in, Image_ test_out);
