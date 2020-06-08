Image Col(int id);
Image Pos(int dx, int dy);
Image Square(int id);
Image Line(int orient, int id);
Image getPos(Image_ img);
Image getSize(Image_ img);
Image hull(Image_ img);
Image toOrigin(Image img);
Image getW(Image_ img, int id);
Image getH(Image_ img, int id);
Image filterCol(Image_ img, int col);
Image colShape(Image_ shape, int col);
Image compress(Image_ img, Image_ bg = Col(0));
Image Fill(Image_ a);
Image interior(Image_ a);
Image border(Image_ a);
Image center(Image_ img);
//Image transform(Image_ img, int A00, int A01, int A10, int A11);
Image rigid(Image_ img, int id);
Image invert(Image img);
Image interior2(Image_ a);
Image count(Image_ img, int id, int outType);
//Image smear(Image_ base, int id);

Image hull0(Image_ img);
Image getSize0(Image_ img);
Image getRegular(Image_ img);
vImage cut(Image_ img);

//One and a bit of the other
Image Move(Image img, Image_ p);
Image embed(Image_ img, Image_ shape);
Image extend(Image_ img, Image_ room);
Image wrap(Image_ line, Image_ area);

//Two inputs
Image filterCol(Image_ img, Image_ palette);
Image colShape(Image_ col, Image_ shape);
Image broadcast(Image_ col, Image_ shape, int include0 = 1);
Image replaceCols(Image_ base, Image_ cols);
Image smear(Image_ base, Image_ room, int id);
//Image alignx(Image_ a, Image_ b, int id);
//Image aligny(Image_ a, Image_ b, int id);
Image align(Image_ a, Image_ b, int idx, int idy);
Image align(Image_ a, Image_ b);
//Image compose(Image_ a, Image_ b, const function<int(int,int)>&f, int overlap_only);
Image compose(Image_ a, Image_ b, int id = 0);
Image outerProductIS(Image_ a, Image_ b);
Image outerProductSI(Image_ a, Image_ b);
Image myStack(Image_ a, Image b, int orient);


//Image pickMax(const vector<Image>& v, function<int(Image_)> f);
Image pickMax(vImage_ v, int id);
//vector<Image> cut(Image_ img, Image_ a);
//vector<Image> splitCols(Image_ img, int include0 = 0);
//Image compose(const vector<Image>&imgs, int id);
//void getRegular(vector<int>&col);
//Image getRegular(Image_ img);
Image cutPickMax(Image_ a, int id);
//Image regularCutPickMax(Image_ a, int id);
Image splitPickMax(Image_ a, int id, int include0 = 0);
//Image cutCompose(Image_ a, Image_ b, int id);
//Image regularCutCompose(Image_ a, int id);
//Image splitCompose(Image_ a, int id, int include0 = 0);
//Image cutIndex(Image_ a, Image_ b, int ind);
Image splitPickMaxes(Image_ a, int id);
Image cutIndex(Image_ a, int ind);
Image cutPickMaxes(Image_ a, int id);
Image heuristicCut(Image_ a);

vImage pickNotMaxes(vImage_ v, int id);

Image cutPickMax(Image_ a, Image_ b, int id);
Image cutPickMaxes(Image_ a, Image_ b, int id);
vImage pickMaxes(vImage_ v, int id);
Image compose(vImage_ imgs, int id);
vImage cut(Image_ img, Image_ a);
vImage splitCols(Image_ img, int include0 = 0);

Image repeat(Image_ a, Image_ b, int pad = 0);
Image mirror(Image_ a, Image_ b, int pad = 0);
Image majCol(Image_ img);
