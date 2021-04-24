/*
 * Count the number of distinct Atari states reachable in 0..max_depth frames,
 * using breadth-first search or iterative deepening.
 *
 * Mark J. Nelson, 2021
 */

#include "ale_interface.hpp"

#include <iostream>
#include <string>
#include <cmath>
#include <set>
#include <queue>
#include <array>

ale::ALEInterface a;

// for 1-player games, the RAM plus the left paddle position is the complete game state
constexpr size_t kRamSize = 128;
using ram_t = std::array<ale::byte_t, kRamSize>;
using state_t = std::pair<ram_t, int>;
state_t current_state()
{
    const ale::byte_t* ram_temp = a.getRAM().array();
    ram_t ram;
    std::copy(ram_temp, ram_temp + kRamSize, ram.begin());
    return std::make_pair(ram, a.getLeftPaddle());
}

void countstates_dfs_rec(int limit, std::set<state_t>& seen)
{
    auto [_, was_new] = seen.insert(current_state());

    if (was_new && limit > 0 && !a.game_over())
    {
        ale::ActionVect actions = a.getMinimalActionSet();
        for (auto action : actions)
        {
            a.saveState();
            a.act(action);
            countstates_dfs_rec(limit-1, seen);
            a.loadState();
        }
    }
}

size_t countstates_dfs(int limit)
{
    std::set<state_t> seen;
    countstates_dfs_rec(limit, seen);
    return seen.size();
}

size_t countstates_bfs(int limit, std::set<state_t>& seen, std::queue<std::pair<ale::ALEState,int>>& frontier)
{
    while (!frontier.empty())
    {
        auto [state,depth] = frontier.front();
        if (depth >= limit)
            break;

        frontier.pop();
        ale::ActionVect actions = a.getMinimalActionSet();
        for (auto action : actions)
        {
            a.restoreState(state);
            a.act(action);
            auto [_, was_new] = seen.insert(current_state());
            if (was_new && !a.game_over())
            {
                ale::ALEState successor = a.cloneState();
                frontier.push(std::make_pair(successor,depth+1));
            }
        }
    }
    return seen.size();
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "Usage: countstates rom_file max_depth bfs|id" << std::endl;
        return -1;
    }
    std::string rom_file(argv[1]);
    int max_depth = std::stoi(argv[2]);
    std::string search_type(argv[3]);
    if (search_type != "bfs" && search_type != "id")
    {
        std::cerr << "Invalid search type: " << search_type << std::endl;
        return -1;
    }

    a.setInt("frame_skip", 1); // I think this is default, but to be sure
    a.setFloat("repeat_action_probability", 0.0); // synthetic stochasticity? no thanks!
    a.loadROM(rom_file);

    size_t num_actions = a.getMinimalActionSet().size();
    std::cerr << rom_file << " has " << num_actions << " possible actions." << std::endl;
    std::cerr << "Upper bound on number of states: " << num_actions << "^" << max_depth << " ≈ 10^" << max_depth * std::log10(num_actions) << std::endl;

    std::cout << "depth,states" << std::endl;
    if (search_type == "id")
    {
        for (int limit = 0; limit <= max_depth; limit++)
        {
            size_t states = countstates_dfs(limit);
            std::cout << limit << "," << states << std::endl;
        }
    }
    else // bfs
    {
        std::set<state_t> seen;
        std::queue<std::pair<ale::ALEState,int>> frontier; // (node,depth)

        ale::ALEState root = a.cloneState();
        seen.insert(current_state());
        frontier.push(std::make_pair(root, 0));

        for (int limit = 0; limit <= max_depth; limit++)
        {
            size_t states = countstates_bfs(limit, seen, frontier);
            std::cout << limit << "," << states << std::endl;
        }
    }
}
