#include <iostream>
#include <fstream>
#include <string>

const int firstFileGroupCount = 3;
const int secondFileGroupCount = 2;

struct filemap {
    int* map;

    int firstGroupSize;
    int currentFirstGroupSize;

    int secondGroupSize;

    int size;
    
    filemap(int firstGroupSize, int secondGroupSize) {
        this->size = firstGroupSize + secondGroupSize;
        this->firstGroupSize = firstGroupSize;
        this->currentFirstGroupSize = firstGroupSize;
        this->secondGroupSize = secondGroupSize;
        map = new int[firstGroupSize + secondGroupSize];
        for (int i = 0; i < firstGroupSize + secondGroupSize; i++) {
            map[i] = i;
        }
    }
    ~filemap() {
        delete[] map;
    }
    int operator[](int index) {
        return map[index];
    }
    void swap() {
        int* temp = new int[firstGroupSize];
        for (int i = 0; i < firstGroupSize; i++) {
            temp[i] = map[i];
        }
        for (int i = 0; i < secondGroupSize; i++) {
            map[i] = map[firstGroupSize + i];
        }
        for (int i = 0; i < firstGroupSize; i++) {
            map[secondGroupSize + i] = temp[i];
        }
        int tempSize = firstGroupSize;
        firstGroupSize = secondGroupSize;
        secondGroupSize = tempSize;
        currentFirstGroupSize = firstGroupSize;
        delete[] temp;
    }
    void excludeFile(int index) {
        if (index < currentFirstGroupSize) {
            currentFirstGroupSize--;
            for (int i = index; i < currentFirstGroupSize; i++) {
                std::swap(map[i], map[i + 1]);
            }
        }
    }
    void print() {
        std::cout << "Map: ";
        for (int i = 0; i < size; i++) {
            std::cout << map[i] << ' ';
        }
        std::cout << std::endl;
    }
};

struct sequence {
    int element = 0;
    std::fstream file;
    bool eof = false;
    bool eor = false;
    bool empty = true;
    std::string filename;
};

#pragma region legacy
// void readNext(Sequence& s) {
//     int prev = s.element;
//     if (s.file >> s.element) {
//         if (prev > s.element) {
//             s.eor = true;
//         }
//     }
//     else {
//         s.eof = true;
//         s.eor = true;
//     }
// }

// void startRead(Sequence& s) {
//     s.file.open(s.filename, std::fstream::in);
//     s.file >> s.element;
// }

// void startWrite(Sequence& s) {
//     s.file.open(s.filename, std::fstream::out);
//     s.eor = false;
//     s.eof = false;
// }

// void copyElem(Sequence& s1, Sequence& s2) {
//     s2.element = s1.element;
//     s2.file << s1.element << ' ';
//     readNext(s1);
// }

// void copyRun(Sequence& s1, Sequence& s2) {
//     while (!s1.eor) {
//         copyElem(s1, s2);
//     }
// }
#pragma endregion

// Open files for reading
void open(sequence* files, filemap& fm) {
    for (int i = 0; i < fm.firstGroupSize; i++) {
        files[fm[i]].file.open(files[fm[i]].filename, std::fstream::in);
        bool isAvailable = bool(files[fm[i]].file >> files[fm[i]].element);
        if (!isAvailable) {
            files[fm[i]].eof = true;
            fm.excludeFile(i);
        }
    }
    for (int i = fm.firstGroupSize; i < fm.size; i++) {
        files[fm[i]].file.open(files[fm[i]].filename, std::fstream::out);
    }
}

// Closes all files
void reset(sequence* files, filemap& fm) {
    for (int i = 0; i < fm.size; i++) {
        files[i].file.close();
        files[i].eor = false;
        files[i].eof = false;
        files[i].empty = true;
    }
}

void removeFiles(sequence* files) {
    for (int i = 0; i < firstFileGroupCount + secondFileGroupCount; i++) {
        std::remove(files[i].filename.c_str());
    }
}

bool isAllEOF(sequence* files, filemap& fm) {
    bool result = true;
    for (int i = 0; i < fm.currentFirstGroupSize; i++) {
        result = result && files[fm[i]].eof;
    }
    return result;
}

bool isAllEOR(sequence* files, filemap& fm) {
    bool result = true;
    for (int i = 0; i < fm.currentFirstGroupSize; i++) {
        result = result && files[fm[i]].eor;
    }
    return result;
}

void copyElement(sequence& source, sequence& target) {
    if (!target.file.is_open()) {
        target.file.open(target.filename);
    }
    target.file << source.element << ' ';
    target.element = source.element;
    target.eof = true;
}

void readNext(sequence& file) {
    int prev = file.element;
    bool isopen = file.file.is_open();
    bool eof = !bool(file.file >> file.element);
    if (!eof) {
        if (prev > file.element) {
            file.eor = true;
        }
    }
    else {
        file.eof = true;
    }
}

bool hasOnlyOneFile(sequence* files, filemap& fm) {
    int counter = 0;
    for (int i = fm.firstGroupSize; i < fm.size; i++) {
        if (files[fm[i]].eof) {
            counter++;
        }
    }
    return counter == 1;
}

int mergeIntoFile(sequence* files, sequence& target, filemap& fm) {
    int elementCounter = 0;
    int firstElement;
    while (!isAllEOF(files, fm) && !isAllEOR(files, fm)) {
        sequence* min = &files[fm[0]];
        int index = 0;
        for (int i = 1; i < fm.currentFirstGroupSize; i++) {
            if (files[fm[i]].element < min->element) {
                min = &files[fm[i]];
                index = i;
            }
        }
        if (elementCounter == 0) {
            firstElement = min->element;
            elementCounter++;
        }
        copyElement(*min, target);
        readNext(*min);
        if (min->eor || min->eof) {
            fm.excludeFile(index);
            fm.print();
        }
    }
    return firstElement;
}

bool merge(sequence* files, filemap& fm) {
    while (!isAllEOF(files, fm)) {
        for (int i = fm.firstGroupSize; i < fm.size; i++) {
            if (files[i].empty) {
                mergeIntoFile(files, files[i], fm);
            }
            else {
                int endOfOld = files[i].element;
                int beginOfNew = mergeIntoFile(files, files[i], fm);
                if (endOfOld <= beginOfNew) {
                    mergeIntoFile(files, files[i], fm);
                }
            }
            fm.currentFirstGroupSize = fm.firstGroupSize;
            for (int j = fm.firstGroupSize - 1; j >= 0; j--) {
                files[fm[j]].eor = false;
                if (files[fm[j]].eof) {
                    fm.excludeFile(j);
                    fm.print();
                }
            }
        }
    }
    return hasOnlyOneFile(files, fm);
}

// * undone
// Merges m files into n files. Returns true if there's only one file filled with numbers
// bool merge(sequence* files, filemap& fm) {
//     while (!isAllEOF(files, fm)) {
//         for (int i = fm.firstGroupSize; i < fm.size; i++) {
//             while (!isAllEOF(files, fm) && !isAllEOR(files, fm)) {
//                 // !
//                 /*
//                 if (files[i].empty) {
//                     mergeIntoFile(...);
//                 }
//                 else {
//                     int endOfOld = files[fm[i]];
//                     int beginOfNew = mergeInfoFile(...);
//                     if (endOfOld <= beginOfNew) {
//                         mergeIntoFile(...);
//                     }
//                 }
//                 */
//                 sequence* min = &files[fm[0]];
//                 int index = 0;
//                 for (int j = 1; j < fm.currentFirstGroupSize; j++) {
//                     if (files[fm[j]].element < min->element) {
//                         min = &files[fm[j]];
//                         index = j;
//                     }
//                 }
//                 copyElement(*min, files[fm[i]]);
//                 readNext(*min);
//                 // !
//                 if (min->eor || min->eof) {
//                     fm.excludeFile(index);
//                     fm.print();
//                 }
//             }
//             fm.currentFirstGroupSize = fm.firstGroupSize;
//             for (int j = fm.firstGroupSize - 1; j >= 0; j--) {
//                 files[fm[j]].eor = false;
//                 if (files[fm[j]].eof) {
//                     fm.excludeFile(j);
//                     fm.print();
//                 }
//             }
//         }
//     }
//     return hasOnlyOneFile(files, fm);
// }

// * done
// Places one series from input file to one of output files
bool distributeUntilEndOfRun(std::ifstream& source, sequence& file, int& initialValue) {
    int prev = initialValue;
    file.element = prev;
    int current;
    file.file << prev << ' ';
    bool isReadable = bool(source >> current);
    file.empty = !isReadable;
    while (isReadable && prev <= current) {
        file.file << current << ' ';
        file.element = current;
        prev = current;
        isReadable = bool(source >> current);
    }
    initialValue = current;
    return isReadable;
}

// * done
// Initial distribute from input file
bool distribute(std::ifstream& source, sequence* files) {
    bool isSorted = true;
    for (int i = 0; i < firstFileGroupCount; i++) {
        files[i].file.open(files[i].filename, std::fstream::out);
        files[i].eof = false;
        files[i].eor = false;
    }
    int value;
    bool isReadable = bool(source >> value);
    while (isReadable) {
        // Move first occurence met in origin file to first distribution file
        // and check if all numbers from the origin file were copied to the distribution file
        // if so then the origin file is already sorted
        // then distribute within the rest distribution files
        for (int i = 0; i < firstFileGroupCount && isReadable; i++) {
            if (files[i].empty) {
                isReadable = distributeUntilEndOfRun(source, files[i], value);
            }
            else {
                int endOfOld = files[i].element;
                int beginOfNew = value;
                isReadable = distributeUntilEndOfRun(source, files[i], value);
                if (endOfOld <= beginOfNew) {
                    isReadable = distributeUntilEndOfRun(source, files[i], value);
                }
            }
            if (i > 0) {
                isSorted = false;
            }
        }
    }
    for (int i = 0; i < firstFileGroupCount; i++) {
        files[i].file.close();
    }
    return isSorted;
}

void copyFile(std::string& sourceFilename, std::string& targetFilename) {
    std::ifstream reader(sourceFilename);
    std::ofstream writer(targetFilename);
    int value;
    while (reader >> value) {
        writer << value << ' ';
    }
    reader.close();
    writer.close();
}

void printFromFile(std::string& filename) {
    std::cout << std::endl;
    std::ifstream reader(filename);
    if (reader) {
        int value;
        while (reader >> value) {
            std::cout << value << ' ';
        }
        std::cout << std::endl;
    }
    else {
        std::cout << "<file not found>\n";
    }
}

// * done
void sort(std::string& filename, std::string& outputFilename) {
    sequence files[firstFileGroupCount + secondFileGroupCount];
    for (int i = 1; i <= firstFileGroupCount + secondFileGroupCount; i++) {
        std::string filename = "resources/f" + std::to_string(i) + ".txt";
        files[i - 1].filename = filename;
    }
    // File map which indicates what files are for distributing and merging
    filemap fm(firstFileGroupCount, secondFileGroupCount);
    std::ifstream source(filename);
    // Shows if origin file was already sorted
    bool isSorted = distribute(source, files);
    // Loops until it's sorted
    while (!isSorted) {
        open(files, fm);
        isSorted = merge(files, fm);
        reset(files, fm);
        fm.swap();
        fm.print();
    }
    copyFile(files[fm[0]].filename, outputFilename);
    removeFiles(files);
    // On this step the sort is done. The only thing left to do is move the sorted array to result.txt or smth like that and remove unneccessary files
    // to be done here
}


int main() {
    std::string filename = "resources/base.txt";
    std::string outputFilename = "resources/output.txt";

    std::cout << "Given array: ";
    printFromFile(filename);

    sort(filename, outputFilename);

    std::cout << "Sorted array: ";
    printFromFile(outputFilename);
}