/*#include <vector>
#include <regex>
#include <cassert>
#include <iostream>*/
#include "precompiled_stl.hpp"
#include <experimental/filesystem>

using namespace std;
#include "utils.hpp"
#include "read.hpp"

Sample::Sample(string filename) {
  vector<Image> train_input, train_output;
  vector<Image> test_input, test_output;

  regex re(".*/([a-z0-9]{8}).json");
  smatch match;
  assert(std::regex_search(filename, match, re) && match.size() == 2);
  id = match.str(1);

  fp = fopen(filename.c_str(), "r");
  assert(fp);
  expect('{');
  while (1) {
    string train_test = getQuote();
    expect('[');
    while (1) {
      expect('{');
      while (1) {
	string input_output = getQuote();
	Image img = readImage();
	if (train_test == "train" && input_output == "input") {
	  train_input.push_back(img);
	} else if (train_test == "train" && input_output == "output") {
	  train_output.push_back(img);
	} else if (train_test == "test" && input_output == "input") {
	  test_input.push_back(img);
	} else if (train_test == "test" && input_output == "output") {
	  test_output.push_back(img);
	} else {
	  cerr << train_test << ' ' << input_output << endl;
	  assert(!"Unexpected tag");
	}
	if (end('}')) break;
      }
      if (end(']')) break;
    }
    if (end('}')) break;
  }
  assert(mygetc() == -1);
  assert(feof(fp));
  fclose(fp);

  assert(train_input.size() == train_output.size());
  for (int i = 0; i < (int)train_input.size(); i++) {
    train.emplace_back(train_input[i], train_output[i]);
  }
  if (test_input.size() == test_output.size()) {
    for (int i = 0; i < (int)test_input.size(); i++) {
      test.emplace_back(test_input[i], test_output[i]);
    }
  } else {
    assert(test_output.empty());
    for (int i = 0; i < (int)test_input.size(); i++) {
      test.emplace_back(test_input[i], badImg);
    }
  }
}

vector<Sample> Sample::split() {
  vector<Sample> ret;
  for (int i = 0; i < test.size(); i++) {
    ret.push_back(*this);
    Sample&s = ret.back();
    tie(s.test_in, s.test_out) = test[i];
    s.test = {{s.test_in, s.test_out}};
    s.id_ind = i;
  }
  return ret;
}

char Sample::mygetc() {
  char c = fgetc(fp);
  while (c == ' ' || c == '\n') c = fgetc(fp);
  return c;
}
int Sample::end(char endc) {
  char c = mygetc();
  if (c == ',') {
    return 0;
  } else {
    if (c != endc)
      cerr << "|"<<c<<"|" << " != " << "|"<<endc<<"|" << endl;
    assert(c == endc);
    return 1;
  }
}
void Sample::expect(char c) {
  char got = mygetc();
  if (got != c)
    cerr << "|"<<got<<"|" << " != " << "|"<<c<<"|" << endl;
  assert(got == c);
}
string Sample::getQuote() {
  expect('\"');
  char str[200];
  assert(fscanf(fp, "%[^\"]", str));
  expect('\"');
  expect(':');
  return string(str);
}
vector<int> Sample::readRow() {
  vector<int> ret;
  expect('[');
  while (1) {
    int v;
    assert(fscanf(fp, "%d", &v) == 1);
    ret.push_back(v);
    if (end(']')) break;
  }
  return ret;
}
Image Sample::readImage() {
  Image ret;
  ret.p = {0,0};
  expect('[');
  vector<int> widths;
  while (1) {
    vector<int> row = readRow();
    widths.push_back(row.size());
    for (int col : row)
      ret.mask.push_back(col);
    if (end(']')) break;
  }
  ret.h = widths.size();
  assert(ret.h >= 1 && ret.h <= 30);
  ret.w = widths[0];
  for (int i = 0; i < ret.h; i++)
    assert(widths[i] == ret.w);
  assert(ret.w >= 1 && ret.w <= 30);
  return ret;
}


vector<Sample> readAll(string path, int maxn) { //maxn = -1
  const string base_path[2] = {"/kaggle/input/abstraction-and-reasoning-challenge/", "./dataset/"};

  int base_pathi = 0;
  while (!experimental::filesystem::exists(base_path[base_pathi]+path)) {
    base_pathi++;
    assert(base_pathi < 2);
  }

  vector<string> files;
  for (auto magic_file_type : experimental::filesystem::directory_iterator(base_path[base_pathi]+path)) {
    string name = magic_file_type.path().u8string();
    if (name.size() >= 5 && name.substr(name.size()-5,5) == ".json")
      files.push_back(name);
  }
  if (maxn >= 0) files.resize(maxn);
  vector<Sample> sample;
  for (string sid : files) {
    //sample.push_back(Sample(sid));
    for (Sample s : Sample(sid).split()) {
      //if (maxn < 0 || sample.size() < maxn)
      sample.push_back(s);
    }
  }
  return sample;
}


Writer::Writer(string filename) { //filename = "submission_part.csv"
  filename = filename;
  fp = fopen(filename.c_str(), "w");
  assert(fp);
  fprintf(fp, "output_id,output\n");
}

void Writer::operator()(const Sample& s, vector<Image> imgs) {
  //int cnt = seen[s.id]++;
  //assert(cnt == s.id_ind);
  fprintf(fp, "%s_%d,", s.id.c_str(), s.id_ind);
  //TODO: is empty output allowed? Looks like no!
  if (imgs.empty()) imgs = {dummyImg};
  assert(imgs.size() <= 3);
  int notfirst = 0;
  for (Image_ img : imgs) {
    assert(img.p == point({0,0}));
    assert(img.w >= 1 && img.w <= 30 && img.h >= 1 && img.h <= 30);
    if (notfirst++) fprintf(fp, " ");
    fprintf(fp, "|");
    for (int i = 0; i < img.h; i++) {
      for (int j = 0; j < img.w; j++) {
	int c = img(i,j);
	assert(c >= 0 && c <= 9);
	fprintf(fp, "%d", c);
      }
      fprintf(fp, "|");
    }
  }
  fprintf(fp, "\n");
}

Writer::~Writer() {
  fclose(fp);
}


void writeAnswersWithScores(const Sample&s, string fn, vector<Image> imgs, vector<double> scores) {
  FILE*fp = fopen(fn.c_str(), "w");
  assert(fp);
  fprintf(fp, "%s_%d\n", s.id.c_str(), s.id_ind);
  assert(imgs.size() == scores.size());
  if (imgs.empty()) imgs = {dummyImg}, scores = {-1};
  assert(imgs.size() <= 3);

  for (int i = 0; i < imgs.size(); i++) {
    Image_ img = imgs[i];
    double score = scores[i];
    assert(img.p == point({0,0}));
    assert(img.w >= 1 && img.w <= 30 && img.h >= 1 && img.h <= 30);
    fprintf(fp, "|");
    for (int i = 0; i < img.h; i++) {
      for (int j = 0; j < img.w; j++) {
	int c = img(i,j);
	assert(c >= 0 && c <= 9);
	fprintf(fp, "%d", c);
      }
      fprintf(fp, "|");
    }
    fprintf(fp, " %.20f", score);
    fprintf(fp, "\n");
  }
  fclose(fp);
}
