#include "Constructor.h"

std::string Constructor::getName() const {
    return name;
}

void Constructor::setName(std::string name) {
    this->name = name;
}

Constructor* Constructor::getProperty(int index) const {
    if ((index + 1) > properties.size()) return new Constructor();
    return properties.at(index);
}

Constructor* Constructor::getProperty(std::string name) const {
    for (int i = 0; i < properties.size(); i++) {
        if (properties.at(i)->getName() == name) {
            return properties.at(i);
        }
    }
    return new Constructor();
}

bool Constructor::propertyExist(int index) const {
    return properties.size() >= (index + 1);
}

bool Constructor::propertyExist(std::string name) const {
    for (int i = 0; i < properties.size(); i++) {
        if (properties.at(i)->getName() == name) {
            return true;
        }
    }
    return false;
}

std::vector<Constructor*> Constructor::getProperties() const {
    return properties;
}

void Constructor::addProperty(std::string name, std::string value) {
    this->value = "[object]";
    Constructor* valueConstructor = new Constructor();
    valueConstructor->setName(name);
    valueConstructor->setValue(value);
    properties.push_back(valueConstructor);
}

void Constructor::addProperty(Constructor* property) {
    value = "[object]";
    properties.push_back(property);
}

void Constructor::setValue(std::string value) {
    this->value = value;
}

std::string Constructor::getValue() {
    return value;
}

std::string Constructor::getValue() const {
    return value;
}

std::string Constructor::toString() const {
    if (value != "[object]") {
        return name;
    }
    std::string str = name;
    str.append("{");
    for (int i = 0; i < properties.size(); i++) {
        if (i != 0) {
            str.append(",");
        }
        
        str.append(properties.at(i)->toString());
    }
    str.append("}");
    return str;
}

std::string Constructor::toPrettyString(int depth) const {
    if (value != "[object]") {
        std::string str = "";
        for (int i = 0; i < depth; i++) {
            str.append("  ");
        }
        str.append(name);
        str.append(": ");
        str.append(value);
        return str;
    }
    std::string str = "";
    for (int i = 0; i < depth; i++) {
        str.append("  ");
    }
    str.append(name);
    str.append(": {\n");
    for (int i = 0; i < properties.size(); i++) {
        if (i != 0) {
            str.append(",\n");
        }
        
        str.append(properties.at(i)->toPrettyString(depth + 1));
    }
    str.append("\n");
    for (int i = 0; i < depth; i++) {
        str.append("  ");
    }
    str.append("}");
    return str;
}