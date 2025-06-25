#include "MyData.hpp"
#include <sstream>
#include <iostream>

// Default constructor
MyData::MyData() : name(""), city(""), age(0) {
    std::cout << "[MyData] Default constructor called\n";
}

// Parameterized constructor
MyData::MyData(const std::string& name, const std::string& city, int age) : name(name), city(city), age(age) {
    std::cout << "[MyData] Parameterized constructor called\n";
}

// Destructor
MyData::~MyData() {
    std::cout << "[MyData] Destructor called\n";
}

void MyData::serialize(std::ostream& os) const {
    size_t len1 = name.size();
    os.write(reinterpret_cast<const char*>(&len1), sizeof(len1));
    os.write(name.data(), len1);

    size_t len2 = city.size();
    os.write(reinterpret_cast<const char*>(&len2), sizeof(len2));
    os.write(city.data(), len2);

    os.write(reinterpret_cast<const char*>(&age), sizeof(age));
}

void MyData::deserialize(std::istream& is) {
    size_t len = 0;

    is.read(reinterpret_cast<char*>(&len), sizeof(len));
    name.resize(len);
    is.read(&name[0], len);
    
    is.read(reinterpret_cast<char*>(&len), sizeof(len));
    city.resize(len);
    is.read(&city[0], len);

    is.read(reinterpret_cast<char*>(&age), sizeof(age));
}
