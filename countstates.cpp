/*
 * Iteratively count the number of distinct Atari states reachable in
 * 0..max_depth frames.
 *
 * Mark J. Nelson, 2/2021
 */

#include "ale_interface.hpp"

#include <iostream>
#include <string>
#include <cmath>
#include <unordered_set>

ale::ALEInterface a;

void countstates_rec(int limit, std::unordered_set<std::string>& seen)
{
    std::string state = a.cloneState().serialize();
// alternately: use RAM as state? undercounts in paddle-based games, where paddle position is also persistent state
//    ale::ALERAM ram = a.getRAM();
//    std::string state(reinterpret_cast<const char *>(ram.array()), ram.size());

    auto [_, was_new] = seen.insert(state);

    if (was_new && limit > 0)
    {
        ale::ActionVect actions = a.getMinimalActionSet();
        for (auto action : actions)
        {
            a.saveState();
            a.act(action);
            countstates_rec(limit-1, seen);
            a.loadState();
        }
    }
}

size_t countstates(int limit)
{
    std::unordered_set<std::string> seen;
    countstates_rec(limit, seen);
    return seen.size();
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: countstates rom_file max_depth" << std::endl;
        return -1;
    }
    std::string rom_file(argv[1]);
    int max_depth = std::stoi(argv[2]);

    a.setInt("frame_skip", 1); // I think this is default, but to be sure
    a.setFloat("repeat_action_probability", 0.0); // synthetic stochasticity? no thanks!
    a.loadROM(rom_file);

    size_t num_actions = a.getMinimalActionSet().size();
    std::cerr << rom_file << " has " << num_actions << " possible actions." << std::endl;
    std::cerr << "Upper bound on number of states: " << num_actions << "^" << max_depth << " ≈ 10^" << max_depth * std::log10(num_actions) << std::endl;

    std::cout << "depth,states" << std::endl;
    for (int limit = 0; limit <= max_depth; limit++)
    {
        size_t states = countstates(limit);
        std::cout << limit << "," << states << std::endl;
        a.reset_game();
    }
}
