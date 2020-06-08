vImage splitAll(Image_ img);
Image eraseCol(Image img, int col);
vImage insideMarked(Image_ in);
Image makeBorder(Image_ img, int bcol = 1);
Image makeBorder2(Image_ img, int usemaj = 1);
Image makeBorder2(Image_ img, Image_ bord);
Image compress2(Image_ img);
Image compress3(Image_ img);
Image greedyFillBlack(Image_ img, int N = 3);
Image greedyFillBlack2(Image_ img, int N = 3);
Image extend2(Image_ img, Image_ room);
Image connect(Image_ img, int id);
Image replaceTemplate(Image_ in, Image_ need_, Image_ marked_, int overlapping = 0, int rigids = 0);
Image swapTemplate(Image_ in, Image_ a, Image_ b, int rigids = 0);
Image spreadCols(Image img, int skipmaj = 0);
vImage splitColumns(Image_ img);
vImage splitRows(Image_ img);
Image half(Image_ img, int id);
Image smear(Image_ img, int id);
Image mirror2(Image_ a, Image_ line);
vImage gravity(Image_ in, int d);

Image myStack(vImage_ lens, int id);
Image stackLine(vImage_ shapes);
Image composeGrowing(vImage_ imgs);

Image pickUnique(vImage_ imgs, int id);
