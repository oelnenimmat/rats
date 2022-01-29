#pragma once

// #include <string>
// #include <vector>
// #include <unordered_map>

// #include <nlohmann/json.hpp>

#include <MetaStuff/Meta.h>
// #include "StringCast.h"

// using json = nlohmann::json;

template <typename T>
void to_json(nlohmann::json& j, const T& obj);

template <typename T>
void from_json(const nlohmann::json& j, T& obj);

namespace meta
{

/////////////////// SERIALIZATION

// template <typename Class,
//     typename = std::enable_if_t <meta::isRegistered<Class>()>>
template <typename T>
nlohmann::json serialize(T const & obj);

// template <typename Class,
//     typename = std::enable_if_t <!meta::isRegistered<Class>()>,
//     typename = void>
// nlohmann::json serialize(const Class& obj);

/*
template <typename Class>
nlohmann::json serialize_basic(const Class& obj);

// specialization for std::vector
template <typename T>
nlohmann::json serialize_basic(const std::vector<T>& obj);

// specialization for std::unodered_map
template <typename K, typename V>
nlohmann::json serialize_basic(const std::unordered_map<K, V>& obj);
*/

/////////////////// DESERIALIZATION
//
//template<typename Class>
//Class deserialize(const nlohmann::json& obj);

// template <typename Class,
//     typename = std::enable_if_t<meta::isRegistered<Class>()>>

template <typename T>
void deserialize(T & obj, const nlohmann::json& object);

// template <typename Class,
//     typename = std::enable_if_t<!meta::isRegistered<Class>()>,
//     typename = void>
// void deserialize(Class& obj, const nlohmann::json& object);

/*
// specialization for std::vector
template <typename T>
void deserialize(std::vector<T>& obj, const nlohmann::json& object);

// specialization for std::unodered_map
template <typename K, typename V>
void deserialize(std::unordered_map<K, V>& obj, const nlohmann::json& object);
*/

}

#include "JsonCast.inl"