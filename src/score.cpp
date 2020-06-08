#include "precompiled_stl.hpp"
using namespace std;
#include "utils.hpp"
#include "core_functions.hpp"
#include "image_functions.hpp"

#include "brute2.hpp"
#include "pieces.hpp"
#include "compose2.hpp"

int scoreCands(const vector<Candidate>&cands, Image_ test_in, Image_ test_out) {
  for (const Candidate&cand : cands)
    if (cand.imgs.back() == test_out) return 1;
  return 0;
}

int scoreAnswers(vImage_ answers, Image_ test_in, Image_ test_out) {
  assert(answers.size() <= 3);
  for (Image_ answer : answers)
    if (answer.sz == test_out.sz && answer.mask == test_out.mask) return 1;
  return 0;
}
