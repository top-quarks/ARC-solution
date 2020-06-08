/*#include "stdio.h"
#include <vector>
#include <string>
#include <tuple>
#include <cassert>
#include <functional>*/
#include "precompiled_stl.hpp"
using namespace std;
#include "utils.hpp"
#include "visu.hpp"

Visu::Visu() {
  fp = fopen("visu.txt", "w");
}
Visu::~Visu() {
  fclose(fp);
}
void Visu::next(string s) {
  fprintf(fp, "Task %s\n", s.c_str());
}
void Visu::add(Image in, Image out) {
  fprintf(fp, "Pair\n");
  for (Image_ img : {in,out}) {
    fprintf(fp, "Image %d %d\n", img.w, img.h);
    for (int i = 0; i < img.h; i++) {
      for (int j = 0; j < img.w; j++) {
	int col = img(i,j);
	fprintf(fp, "%d", col);
      }
      fprintf(fp, "\n");
    }
  }
}


void plot(const vector<vector<int>>&inp, const char*filename) { //filename = out.ppm
  int h = inp.size();
  int w = inp[0].size();

  int tw = 512/max(w,h);
  int bw = max(1, 10/max(w,h));

  int W = (tw+bw)*w+bw, H = (tw+bw)*h+bw;
  FILE*fp = fopen(filename, "w");
  fprintf(fp, "P6\n%d %d\n255\n", W, H);
  vector<int> cols = {0x000000, 0x0074D9, 0xFF4136, 0x2ECC40, 0xFFDC00, 0xAAAAAA, 0xF012BE, 0xFF851B, 0x7FDBFF, 0x870C25};

  vector<unsigned char> output(W*H*3, 0x60);
  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      for (int k = 0; k < tw; k++) {
	for (int l = 0; l < tw; l++) {
	  for (int c = 0; c < 3; c++) {
	    output[((i*(tw+bw)+bw+k)*W+
		    (j*(tw+bw)+bw+l))*3 + c] = cols[inp[i][j]] >> (2-c)*8 & 255;
	  }
	}
      }
    }
  }
  fwrite(output.data(), 1, W*H*3, fp);
  fclose(fp);
}

void print(Image img) {
  printf("[%d %d %d %d]\n", img.p.x, img.p.y, img.w, img.h);
  for (int i = 0; i < img.h; i++) {
    for (int j = 0; j < img.w; j++) {
      int col = img(i,j);
      if (col)
	printf("%d", col);
      else printf(".");
    }
    printf("\n");
  }
}
