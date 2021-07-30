import hello;

import<cassert>;
import<sstream>;
import<stdexcept>;

int main() {
  using namespace std;

  // Basics.
  //
  {
    ostringstream o;
    hello::say_hello(o, "World");
    assert(o.str() == "Hello, World!\n");
  }

  // Empty name.
  //
  try {
    ostringstream o;
    hello::say_hello(o, "");
    assert(false);
  } catch (const invalid_argument &e) {
    assert(e.what() == string("empty name"));
  }
}
