/*
 * Count the number of distinct Atari states reachable in N frames, using
 * breadth-first search or iterative deepening.
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

void countstates_dfs_rec(int limit, std::map<state_t, int>& seen)
{
    // The logic here: expand a state's sucessors if it's new, *or* it is a
    // previously seen state, but now seen at a shallower depth (at a higher
    // 'limit') than before. The reason for the 2nd part: it might turn out
    // that more of its successors are now reachable within the depth limit, so
    // we need to re-expand its children with the new higher limit. (BFS
    // doesn't need to do this, because its first visit to a given state is
    // always via a shortest path.)
    const auto cur_state = current_state();
    const auto found = seen.find(cur_state);
    bool was_new_or_shallower = false;
    if (found == seen.end()) // not found
    {
        seen.try_emplace(cur_state, limit);
        was_new_or_shallower = true;
    }
    else
    {
        if (limit > found->second)
	{
            found->second = limit;
            was_new_or_shallower = true;
        }
    }

    if (was_new_or_shallower && limit > 0 && !a.game_over())
    {
        const auto actions = a.getMinimalActionSet();
        for (const auto action : actions)
        {
            const auto prev_state = a.cloneState();
            a.act(action);
            countstates_dfs_rec(limit-1, seen);
            a.restoreState(prev_state);
        }
    }
}

size_t countstates_dfs(int limit)
{
    std::map<state_t, int> seen;
    countstates_dfs_rec(limit, seen);
    return seen.size();
}

size_t countstates_bfs(int limit, int max_states, std::set<state_t>& seen, std::queue<std::pair<ale::ALEState,int>>& frontier)
{
    while (!frontier.empty())
    {
        const auto [state,depth] = frontier.front();
        if (depth >= limit)
            break;

        frontier.pop();
        const auto actions = a.getMinimalActionSet();
        for (const auto action : actions)
        {
            a.restoreState(state);
            a.act(action);
            const auto [_, was_new] = seen.insert(current_state());
            if (was_new && !a.game_over() && seen.size() < max_states)
                frontier.emplace(a.cloneState(), depth+1);
        }
    }
    return seen.size();
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "Usage: countstates rom_file max_states bfs|id" << std::endl;
        std::cerr << std::endl << "Note: When max_states is hit, will still complete the current frame's count." << std::endl;
        return -1;
    }
    const std::string rom_file(argv[1]);
    const int max_states = std::stoi(argv[2]);
    const std::string search_type(argv[3]);
    if (search_type != "bfs" && search_type != "id")
    {
        std::cerr << "Invalid search type: " << search_type << std::endl;
        return -1;
    }

    // trondead starts in a 'dead' initial state: we need to take one dummy
    // action to get the emulation going properly it seems.
    bool initial_noop = false;
    if (rom_file.find("trondead") != std::string::npos)
        initial_noop = true;

    a.setInt("frame_skip", 1); // I think this is default, but to be sure
    a.setFloat("repeat_action_probability", 0.0); // synthetic stochasticity? no thanks!
    a.loadROM(rom_file);
    // there is some weirdness around whether a reset game is always identical
    // to a freshly loaded ROM, so reset to be safe
    a.reset_game();
    if (initial_noop)
        a.act(a.getMinimalActionSet()[0]);

    const size_t num_actions = a.getMinimalActionSet().size();
    std::cerr << rom_file << " has " << num_actions << " possible actions." << std::endl;

    std::cout << "depth,states" << std::endl;
    if (search_type == "id")
    {
        int limit = 0;
        size_t states = 0;
        while (states < max_states)
        {
            states = countstates_dfs(limit);
            std::cout << limit << "," << states << std::endl;
            limit++;
            a.reset_game();
            if (initial_noop)
                a.act(a.getMinimalActionSet()[0]);
        }
    }
    else // bfs
    {
        std::set<state_t> seen;
        std::queue<std::pair<ale::ALEState,int>> frontier; // (node,depth)

        seen.insert(current_state());
        frontier.emplace(a.cloneState(), 0);

        int limit = 0;
        size_t states = 0;
        while (states < max_states)
        {
            states = countstates_bfs(limit, max_states, seen, frontier);
            std::cout << limit << "," << states << std::endl;
            limit++;
        }
    }
}
