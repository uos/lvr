#pragma once

#ifndef LVR2_TYPES_VARIANTCHANNELMAP
#define LVR2_TYPES_VARIANTCHANNELMAP

#include <unordered_map>
#include <iostream>
#include "VariantChannel.hpp"

namespace lvr2 {

template<typename... T>
class VariantChannelMap
: public std::unordered_map<std::string, VariantChannel<T...> > 
{

protected:
    template <class T1, class Tuple>
    struct TupleIndex;

public:
    using key_type = std::string;
    using val_type = VariantChannel<T...>;
    using types = std::tuple<T...>;

    /**
     * @brief Access type index by type.
     * @details Example usage: ChanneVariantMap<int, float> my_map;
     *            ChanneVariantMap<int, float>::type_index<int>::value -> 0
     */
    template<class U>
    struct index_of_type {
        static const std::size_t value = TupleIndex<U, types>::value;
    };

    template <std::size_t N>
    using type_of_index = typename std::tuple_element<N, types>::type;

    /**
     * @brief Adds an Key + AttributeChannel to the map.
     * 
     * @param[in] name Key of the channel.
     * @param[in] channel The channel.
     * 
     */
    template<typename U>
    void add(const std::string& name, Channel<U> channel);

    /**
     * @brief Gets AttributeChannel with type U from map as reference.
     * 
     * @param[in] name Key of the channel.
     */
    template<typename U>
    Channel<U>& get(const std::string& name);

    /**
     * @brief Gets AttributeChannel by type U from map.
     * 
     * @param[in] name Key of the channel.
     * 
     */
    template<typename U>
    const Channel<U>& get(const std::string& name) const;

    /**
     * @brief Gets type index of a map entry.
     * 
     * @param[in] Key of the channel.
     * @return Index of type tuple of the variant.
     */
    int type(const std::string& name) const;

    /**
     * @brief Checks if key has specific type U.
     * @details Example Usage: cm.is_type<float>("points") -> true
     * 
     * @param[in] name Key of the channel.
     * @tparam U Type of the value to check.
     * @return true If the type is equal.
     * @return false If the type is unequal.
     */
    template<typename U>
    bool is_type(const std::string& name) const;

    /**
     * @brief Output cout
     * 
     */
    friend std::ostream& operator<<(std::ostream& os, const VariantChannelMap<T...>& cm)
    {
        std::cout << "[VariantChannelMap]\n";
        for(auto it : cm)
        {
            std::cout << it.first << ": " << it.second  << "\n";
        }
        return os;
    }

protected:
    template <class T1, class... Types>
    struct TupleIndex<T1, std::tuple<T1, Types...>> {
        static const std::size_t value = 0;
    };

    template <class T1, class U, class... Types>
    struct TupleIndex<T1, std::tuple<U, Types...>> {
        static const std::size_t value = 1 + TupleIndex<T1, std::tuple<Types...>>::value;
    };

};

} // namespace lvr2

#include "VariantChannelMap.tcc"

#endif // LVR2_TYPES_VARIANTCHANNELMAP

