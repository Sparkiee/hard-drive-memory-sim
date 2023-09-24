# Hard Drive Simulator
**Author:** Maxim Shteingard  

---

## Description
The Hard Disk Simulator is developed in C++ and emulates a basic model of a computer's hard disk memory system. This simulator provides a foundational insight into the operations of hard disk memory management using various data structures.

### Data Structures:

- **BitVector**: Utilized for monitoring the status of each block on the disk as free or occupied.
- **Inode**: Represents information about a file, which includes its name, size, and disk location.
- **MainDir**: A directory system that lists file names and their associated inode numbers.
- **OpenFileDescriptor**: Manages open and unused file descriptors.

### Operations:

- **Create file**: Establishes a new file with a designated name and allocates disk space.
- **Delete file**: Removes the chosen file and releases the associated disk space.
- **Read file**: Extracts data from a chosen file starting from a defined offset.
- **Write file**: Instills data into the chosen file starting from a defined offset.
- **List files**: Showcases all files present on the disk.
- **Format disk**: Initializes the disk, eradicating all pre-existing files.

---

## Features

- **Disk Simulation**: Produces a virtual disk with a predetermined number of blocks.
- **File Operations**: Facilitates file creation, deletion, renaming, reading, and writing.
- **BitVector Tracking**: Employs a bit vector to oversee the status of every block on the disk.
- **Directory Structure**: Instates a rudimentary directory framework to archive file names and inode numbers.

---

## Output
Showcases the hard drive's textual representation and the list of present files.

---

## Compilation & Execution
To compile and run the simulator:

\```bash
g++ -g hdd-memory-sim.cpp -o a.out
./a.out
\```

---
