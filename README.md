# distributed-monitor

This is simple implementation of distributed mutex (Ricart–Agrawala algorithm) without failure detection.

Solution provides distributed_mutex class with lock/unlock methods and
process_monitor class (runs in signle std::thread), which controls Open MPI communication for the implementation of algorithm.

See test directory. 
