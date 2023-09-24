#include <iostream>
#include <vector>
#include <map>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <algorithm>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

#define DISK_SIZE 512
#define BLOCK_SIZE 4

// Function to convert decimal to binary char
char decToBinary(int n) {
    return static_cast<char>(n);
}

// #define SYS_CALL
// ============================================================================
class fsInode {
private:
    int fileSize;
    int block_in_use;

    int directBlock1;
    int directBlock2;
    int directBlock3;

    int singleInDirect;
    int doubleInDirect;
    int block_size;


public:
    // Constructor
    fsInode(int _block_size) :
            fileSize(0),
            block_in_use(0),
            block_size(_block_size),
            directBlock1(-1),
            directBlock2(-1),
            directBlock3(-1),
            singleInDirect(-1),
            doubleInDirect(-1) {}


    // Destructor
    ~fsInode() {
        // Currently empty, but you can add cleanup code if necessary
    }

    // Getter functions
    int getFileSize() const {
        if (this != nullptr) {
            return fileSize;
        } else {
            return 0;
        }
    }

    int getBlockInUse() const {
        return block_in_use;
    }

    int getBlockSize() const {
        return block_size;
    }

    int getDirectBlock1() const {
        return directBlock1;
    }

    int getDirectBlock2() const {
        return directBlock2;
    }

    int getDirectBlock3() const {
        return directBlock3;
    }

    int getSingleInDirect() const {
        return singleInDirect;
    }

    int getDoubleInDirect() const {
        return doubleInDirect;
    }

    // Setter functions
    void setFileSize(int _fileSize) {
        fileSize = _fileSize;
    }

    void setBlockInUse(int _block_in_use) {
        block_in_use = _block_in_use;
    }

    void setDirectBlock1(int block) {
        directBlock1 = block;
    }

    void setDirectBlock2(int block) {
        directBlock2 = block;
    }

    void setDirectBlock3(int block) {
        directBlock3 = block;
    }

    void setSingleInDirect(int block) {
        singleInDirect = block;
    }

    void setDoubleInDirect(int block) {
        doubleInDirect = block;
    }

    void setDirectBlock(int index, int blockValue) {
        switch (index) {
            case 0:
                directBlock1 = blockValue;
                break;
            case 1:
                directBlock2 = blockValue;
                break;
            case 2:
                directBlock3 = blockValue;
                break;
            default: // this should never trigger
                cout << "Error: Invalid direct block index." << endl;
                break;
        }
    }

};


// ============================================================================
class FileDescriptor {
    pair<string, fsInode*> file;
    bool inUse;

    public:
    FileDescriptor(string FileName, fsInode* fsi) {
        file.first = FileName;
        file.second = fsi;
        inUse = true;

    }

    string getFileName() {
        return file.first;
    }

    void setFileName(string fileName) {
        file.first = fileName;
    }

    fsInode* getInode() {

        return file.second;

    }

    int GetFileSize() {
        return file.second->getFileSize();
    }

    bool isInUse() {
        return (inUse);
    }

    void setInUse(bool _inUse) {
        inUse = _inUse;
    }
};

#define DISK_SIM_FILE "DISK_SIM_FILE.txt"
// ============================================================================
class fsDisk {
    FILE *sim_disk_fd;

    bool is_formated = false;

	// BitVector - "bit" (int) vector, indicate which block in the disk is free
	//              or not.  (i.e. if BitVector[0] == 1 , means that the
	//             first block is occupied.
    int BitVectorSize;
    int *BitVector;

    // Unix directories are lists of association structures,
    // each of which contains one filename and one inode number.
    map<string, fsInode*>  MainDir ;

    // OpenFileDescriptors --  when you open a file,
	// the operating system creates an entry to represent that file
    // This entry number is the file descriptor.
    vector< FileDescriptor > OpenFileDescriptors;

    int block_size;

    public:






    // ------------------------------------------------------------------------
    // allocates a free block in the memory array
    int allocateMemoryBlock() {
        for (int i = 0; i < BitVectorSize; i++) {
            if (BitVector[i] == 0) {  // If the block is free
                BitVector[i] = 1;    // Mark it as occupied
                return i;            // Return the block number
            }
        }

        // If no free blocks are available
        cout << "ERR" << endl;
        return -1;
    }

    // ------------------------------------------------------------------------
    int getFileDescriptorForFileName(string filename) {
        for (int i = 0; i < OpenFileDescriptors.size(); i++) {
            if (OpenFileDescriptors[i].getFileName() == filename && OpenFileDescriptors[i].isInUse()) {
                return i;  // Return the FD if the file is in use and the name matches
            }
        }
        return -1;  // Return -1 if no FD is found for the file or it's not in use
    }

    // ------------------------------------------------------------------------
    // checks if the block size is even valid
    bool validBlockSize(int block_size)
    {
        if(block_size > DISK_SIZE || block_size <= 1)
        {
            return false;
        }
        return true;
    }

    // ------------------------------------------------------------------------
    // counts how many blocks are currently free
    int countFreeBlocks()
    {
        int count = 0;
        for (int i = 0; i < BitVectorSize; ++i) {
            if(BitVector[i] == 0)
                count++;
        }
        return count;
    }

    // ------------------------------------------------------------------------
    fsDisk() {
        sim_disk_fd = fopen( DISK_SIM_FILE , "w+" );
        assert(sim_disk_fd);
        for (int i=0; i < DISK_SIZE ; i++) {
            int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
            ret_val = fwrite( "\0" ,  1 , 1, sim_disk_fd );
            assert(ret_val == 1);
        }
        fflush(sim_disk_fd);
    }

    // ------------------------------------------------------------------------
    void listAll() {
        int i = 0;
        for ( auto it = begin (OpenFileDescriptors); it != end (OpenFileDescriptors); ++it) {
            cout << "index: " << i << ": FileName: " << it->getFileName() <<  " , isInUse: "
                << it->isInUse() << " file Size: " << it->GetFileSize() << endl;
            i++;
        }
        char bufy;
        cout << "Disk content: '" ;
        for (i=0; i < DISK_SIZE ; i++) {
            int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
            ret_val = fread(  &bufy , 1 , 1, sim_disk_fd );
             cout << bufy;
        }
        cout << "'" << endl;
    }

    // ------------------------------------------------------------------------
    void fsFormat(int blockSize = 4) {
        // Reset the disk, insert \0
        fseek(sim_disk_fd, 0, SEEK_SET); // Go to the beginning of the file
        for (int i=0; i < DISK_SIZE; i++) {
            fwrite("\0", 1, 1, sim_disk_fd); // Write null bytes to the entire disk
        }
        fflush(sim_disk_fd); // Ensure all data is written to the file

        // Initialize BitVector
        BitVectorSize = DISK_SIZE / blockSize;  // Number of blocks in the disk

        BitVector = new int[BitVectorSize];
        memset(BitVector, 0, sizeof(int) * BitVectorSize);  // Initially, all blocks are free

        // 3. Clear Main Directory
        for (auto& entry : MainDir) {
            delete[] entry.second;  // Free the inodes
        }
        MainDir.clear();  // Remove all entries

        // Close all open files
        OpenFileDescriptors.clear();

        // invalid block size and therefore doesn't format the disk
        if(!validBlockSize(blockSize)) {
            cout << "ERR" << endl;
            is_formated = false;
        }
        else
        {
            // set disk as formatted
            is_formated = true;
            this->block_size = blockSize;
        }

    }

    // ------------------------------------------------------------------------
    int CreateFile(string fileName) {
        if(!this->is_formated)
        {
            // trying to create a file on a not formatted disk
            cout << "ERR" << endl;
            return -1;
        }

        // file name already exists
        if(MainDir.find(fileName) != MainDir.end()) {
            cout << "ERR" << endl;
            return -1;
        }

        // creating new inode
        fsInode* newInode = new fsInode(this->block_size);

        // assigning it to file & creating new fd
        MainDir[fileName] = newInode;
        FileDescriptor newFD(fileName, newInode);
        int index = -1;
        for ( auto it = begin (OpenFileDescriptors); it != end (OpenFileDescriptors); ++it) {
            if(!it->isInUse())
            {
                index = it - begin(OpenFileDescriptors);
                OpenFileDescriptors[index] = newFD;
                break;
            }
        }
        if(index == -1) {
            OpenFileDescriptors.push_back(newFD);
            index = OpenFileDescriptors.size()-1;
        }
        return index;  // Return the file descriptor (index in the vector).
    }

    // ------------------------------------------------------------------------
    int OpenFile(string FileName ) {
        // making sure disk is formatted
        if(!is_formated)
        {
            cout << "ERR" << endl;
            return -1;
        }

        // Get the inode for the file.
        fsInode* inode = MainDir[FileName];

        // Check if the file is already open.
        for (auto& fd : OpenFileDescriptors) {
            if (fd.getFileName() == FileName && fd.isInUse()) {
                // File is already open.
                cout << "ERR" << endl;
                return -1;
            }
        }

        // creating a fd
        FileDescriptor newFD(FileName, inode);
        int index = -1;
        for ( auto it = begin (OpenFileDescriptors); it != end (OpenFileDescriptors); ++it) {
            if(!it->isInUse())
            {
                index = it - begin(OpenFileDescriptors);
                OpenFileDescriptors[index] = newFD;
                break;
            }
        }
        if(index == -1) {
            OpenFileDescriptors.push_back(newFD);
            index = OpenFileDescriptors.size()-1;
        }
        return index;
    }

    // ------------------------------------------------------------------------
    string CloseFile(int fd) {
        // making sure disk is formatted
        if(!is_formated)
        {
            cout << "ERR" << endl;
            return "-1";
        }

        if (fd < 0 || fd >= OpenFileDescriptors.size()) {
            cout << "ERR" << endl;
            return "-1";  // Return an empty string or another error indicator
        }

        // Check if the FD is marked as in use
        if (!OpenFileDescriptors[fd].isInUse()) {
            cout << "ERR" << endl;
            return "-1";  // Return an empty string or another error indicator
        }

        // Check if the file name associated with this FD exists in the MainDir
        string fileName = OpenFileDescriptors[fd].getFileName();
        if (MainDir.find(fileName) == MainDir.end()) {
            cout << "ERR" << endl;
            return "-1";  // Return an empty string or another error indicator
        }

        OpenFileDescriptors[fd].setInUse(false);

        return "1";
    }

    // ------------------------------------------------------------------------

    int WriteToFile(int fd, char *buf, int len) {
        // making sure disk is formatted
        if(!is_formated)
        {
            cout << "ERR" << endl;
            return -1;
        }

        // checking if fd is valid and open
        if (fd < 0 || fd >= OpenFileDescriptors.size() || !OpenFileDescriptors[fd].isInUse()) {
            cout << "ERR" << endl;
            return -1;
        }

        // getting inode
        fsInode *inode = OpenFileDescriptors[fd].getInode();
        int currentPosition = inode->getFileSize();
        int bytesWritten = 0;
        int bytesLeft = len;

        // calculating the blocks needed
        int maxBlocks = 3 + block_size + block_size*block_size;
        int maxFile = maxBlocks * block_size;
        // checking if there are enough blocks
        int freeBlocks = countFreeBlocks();

        // total length exceeds limit
        if(inode->getFileSize() == maxFile && bytesLeft > 0)
        {
            cout << "ERR" << endl;
            return -1;
        }

        // there are stuff to write but no space in disk
        if(freeBlocks == 0 && bytesLeft > 0)
        {
            cout << "ERR" << endl;
            return -1;
        }

        if(inode->getBlockInUse() == maxBlocks && bytesLeft > 0)
        {
            cout << "ERR" << endl;
            return -1;
        }

        // Direct Block Handling
        int directBlocks[3] = { inode->getDirectBlock1(), inode->getDirectBlock2(), inode->getDirectBlock3() };

        // the actual writing
        // direct block
        while (bytesLeft > 0 && currentPosition < 3 * block_size) {
            int currentBlockIndex = currentPosition / block_size;
            int offsetInBlock = currentPosition % block_size;

            if (directBlocks[currentBlockIndex] == -1) {
                int newBlock = allocateMemoryBlock();
                if (newBlock == -1) {
                    return -1;
                }
                inode->setBlockInUse(inode->getBlockInUse() + 1);
                directBlocks[currentBlockIndex] = newBlock;
                inode->setDirectBlock(currentBlockIndex, newBlock);
            }

            fseek(sim_disk_fd, directBlocks[currentBlockIndex] * block_size + offsetInBlock, SEEK_SET);

            int spaceInCurrentBlock = block_size - offsetInBlock;
            int sizeToWrite = min(spaceInCurrentBlock, bytesLeft);

            fwrite(buf, sizeof(char), sizeToWrite, sim_disk_fd);
            bytesLeft -= sizeToWrite;
            currentPosition += sizeToWrite;
            buf += sizeToWrite;
        }

        // Single Indirect Block Handling
        if (bytesLeft > 0) {
            // Load or initialize single indirect block
            char pointersChar[block_size];
            if (inode->getSingleInDirect() == -1) {
                int newBlock = allocateMemoryBlock();
                if (newBlock == -1) {
                    return -1;
                }
                inode->setBlockInUse(inode->getBlockInUse() + 1);
                inode->setSingleInDirect(newBlock);
                memset(pointersChar, -1, block_size);
            } else {
                fseek(sim_disk_fd, inode->getSingleInDirect() * block_size, SEEK_SET);
                fread(pointersChar, 1, block_size, sim_disk_fd);
            }

            while (bytesLeft > 0 && currentPosition < 3 * block_size + block_size * block_size) {
                int offsetInBlock = currentPosition % block_size;
                int indirectIndex = (currentPosition - 3 * block_size) / block_size;
                int indirectBlock = static_cast<int>(pointersChar[indirectIndex]);

                if (indirectBlock == (char)-1) {
                    indirectBlock = allocateMemoryBlock();
                    if (indirectBlock == -1) {
                        return -1;
                    }
                    inode->setBlockInUse(inode->getBlockInUse() + 1);
                    pointersChar[indirectIndex] = decToBinary(indirectBlock);
                }

                fseek(sim_disk_fd, indirectBlock * block_size + offsetInBlock, SEEK_SET);

                int spaceInCurrentBlock = block_size - offsetInBlock;
                int sizeToWrite = std::min(spaceInCurrentBlock, bytesLeft);

                fwrite(buf, sizeof(char), sizeToWrite, sim_disk_fd);
                bytesLeft -= sizeToWrite;
                currentPosition += sizeToWrite;
                buf += sizeToWrite;
            }

            // Save updated pointers of single indirect block back to the disk
            fseek(sim_disk_fd, inode->getSingleInDirect() * block_size, SEEK_SET);
            fwrite(pointersChar, 1, block_size, sim_disk_fd);
        }


        // Double indirect Block Handling
        if (bytesLeft > 0) {
            char doubleIndirectPointers[block_size];

            // Initialize or load the double indirect block
            if (inode->getDoubleInDirect() == -1) {
                int newBlock = allocateMemoryBlock();
                if (newBlock == -1) {
                    return -1;
                }
                inode->setBlockInUse(inode->getBlockInUse() + 1);
                inode->setDoubleInDirect(newBlock);
                memset(doubleIndirectPointers, -1, block_size);
            } else {
                fseek(sim_disk_fd, inode->getDoubleInDirect() * block_size, SEEK_SET);
                fread(doubleIndirectPointers, 1, block_size, sim_disk_fd);
            }

            int totalWrittenBeforeDbl = 3 * block_size + block_size * block_size;

            int dblIndirectIndex = (currentPosition - totalWrittenBeforeDbl) / (block_size * block_size);
            int singleIndirectIndex = ((currentPosition - totalWrittenBeforeDbl) % (block_size * block_size)) / block_size;
            int dataBlockOffset = currentPosition % block_size;

            while (bytesLeft > 0 && dblIndirectIndex < block_size) {
                char singleIndirectPointers[block_size];
                int singleIndirectBlockNum = static_cast<int>(doubleIndirectPointers[dblIndirectIndex]);

                if (singleIndirectBlockNum == (char)-1) {
                    singleIndirectBlockNum = allocateMemoryBlock();
                    if (singleIndirectBlockNum == -1) {
                       return -1;
                    }
                    inode->setBlockInUse(inode->getBlockInUse() + 1);
                    doubleIndirectPointers[dblIndirectIndex] = decToBinary(singleIndirectBlockNum);
                    memset(singleIndirectPointers, -1, block_size);
                } else {
                    fseek(sim_disk_fd, singleIndirectBlockNum * block_size, SEEK_SET);
                    fread(singleIndirectPointers, 1, block_size, sim_disk_fd);
                }

                while (bytesLeft > 0 && singleIndirectIndex < block_size) {
                    int dataBlockNum = static_cast<int>(singleIndirectPointers[singleIndirectIndex]);

                    if (dataBlockNum == (char)-1) {
                        dataBlockNum = allocateMemoryBlock();
                        if (dataBlockNum == -1) {
                            return -1;
                        }
                        inode->setBlockInUse(inode->getBlockInUse() + 1);
                        singleIndirectPointers[singleIndirectIndex] = decToBinary(dataBlockNum);
                    }

                    fseek(sim_disk_fd, dataBlockNum * block_size + dataBlockOffset, SEEK_SET);

                    int spaceInBlock = block_size - dataBlockOffset;
                    int sizeToWrite = min(spaceInBlock, bytesLeft);

                    fwrite(buf + bytesWritten, sizeof(char), sizeToWrite, sim_disk_fd);
                    bytesLeft -= sizeToWrite;
                    bytesWritten += sizeToWrite;
                    currentPosition += sizeToWrite;

                    dataBlockOffset = 0;  // Reset for subsequent blocks
                    singleIndirectIndex++;
                }

                // Save the updated pointers of the single indirect block
                fseek(sim_disk_fd, singleIndirectBlockNum * block_size, SEEK_SET);
                fwrite(singleIndirectPointers, 1, block_size, sim_disk_fd);

                singleIndirectIndex = 0; // Reset for the next set of blocks
                dblIndirectIndex++;
            }

            // Save the updated pointers of the double indirect block
            fseek(sim_disk_fd, inode->getDoubleInDirect() * block_size, SEEK_SET);
            fwrite(doubleIndirectPointers, 1, block_size, sim_disk_fd);
        }

        // Update inode's file size and other attributes
        inode->setFileSize(currentPosition);

        inode->setBlockInUse(inode->getBlockInUse() + 1);
        return 1;

    }

    // ------------------------------------------------------------------------
    int DelFile(string fileName) {
        // making sure disk is formatted
        if(!is_formated)
        {
            cout << "ERR" << endl;
            return -1;
        }
        // Check if fileName exists in MainDir
        if (MainDir.find(fileName) == MainDir.end()) {
            cout << "ERR" << endl;
            return -1;
        }

        // Ensure the file is not currently open
        for (auto& fd : OpenFileDescriptors) {
            if (fd.isInUse() && fd.getFileName() == fileName) {
                cout << "ERR" << endl;
                return -1;
            }
        }

        // Free up the blocks used by the file
        fsInode* inode = MainDir[fileName];

        if(!validBlockSize(inode->getBlockSize()))
        {
            cout << "ERR" << endl;
            return -1;
        }

        if (inode->getDirectBlock1() != -1) {
            BitVector[inode->getDirectBlock1()] = 0; // Free the block
        }
        if (inode->getDirectBlock2() != -1) {
            BitVector[inode->getDirectBlock2()] = 0; // Free the block
        }
        if (inode->getDirectBlock3() != -1) {
            BitVector[inode->getDirectBlock3()] = 0; // Free the block
        }

        // single indirect
        if (inode->getSingleInDirect() != -1) {
            char pointersChar[block_size];
            fseek(sim_disk_fd, inode->getSingleInDirect() * block_size, SEEK_SET);
            fread(pointersChar, 1, block_size, sim_disk_fd);

            for (int i = 0; i < block_size; ++i) {
                if (pointersChar[i] != (char)-1) {
                    BitVector[static_cast<int>(pointersChar[i])] = 0;
                }
            }
            BitVector[inode->getSingleInDirect()] = 0;
        }

        // double indirect blocks
        if (inode->getDoubleInDirect() != -1) {
            char doubleIndirectPointers[block_size];
            fseek(sim_disk_fd, inode->getDoubleInDirect() * block_size, SEEK_SET);
            fread(doubleIndirectPointers, 1, block_size, sim_disk_fd);

            for (int i = 0; i < block_size; ++i) {
                if (doubleIndirectPointers[i] != (char)-1) {
                    int singleIndirectBlockNum = static_cast<int>(doubleIndirectPointers[i]);
                    char singleIndirectPointers[block_size];
                    fseek(sim_disk_fd, singleIndirectBlockNum * block_size, SEEK_SET);
                    fread(singleIndirectPointers, 1, block_size, sim_disk_fd);

                    for (int j = 0; j < block_size; ++j) {
                        if (singleIndirectPointers[j] != (char)-1) {
                            BitVector[static_cast<int>(singleIndirectPointers[j])] = 0;
                        }
                    }
                    BitVector[singleIndirectBlockNum] = 0;
                }
            }
            BitVector[inode->getDoubleInDirect()] = 0;
        }

        // Delete the file's inode
        delete inode;

        // Remove the file from MainDir and from OpenFileDescriptor if it's there
        MainDir.erase(fileName);

        for (auto it = OpenFileDescriptors.begin(); it != OpenFileDescriptors.end(); /* no increment here */) {
            if (it->getFileName() == fileName) {
                //it = OpenFileDescriptors.erase(it);
                it->setInUse(false);
                it->getInode()->setFileSize(0);
                it->setFileName("");
                break;
            } else {
                ++it;
            }
        }
        return 1; // Indicate successful deletion
    }

    // ------------------------------------------------------------------------
    int ReadFromFile(int fd, char *buf, int len) {
        // making sure disk is formatted
        if(!is_formated)
        {
            cout << "ERR" << endl;
            return -1;
        }

        if (fd < 0 || fd >= OpenFileDescriptors.size() || !OpenFileDescriptors[fd].isInUse()) {
            cout << "ERR" << endl;
            return -1;
        }

        memset(buf, 0, len);  // Reset the buffer

        fsInode *inode = OpenFileDescriptors[fd].getInode();
        int currentPosition = 0;  // Starting from the beginning of the file
        int bytesRead = 0;
        int bytesLeft = len;

        if(!validBlockSize(inode->getBlockSize()))
        {
            cout << "ERR" << endl;
            return -1;
        }

        // Direct Block Handling
        int directBlocks[3] = { inode->getDirectBlock1(), inode->getDirectBlock2(), inode->getDirectBlock3() };

        while (bytesLeft > 0 && currentPosition < 3 * block_size) {
            int currentBlockIndex = currentPosition / block_size;
            int offsetInBlock = currentPosition % block_size;

            fseek(sim_disk_fd, directBlocks[currentBlockIndex] * block_size + offsetInBlock, SEEK_SET);

            int spaceInCurrentBlock = block_size - offsetInBlock;
            int sizeToRead = std::min(spaceInCurrentBlock, bytesLeft);

            fread(buf, sizeof(char), sizeToRead, sim_disk_fd);

            bytesLeft -= sizeToRead;
            currentPosition += sizeToRead;
            buf += sizeToRead;

            offsetInBlock = 0; // Reset for subsequent blocks
        }

        // Single Indirect Block Handling
        if (bytesLeft > 0 && currentPosition < 3 * block_size + block_size * block_size) {
            char pointersChar[block_size];
            if (inode->getSingleInDirect() == -1) {
                cout << "ERR" << endl;
                return -1;
            }

            fseek(sim_disk_fd, inode->getSingleInDirect() * block_size, SEEK_SET);
            fread(pointersChar, 1, block_size, sim_disk_fd);

            while (bytesLeft > 0 && currentPosition < 3 * block_size + block_size * block_size) {
                int indirectIndex = (currentPosition - 3 * block_size) / block_size;
                int indirectBlock = static_cast<int>(pointersChar[indirectIndex]);
                int offsetInBlock = currentPosition % block_size;

                fseek(sim_disk_fd, indirectBlock * block_size + offsetInBlock, SEEK_SET);

                int spaceInCurrentBlock = block_size - offsetInBlock;
                int sizeToRead = std::min(spaceInCurrentBlock, bytesLeft);

                fread(buf, sizeof(char), sizeToRead, sim_disk_fd);

                bytesLeft -= sizeToRead;
                currentPosition += sizeToRead;
                buf += sizeToRead;

                offsetInBlock = 0; // Reset for subsequent blocks
            }
        }

        // Double Indirect Block Handling
        if (bytesLeft > 0) {
            char doubleIndirectPointers[block_size];

            if (inode->getDoubleInDirect() == -1) {
                cout << "ERR" << endl;
                return -1;
            }

            fseek(sim_disk_fd, inode->getDoubleInDirect() * block_size, SEEK_SET);
            fread(doubleIndirectPointers, 1, block_size, sim_disk_fd);

            int totalWrittenBeforeDbl = 3 * block_size + block_size * block_size;
            int dblIndirectIndex = (currentPosition - totalWrittenBeforeDbl) / (block_size * block_size);
            int singleIndirectIndex = ((currentPosition - totalWrittenBeforeDbl) % (block_size * block_size)) / block_size;

            while (bytesLeft > 0 && dblIndirectIndex < block_size) {
                char singleIndirectPointers[block_size];
                int singleIndirectBlockNum = static_cast<int>(doubleIndirectPointers[dblIndirectIndex]);

                if (singleIndirectBlockNum == (char)-1) {
                    cout << "ERR" << endl;
                    return -1;
                }

                fseek(sim_disk_fd, singleIndirectBlockNum * block_size, SEEK_SET);
                fread(singleIndirectPointers, 1, block_size, sim_disk_fd);

                int offsetInBlock = currentPosition % block_size;

                while (bytesLeft > 0 && singleIndirectIndex < block_size) {
                    int dataBlockNum = static_cast<int>(singleIndirectPointers[singleIndirectIndex]);

                    if (dataBlockNum == (char)-1) {
                        cout << "ERR" << endl;
                        return -1;
                    }

                    fseek(sim_disk_fd, dataBlockNum * block_size + offsetInBlock, SEEK_SET);

                    int spaceInBlock = block_size - offsetInBlock;
                    int sizeToRead = std::min(spaceInBlock, bytesLeft);

                    fread(buf, sizeof(char), sizeToRead, sim_disk_fd);

                    bytesLeft -= sizeToRead;
                    currentPosition += sizeToRead;
                    buf += sizeToRead;

                    offsetInBlock = 0;  // Reset for subsequent blocks
                    singleIndirectIndex++;
                }

                singleIndirectIndex = 0; // Reset for the next set of blocks
                dblIndirectIndex++;
            }
        }

        buf[bytesRead] = '\0';  // Ensure null termination if being used as a string
        return 1;
    }


    // ------------------------------------------------------------------------
    int GetFileSize(int fd) {
        // making sure disk is formatted
        if(!is_formated)
        {
            cout << "ERR" << endl;
            return -1;
        }
        if (fd < 0 || fd >= OpenFileDescriptors.size() || !OpenFileDescriptors[fd].isInUse()) {
            // Invalid file descriptor or file not currently open.
            cout << "ERR" << endl;
            return -1;  // Return an empty string or another error indicator
        }

        // grab inode for the file
        fsInode* inode = OpenFileDescriptors[fd].getInode();

        return inode->getFileSize();
    }

    // ------------------------------------------------------------------------
    int CopyFile(string srcFileName, string destFileName) {
        // making sure disk is formatted
        if(!is_formated)
        {
            cout << "ERR" << endl;
            return -1;
        }
        // Check if srcFileName exists in MainDir
        if (MainDir.find(srcFileName) == MainDir.end()) {
            cout << "ERR" << endl;
            return -1;
        }
        bool destExists = true;
        // Ensure destFileName doesn't already exist
        if (MainDir.find(destFileName) == MainDir.end()) {
            destExists = false;
        }
        // checking that the src and dest are different files
        if(srcFileName == destFileName)
        {
            cout << "ERR" << endl;
            return -1;
        }

        if(getFileDescriptorForFileName(srcFileName) != -1)
        {
            // src file descriptor is open and is in use
            cout << "ERR" << endl;
            return -1;
        }
        if(destExists && getFileDescriptorForFileName(destFileName) != -1)
        {
            // dest exists and file descriptor is open and is in use
            cout << "ERR" << endl;
            return -1;
        }
        char str_to_read[DISK_SIZE];
        int size_to_read;
        // we open the srcfile to read it and get its fd
        OpenFile(srcFileName);
        int src_fd = getFileDescriptorForFileName(srcFileName);
        size_to_read =
                OpenFileDescriptors[src_fd].getInode()->getFileSize();
        ReadFromFile(getFileDescriptorForFileName(srcFileName), str_to_read, size_to_read);
        CloseFile(src_fd);
        // if dest exists rn we delete and override, otherwise we just create
        if(destExists) {
            DelFile(destFileName);
        }
        CreateFile(destFileName);
        // we get fd of dest
        int dst_fd = getFileDescriptorForFileName(destFileName);

        // we write into the destination
        WriteToFile(dst_fd, str_to_read, strlen(str_to_read));
        // close the file
        CloseFile(dst_fd);
        return 1;
    }

    // ------------------------------------------------------------------------
    int RenameFile(string oldFileName, string newFileName) {
        // making sure disk is formatted
        if(!is_formated)
        {
            cout << "ERR" << endl;
            return -1;
        }
        // Check if oldFileName exists in MainDir
        if (MainDir.find(oldFileName) == MainDir.end()) {
            cout << "ERR" << endl;
            return -1;
        }

        // Check if newFileName already exists in MainDir
        if (MainDir.find(newFileName) != MainDir.end()) {
            cout << "ERR" << endl;
            return -1;
        }

        // checking if file is open, if it is, we can't rename
        for (auto it = OpenFileDescriptors.begin(); it != OpenFileDescriptors.end(); /* no increment here */) {
            if (it->getFileName() == oldFileName && !it->isInUse()) {
                it = OpenFileDescriptors.erase(it);
            } else {
                ++it;
            }
        }


        fsInode *inode = MainDir[oldFileName];
        MainDir.erase(oldFileName);
        MainDir[newFileName] = inode;
        return 1;
    }

    ~fsDisk()
    {
        fclose(sim_disk_fd);
        // Delete the bit vector.
        delete[] BitVector;

        // Delete the main directory inodes.
        for (auto entry : MainDir) {
            delete entry.second;
        }
    }

};

int main() {
    int blockSize;
	int direct_entries;
    string fileName;
    string fileName2;
    char str_to_write[DISK_SIZE];
    char str_to_read[DISK_SIZE];
    int size_to_read;
    int _fd;

    fsDisk *fs = new fsDisk();
    int cmd_;
    while(1) {
        cin >> cmd_;
        switch (cmd_)
        {
            case 0:   // exit
				delete fs;
				exit(0);
                break;

            case 1:  // list-file
                fs->listAll();
                break;

            case 2:    // format
                cin >> blockSize;
                fs->fsFormat(blockSize);
                break;

            case 3:    // creat-file
                cin >> fileName;
                _fd = fs->CreateFile(fileName);
                cout << "CreateFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 4:  // open-file
                cin >> fileName;
                _fd = fs->OpenFile(fileName);
                cout << "OpenFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 5:  // close-file
                cin >> _fd;
                fileName = fs->CloseFile(_fd);
                cout << "CloseFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 6:   // write-file
                cin >> _fd;
                cin >> str_to_write;
                fs->WriteToFile( _fd , str_to_write , strlen(str_to_write) );
                break;

            case 7:    // read-file
                cin >> _fd;
                cin >> size_to_read ;
                fs->ReadFromFile( _fd , str_to_read , size_to_read );
                cout << "ReadFromFile: " << str_to_read << endl;
                break;

            case 8:   // delete file
                 cin >> fileName;
                _fd = fs->DelFile(fileName);
                cout << "DeletedFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 9:   // copy file
                cin >> fileName;
                cin >> fileName2;
                fs->CopyFile(fileName, fileName2);
                break;

            case 10:  // rename file
                cin >> fileName;
                cin >> fileName2;
                fs->RenameFile(fileName, fileName2);
                break;

            default:
                break;
        }
    }
    return 0;
}