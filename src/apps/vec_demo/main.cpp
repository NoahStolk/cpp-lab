#include <iostream>

#include <lab_math/vec.h>

int main()
{
    constexpr const char* lang = "C++";
    std::cout << "Hello and welcome to " << lang << "!\n";

    const Vec3 pos = Vec3(1, 2, 3);

    std::cout << pos.len() << "\n";
    std::cout << pos.len_squared() << "\n";

    const Vec3 dir = Vec3(1, 0, 0);
    std::cout << dir.z << "\n";

    const Vec3 posB = pos + Vec3(4, 4, 4);
    std::cout << posB << "\n";

    return 0;
}
