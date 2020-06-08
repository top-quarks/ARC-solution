#include <stdio.h>
#include <string>
using std::string;
#include "load.hpp"

const int loading_len = 40;

void Loader::operator()() {
  long long a = counter++, b = n;
  const char*title = text.c_str();

  long long pb = prev*b*2, x = a*2000+b;
  if (pb > x-b*2 && pb <= x) return;

#pragma omp critical
  {
    long long p = prev = x/(2*b);

    if (title) printf("%s ", title);
    printf("|");
    int dist = (a*2LL*loading_len+(b>>1))/b;
    for (int i = 0; i < dist>>1; i++)
      printf("=");
    if (dist&1)
      printf("-");
    for (int i = (dist+1)>>1; i < loading_len; i++)
      printf(" ");
    printf("| %lld / %lld (%lld.%lld%%)\r", a, b, p/10, p%10);
    fflush(stdout);
  }
}

Loader::~Loader() {
#pragma omp critical
  {
    const char*title = text.c_str();
    if (title) {
      for (int i = 0; title[i]; i++)
	printf(" ");
    }
    for (int i = 0; i < loading_len+35+5; i++)
      printf(" ");
    printf("\r");
    fflush(stdout);
    if (title && keep_title) {
      printf("%s \n", title);
    }
  }
}
