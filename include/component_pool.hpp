#ifndef COMPONENT_POOL_H
#define COMPONENT_POOL_H

#include <cstddef>
#include <vector>
#include <unordered_map>
#include <cstddef>
#include <functional>
#include <optional>

#include "entity.hpp"


template<typename T>
struct ComponentEntry {
    EntityID    owner;
    T           data;
};

using SwapNotifyFunc = std::function<void(EntityID eid, size_t dst_idx)>;

template<typename T>
class ComponentPool {
    public:
        using iterator = typename std::vector<T>::iterator;

    public:
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
                
                if (m_swap_notify) m_swap_notify(m_data[idx].owner, idx);
            }

            m_data.pop_back();
            m_lookup.erase(it);
            return true;
        }

        size_t size() const {
            return m_data.size();
        }

        void set_swap_notify_func(SwapNotifyFunc fn) { m_swap_notify = std::move(fn); }

        iterator begin() {
            return m_data.begin();
        }
        iterator end() {
            return m_data.end();
        }

    private:
        std::vector<ComponentEntry<T>>  m_data;
        std::unordered_map<EntityID, std::size_t> m_lookup;

        SwapNotifyFunc  m_swap_notify;
};

#endif
