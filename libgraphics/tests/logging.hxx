#ifndef LOGGING_H_
#define LOGGING_H_

#include <iostream>
inline std::ostream &operator<<(std::ostream &o, const glm::vec3 &v)
{
    o << "glm::vec3(" << v.x << "," << v.y << "," << v.z << ")";
    return o;
}

#endif // LOGGING_H_
