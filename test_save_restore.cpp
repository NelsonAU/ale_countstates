#include "ale_interface.hpp"

#include <iostream>
#include <string>
#include <array>
#include <stack>
#include <algorithm>
#include <cstdlib>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: test_save_restore rom_file num_actions" << std::endl;
        return -1;
    }
    std::string rom_file(argv[1]);
    int num_actions = std::stoi(argv[2]);

    ale::ALEInterface ale;
    ale.setFloat("repeat_action_probability", 0.0); // take ALE stochasticity out of the picture
    ale.loadROM(rom_file);

    // test if saving and restoring a sequence of N arbitrary actions works as expected
    constexpr int kRamSize = 128;
    std::stack<std::array<ale::byte_t, kRamSize>> ram_history;
    std::srand(22); // arbitrary reproducible value
    for (int i = 0; i < num_actions; i++)
    {
        // save current RAM
        auto ram = ale.getRAM().array();
        std::array<ale::byte_t, kRamSize> ram_copy;
        std::copy(ram, ram+kRamSize, ram_copy.begin());
        ram_history.push(ram_copy);

        // save the ALE state
        ale.saveState();

        // take a pseudorandom action
        auto actions = ale.getMinimalActionSet();
        ale.act(actions[rand() % actions.size()]);
    }

    // undo the N actions, and check that we get back the original RAM states
    for (int i = 0; i < num_actions; i++)
    {
        ale.loadState();

        auto restored = ale.getRAM().array();
        auto expected = ram_history.top();
        if (!std::equal(restored, restored+kRamSize, expected.begin()))
        {
            std::cout << "Uh oh, loadState() didn't restore previous RAM!" << std::endl;
            std::cout << "Expected: ";
            for (int i = 0; i < kRamSize; i++)
                std::cout << std::hex << (int)expected[i];
            std::cout << std::endl << "Actual:   ";
            for (int i = 0; i < kRamSize; i++)
                std::cout << std::hex << (int)restored[i];
            std::cout << std::endl;

            std::exit(-1);
        }
    }
    std::cout << "Test successful!" << std::endl;

    return 0;
}
