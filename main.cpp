#include <iostream>
#include <fstream>
#include <string>

struct Sequence {
    int element = 0;
    std::fstream file;
    bool eof = false;
    bool eor = false;
    std::string filename;
};

void readNext(Sequence& s) {
    int prev = s.element;
    if (s.file >> s.element) {
        if (prev > s.element) {
            s.eor = true;
        }
    }
    else {
        s.eof = true;
        s.eor = true;
    }
}

void startRead(Sequence& s) {
    s.file.open(s.filename, std::fstream::in);
    s.file >> s.element;
}

void startWrite(Sequence& s) {
    s.file.open(s.filename, std::fstream::out);
    s.eor = false;
    s.eof = false;
}

void copyElem(Sequence& s1, Sequence& s2) {
    s2.element = s1.element;
    s2.file << s1.element << ' ';
    readNext(s1);
}

void copyRun(Sequence& s1, Sequence& s2) {
    while (!s1.eor) {
        copyElem(s1, s2);
    }
}

int main() {
    std::cout << "Hello world";
}