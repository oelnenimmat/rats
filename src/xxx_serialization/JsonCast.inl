#include "JsonCast.h"

// Todo(Leo): if we keep using this meta stuff, then use sfinae with these, so they do not 
// inccorrectly complain that no meta registration is done, when we are manually implementing
// these functions. Or store that info into meta info, so we wont even try to call these without
// registration
template <typename T>
void to_json(nlohmann::json& j, const T& obj)
{
    static_assert(meta::isRegistered<T>(), "Type not registered with meta::registerMembers()");
    j = meta::serialize(obj);
}

template <typename T>
void from_json(const nlohmann::json& j, T& obj)
{
    static_assert(meta::isRegistered<T>(), "Type not registered with meta::registerMembers()");
    meta::deserialize(obj, j);
}

namespace meta
{

/////////////////// SERIALIZATION

// template <typename Class,
//     typename>
template<typename T>
nlohmann::json serialize(const T& obj)
{
    nlohmann::json value;
    meta::doForAllMembers<T>(
        [&obj, &value](auto& member)
        {
            if (member.canGetConstRef()) 
            {
                auto& valueName = value[member.getName()];
                valueName = member.get(obj);
            } 
            else if (member.hasGetter() && member.hasSetter()) // no point saving, if there is only getter
            {
                auto& valueName = value[member.getName()];
                valueName = member.getCopy(obj); // passing copy as const ref, it's okay
            }
        }
    );
    return value;
}


// template <typename Class>
// nlohmann::json serialize_basic(const Class& obj)
// {
//     return nlohmann::json(obj);
// }
/*
template <typename Class,
    typename, typename>
nlohmann::json serialize(const Class& obj)
{
    return serialize_basic(obj);
}
// specialization for std::vector
template <typename T>
nlohmann::json serialize_basic(const std::vector<T>& obj)
{
    nlohmann::json value;
    int i = 0;
    for (auto& elem : obj) {
        value[i] = elem;
        ++i;
    }
    return value;
}

// specialization for std::unordered_map
template <typename K, typename V>
nlohmann::json serialize_basic(const std::unordered_map<K, V>& obj)
{
    nlohmann::json value;
    for (auto& pair : obj) {
        value.emplace(castToString(pair.first), pair.second);
    }
    return value;
}
*/

/////////////////// DESERIALIZATION

// template <typename Class>
// Class deserialize(const nlohmann::json& obj)
// {
//     Class c;
//     deserialize(c, obj);
//     return c;
// }

// template <typename Class,
    // typename>
template<typename T>
void deserialize(T & obj, const nlohmann::json& object)
{
    if (object.is_object()) {
        meta::doForAllMembers<T>(
            [&obj, &object](auto& member)
            {
                // Note(Leo): I don't care about missing fields, they get generated anyway.
                if (object.find(member.getName()) == object.end())
                {
                    return;
                }

                auto& objName = object[member.getName()];
                if (!objName.is_null()) {
                    using MemberT = meta::get_member_type<decltype(member)>;
                    if (member.hasSetter()) {
                        member.set(obj, objName.template get<MemberT>());
                    } else if (member.canGetRef()) {
                        member.getRef(obj) = objName.template get<MemberT>();
                    } else {
                        throw std::runtime_error("Error: can't deserialize member because it's read only");
                    }
                }
            }
        );
    } else {
        throw std::runtime_error("Error: can't deserialize from nlohmann::json to Class.");
    }
}

// template <typename Class,
//     typename, typename>
// void deserialize(Class& obj, const nlohmann::json& object)
// {
//     obj = object.get<Class>();
// }

/*
// specialization for std::vector
template <typename T>
void deserialize(std::vector<T>& obj, const nlohmann::json& object)
{
    obj.reserve(object.size()); // vector.resize() works only for default constructible types
    for (auto& elem : object) {
        obj.push_back(elem); // push rvalue
    }
}

// specialization for std::unodered_map
template <typename K, typename V>
void deserialize(std::unordered_map<K, V>& obj, const nlohmann::json& object)
{
    for (auto it = object.begin(); it != object.end(); ++it) {
        obj.emplace(fromString<K>(it.key()), it.value());
    }
}
*/
}
