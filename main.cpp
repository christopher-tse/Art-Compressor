
#include "util/HSLAPixel.h"
#include "util/PNG.h"
#include "toqutree.h"

using namespace util;
using namespace std;

int main()
{
	PNG infinitywar;
	PNG neverland;
	PNG nature;

	infinitywar.readFromFile("images/infinitywar.png");
	neverland.readFromFile("images/neverland.png");
	nature.readFromFile("images/nature.png");

	toqutree t1(infinitywar, 9);
	toqutree t2(neverland, 9);
	toqutree t3(nature, 9);

	t1.prune(0.025);
	t2.prune(0.025);
	t3.prune(0.055);

	PNG infinitywarPruned = t1.render();
	PNG neverlandPruned = t2.render();
	PNG naturePruned = t3.render();

	infinitywarPruned.convert();
	neverlandPruned.convert();
	naturePruned.convert();

	infinitywarPruned.writeToFile("output/infinitywarPruned.png");
	neverlandPruned.writeToFile("output/neverlandPruned.png");
	naturePruned.writeToFile("output/naturePruned.png");

  return 0;
}
