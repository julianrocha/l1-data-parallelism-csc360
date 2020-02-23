# l1-data-parallelism-csc360
Brute force C implementation of the L1 data fitting algorithm. Comparing and contrasting the performance of a single threaded implementation vs. multithreaded.

In order to run, run make to create dataPar executable. Running ./dataPar will run the test harness found in main, which compute the L1 line for both canadian_cpi_time_series.csv and stremflow_time_series.csv using various number of data points and comparing the results from single threaded and multi-threaded implementations of L1. 
