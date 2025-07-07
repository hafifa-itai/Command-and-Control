#pragma once

#include <string>
#include <iostream>
#include <sstream>

class MyData {
public:
    std::string name;
    std::string city;
    int age;

    MyData();
    MyData(const std::string& name, const std::string& city, int age);
    ~MyData();

    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
};
