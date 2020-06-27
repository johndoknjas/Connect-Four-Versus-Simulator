Steps for using the Versus Simulator to test a proposed new version of the engine:

1) Have Database_C downloaded somewhere on your computer: https://drive.google.com/file/d/1f0q-X5edDGx_Rh-TuJIHSRoRlc2R1WED/view?usp=sharing. Update the directory.txt file with the absolute path to it.

2) It's fine to delete bin/Debug and obj/Debug.

3) Use the "EngineVersion44.h" and "EngineVersion45.h" files for copying the old and new code for the engine's position.h file.

4) In EngineVersion44.h, select everything from "class position : public tool" to the very end, excluding the ending bracket for the namespace, and delete it. Replace it with the class in the old position.h of the engine you were working on.

    4a) Find the two lines with auto rng... and shuffle. Comment them out.
  
    4b) Go to the implementation of the "think_on_game_position" function (pick the one with more parameters). Set max_depth_limit to 9             instead of UNDEFINED. This will ensure the engine thinks according to a deterministic depth_limit, instead of a time limit.

Repeat step 4 for the new source code of position.h with your changes. Use "EngineVersion45.h".

5) Compile all the files and run main.cpp.  

    5a) Enter 500 for the number of trials.
  
    5b) For the thinking time, just enter 1.0. The engines will use a depth limit so they won't actually be aiming for 1 second to think.
  
    5c) For the number of trials to check uneven trials, enter 0.
  
    5d) For the first engine's version, choose 44. Choose 45 for the second one.
  
    5e) Enter -1 for the line asking for engines in a second match.
 
6) At this point the engine will finish after a number of hours. The results will be available in Simulation_Data.txt.
