#pragma once

#include <vector>
#include <string>

class Constructor {
private:
    std::string name;
    std::vector<Constructor> properties;

public:
    std::string getName();
    void setName(std::string name);
    class Constructor getProperty(int index);
    class Constructor getProperty(std::string name);
    std::vector<Constructor> getProperties();
    void addProperty(std::string name, std::string value);
    void addProperty(Constructor property);
    std::string getValue();
    std::string toString();
    std::string toPrettyString(int depth = 0);
};