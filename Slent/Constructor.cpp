#include "Constructor.h"
#include <iostream>

using namespace Slent;

Constructor::~Constructor() {
    for (Constructor* prop : properties) {
        delete prop;
    }
}

std::string Constructor::getName() const {
    return name;
}

void Constructor::setName(std::string name) {
    this->name = name;
}

Constructor* Constructor::getProperty(int index) const {
    if (index >= 0 && index < properties.size()) {
        return properties.at(index);
    }
    return nullptr;
}

Constructor* Constructor::getProperty(std::string name) const {
    for (Constructor* prop : properties) {
        if (prop->getName() == name) {
            return prop;
        }
    }
    return nullptr;
}

bool Constructor::propertyExist(int index) const {
    return properties.size() > index;
}

bool Constructor::propertyExist(std::string name) const {
    for (Constructor* prop : properties) {
        if (prop->getName() == name) {
            return true;
        }
    }
    return false;
}

std::vector<Constructor*> Constructor::getProperties() const {
    return properties;
}

void Constructor::addProperty(std::string name, std::string value) {
    Constructor* valueConstructor = new Constructor(name, value);
    properties.push_back(valueConstructor);
}

void Constructor::addProperty(Constructor* property) {
    properties.push_back(property);
}

void Constructor::addProperty(std::string prop_name, Constructor* child_node) {
    Constructor* wrapper_node = new Constructor(prop_name);
    wrapper_node->addProperty(child_node);
    properties.push_back(wrapper_node);
}

void Constructor::clearProperty() {
    for (Constructor* prop : properties) {
        delete prop;
    }
    properties.clear();
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
        return name + ": " + value;
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