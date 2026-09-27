#include <iostream>

#include "ui.h"

int main()
{
    Ui ui = Ui();

    while (true)
    {
        ui.print();
        std::cin.ignore();

        system("clear");
        ui.update();
    }

    return 0;
}
