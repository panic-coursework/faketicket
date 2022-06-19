#include <file/bptree.h>
#include <file/varchar.h>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using ticket::file::Varchar, std::cin, std::cout, std::endl;

int main () {
  ticket::file::BpTree<Varchar<64>, int> bpt {"file.o"};
  std::string cmd;
  int n;
  cin >> n;
  while (n-- > 0) {
    std::string index;
    cin >> cmd >> index;
#ifdef DEBUG
    ;// std::cerr << "\n> " << cmd << ' ' << index << endl << endl;
#endif
    if (cmd == "insert") {
      int value;
      cin >> value;
      bpt.insert(index, value);
    }
    else if (cmd == "delete") {
      int value;
      cin >> value;
      if (bpt.includes(index, value)) bpt.remove(index, value);
    }
    else if (cmd == "find") {
      auto result = bpt.findMany(index);
      std::sort(result.begin(), result.end());
      for (const int &i : result) cout << i << " ";
      if (result.empty()) cout << "null";
      cout << '\n';
    }
    bpt.clearCache();
  }
  return 0;
}
