#include "Constructor.h"

std::string Constructor::getName() {
    return name;
}

void Constructor::setName(std::string name) {
    this->name = name;
}

Constructor Constructor::getProperty(int index) {
    if ((index + 1) > properties.size()) return Constructor();
    return properties.at(index);
}

Constructor Constructor::getProperty(std::string name) {
    for (int i = 0; i < properties.size(); i++) {
        if (properties.at(i).getName() == name) {
            return properties.at(i);
        }
    }
    return Constructor();
}

bool Constructor::propertyExist(int index) {
    return properties.size() >= (index + 1);
}

bool Constructor::propertyExist(std::string name) {
    for (int i = 0; i < properties.size(); i++) {
        if (properties.at(i).getName() == name) {
            return true;
        }
    }
    return false;
}

std::vector<Constructor> Constructor::getProperties() {
    return properties;
}

void Constructor::addProperty(std::string name, std::string value) {
    Constructor constructor = Constructor();
    constructor.setName(name);
    Constructor value_ = Constructor();
    value_.setName(value);
    constructor.addProperty(value_);
    properties.push_back(constructor);
}

void Constructor::addProperty(Constructor property) {
    properties.push_back(property);
}

std::string Constructor::getValue() {
    return properties.at(0).getName();
}

std::string Constructor::toString() {
    if (properties.empty()) {
        return name;
    }
    std::string str = name.append("{");
    for (int i = 0; i < properties.size(); i++) {
        if (i != 0) {
            str.append(",");
        }
        str.append(properties.at(i).toString());
    }
    str.append("}");
    return str;
}

std::string Constructor::toPrettyString(int depth) {
    if (properties.empty()) {
        std::string str = "";
        for (int i = 0; i < depth; i++) {
            str.append("  ");
        }
        str.append(name);
        return str;
    }
    std::string str = "";
    for (int i = 0; i < depth; i++) {
        str.append("  ");
    }
    str += name.append("{\n");
    for (int i = 0; i < properties.size(); i++) {
        if (i != 0) {
            str.append(",\n");
        }
        str.append(properties.at(i).toPrettyString(depth + 1));
    }
    str.append("\n");
    for (int i = 0; i < depth; i++) {
        str.append("  ");
    }
    str.append("}");
    return str;
}