/*
#include "EngineVersion17.h"
#include "EngineVersion18.h"
#include "EngineVersion19.h"
#include "EngineVersion20.h"
#include "EngineVersion21.h"
#include "EngineVersion22.h"
#include "EngineVersion23.h"
#include "EngineVersion24.h"
#include "EngineVersion25.h"
#include "EngineVersion26.h"
#include "EngineVersion27.h"
#include "EngineVersion28.h"
#include "EngineVersion29.h"
#include "EngineVersion30.h"
#include "EngineVersion31.h"
#include "EngineVersion32.h"
#include "EngineVersion33.h"
#include "EngineVersion34.h"
#include "EngineVersion35.h"
#include "EngineVersion36.h"
#include "EngineVersion37.h"
#include "EngineVersion38.h"
#include "EngineVersion39.h"
#include "EngineVersion40.h"
#include "EngineVersion41.h"
#include "EngineVersion42.h"
#include "EngineVersion43.h"
*/
#include "EngineVersion44.h"
#include "EngineVersion45.h"
//#include "EngineVersion49.h"

#include "tool.h"
#include "CommonNonClassData.h"
#include <iostream>
#include <vector>
#include <time.h>
#include <cstdlib>
#include <fstream>
#include <ctime>
#include <ratio>
#include <chrono>
#include <random>
#include <stdexcept>
#include <algorithm>

using namespace std;

using namespace std::chrono;

struct RunningData
{
    int num_EngineOne_wins;

    int num_EngineTwo_wins;

    int num_draws;

    duration<double> total_time_EngineOne_has_thought;

    duration<double> total_time_EngineTwo_has_thought;

    int num_instances_EngineOne_thought; // Stores the number of times EngineOne thought for the normal 0.30s.

    int num_instances_EngineTwo_thought; // Stores the number of times EngineTwo thought for the normal 0.30s.

    int num_level_trials; // stores the number of trials where the score was 1-1.

    int num_trials_favouring_EngineOne; // stores the number of trials where EngineOne scored 2-0 or 1.5-0.5

    int num_trials_favouring_EngineTwo; // stores the number of trials where EngineTwo scored 2-0 or 1.5-0.5

    duration<double> time_EngineOne_spent_in_find_best_move;

    duration<double> time_EngineTwo_spent_in_find_best_move;

    int num_instances_EngineOne_used_find_best_move;

    int num_instances_EngineTwo_used_find_best_move;

    double average_number_moves_in_games_EngineOne_won;

    double average_number_moves_in_games_EngineTwo_won;

    double average_number_moves_in_games_drawn;

    duration<double> max_time_EngineOne_spent_at_once_in_find_best_move;

    duration<double> max_time_EngineTwo_spent_at_once_in_find_best_move;

    double time_EngineOne_thought_in_current_trial;

    double time_EngineTwo_thought_in_current_trial;

    vector<vector<vector<char>>> all_ending_boards; // stores all the ending boards for each game.

    double total_time_EngineOne_spent_on_zero_thinks = 0.0; // thinks where thinking_time = 0.

    int num_EngineOne_zero_thinks = 0;

    double max_time_EngineOne_spent_on_zero_think = 0.0;

    double total_time_EngineTwo_spent_on_zero_thinks = 0.0;

    int num_EngineTwo_zero_thinks = 0;

    double max_time_EngineTwo_spent_on_zero_think = 0.0;
};

struct UnevenTrialStats
{
    int EngineOne_wins_in_trial;

    int EngineTwo_wins_in_trial;

    vector<coordinate> set_of_moves_for_starting_position;

    int EngineOneVersion;

    int EngineTwoVersion;

    double rate_EngineOne_thought_in_trial;

    double rate_EngineTwo_thought_in_trial;

    vector<vector<vector<char>>> ending_boards;
};

struct Engine_numbers_for_a_match
{
    int first_Engine_version;

    int second_Engine_version;
};

struct set_package
{
    vector<coordinate> set_of_moves;
    int hash_value;
    vector<vector<char>> board;
};

const int max_row_index = 5;
const int max_col_index = 6;

void play_starting_moves_on_to_board(vector<vector<char>>& board, const vector<coordinate>& starting_moves)
{
    char current_char = 'C';

    for (const coordinate& current_move: starting_moves)
    {
        board[current_move.row][current_move.col] = current_char;

        if (current_char == 'C')
        {
            current_char = 'U';
        }

        else
        {
            current_char = 'C';
        }
    }
}

void initialize_empty_board(vector<vector<char>>& board) {
    board.assign(6, vector<char>(7, ' '));
}

vector<vector<char>> return_board_with_starting_moves_played(const vector<coordinate>& moves) {
    vector<vector<char>> board;
    initialize_empty_board(board);
    play_starting_moves_on_to_board(board, moves);
    return board;
}

int get_hash_value_of_moves(const vector<coordinate>& set_of_moves) {
    int hash_value = 0;
    vector<vector<char>> board;
    initialize_empty_board(board);
    for (const coordinate& current_move: set_of_moves) {
        board[current_move.row][current_move.col] = 'C'; 
        // I could also alternate between placing 'C' and 'U', but it doesn't really matter.
        // The hash_value is just to serve as a rough representation, to make the remove_duplicate_sets
        // function faster (the hash_value doesn't have to be perfect or anything).
    }

    for (int r = 0; r <= max_row_index; ++r) {
        for (int c = 0; c <= max_col_index; ++c) {
            int base = (c+1)*(r+1);
            int exponent = board[r][c] == 'C' ? 4 : 2; // 4 if the square has a piece, 2 if it's empty.
            hash_value += pow(base, exponent);
        }
    }
    return hash_value;
}

bool compare_sets_of_starting_moves(const set_package& first_set, const set_package& second_set)
{
    if (first_set.set_of_moves.size() < second_set.set_of_moves.size())
    {
        return true;
    }

    else if (first_set.set_of_moves.size() > second_set.set_of_moves.size())
    {
        return false;
    }

    return (first_set.hash_value < second_set.hash_value);
}

void remove_duplicate_sets(vector<set_package>& sets)
{
    vector<set_package> replacement;

    for (int i = 0; i < sets.size(); i++)
    {
        bool is_duplicate = false;

        for (int x = i-1; x >= 0 && sets[x].hash_value == sets[i].hash_value &&
             sets[x].set_of_moves.size() == sets[i].set_of_moves.size(); x--)
        {
            if (sets[x].board == sets[i].board)
            {
                is_duplicate = true;

                break;
            }
        }

        if (!is_duplicate)
        {
            replacement.push_back(sets[i]);
        }
    }

    sets = replacement;
}

vector<vector<coordinate>> record_all_sets_of_possible_starting_moves(int max_num_moves_to_make)
{
    vector<set_package> raw_sets;

    vector<coordinate> moves;

    for (int c = 0; c <= max_col_index; c++)
    {
        coordinate temp;
        temp.row = max_row_index;
        temp.col = c;
        moves.push_back(temp);
    }

    random_device rd;
    mt19937 g(rd());

    for (int i = 1; i <= 1000000; i++)
    {
        vector<coordinate> copy_moves = moves;

        vector<coordinate> current_set; // Will be added to raw_sets.

        uniform_int_distribution<int> uid_max_num_moves(1, max_num_moves_to_make);

        int num_pieces = uid_max_num_moves(g);

        for (int x = 1; x <= num_pieces; x++)
        {
            uniform_int_distribution<int> uid_copy_moves_size(1, copy_moves.size());

            int rand_index = uid_copy_moves_size(g) - 1;

            current_set.push_back(copy_moves[rand_index]);

            copy_moves[rand_index].row--;

            if (copy_moves[rand_index].row == -1)
            {
                copy_moves.erase(copy_moves.begin() + rand_index);
            }
        }

        raw_sets.push_back({current_set, get_hash_value_of_moves(current_set),
                            return_board_with_starting_moves_played(current_set)});
    }

    cout << raw_sets.size() << " starting positions generated\n";

    sort(raw_sets.begin(), raw_sets.end(), compare_sets_of_starting_moves);
    // helps the remove_duplicate_sets function run faster.

    remove_duplicate_sets(raw_sets);

    shuffle(raw_sets.begin(), raw_sets.end(), g);

    vector<vector<coordinate>> return_vec;

    for (const set_package& current_set: raw_sets)
    {
        return_vec.push_back(current_set.set_of_moves);
    }

    return return_vec;
}

void make_it_fair_for_smaller_sets(vector<vector<coordinate>>& sets, int largest_size_of_a_set)
{
    // There are far fewer smaller sets than larger sets. Make it so all smaller sets get presented
    // in equal number as all other sets, until that smaller set is completely represented (nothing remaining).
    // All sets range in size from 1 - 8 elements.

    vector<vector<vector<coordinate>>> sets_separated_by_their_sizes;

    for (int i = 1; i <= largest_size_of_a_set; i++)
    {
        vector<vector<coordinate>> temp;
        sets_separated_by_their_sizes.push_back(temp);
    }

    for (const vector<coordinate>& current_set: sets)
    {
        sets_separated_by_their_sizes[current_set.size()-1].push_back(current_set);
    }

    // Randomize the order of each group of sets in sets_separated_by_their_sizes:
    random_device rd;
    mt19937 g(rd());
    for (vector<vector<coordinate>>& current_group: sets_separated_by_their_sizes)
    {
        shuffle(current_group.begin(), current_group.end(), g);
    }

    vector<vector<coordinate>> replacement; // sets parameter will be set equal to it.

    int current_index = 0;

    while (sets_separated_by_their_sizes.size() > 0)
    {
        replacement.push_back(sets_separated_by_their_sizes[current_index].back());

        sets_separated_by_their_sizes[current_index].pop_back();

        if (sets_separated_by_their_sizes[current_index].size() == 0)
        {
            sets_separated_by_their_sizes.erase(sets_separated_by_their_sizes.begin() + current_index);
        }

        current_index ++;

        if (current_index >= sets_separated_by_their_sizes.size())
        {
            current_index = 0;
        }
    }

    sets = replacement;
}

void display_board(const vector<vector<char>>& board)
{
    cout << "\n    A   B   C   D   E   F   G\n\n";

    for (int row = 0; row <= 5; row++)
    {
        cout << (6 - row) << " | "; // Since I want row numbers to be displayed increasing from bottom up, not top down.

        for (int col = 0; col <= 6; col++)
        {
            cout << board[row][col] << " | ";
        }

        cout << "\n" << "  |---|---|---|---|---|---|---|\n";
    }
}

void print_board_with_starting_moves(const vector<coordinate>& starting_moves) // Function is for testing.
{
    vector<vector<char>> board;

    initialize_empty_board(board);

    play_starting_moves_on_to_board(board, starting_moves);

    display_board(board);
}

void point_to_correct_derived_class(unique_ptr<tool>& pt, int version_number_of_Engine, double thinking_time)
{
    double old_thinking_time = thinking_time; // just to be safe, this variable doesn't need to even be given a value yet.

    coordinate placeholder = {EngineVersion44::position::UNDEFINED, EngineVersion44::position::UNDEFINED};
/*
    if (version_number_of_Engine == 17)
    {
        old_thinking_time = EngineVersion17::position::thinking_time;

        EngineVersion17::position::thinking_time = 0;

        pt = EngineVersion17::position::think_on_game_position(true, true);
    }

    else if (version_number_of_Engine == 18)
    {
        old_thinking_time = EngineVersion18::position::thinking_time;

        EngineVersion18::position::thinking_time = 0;

        pt = EngineVersion18::position::think_on_game_position(true, true);
    }

    else if (version_number_of_Engine == 19)
    {
        old_thinking_time = EngineVersion19::position::thinking_time;

        EngineVersion19::position::thinking_time = 0;

        pt = EngineVersion19::position::think_on_game_position(true, true);
    }

    else if (version_number_of_Engine == 20)
    {
        old_thinking_time = EngineVersion20::position::thinking_time;

        EngineVersion20::position::thinking_time = 0;

        pt = EngineVersion20::position::think_on_game_position(true, true);
    }

    else if (version_number_of_Engine == 21)
    {
        old_thinking_time = EngineVersion21::position::thinking_time;

        EngineVersion21::position::thinking_time = 0;

        pt = EngineVersion21::position::think_on_game_position(true, true);
    }

    else if (version_number_of_Engine == 22)
    {
        old_thinking_time = EngineVersion22::position::thinking_time;

        EngineVersion22::position::thinking_time = 0;

        pt = EngineVersion22::position::think_on_game_position(true, true);
    }

    else if (version_number_of_Engine == 23)
    {
        old_thinking_time = EngineVersion23::position::thinking_time;

        EngineVersion23::position::thinking_time = 0;

        pt = EngineVersion23::position::think_on_game_position(true, true);
    }

    else if (version_number_of_Engine == 24)
    {
        old_thinking_time = EngineVersion24::position::thinking_time;

        EngineVersion24::position::thinking_time = 0;

        pt = EngineVersion24::position::think_on_game_position(true, true);
    }

    else if (version_number_of_Engine == 25)
    {
        old_thinking_time = EngineVersion25::position::thinking_time;

        EngineVersion25::position::thinking_time = 0;

        pt = EngineVersion25::position::think_on_game_position(true, true);
    }

    else if (version_number_of_Engine == 26)
    {
        old_thinking_time = EngineVersion26::position::thinking_time;

        EngineVersion26::position::thinking_time = 0;

        pt = EngineVersion26::position::think_on_game_position(true, true);
    }

    else if (version_number_of_Engine == 27)
    {
        old_thinking_time = EngineVersion27::position::thinking_time;

        EngineVersion27::position::thinking_time = 0;

        pt = EngineVersion27::position::think_on_game_position(true, true);
    }

    else if (version_number_of_Engine == 28)
    {
        old_thinking_time = EngineVersion28::position::thinking_time;

        EngineVersion28::position::thinking_time = 0;

        pt = EngineVersion28::position::think_on_game_position(true, true);
    }

    else if (version_number_of_Engine == 29)
    {
        old_thinking_time = EngineVersion29::position::thinking_time;

        EngineVersion29::position::thinking_time = 0;

        pt = EngineVersion29::position::think_on_game_position(true, true);
    }

    else if (version_number_of_Engine == 30)
    {
        old_thinking_time = EngineVersion30::position::thinking_time;

        EngineVersion30::position::thinking_time = 0;

        pt = EngineVersion30::position::think_on_game_position(true, true);
    }

    else if (version_number_of_Engine == 31)
    {
        old_thinking_time = EngineVersion31::position::thinking_time;

        EngineVersion31::position::thinking_time = 0;

        pt = EngineVersion31::position::think_on_game_position(true, true);
    }

    else if (version_number_of_Engine == 32)
    {
        old_thinking_time = EngineVersion32::position::thinking_time;

        EngineVersion32::position::thinking_time = 0;

        pt = EngineVersion32::position::think_on_game_position(true, true);
    }

    else if (version_number_of_Engine == 33)
    {
        old_thinking_time = EngineVersion33::position::thinking_time;

        EngineVersion33::position::thinking_time = 0;

        pt = EngineVersion33::position::think_on_game_position(true, true);
    }

    else if (version_number_of_Engine == 34)
    {
        old_thinking_time = EngineVersion34::position::thinking_time;

        EngineVersion34::position::thinking_time = 0;

        pt = EngineVersion34::position::think_on_game_position(true, true);
    }

    else if (version_number_of_Engine == 35)
    {
        old_thinking_time = EngineVersion35::position::thinking_time;

        EngineVersion35::position::thinking_time = 0;

        pt = EngineVersion35::position::think_on_game_position(true, true);
    }

    else if (version_number_of_Engine == 36)
    {
        old_thinking_time = EngineVersion36::position::thinking_time;

        EngineVersion36::position::thinking_time = 0;

        pt = EngineVersion36::position::think_on_game_position(true, true);
    }

    else if (version_number_of_Engine == 37)
    {
        old_thinking_time = EngineVersion37::position::thinking_time;

        EngineVersion37::position::thinking_time = 0;

        pt = EngineVersion37::position::think_on_game_position(true, true);
    }

    else if (version_number_of_Engine == 38)
    {
        old_thinking_time = EngineVersion38::position::thinking_time;

        EngineVersion38::position::thinking_time = 0;

        pt = EngineVersion38::position::think_on_game_position(true, true);
    }

    else if (version_number_of_Engine == 39)
    {
        old_thinking_time = EngineVersion39::position::thinking_time;

        EngineVersion39::position::thinking_time = 0;

        pt = EngineVersion39::position::think_on_game_position(true, true, placeholder, false);
    }

    else if (version_number_of_Engine == 40)
    {
        old_thinking_time = EngineVersion40::position::thinking_time;

        EngineVersion40::position::thinking_time = 0;

        pt = EngineVersion40::position::think_on_game_position(true, true, placeholder, false);
    }

    else if (version_number_of_Engine == 41)
    {
        old_thinking_time = EngineVersion41::position::thinking_time;

        EngineVersion41::position::thinking_time = 0;

        pt = EngineVersion41::position::think_on_game_position(true, true, placeholder, false);
    }

    else if (version_number_of_Engine == 42)
    {
        old_thinking_time = EngineVersion42::position::thinking_time;

        EngineVersion42::position::thinking_time = 0;

        pt = EngineVersion42::position::think_on_game_position(true, true, placeholder, false);
    }

    else if (version_number_of_Engine == 43)
    {
        old_thinking_time = EngineVersion43::position::thinking_time;

        EngineVersion43::position::thinking_time = 0;

        pt = EngineVersion43::position::think_on_game_position(true, true, placeholder, false);
    }
*/

    if (version_number_of_Engine == 44)
    {
        old_thinking_time = EngineVersion44::position::thinking_time;

        EngineVersion44::position::thinking_time = 0;

        pt = EngineVersion44::position::think_on_game_position(true, true, placeholder, false);
    }

    else if (version_number_of_Engine == 45)
    {
        old_thinking_time = EngineVersion45::position::thinking_time;

        EngineVersion45::position::thinking_time = 0;

        pt = EngineVersion45::position::think_on_game_position(true, true, placeholder, false);
    }
/*
    else if (version_number_of_Engine == 49)
    {
        old_thinking_time = EngineVersion49::position::thinking_time;

        EngineVersion49::position::thinking_time = 0;

        pt = EngineVersion49::position::think_on_game_position(true, true, placeholder, false);
    }
*/
    // Continue writing else if statements here for future Versions added.......

    else
    {
        throw runtime_error("Version doesn't exist in this Simulation!\n");
    }

    pt->set_static_thinking_time(old_thinking_time);
}

void get_Engine_primed_to_play(const vector<coordinate>& starting_moves, unique_ptr<tool>& pt,
                                bool is_engine_moving_first_in_destination_position, int version_number_of_Engine,
                                double thinking_time, bool& someone_already_wins)
{
    // Function gets pt (passed by reference) to the position reached after starting_moves.

    // On the way to this position, pt will not be given time to think deeply (will only be able to think to depth 1).
    // pt will only think deeply at normal thinking time when it reaches the destination position.

    point_to_correct_derived_class(pt, version_number_of_Engine, thinking_time);
    // Gets base class pointer pt to point to the right position class by calling a think_ function with 0 passed in
    // for temporary thinking_time.

    pt->set_static_thinking_time(thinking_time);

    double old_thinking_time = pt->get_static_thinking_time();

    pt->set_static_thinking_time(0);

    vector<vector<char>> assisting_board; // the moves will be played on this board and sent to pt's position class.

    initialize_empty_board(assisting_board);

    char current_char_to_be_placed = 'U';

    if ((starting_moves.size() % 2 == 0 && is_engine_moving_first_in_destination_position) ||
        (starting_moves.size() % 2 != 0 && !is_engine_moving_first_in_destination_position))
    {
        current_char_to_be_placed = 'C';
    }

    pt = pt->call_static_think_on_game_position(current_char_to_be_placed == 'C', true);

    for (int i = 0; i < starting_moves.size(); i++)
    {
        assisting_board[starting_moves[i].row][starting_moves[i].col] = current_char_to_be_placed;

        if (current_char_to_be_placed == 'C')
        {
            current_char_to_be_placed = 'U';
        }

        else
        {
            current_char_to_be_placed = 'C';
        }

        if (i == starting_moves.size() - 1)
        {
            // Final move - think at normal duration:

            pt->set_static_thinking_time(old_thinking_time);
        }

        pt = pt->call_static_think_on_game_position(assisting_board, current_char_to_be_placed == 'C',
                                                    starting_moves[i], pt->get_squares_amplifying_comp_2(),
                                                    pt->get_squares_amplifying_comp_3(), pt->get_squares_amplifying_user_2(),
                                                    pt->get_squares_amplifying_user_3(), true);

        if (pt->get_evaluation() == INT_MAX || pt->get_evaluation() == INT_MIN)
        {
            someone_already_wins = true;

            return;
        }
    }

    if (pt->get_static_thinking_time() != old_thinking_time)
    {
        throw runtime_error("EngineOne's thinking_time was not set back to normal.\n");
    }
}

bool fair(const vector<coordinate>& starting_moves, int version_EngineOne, int version_EngineTwo, double thinking_time)
{
    // Returns true if both Engines agree that the passed in set of starting moves leads to a fair position.
    // I.e., > -20 and < 20

    bool someone_already_wins = false;

    unique_ptr<tool> pt_EngineOne;
    get_Engine_primed_to_play(starting_moves, pt_EngineOne, true, version_EngineOne, thinking_time, someone_already_wins);

    if (someone_already_wins)
    {
        return false;
    }

    unique_ptr<tool> pt_EngineTwo;
    get_Engine_primed_to_play(starting_moves, pt_EngineTwo, true, version_EngineTwo, thinking_time, someone_already_wins);

    if (someone_already_wins)
    {
        return false;
    }

    if (pt_EngineOne->get_board() != pt_EngineTwo->get_board())
    {
        throw runtime_error("boards not equal in fair()\n");
    }

    if (pt_EngineOne->get_evaluation() > -20 && pt_EngineOne->get_evaluation() < 20 &&
        pt_EngineTwo->get_evaluation() > -20 && pt_EngineTwo->get_evaluation() < 20)
    {
        display_board(pt_EngineOne->get_board());

        cout << "EngineOne's evaluation: " << pt_EngineOne->get_evaluation() << "\n";

        cout << "EngineTwo's evaluation: " << pt_EngineTwo->get_evaluation() << "\n";

        return true;
    }

    return false;
}

void flip_board(vector<vector<char>>& board)
{
    for (int row = 0; row <= max_row_index; row++)
    {
        for (int col = 0; col <= max_col_index; col++)
        {
            if (board[row][col] == 'C')
            {
                board[row][col] = 'U';
            }

            else if (board[row][col] == 'U')
            {
                board[row][col] = 'C';
            }
        }
    }
}

bool is_board_legal(const vector<vector<char>>& board)
{
    for (int col = 0; col <= max_col_index; col++)
    {
        bool encountered_piece_yet = false;

        for (int row = 0; row <= max_row_index; row++)
        {
            if (encountered_piece_yet && board[row][col] == ' ')
            {
                return false;
            }

            else if (board[row][col] != ' ')
            {
                encountered_piece_yet = true;
            }
        }
    }

    return true;
}

void playout_game(RunningData& simulation_stats, const vector<coordinate>& starting_moves, bool EngineOne_moves_first,
                  int version_EngineOne, int version_EngineTwo, double thinking_time)
{
    bool someone_already_won = false;

    unique_ptr<tool> pt_EngineOne;
    get_Engine_primed_to_play(starting_moves, pt_EngineOne, EngineOne_moves_first, version_EngineOne, thinking_time,
                              someone_already_won);

    unique_ptr<tool> pt_EngineTwo;
    get_Engine_primed_to_play(starting_moves, pt_EngineTwo, !EngineOne_moves_first, version_EngineTwo, thinking_time,
                              someone_already_won);

    if (someone_already_won)
    {
        throw runtime_error("Engines were about to play a position where someone already had a 4-in-a-row");
    }

    vector<vector<char>> assisting_board;

    if (EngineOne_moves_first) // Then EngineOne's board is ready for EngineOne to place a 'C' on it on the next turn:
    {
        assisting_board = pt_EngineOne->get_board();
    }

    else
    {
        assisting_board = pt_EngineTwo->get_board();
    }

    if (!is_board_legal(assisting_board))
    {
        throw runtime_error("Illegal starting board in playout_game()\n");
    }

    const double old_thinking_time_EngineOne = pt_EngineOne->get_static_thinking_time();

    const double old_thinking_time_EngineTwo = pt_EngineTwo->get_static_thinking_time();

    if (old_thinking_time_EngineOne != thinking_time || old_thinking_time_EngineTwo != thinking_time)
    {
        throw runtime_error("Standard thinking time isn't standard value for one of the Engines.\n");
    }

    bool EngineOne_turn = EngineOne_moves_first;

    int num_moves_played = 0; // Will store the number of moves played in the game.

    while (!pt_EngineOne->did_computer_win() && !pt_EngineOne->did_opponent_win() && !pt_EngineOne->is_game_drawn() &&
           !pt_EngineTwo->did_computer_win() && !pt_EngineTwo->did_opponent_win() && !pt_EngineTwo->is_game_drawn())
    {
        if (EngineOne_turn)
        {
            steady_clock::time_point start_time = steady_clock::now();

            coordinate chosen_move = pt_EngineOne->find_best_move_for_comp();

            duration<double> time_just_spent = duration_cast<duration<double>>(steady_clock::now() - start_time);

            simulation_stats.time_EngineOne_spent_in_find_best_move += time_just_spent;

            if (time_just_spent.count() > simulation_stats.max_time_EngineOne_spent_at_once_in_find_best_move.count())
            {
                simulation_stats.max_time_EngineOne_spent_at_once_in_find_best_move = time_just_spent;
            }

            simulation_stats.num_instances_EngineOne_used_find_best_move ++;

            if (chosen_move.row < 0 || chosen_move.row > 6 || chosen_move.col < 0 || chosen_move.col > 7) {
                throw runtime_error("EngineOne trying to play a move that's out of bouds.\n");
            }

            if (assisting_board[chosen_move.row][chosen_move.col] != ' ')
            {
                throw runtime_error("EngineOne trying to play in a filled up square.\n");
            }

            if (chosen_move.row != max_row_index && assisting_board[chosen_move.row+1][chosen_move.col] == ' ')
            {
                throw runtime_error("EngineOne trying to play in a square with no piece below it.\n");
            }

            assisting_board[chosen_move.row][chosen_move.col] = 'C';

            if (!is_board_legal(assisting_board))
            {
                throw runtime_error("EngineOne made the board illegal with its last move.\n");
            }

            num_moves_played ++;

            // Now to get BOTH EngineOne and EngineTwo to think on this new position. Since EngineOne has made its
            // move and its turn is over, I'll make its thinking_time = 0 temporarily. This will allow it to
            // update its amplifying vectors, while only being able to think at depth 1 (which doesn't boost its TT's strength).

            pt_EngineOne->set_static_thinking_time(0);

            steady_clock::time_point zero_start_point = steady_clock::now();

            pt_EngineOne = pt_EngineOne->call_static_think_on_game_position(assisting_board, false, chosen_move,
                                                                            pt_EngineOne->get_squares_amplifying_comp_2(),
                                                                            pt_EngineOne->get_squares_amplifying_comp_3(),
                                                                            pt_EngineOne->get_squares_amplifying_user_2(),
                                                                            pt_EngineOne->get_squares_amplifying_user_3(), false);

            duration<double> time_spent_on_zero_think = duration_cast<duration<double>>(steady_clock::now() - zero_start_point);

            simulation_stats.total_time_EngineOne_spent_on_zero_thinks += time_spent_on_zero_think.count();

            simulation_stats.num_EngineOne_zero_thinks ++;

            if (time_spent_on_zero_think.count() >
                simulation_stats.max_time_EngineOne_spent_on_zero_think)
            {
                simulation_stats.max_time_EngineOne_spent_on_zero_think = time_spent_on_zero_think.count();
            }

            pt_EngineOne->set_static_thinking_time(old_thinking_time_EngineOne);

            flip_board(assisting_board);

            start_time = steady_clock::now();

            pt_EngineTwo = pt_EngineTwo->call_static_think_on_game_position(assisting_board, true, chosen_move,
                                                                            pt_EngineTwo->get_squares_amplifying_comp_2(),
                                                                            pt_EngineTwo->get_squares_amplifying_comp_3(),
                                                                            pt_EngineTwo->get_squares_amplifying_user_2(),
                                                                            pt_EngineTwo->get_squares_amplifying_user_3(), false);

            simulation_stats.total_time_EngineTwo_has_thought += duration_cast<duration<double>>(steady_clock::now() - start_time);

            simulation_stats.time_EngineTwo_thought_in_current_trial += duration_cast<duration<double>>(steady_clock::now() - start_time).count();

            simulation_stats.num_instances_EngineTwo_thought ++;
        }

        else // EngineTwo's turn:
        {
            steady_clock::time_point start_time = steady_clock::now();

            coordinate chosen_move = pt_EngineTwo->find_best_move_for_comp();

            duration<double> time_just_spent = duration_cast<duration<double>>(steady_clock::now() - start_time);

            simulation_stats.time_EngineTwo_spent_in_find_best_move += time_just_spent;

            if (time_just_spent.count() > simulation_stats.max_time_EngineTwo_spent_at_once_in_find_best_move.count())
            {
                simulation_stats.max_time_EngineTwo_spent_at_once_in_find_best_move = time_just_spent;
            }

            simulation_stats.num_instances_EngineTwo_used_find_best_move ++;

            if (chosen_move.row < 0 || chosen_move.row > 6 || chosen_move.col < 0 || chosen_move.col > 7) {
                throw runtime_error("EngineTwo trying to play a move that's out of bouds.\n");
            }

            if (assisting_board[chosen_move.row][chosen_move.col] != ' ')
            {
                throw runtime_error("EngineTwo trying to play in a filled up square.\n");
            }

            if (chosen_move.row != max_row_index && assisting_board[chosen_move.row+1][chosen_move.col] == ' ')
            {
                throw runtime_error("EngineTwo trying to play in a square with no piece below it.\n");
            }

            assisting_board[chosen_move.row][chosen_move.col] = 'C';

            if (!is_board_legal(assisting_board))
            {
                throw runtime_error("EngineTwo made the board illegal with its last move.\n");
            }

            num_moves_played ++;

            // Now to get BOTH EngineOne and EngineTwo to think on this new position. Since EngineTwo has made its
            // move and its turn is over, I'll make its thinking_time = 0 temporarily. This will allow it to
            // update its amplifying vectors, while only being able to search at depth 1 (not boosting its TT's strength at all).

            pt_EngineTwo->set_static_thinking_time(0);

            steady_clock::time_point zero_start_point = steady_clock::now();

            pt_EngineTwo = pt_EngineTwo->call_static_think_on_game_position(assisting_board, false, chosen_move,
                                                                            pt_EngineTwo->get_squares_amplifying_comp_2(),
                                                                            pt_EngineTwo->get_squares_amplifying_comp_3(),
                                                                            pt_EngineTwo->get_squares_amplifying_user_2(),
                                                                            pt_EngineTwo->get_squares_amplifying_user_3(), false);

            duration<double> time_spent_on_zero_think = duration_cast<duration<double>>(steady_clock::now() - zero_start_point);

            simulation_stats.total_time_EngineTwo_spent_on_zero_thinks += time_spent_on_zero_think.count();

            simulation_stats.num_EngineTwo_zero_thinks ++;

            if (time_spent_on_zero_think.count() >
                simulation_stats.max_time_EngineTwo_spent_on_zero_think)
            {
                simulation_stats.max_time_EngineTwo_spent_on_zero_think = time_spent_on_zero_think.count();
            }

            pt_EngineTwo->set_static_thinking_time(old_thinking_time_EngineTwo);

            flip_board(assisting_board);

            start_time = steady_clock::now();

            pt_EngineOne = pt_EngineOne->call_static_think_on_game_position(assisting_board, true, chosen_move,
                                                                            pt_EngineOne->get_squares_amplifying_comp_2(),
                                                                            pt_EngineOne->get_squares_amplifying_comp_3(),
                                                                            pt_EngineOne->get_squares_amplifying_user_2(),
                                                                            pt_EngineOne->get_squares_amplifying_user_3(), false);

            simulation_stats.total_time_EngineOne_has_thought += duration_cast<duration<double>>(steady_clock::now() - start_time);

            simulation_stats.time_EngineOne_thought_in_current_trial += duration_cast<duration<double>>(steady_clock::now() - start_time).count();

            simulation_stats.num_instances_EngineOne_thought ++;
        }

        EngineOne_turn = !EngineOne_turn;
    }

    if (!is_board_legal(assisting_board))
    {
        throw runtime_error("The ending board is illegal.\n");
    }

    simulation_stats.all_ending_boards.push_back(assisting_board);

    // Now see what result to record, and make sure the Engines agree on this result.

    if (pt_EngineOne->did_computer_win() && pt_EngineTwo->did_opponent_win())
    {
        simulation_stats.num_EngineOne_wins ++;

        simulation_stats.average_number_moves_in_games_EngineOne_won += num_moves_played;

        cout << "EngineOne won.\n";
    }

    else if (pt_EngineOne->did_opponent_win() && pt_EngineTwo->did_computer_win())
    {
        simulation_stats.num_EngineTwo_wins ++;

        simulation_stats.average_number_moves_in_games_EngineTwo_won += num_moves_played;

        cout << "EngineTwo won.\n";
    }

    else if (pt_EngineOne->is_game_drawn() && pt_EngineTwo->is_game_drawn())
    {
        simulation_stats.num_draws ++;

        simulation_stats.average_number_moves_in_games_drawn += num_moves_played;

        cout << "The game is drawn.\n";
    }

    else
    {
        throw runtime_error("Engines couldn't agree on a result for the game they played.\n");
    }
}

void write_board_to_specified_file(ofstream& fout, const vector<vector<char>>& board)
{
    fout << "\n    A   B   C   D   E   F   G\n\n";

    for (int row = 0; row <= 5; row++)
    {
        fout << (6 - row) << " | "; // Since I want row numbers to be displayed increasing from bottom up, not top down.

        for (int col = 0; col <= 6; col++)
        {
            fout << board[row][col] << " | ";
        }

        fout << "\n" << "  |---|---|---|---|---|---|---|\n";
    }
}

void print_percentage_finished(int num_finished, int num_total)
{
    double percentage = (static_cast<double>(num_finished)) / (static_cast<double>(num_total));

    double prev_percentage = (static_cast<double>(num_finished - 1)) / (static_cast<double>(num_total));

    int current_percentage_as_int = 90;

    for (double i = 0.9; i >= 0.1; i -= 0.1)
    {
        if (percentage >= i && prev_percentage < i)
        {
            cout << current_percentage_as_int << "% done.\n";

            return;
        }

        current_percentage_as_int -= 10;
    }
}

void examine_uneven_trials(const vector<UnevenTrialStats>& uneven_trials, double thinking_time,
                           int num_trials_to_run_for_uneven_matches)
{
    // NOTE - no longer maintaining this function, so results in the
    // UnevenTrialsStats.txt file could be buggy.

    // I don't care, as I don't use that file for judging the results of matches.

    ofstream fout("UnevenTrialsStats.txt");

    for (int i = 0; i < uneven_trials.size(); i++)
    {
        UnevenTrialStats current_trial = uneven_trials[i];

        int EngineOneVersion = current_trial.EngineOneVersion;
        int EngineTwoVersion = current_trial.EngineTwoVersion;

        string EngineOne_score_to_display = "";
        string EngineTwo_score_to_display = "";

        if (current_trial.EngineOne_wins_in_trial == 2)
        {
            EngineOne_score_to_display = "2";
            EngineTwo_score_to_display = "0";
        }

        else if (current_trial.EngineTwo_wins_in_trial == 2)
        {
            EngineOne_score_to_display = "0";
            EngineTwo_score_to_display = "2";
        }

        else if (current_trial.EngineOne_wins_in_trial == 0 && current_trial.EngineTwo_wins_in_trial == 1)
        {
            EngineOne_score_to_display = "0.5";
            EngineTwo_score_to_display = "1.5";
        }

        else if (current_trial.EngineOne_wins_in_trial == 1 && current_trial.EngineTwo_wins_in_trial == 0)
        {
            EngineOne_score_to_display = "1.5";
            EngineTwo_score_to_display = "0.5";
        }

        else
        {
            EngineOne_score_to_display = "Something went wrong...";
            EngineTwo_score_to_display = "Something went wrong...";
        }

        fout << "Version " << EngineOneVersion << " scored " << EngineOne_score_to_display << " points in the trial.\n";
        fout << "Version " << EngineTwoVersion << " scored " << EngineTwo_score_to_display << " points in the trial.\n";

        fout << "\nAverage rate Version " << EngineOneVersion << " thought in the actual trial: ";
        fout << current_trial.rate_EngineOne_thought_in_trial << " seconds/move.\n";

        fout << "\nAverage rate Version " << EngineTwoVersion << " thought in the actual trial: ";
        fout << current_trial.rate_EngineTwo_thought_in_trial << " seconds/move.\n";

        vector<vector<char>> board = return_board_with_starting_moves_played(current_trial.set_of_moves_for_starting_position);

        fout << "Starting board:\n\n";

        write_board_to_specified_file(fout, board);

        fout << "\n\n";

        fout << "Ending board #1:\n";

        write_board_to_specified_file(fout, current_trial.ending_boards[0]);

        fout << "\nEnding board #2:\n";

        write_board_to_specified_file(fout, current_trial.ending_boards[1]);

        fout << "\n\n";

        RunningData mini_sim_stats;
        mini_sim_stats.num_EngineOne_wins = 0;
        mini_sim_stats.num_EngineTwo_wins = 0;
        mini_sim_stats.num_draws = 0;
        mini_sim_stats.total_time_EngineOne_has_thought = duration_cast<duration<double>>(steady_clock::now() - steady_clock::now());
        mini_sim_stats.total_time_EngineTwo_has_thought = duration_cast<duration<double>>(steady_clock::now() - steady_clock::now());
        mini_sim_stats.num_instances_EngineOne_thought = 0;
        mini_sim_stats.num_instances_EngineTwo_thought = 0;

        for (int i = 1; i <= num_trials_to_run_for_uneven_matches; i++)
        {
            playout_game(mini_sim_stats, current_trial.set_of_moves_for_starting_position, i % 2 == 0, EngineOneVersion,
                         EngineTwoVersion, thinking_time);

            playout_game(mini_sim_stats, current_trial.set_of_moves_for_starting_position, i % 2 != 0, EngineOneVersion,
                         EngineTwoVersion, thinking_time);
        }

        fout << "\nAfter running " << num_trials_to_run_for_uneven_matches << " checking trials:\n";
        fout << "Version " << EngineOneVersion << " won " << mini_sim_stats.num_EngineOne_wins << " games.\n";
        fout << "Version " << EngineTwoVersion << " won " << mini_sim_stats.num_EngineTwo_wins << " games.\n";
        fout << "There were " << mini_sim_stats.num_draws << " draws.\n";

        double thinking_rate_EngineOne = mini_sim_stats.total_time_EngineOne_has_thought.count();
        thinking_rate_EngineOne /= static_cast<double>(mini_sim_stats.num_instances_EngineOne_thought);

        double thinking_rate_EngineTwo = mini_sim_stats.total_time_EngineTwo_has_thought.count();
        thinking_rate_EngineTwo /= static_cast<double>(mini_sim_stats.num_instances_EngineTwo_thought);

        fout << "Version " << EngineOneVersion << " thought at a rate of " << thinking_rate_EngineOne << " seconds/move ";
        fout << "in these checking trials.\n";

        fout << "Version " << EngineTwoVersion << " thought at a rate of " << thinking_rate_EngineTwo << " seconds/move ";
        fout << "in these checking trials.\n";

        fout << "\n6 Ending boards from these checking trials:\n";

        for (int i = 0; i < mini_sim_stats.all_ending_boards.size(); i++)
        {
            write_board_to_specified_file(fout, mini_sim_stats.all_ending_boards[i]);

            fout << "\n";
        }

        fout << "\n\n*-------------------------*\n\n";

        print_percentage_finished((i + 1), uneven_trials.size());
    }
}

int main()
{
    // Ready to use tool pointers, with all methods in position classes ready for accessing static methods too!

    srand(time(NULL)); // Should do nothing now, since rand() isn't used in the program anymore.

    ofstream fout("Simulation_Data.txt");

    if (fout.fail())
    {
        throw runtime_error("fout failed to make a file.\n");
    }

    int num_trials = 0;

    double thinking_time = 0.0;

    int num_checking_trials_to_run_for_uneven_trials;

    cout << "Enter the number of trials/starting positions to play out in each \"mini-match\": ";

    cin >> num_trials;

    cout << "Enter the thinking time for the Engines: ";

    cin >> thinking_time;

    cout << "Enter how many trials to run for checking uneven trials: ";

    cin >> num_checking_trials_to_run_for_uneven_trials;

    vector<Engine_numbers_for_a_match> matchups; // Each element stores the version numbers (as ints) for the two engines to
                                                 // play in a mini-match.

    for (int i = 1; ; i++)
    {
        Engine_numbers_for_a_match temp;

        cout << "Enter the first Engine's version for mini-match #" << i << ", or enter a negative number to stop: ";

        cin >> temp.first_Engine_version;

        if (temp.first_Engine_version < 0)
        {
            break;
        }

        cout << "Enter the second Engine's version for mini-match #" << i << ": ";

        cin >> temp.second_Engine_version;

        matchups.push_back(temp);
    }

    vector<vector<coordinate>> all_starting_sets_of_moves = record_all_sets_of_possible_starting_moves(8);

    cout << all_starting_sets_of_moves.size() << " different starting sets of moves after removing duplicates.\n";

    make_it_fair_for_smaller_sets(all_starting_sets_of_moves, 8);

    vector<UnevenTrialStats> uneven_trials;

    for (const Engine_numbers_for_a_match& current_match_players: matchups)
    {
        RunningData simulation_stats;
        simulation_stats.num_draws = 0;
        simulation_stats.num_EngineOne_wins = 0;
        simulation_stats.num_EngineTwo_wins = 0;
        simulation_stats.total_time_EngineOne_has_thought = duration_cast<duration<double>>(steady_clock::now() - steady_clock::now());
        simulation_stats.total_time_EngineTwo_has_thought = duration_cast<duration<double>>(steady_clock::now() - steady_clock::now());
        simulation_stats.num_instances_EngineOne_thought = 0;
        simulation_stats.num_instances_EngineTwo_thought = 0;
        simulation_stats.num_level_trials = 0;
        simulation_stats.num_trials_favouring_EngineOne = 0;
        simulation_stats.num_trials_favouring_EngineTwo = 0;
        simulation_stats.time_EngineOne_spent_in_find_best_move = duration_cast<duration<double>>(steady_clock::now() - steady_clock::now());
        simulation_stats.time_EngineTwo_spent_in_find_best_move = duration_cast<duration<double>>(steady_clock::now() - steady_clock::now());
        simulation_stats.num_instances_EngineOne_used_find_best_move = 0;
        simulation_stats.num_instances_EngineTwo_used_find_best_move = 0;
        simulation_stats.average_number_moves_in_games_EngineOne_won = 0.0;
        simulation_stats.average_number_moves_in_games_EngineTwo_won = 0.0;
        simulation_stats.average_number_moves_in_games_drawn = 0.0;
        simulation_stats.max_time_EngineOne_spent_at_once_in_find_best_move = duration_cast<duration<double>>(steady_clock::now() - steady_clock::now());
        simulation_stats.max_time_EngineTwo_spent_at_once_in_find_best_move = duration_cast<duration<double>>(steady_clock::now() - steady_clock::now());
        simulation_stats.time_EngineOne_thought_in_current_trial = 0.0;
        simulation_stats.time_EngineTwo_thought_in_current_trial = 0.0;

        simulation_stats.max_time_EngineOne_spent_on_zero_think = 0.0;
        simulation_stats.max_time_EngineTwo_spent_on_zero_think = 0.0;
        simulation_stats.num_EngineOne_zero_thinks = 0;
        simulation_stats.num_EngineTwo_zero_thinks = 0;
        simulation_stats.total_time_EngineOne_spent_on_zero_thinks = 0.0;
        simulation_stats.total_time_EngineTwo_spent_on_zero_thinks = 0.0;

        int current_index_in_sets = 0; // Stores the index of the current set being examined in all_starting_sets_of_moves.

        for (int i = 1; i <= num_trials; i++)
        {
            simulation_stats.time_EngineOne_thought_in_current_trial = 0.0;
            simulation_stats.time_EngineTwo_thought_in_current_trial = 0.0;

            while (!fair(all_starting_sets_of_moves[current_index_in_sets], current_match_players.first_Engine_version,
                         current_match_players.second_Engine_version, thinking_time))
            {
                current_index_in_sets ++;

                if (current_index_in_sets >= all_starting_sets_of_moves.size())
                {
                    throw runtime_error("Ran out of starting positions!\n");
                }
            }

            // Get the Engines to play out starting_sets_of_moves[current_index_in_sets],
            // as both sides (2 games).

            RunningData old_stats = simulation_stats;

            playout_game(simulation_stats, all_starting_sets_of_moves[current_index_in_sets], i % 2 == 0,
                         current_match_players.first_Engine_version, current_match_players.second_Engine_version, thinking_time);

            playout_game(simulation_stats, all_starting_sets_of_moves[current_index_in_sets], i % 2 != 0,
                         current_match_players.first_Engine_version, current_match_players.second_Engine_version, thinking_time);

            cout << "Up to this point, Version " << current_match_players.first_Engine_version << " has ";
            cout << simulation_stats.num_EngineOne_wins << " wins, ";
            cout << "Version " << current_match_players.second_Engine_version << " has ";
            cout << simulation_stats.num_EngineTwo_wins << " wins, and ";
            cout << "there have been " << simulation_stats.num_draws << " draws.\n\n";

            double thinking_rate_EngineOne = simulation_stats.total_time_EngineOne_has_thought.count();

            thinking_rate_EngineOne /= static_cast<double>(simulation_stats.num_instances_EngineOne_thought);

            cout << "Version " << current_match_players.first_Engine_version << " is thinking at an average rate of ";
            cout << thinking_rate_EngineOne << " seconds/move.\n";

            double zero_think_EngineOne_rate = simulation_stats.total_time_EngineOne_spent_on_zero_thinks;

            zero_think_EngineOne_rate /= double(simulation_stats.num_EngineOne_zero_thinks);

            cout << "In zero thinks, it's thinking at an average rate of ";
            cout << zero_think_EngineOne_rate << " seconds/move, and the max amount of time spent in a zero think has been ";
            cout << simulation_stats.max_time_EngineOne_spent_on_zero_think << " seconds.\n\n";

            double thinking_rate_EngineTwo = simulation_stats.total_time_EngineTwo_has_thought.count();

            thinking_rate_EngineTwo /= static_cast<double>(simulation_stats.num_instances_EngineTwo_thought);

            cout << "Version " << current_match_players.second_Engine_version << " is thinking at an average rate of ";
            cout << thinking_rate_EngineTwo << " seconds/move.\n";

            double zero_think_EngineTwo_rate = simulation_stats.total_time_EngineTwo_spent_on_zero_thinks;

            zero_think_EngineTwo_rate /= double(simulation_stats.num_EngineTwo_zero_thinks);

            cout << "In zero thinks, it's thinking at an average rate of ";
            cout << zero_think_EngineTwo_rate << " seconds/move, and the max amount of time spent in a zero think has been ";
            cout << simulation_stats.max_time_EngineTwo_spent_on_zero_think << " seconds.\n\n";

            double best_move_function_EngineOne_thinking_rate = simulation_stats.time_EngineOne_spent_in_find_best_move.count();

            best_move_function_EngineOne_thinking_rate /= static_cast<double>(simulation_stats.num_instances_EngineOne_used_find_best_move);

            cout << "In the find_best_move() function, ";
            cout << "Version " << current_match_players.first_Engine_version << " is thinking at an average rate of ";
            cout << best_move_function_EngineOne_thinking_rate << " seconds/move.\n";
            cout << "The total time, up until now, spent thinking in find_best_move() is ";
            cout << simulation_stats.time_EngineOne_spent_in_find_best_move.count() << " seconds.\n";
            cout << "The max amount of time spent thinking in ONE call of find_best_move() is ";
            cout << simulation_stats.max_time_EngineOne_spent_at_once_in_find_best_move.count() << "\n\n";

            double best_move_function_EngineTwo_thinking_rate = simulation_stats.time_EngineTwo_spent_in_find_best_move.count();

            best_move_function_EngineTwo_thinking_rate /= static_cast<double>(simulation_stats.num_instances_EngineTwo_used_find_best_move);

            cout << "In the find_best_move() function, ";
            cout << "Version " << current_match_players.second_Engine_version << " is thinking at an average rate of ";
            cout << best_move_function_EngineTwo_thinking_rate << " seconds/move.\n";
            cout << "The total time, up until now, spent thinking in find_best_move() is ";
            cout << simulation_stats.time_EngineTwo_spent_in_find_best_move.count() << " seconds.\n";
            cout << "The max amount of time spent thinking in ONE call of find_best_move() is ";
            cout << simulation_stats.max_time_EngineTwo_spent_at_once_in_find_best_move.count() << "\n";

            int num_EngineOne_wins_from_trial = simulation_stats.num_EngineOne_wins - old_stats.num_EngineOne_wins;
            int num_EngineTwo_wins_from_trial = simulation_stats.num_EngineTwo_wins - old_stats.num_EngineTwo_wins;
            int indicator = num_EngineOne_wins_from_trial - num_EngineTwo_wins_from_trial;

            vector<vector<vector<char>>> ending_boards_from_this_trial;
            ending_boards_from_this_trial.push_back(simulation_stats.all_ending_boards[simulation_stats.all_ending_boards.size()-2]);
            ending_boards_from_this_trial.push_back(simulation_stats.all_ending_boards[simulation_stats.all_ending_boards.size()-1]);

            if (indicator != 0)
            {
                uneven_trials.push_back({num_EngineOne_wins_from_trial, num_EngineTwo_wins_from_trial,
                     all_starting_sets_of_moves[current_index_in_sets],
                     current_match_players.first_Engine_version, current_match_players.second_Engine_version,
                     simulation_stats.time_EngineOne_thought_in_current_trial / static_cast<double>
                     (simulation_stats.num_instances_EngineOne_thought - old_stats.num_instances_EngineOne_thought),
                     simulation_stats.time_EngineTwo_thought_in_current_trial / static_cast<double>
                     (simulation_stats.num_instances_EngineTwo_thought - old_stats.num_instances_EngineTwo_thought),
                      ending_boards_from_this_trial});

                if (indicator > 0)
                {
                    simulation_stats.num_trials_favouring_EngineOne ++;
                }

                else
                {
                    simulation_stats.num_trials_favouring_EngineTwo ++;
                }
            }

            else
            {
                simulation_stats.num_level_trials ++;
            }

            current_index_in_sets ++;
        }

        simulation_stats.average_number_moves_in_games_EngineOne_won /= static_cast<double>(simulation_stats.num_EngineOne_wins);

        simulation_stats.average_number_moves_in_games_EngineTwo_won /= static_cast<double>(simulation_stats.num_EngineTwo_wins);

        simulation_stats.average_number_moves_in_games_drawn /= static_cast<double>(simulation_stats.num_draws);

        fout << "\n\n\n\nResults of the match between Version " << current_match_players.first_Engine_version;
        fout << " and Version " << current_match_players.second_Engine_version << ":\n\n";

        fout << "Version " << current_match_players.first_Engine_version << " won ";
        fout << simulation_stats.num_EngineOne_wins << " games.\n";

        fout << "Version " << current_match_players.second_Engine_version << " won ";
        fout << simulation_stats.num_EngineTwo_wins << " games.\n";

        fout << "There were " << simulation_stats.num_draws << " draws.\n";

        fout << "There were " << simulation_stats.num_trials_favouring_EngineOne << " trials where Version ";
        fout << current_match_players.first_Engine_version << " scored 2-0 or 1.5-0.5\n";

        fout << "There were " << simulation_stats.num_trials_favouring_EngineTwo << " trials where Version ";
        fout << current_match_players.second_Engine_version << " scored 2-0 or 1.5-0.5\n";

        fout << "There were " << simulation_stats.num_level_trials << " trials where the score was equal (1-1)\n";

        double thinking_rate_EngineOne = simulation_stats.total_time_EngineOne_has_thought.count();

        thinking_rate_EngineOne /= static_cast<double>(simulation_stats.num_instances_EngineOne_thought);

        fout << "Version " << current_match_players.first_Engine_version << " spent ";
        fout << simulation_stats.total_time_EngineOne_has_thought.count() << " seconds thinking overall, and ";
        fout << "spent " << thinking_rate_EngineOne << " seconds per instance, on average.\n";
        fout << "Version " << current_match_players.first_Engine_version << " had ";
        fout << simulation_stats.num_instances_EngineOne_thought << " such instances.\n";

        double thinking_rate_EngineTwo = simulation_stats.total_time_EngineTwo_has_thought.count();

        thinking_rate_EngineTwo /= static_cast<double>(simulation_stats.num_instances_EngineTwo_thought);

        fout << "Version " << current_match_players.second_Engine_version << " spent ";
        fout << simulation_stats.total_time_EngineTwo_has_thought.count() << " seconds thinking overall, and ";
        fout << "spent " << thinking_rate_EngineTwo << " seconds per instance, on average.\n";
        fout << "Version " << current_match_players.second_Engine_version << " had ";
        fout << simulation_stats.num_instances_EngineTwo_thought << " such instances.\n";

        fout << "The average number of moves played (by both sides) in games where Version ";
        fout << current_match_players.first_Engine_version << " won was ";
        fout << simulation_stats.average_number_moves_in_games_EngineOne_won << "\n";

        fout << "The average number of moves played (by both sides) in games where Version ";
        fout << current_match_players.second_Engine_version << " won was ";
        fout << simulation_stats.average_number_moves_in_games_EngineTwo_won << "\n";

        fout << "The average number of moves played (by both sides) in games drawn ";
        fout << "was " << simulation_stats.average_number_moves_in_games_drawn << "\n";

        fout << "The max amount of time Version " << current_match_players.first_Engine_version << " spent in ONE call ";
        fout << "to find_best_move() was " << simulation_stats.max_time_EngineOne_spent_at_once_in_find_best_move.count();
        fout << " seconds.\n";

        fout << "The max amount of time Version " << current_match_players.second_Engine_version << " spent in ONE call ";
        fout << "to find_best_move() was " << simulation_stats.max_time_EngineTwo_spent_at_once_in_find_best_move.count();
        fout << " seconds.\n";
    }

    // Now to examine the unven trials, since everything has been written to the file for this trial:

    examine_uneven_trials(uneven_trials, thinking_time, num_checking_trials_to_run_for_uneven_trials);
}

