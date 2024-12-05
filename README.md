# Building H2O

## Project Overview
The **Building H2O** project simulates the creation of water molecules from hydrogen and oxygen atoms. The program consists of three types of processes: the main process, oxygen processes, and hydrogen processes. Hydrogen and oxygen atoms are organized into queues, and they pair up to form water molecules. The program uses semaphores and shared memory to synchronize the processes.

The project is implemented in C and uses process synchronization to ensure that:
1. A molecule can only be created when one oxygen atom and two hydrogen atoms are available.
2. The program correctly handles synchronization and process creation/termination.



## Features
- **Synchronization with Semaphores**: The program synchronizes hydrogen and oxygen processes using semaphores to ensure that molecules are created sequentially and correctly.
- **Shared Memory**: The program uses shared memory to track action numbers, atom states, and synchronization.
- **Output Logging**: The program logs the actions of each process to an output file `proj2.out`.
- **Process Management**: The program correctly manages the creation and termination of processes based on available atoms.
  


## Compilation and Running

### Compilation
To compile the program, use the following command:
```bash
make
```
### Running the Program
Syntax
```bash
./proj2 NO NH TI TB
```
Where 
- NO: Number of oxygen atoms.
- NH: Number of hydrogen atoms.
- TI: Maximum time in milliseconds an atom waits before joining the queue (0–1000).
- TB: Maximum time in milliseconds required to create a molecule (0–1000).

## Example Usage
### Execution Command

```bash
./proj2 3 5 100 100
```
### `proj2.out` output
```bash
1: H 1: started
2: H 3: started
3: O 1: started
4: O 1: going to queue
5: H 2: started
6: H 2: going to queue
7: H 1: going to queue
8: O 3: started
9: O 3: going to queue
10: H 5: started
11: H 4: started
12: O 2: started
13: H 1: creating molecule 1
14: H 4: going to queue
15: O 1: creating molecule 1
16: H 2: creating molecule 1
17: H 5: going to queue
18: H 2: molecule 1 created
19: H 1: molecule 1 created20: O 1: molecule 1 created
21: H 3: going to queue
22: O 3: creating molecule 2
23: O 2: going to queue
24: H 4: creating molecule 2
25: H 5: creating molecule 2
26: O 3: molecule 2 created
27: H 4: molecule 2 created
28: H 5: molecule 2 created
29: H 3: not enough O or H
30: O 2: not enough H
```
