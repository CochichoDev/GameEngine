#ifndef COMPONENT_POOL_H
#define COMPONENT_POOL_H

#include <cstddef>
#include <type_traits>
#include <variant>
#include <vector>
#include <unordered_map>
#include <cstddef>
#include <functional>
#include <optional>

#include <assert.h>

#include "containers/typemap.hpp"

#include "entity.hpp"

template<typename T>
struct ComponentEntry {
    EntityID    owner;
    T           data;
};

template<typename T, typename R>
class ComponentPool;

template<typename T, typename R>
class ComponentPoolTraits {
    template<typename... Ts>
    static void init(ComponentPool<T, R>&, TypeMap<Ts...>&);
};

template<typename T, typename R = void>
class ComponentPool {
    public:
        using value_type = T;
        using iterator = typename std::vector<ComponentEntry<T>>::iterator;

        using RemoveNotifyFn = std::function<void(EntityID owner)>;
        using SwapNotifyFn = std::function<void(EntityID owner, size_t new_idx)>;

    public:
        ComponentPool() {
            static_assert(std::is_void_v<R>, 
                    "Registry constructor must be used for non-void R");
        }

        template<typename U = R, typename = std::enable_if_t<!std::is_void_v<U>>>
        ComponentPool(R* reg) : m_reg (reg) {}

        template<typename... Ts>
        void init(TypeMap<Ts...>& pools) {
            ComponentPoolTraits<T, R>::init(*this, pools);
            m_pool_id = pools.template find<T>();
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

            if constexpr (!std::is_same_v<R, void>) {
                assert(m_reg->find(owner) != std::nullopt && "Registry already has an entry for this component type");
                m_reg->add(owner, {owner, m_pool_id, idx});
            }

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

            if constexpr (!std::is_void_v<R>) {
                m_reg->remove(eid);
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
        uint8_t m_pool_id{0};

        std::vector<ComponentEntry<T>>  m_data;
        std::unordered_map<EntityID, std::size_t> m_lookup;

        std::vector<SwapNotifyFn> m_swap_listeners;
        std::vector<RemoveNotifyFn> m_remove_listeners;

        std::conditional_t<std::is_void_v<R>, std::monostate, R*> m_reg;
};

#endif
