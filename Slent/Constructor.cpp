#include "Constructor.h"

std::string Constructor::getName() const {
    return name;
}

void Constructor::setName(std::string name) {
    this->name = name;
}

Constructor Constructor::getProperty(int index) const {
    if ((index + 1) > properties.size()) return Constructor();
    return properties.at(index);
}

Constructor Constructor::getProperty(std::string name) const {
    for (int i = 0; i < properties.size(); i++) {
        if (properties.at(i).getName() == name) {
            return properties.at(i);
        }
    }
    return Constructor();
}

bool Constructor::propertyExist(int index) const {
    return properties.size() >= (index + 1);
}

bool Constructor::propertyExist(std::string name) const {
    for (int i = 0; i < properties.size(); i++) {
        if (properties.at(i).getName() == name) {
            return true;
        }
    }
    return false;
}

std::vector<Constructor> Constructor::getProperties() const {
    return properties;
}

void Constructor::setValue(std::string name, std::string value) {
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

std::string Constructor::getValue() const {
    return value;
}

std::string Constructor::toString() const {
    if (properties.empty()) {
        return name;
    }
    std::string str = name;
    str.append("{");
    for (int i = 0; i < properties.size(); i++) {
        if (i != 0) {
            str.append(",");
        }

        if (value == "[object]") {
            str.append(properties.at(i).toString());
        }
        else {
            str.append(value);
        }
    }
    str.append("}");
    return str;
}

std::string Constructor::toPrettyString(int depth) const {
    if (properties.empty()) {
        std::string str = "";
        for (int i = 0; i < depth; i++) {
            str.append("  ");
        }
        str.append(name);
        str.append(": ");
        return str;
    }
    std::string str = "";
    for (int i = 0; i < depth; i++) {
        str.append("  ");
    }
    str += name;
    str.append("{\n");
    for (int i = 0; i < properties.size(); i++) {
        if (i != 0) {
            str.append(",\n");
        }

        if (value == "[object]") {
            str.append(properties.at(i).toPrettyString());
        }
        else {
            str.append(value);
        }
    }
    str.append("\n");
    for (int i = 0; i < depth; i++) {
        str.append("  ");
    }
    str.append("}");
    return str;
}