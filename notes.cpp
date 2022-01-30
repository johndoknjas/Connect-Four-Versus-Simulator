// V.33 of the Versus Sim. In main.cpp, I'm now also separately keeping track of the
// average time the engine's spend in the think_ function, when the thinking_time is 0.
    // Checked it over and the changes to main.cpp in this Versus Sim look sound.

// It turns out the average time spent, when thinking_time is 0, is actually a few 0.01 of a second.
    // But this is the case for V.41 (and I assume further back), which is before all the
    // DB stuff started happening. So not due to introducing the DB, reseting the TT, etc.

// I made this feature in order to see if the versions that reset the TT and called SELECT
// on the DB take longer on these thinks that are supposed to be short, and it turns
// out they are doing fine.

// Update: On June 19th, I also made it so that the database_communicator references
// a copy of Database_C.db (via an absolute path) stored outside of this version's folder.
// The purpose of doing this is to save memory when making a new Versus Sim, since now
// there is no Database_C.db in each one.
    // Note that nothing has to be done from here. Just don't delete the directory.txt
    // files in the starting directory, and in /bin/Debug, and it's all good.






// *************NOTE:*********** For all future uses of this Versus Sim, you'll be using Engines that think based off a time limit, not a more deterministic depth limit.
// So, consider running two simulations between two engines. The first with their depth_limits equal, and then with equal time limits.
	// However, if you make an Engine that takes longer to think, the simulation with equal depth_limits won't be fair to the old Engine. Still useful to run though.


/*

NOTE: Each mini-match will use the same starting positions, unless two particular engines disagree on a certain
      starting position.
      Essentially, every mini-match goes through from the beginning of the all_starting_sets_of_moves vector.

WHAT TO ALWAYS DO WHEN SETTING UP A NEW MATCH BETWEEN NEW ENGINES:
-------------------------------------------------------------------

- Remove any randomness by commenting out the two lines involving rng.

- In the think function with more code, set the "max_depth_limit" variable to a value
  for the depth_limit of the engine. Or, if you want to the engine to use a think_limit
  based off the value of thinking_time, set max_depth_limit to UNDEFINED.

    - This is all you have to do, thanks to what I've done to refactor the code in the
      think function.

    - Note that if the engine is under Version 43, you'll have to instead comment/uncomment
      the condition you want in the while loop, and uncomment the caveat before the while
      loop if you want a depth_limit engine. Also, this must be done for both think functions.













- Make depth limit for both engines smaller in order to make the simulation much faster?
    - This is usually a good idea if the Versions have the same depth and the change isn't related to depth.
    - For example, if both Versions had depth = 7, change to depth = 5.

- Remove randomness from the Engines in their position class (ONLY FOR THIS SIMULATION).
  To remove randomness:
    - Make the order of moves/positions the Engines consider first not random, which
      should be done by not randomizing the possible_moves vector.
    - Comment out the auto rng stuff (2 lines) in find_best_move() function for both Engines. Doesn't seem
      to affect anything, but do it anyway to ensure no randomness.


    NEXT TASK:
    ------------
            - Test Version 5 vs Version 7... PROBLEM: Version 5 uses coordinate objects in amplifying vectors, while
              Version 7 uses treasure_spot objects in the amplifying vectors (in order to look at squares joining
              an x-in-a-row with a single square). This is an issue since main.cpp keeps a running record of all the
              data, including vector<treasure_spot> amplifying vectors. Accurate info on the amplfying vectors must
              be given to main.cpp from Version 5, so that Version 7 can be fed accurate treasure_spot objects
              in the amplifying vectors.

                - Possible SOLUTION: In this simulation's main.cpp, follow the behavior of main.cpp in a normal Version project:
                    FOLLOWING ALGORITHM IS FROM ONE OF THE ENGINE'S PERSPECTIVE:
                    --------------------------------------------------------------------------------------------
                    1) Engine thinks about the position and finds the future_position with the best move.

                    2) pos is set equal to this future_position via move semantics. The future_position has
                       up-to-date amplifying vectors, since the future_position added any amplifying
                       vectors to it during the computer's minimax process.

                    3) all_data is updated with the data from new pos.
                        - Then, only flip the board of all_data (all_data SHOULD NOT have amplifying vector attributes!).

                    4) The OTHER ENGINE thinks based off all_data
                       and chooses a move, updating all_data's board and last_move (using Steps 5-3, order not a typo).

                    5) Engine 1 then finds the future_position of its pos that has all_data's board and last_move attributes
                       (after any necessary flipping of the board). Note that all_data.board and all_data.last_move can be
                       used directly (a future_position isn't really needed here to get necessary params to send to constructor).

                    6) Engine 1 sets pos equal to a new position, with future_position's board and last_move, while
                       sending pos' 4 amplifying vectors as params (since any new squares for the amplifying vectors
                       will be figured out in the 2nd constructor).

                    7) REPEAT BACK @ STEP 1.

            - TASK: Remove the Quiescence check in EngineOne (Version 9) in analyze_last_move() before calling smart_evaluation().
                    The quiescence check is just checking if the critical_moves vector's size = 0.
                    Removing this check should,
                    in theory, make EngineOne as strong as EngineTwo (Version 7). I PREDICT they will have an equal
                    match score, and no engine will win both sides of any starting position.
                        - Running the simulation now, will check results in the morning.
                          RESULTS:
                          -------------
                          - They got an equal match score, and no engine won both sides of any starting position.
                          - Each Engine won 48 games, and they drew 4.
                          - Also, the Engines always evaluated each starting position the same.
                          This indicates they are the same Engine (EngineOne/Version 9 just moves faster due to
                          optimizations on the order of the possible_moves vector).
                    DONE (put the Quiescence check back into the code after the simulation concluded).



            - TASK: Make EngineOne and EngineTwo the same engine, and then INCLUDE randomness in the 3rd constructor.
                    See if they have an equal match score in a LONG match (they SHOULD). This test is to ensure that
                    randomness in the 3rd constructor only affects the SPEED the engines find their moves in;
                    as long as there is no randomness in the 1st and 2nd constructor, the Engines should never
                    choose a different move to play in the actual game based off randomness.

            - TASK: Print all moves in each game to a file using ofstream. Then, look at the record of moves
              for games of a starting position where the weaker engine won as both sides. May give some insight
              into where the stronger engine is going wrong.
                - For example, according to my knowledge Version 9 should be beating Version 7 much more, since
                  it should be Version 7 just with Quiescence search implemented.



    - Some logic to infer conclusions from results of the simulation
      (provided a match in the simulation is sufficiently long enough, say 15-20+ games, and there is NO RANDOMNESS
       when an engine is presented with 2+ equal moves):

        1) If the Engines have exactly the same score in the match, this ==> they are identical Engines. The only other
          probable way the match could end in an equal score is if all the starting positions were extremely biased towards
          one side, and since the Engines take turns playing both sides, they each win every other game. However, my tests
          to get the Engines to "agree" on each starting position's fairness ensure most/all of the starting positions
          are NOT very biased towards one side. Thus, an equal match score ==> true equality between two Engines.

        2) If one Engine gets a far higher score than the other engine in the match, this ==> it is a much stronger
          engine. There are two other ways this could happen:

            - Extreme bias towards one side in the starting positions, but this cannot be a factor since the engines play
              both sides of each starting position.
            - Randomness the engines experience when choosing between 2+ equal moves. However, I should always have
              randomness removed in the simulation by making the possible_moves vector NOT be shuffled in the position
              class constructors for both engines (JUST FOR THIS SIMULATION).

        3) If the score is close, like 7-5, then it is likely the Engine with the higher score is slightly better...
            - The starting positions cannot be a factor since the Engines play both sides of the starting positions.
            - Randomness between choosing from 2+ equal moves cannot be a factor since you should have that removed.
            - It's possible the Engine that won the match just plays better in one more of the few selected positions,
              and overall isn't a better Engine. How you interpret this is up to you...


    - After running 30 trials, it takes (on average) 26.666666 seconds for the two Engines (Version 6 & 7) to
      agree on an equal position to play (between 25 and -25 evaluation of board). If they then take around
      60 seconds to play a game (30 moves * 2 seconds per move), each game will take 86.6666 seconds. If the simulation
      does 20 games, 86.6666 seconds * 20 isn't too bad (around half an hour).

    - CommonNonClassData.h stores the structs and independent functions (not part of position class) used by
      EngineOne.h (namespace EngineOne) and EngineTwo.h (namespace EngineTwo).
      This is to allow equality assignments between, for example, a vector<coordinate> variable from an EngineOne position object
      and a vector<coordinate> variable from an EngineTwo position object.

            - Whenever copying a new version's code into EngineOne.h or EngineTwo.h, follow the form of what's there now!
              Just copy the class data in. If the independent non-class data in your new version has not been
              updated, don't copy it in. Just continue to use CommonNonClassData.h (it is #included in EngineOne.h and
              EngineTwo.h and main.cpp).







*/
