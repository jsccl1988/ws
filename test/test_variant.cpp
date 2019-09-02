#include <wspp/util/variant.hpp>

#include <iostream>

using namespace std;

struct Field {
public:
    string name_;
    string type_;
};

struct Table {
public:
    string name_;

    Field id_;
    Field desc_;
    Field title_;
};

int main() {
}