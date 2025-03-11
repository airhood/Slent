#pragma once

#include <vector>
#include <string>
#include <tuple>

class Constructor {
private:
    std::string name;
    std::string value;
    std::vector<Constructor*> properties;

public:
    static std::tuple<Constructor*, bool> merge(const Constructor* a, const Constructor* b) {
        Constructor* merge = Constructor::copy(a);
        if (a->getName() != b->getName()) {
            return std::make_tuple(new Constructor(), false);
        }

        for (auto& property : b->getProperties()) {
            if (a->propertyExist(property->getName())) {
                Constructor* sub_merge = get<Constructor*>(Constructor::merge(a->getProperty(property->getName()), property));
                merge->addProperty(sub_merge);
                continue;
            }
            merge->addProperty(property);
        }

        return std::make_tuple(merge, true);
    }

    static Constructor* copy(const Constructor* constructor) {
        Constructor* new_constructor = new Constructor();
        new_constructor->name = constructor->name;
        new_constructor->value = constructor->value;
        for (auto& element : constructor->properties) {
            new_constructor->properties.push_back(Constructor::copy(element));
        }
        return new_constructor;
    }

    std::string getName() const;
    void setName(std::string name);
    Constructor* getProperty(int index) const;
    Constructor* getProperty(std::string name) const;
    bool propertyExist(int index) const;
    bool propertyExist(std::string name) const;
    std::vector<Constructor*> getProperties() const;
    void addProperty(std::string name, std::string value);
    void addProperty(Constructor* property);
    void setValue(std::string value);
    std::string getValue();
    std::string getValue() const;
    std::string toString() const;
    std::string toPrettyString(int depth = 0) const;
};