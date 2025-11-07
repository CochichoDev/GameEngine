#ifndef PHYSICS_H
#define PHYSICS_H

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>
#include <unordered_map>
#include <cstddef>
#include <thread>

#include "entity.hpp"
#include "vector2d.hpp"
#include "mpsc.hpp"

class PhysicsCore {
    public:
        static constexpr size_t INVALID_TICK = 0;

    public:
        PhysicsCore() = default;
        ~PhysicsCore() {
            m_running.store(false, std::memory_order_relaxed);
            if (m_physics_thread.joinable()) m_physics_thread.join();
        }

        void add_physics_entity(EntityID eid, Vector2D pos, Vector2D speed, Vector2D acc) {
            m_msg.enqueue(PhysicsMsg{eid, 0, PhysicsData{pos, speed, acc}});
        }

        void del_physics_entity(EntityID eid) {
            m_msg.enqueue(PhysicsMsg{eid, 1});
        }

        bool verify_snapshot_valid(uint32_t tick) {
            size_t idx = tick % NUM_SNAPSHOTS;
            
            size_t cur_tick = m_snapshots[idx].tick.load(std::memory_order_acquire);
            return (cur_tick == tick) ? true : false;
        }

        void run() {
            m_running.store(true, std::memory_order_relaxed);
            m_physics_thread = std::thread(&PhysicsCore::loop, this);
        }

    private:
        struct PhysicsData {
            Vector2D    pos;
            Vector2D    speed;
            Vector2D    acc;
        };

        struct PhysicsMsg {
            static const bool ADD = false;
            static const bool DEL = true;

            EntityID    id;
            bool        add_remove;
            PhysicsData data;
        };

        struct PhysicsSnapshot {
            EntityID    id;
            Vector2D    pos;
            Vector2D    speed;
        };

        struct SnapshotEntry {
            std::atomic<uint32_t>   tick; 
            std::vector<PhysicsSnapshot> snapshot;
        };

    private:
        void process_physics_msg() {
            PhysicsMsg msg;
            while (m_msg.dequeue(msg)) {
                if (msg.add_remove == PhysicsMsg::ADD) {
                    on_add(msg.id, msg.data);
                } else {
                    on_del(msg.id);
                }
            }
        }

        void on_add(EntityID eid, const PhysicsData& data) {
            if (m_lookup.find(eid) == m_lookup.end()) {
                m_ids.push_back(eid);
                m_data.push_back(data);
                m_lookup.emplace(eid, m_ids.size()-1);
            }
        }
        void on_del(EntityID eid) {
            auto it = m_lookup.find(eid);
            if (it == m_lookup.end()) return;

            size_t idx = it->second;
            size_t last = m_data.size() - 1;

            if (idx != last) {
                std::swap(m_data[idx], m_data[last]);
                std::swap(m_ids[idx], m_ids[last]);
                m_lookup[m_ids[idx]] = idx;
            }

            m_data.pop_back();
            m_ids.pop_back();
            m_lookup.erase(it);
        }

        void update_state() {
            for (auto& d : m_data) {
                d.speed += d.acc * m_dt;
                d.pos += d.speed * m_dt;
            }
        }

        /* If the last/first indexes of the snapshots are invalid, NUM_SNAPSHOTS, then it's the first entry */
        void publish_snapshot() {
            size_t idx = m_tick % NUM_SNAPSHOTS;

            /* 
             * First check if the tick as wrapped around and invalidates
             * that entry, aka, (tick - NUM_SNAPSHOTS) % NUM_SNAPSHOTS nth entry
             */
            size_t oldest_idx = m_oldest_snapshot_idx.load(std::memory_order_acquire);
            if (oldest_idx == idx) {
                m_oldest_snapshot_idx.store((idx+1) % NUM_SNAPSHOTS, std::memory_order_release);
            }

            m_snapshots[idx].tick.store(m_tick, std::memory_order_release);
            
            m_snapshots[idx].snapshot.resize(m_data.size());
            for (size_t i = 0; i < m_data.size(); ++i) {
                m_snapshots[idx].snapshot[i] = PhysicsSnapshot{m_ids[i], m_data[i].pos, m_data[i].speed};
            }

            m_last_snapshot_idx.store(idx, std::memory_order_release);
        }

        /*
         * The returned value is a REFERENCE, meaning that it's up to the caller to
         * verify, after doing the needed operations, that the returned tick version
         * was still valid or if it was any wrapping done by the physics system, by 
         * calling verify_snapshot_valid. 
         * If no snapshot has been recorded yet, m_last_snapshot_idx should be invalid 
         * the output tick will also have an invalid value, INVALID_TICK.
         */
        std::vector<PhysicsSnapshot>& get_last_snapshot_ref(uint32_t& tick) {
            size_t last_index = m_last_snapshot_idx.load(std::memory_order_acquire);
    
            if (last_index == NUM_SNAPSHOTS) {
                tick = INVALID_TICK;
                return m_snapshots[0].snapshot;
            }

            tick = m_snapshots[last_index].tick.load(std::memory_order_acquire);
            return m_snapshots[last_index].snapshot;
        }

        void loop() {
            auto next = std::chrono::steady_clock::now();
            while (m_running.load(std::memory_order_relaxed)) {
                process_physics_msg();
                update_state();
                publish_snapshot();

                ++m_tick;
                next += m_period;
                std::this_thread::sleep_until(next);
            }
        }

    private:
        uint32_t    m_tick{1};

        std::vector<PhysicsData>    m_data;
        std::vector<EntityID>       m_ids;      // Keep entity id and data separate for SIMD performance
        std::unordered_map<EntityID, size_t>    m_lookup;

        static constexpr size_t NUM_SNAPSHOTS = 64;
        std::array<SnapshotEntry, NUM_SNAPSHOTS> m_snapshots;
        std::atomic<size_t> m_last_snapshot_idx{NUM_SNAPSHOTS};     // Default to an invalid value
        std::atomic<size_t> m_oldest_snapshot_idx{0};

        std::thread         m_physics_thread;
        std::atomic<bool>   m_running;
                                
        MPSCQueue<PhysicsMsg>   m_msg;

        static constexpr double m_dt = 1.0 / 60.0;
        
        static constexpr std::chrono::steady_clock::duration m_period =
            std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                std::chrono::duration<double>{m_dt}
            );
};

#endif
