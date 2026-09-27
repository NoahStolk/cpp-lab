#include <iostream>

#include "ui.h"

static const char* print_dir(const Direction dir)
{
    switch (dir)
    {
    case Direction::Left: return "L";
    case Direction::Up: return "U";
    case Direction::Right: return "R";
    case Direction::Down: return "D";
    default: return "?";
    }
}

void Ui::print() const
{
    for (int x = 0; x < size; x++)
    {
        for (int y = 0; y < size; y++)
        {
            const int index = get_index(x, y);
            const auto [health, direction] = cells[index];

            if (health > 0)
            {
                std::cout << toascii(health) << print_dir(direction) << " ";
            }
            else
            {
                std::cout << "   ";
            }
        }

        std::cout << "\n";
    }
}

void Ui::update()
{
    for (int x = 0; x < size; x++)
    {
        for (int y = 0; y < size; y++)
        {
            const int index = get_index(x, y);

            int target_index;
            switch (cells[index].direction)
            {
            case Direction::Left: target_index = get_index(x - 1, y);
                break;
            case Direction::Up: target_index = get_index(x, y - 1);
                break;
            case Direction::Right: target_index = get_index(x + 1, y);
                break;
            case Direction::Down: target_index = get_index(x, y + 1);
                break;
            default: continue;
            }

            if (target_index < 0 || target_index >= cells.size())
            {
                continue;
            }

            cells[target_index].hit();
        }
    }
}
