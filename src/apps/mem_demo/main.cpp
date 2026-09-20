#include "print_utils.h"
#include "lab_math/vec.h"

static void dummy1() { }
static int dummy2() { return 123; }
static float dummy3() { return 1.5F; }

int main()
{
    header("Stack");
    int x = 123;
    print(&x);

    Vec3 vec3 = Vec3(1, 2, 3);
    print(&vec3);

    header("Functions");
    print(dummy1);
    print(dummy2);
    print(dummy3);

    header("Heap");
    int* p = new int(123);
    print(p);

    delete p;
    print(p);
}
