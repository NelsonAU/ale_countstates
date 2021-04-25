#include "ale_interface.hpp"

#include <iostream>
#include <string>
#include <array>
#include <algorithm>
#include <cstdlib>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: test_save_restore rom_file" << std::endl;
        return -1;
    }
    std::string rom_file(argv[1]);

    ale::ALEInterface ale;
    ale.setFloat("repeat_action_probability", 0.0); // take ALE stochasticity out of the picture
    ale.loadROM(rom_file);

    // test if taking two actions, restoring the previous state, and taking the
    // same actions again results in the same state
    constexpr int kRamSize = 128;
    std::srand(22); // arbitrary reproducible value

    // save the ALE state
    auto init_state = ale.cloneState();

    // take two pseudorandom actions
    auto actions = ale.getMinimalActionSet();
    auto action1 = actions[rand() % actions.size()];
    auto action2 = actions[rand() % actions.size()];
    ale.act(action1);
    ale.act(action2);

    // save the resulting RAM
    auto ram = ale.getRAM().array();
    std::array<ale::byte_t, kRamSize> ram_copy;
    std::copy(ram, ram+kRamSize, ram_copy.begin());

    // restore the initial state
    ale.restoreState(init_state);

    // take the same actions again
    ale.act(action1);
    ale.act(action2);

    // the RAM should now be identical to our saved RAM copy
    ram = ale.getRAM().array();
    if (!std::equal(ram, ram+kRamSize, ram_copy.begin()))
    {
        std::cout << "Took same actions from same state but got different results!" << std::endl;
        std::cout << "Expected RAM: ";
        std::cout << std::setfill('0') << std::hex;
        for (int i = 0; i < kRamSize; i++)
            std::cout << std::setw(2) << (int)ram_copy[i];
        std::cout << std::endl << "Actual RAM:   ";
        for (int i = 0; i < kRamSize; i++)
            std::cout << std::setw(2) << (int)ram[i];
        std::cout << std::endl;
        return -2;
    }

    std::cout << "No inconsistencies found!" << std::endl;

    return 0;
}
