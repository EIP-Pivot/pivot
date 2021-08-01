import <iostream>;
import <glm/vec2.hpp>; // glm::vec3
import <glm/mat4x4.hpp>; // glm::mat4
import hello;
import hello.glm;


int main (int argc, char* argv[])
{
  using namespace std;

  if (argc < 2)
  {
    cerr << "error: missing name" << endl;
    return 1;
  }
  hello::say_hello(cout, argv[1]);

  glm::vec2 v{1.0, 2.0};
  auto res = hello::camera(0.2, v);
  std::cout << res[0][0] << std::endl;
}
