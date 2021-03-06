# Vector/List/Set performance for positional delete and unique insertion

Results are HTML graphs. Open with your favorite web browser, or compile/run the code.

Three macros control the tests that get run:
   - `VLM_SIZE` => define to an integer. Default 1,000,000 (warning: takes a long time)
   - `VLM_INSERTION` => define to test unique insertion time
   - `VLM_DELETION` => define to test positional delete

Example compilation command:
   - `g++ -std=c++14 -I"vendor/" -I"vendor\nonius\deps\nonius.deps\include" -O3 -o vlm_performance -DVLM_SIZE=1000 -DVLM_INSERTION`

You can then pass arguments to the executable.
   - `-s` => number of samples to generate (should be small for high VLM_SIZE)
   - `-r` => reporter type. Takes html, junit, or csv
   - `-o` => output file
   - For more ways to call the compiled executable, see: https://nonius.io/ 
	 
Example run command:
   - `main.cpp ./vlm_performance -s 10 -r html -o "vlm_performance - x64 Bench.html"`
