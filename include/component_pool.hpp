#ifndef COMPONENT_POOL_H
#define COMPONENT_POOL_H

#include <cstddef>
#include <vector>
#include <unordered_map>
#include <cstddef>
#include <functional>
#include <optional>

#include "entity.hpp"
#include "ctime_typemap.hpp"


template<typename T>
struct ComponentEntry {
    EntityID    owner;
    T           data;
};

template<typename T>
class ComponentPool;

template<typename T>
class ComponentPoolTraits {
    template<typename... Ts>
    static void init(ComponentPool<T>&, TypeMap<Ts...>&);
};

template<typename T>
class ComponentPool {
    public:
        using iterator = typename std::vector<T>::iterator;

        using RemoveNotifyFn = std::function<void(EntityID owner)>;
        using SwapNotifyFn = std::function<void(EntityID owner, size_t new_idx)>;

    public:
        ComponentPool() = default;

        template<typename... Ts>
        void init(TypeMap<Ts...>& pools) {
            ComponentPoolTraits<T>::init(*this, pools);
        }

        void reserve(std::size_t size) {
            m_data.reserve(size);
        }

        std::optional<std::size_t> find(EntityID owner) {
            auto it = m_lookup.find(owner);
            if (it == m_lookup.end()) return std::nullopt;

            return it->second;
        }

        const ComponentEntry<T>& entry_at(size_t idx) const {
            assert(idx < m_data.size());
            return m_data[idx];
        }
        ComponentEntry<T>& entry_at(size_t idx) {
            assert(idx < m_data.size());
            return m_data[idx];
        }

        size_t add(EntityID owner, const T& data) {
            size_t idx = m_data.size();
            m_data.emplace_back(ComponentEntry<T>(owner, data));
            m_lookup.emplace(owner, idx);
            return idx;
        }

        bool remove(EntityID eid) {
            auto it = m_lookup.find(eid);
            if (it == m_lookup.end()) return false;
            size_t idx = it->second;
            size_t last_idx = m_data.size() - 1;
            if (idx != last_idx) {
                std::swap(m_data[idx], m_data[last_idx]);
                m_lookup[m_data[idx].owner] = idx;
                
                for (auto& fn : m_swap_listeners) {
                    fn(m_data[idx].owner, idx);
                }
            }

            for (auto& fn : m_remove_listeners) {
                fn(eid);
            }
            m_data.pop_back();
            m_lookup.erase(it);

            return true;
        }

        size_t size() const {
            return m_data.size();
        }

        void subscribe_swap_listener(SwapNotifyFn& fn) {
            m_swap_listeners.push_back(std::move(fn));
        }
        void subscribe_remove_listener(RemoveNotifyFn fn) {
            m_remove_listeners.push_back(std::move(fn));
        }

        iterator begin() {
            return m_data.begin();
        }
        iterator end() {
            return m_data.end();
        }

    private:
        std::vector<ComponentEntry<T>>  m_data;
        std::unordered_map<EntityID, std::size_t> m_lookup;

        std::vector<SwapNotifyFn> m_swap_listeners;
        std::vector<RemoveNotifyFn> m_remove_listeners;
};

#endif
