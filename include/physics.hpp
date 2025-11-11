#ifndef PHYSICS_H
#define PHYSICS_H

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <utility>
#include <vector>
#include <unordered_map>
#include <cstddef>
#include <thread>

#include "entity.hpp"
#include "vector2d.hpp"
#include "mpsc.hpp"

struct PhysicsSnapshot {
    EntityID    id;
    Vector2D    pos;
    Vector2D    speed;

    std::size_t transform_idx;
};

class PhysicsCore {
    public:
        static constexpr size_t INVALID_TICK = 0;


    public:
        PhysicsCore() = default;
        ~PhysicsCore() {
            m_running.store(false, std::memory_order_relaxed);
            if (m_physics_thread.joinable()) m_physics_thread.join();
        }

        void add_physics_entity(EntityID eid, std::size_t transform_idx,
                Vector2D pos, Vector2D speed, Vector2D acc) {
            m_msg.enqueue(PhysicsMsg{PhysicsMsg::ADD, eid, transform_idx, PhysicsData{pos, speed, acc}});
        }

        void del_physics_entity(EntityID eid) {
            m_msg.enqueue(PhysicsMsg{PhysicsMsg::DEL, eid});
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

        /*
         * The returned value is a REFERENCE, meaning that it's up to the caller to
         * verify, after doing the needed operations, that the returned tick version
         * was still valid or if it was any wrapping done by the physics system, by 
         * calling verify_snapshot_valid. 
         * If no snapshot has been recorded yet, m_last_snapshot_idx should be invalid 
         * the output tick will also have an invalid value, INVALID_TICK.
         */
        const std::vector<PhysicsSnapshot>& get_last_snapshot_ref(uint32_t& tick) const {
            size_t last_index = m_last_snapshot_idx.load(std::memory_order_acquire);
    
            if (last_index == NUM_SNAPSHOTS) {
                tick = INVALID_TICK;
                return m_snapshots[0].snapshot;
            }

            tick = m_snapshots[last_index].tick.load(std::memory_order_acquire);
            return m_snapshots[last_index].snapshot;
        }

    private:
        struct PhysicsData {
            Vector2D    pos;
            Vector2D    speed;
            Vector2D    acc;
        };

        struct PhysicsMsg {
            enum MsgType {
                ADD = 0,
                DEL,
                SWAP,
            } type;

            EntityID    id;
            std::size_t transform_idx{0};
            PhysicsData data{};
        };

        struct SnapshotEntry {
            std::atomic<uint32_t>   tick; 
            std::vector<PhysicsSnapshot> snapshot;
        };

    private:
        void process_physics_msg() {
            PhysicsMsg msg;
            while (m_msg.dequeue(msg)) {
                switch (msg.type) {
                    case PhysicsMsg::ADD:
                        on_add(msg.id, msg.transform_idx, msg.data);
                        break;
                    case PhysicsMsg::DEL:
                        on_del(msg.id);
                        break;
                    case PhysicsMsg::SWAP:
                        break;
                }
            }
        }

        void on_add(EntityID eid, std::size_t transform_idx, const PhysicsData& data) {
            if (m_lookup.find(eid) == m_lookup.end()) {
                m_ids.push_back(eid);
                m_transforms.push_back(transform_idx);
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
                std::swap(m_transforms[idx], m_transforms[last]);
                std::swap(m_ids[idx], m_ids[last]);
                m_lookup[m_ids[idx]] = idx;
            }

            m_data.pop_back();
            m_transforms.pop_back();
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
                m_snapshots[idx].snapshot[i] = PhysicsSnapshot{m_ids[i], m_data[i].pos, m_data[i].speed, m_transforms[i]};
            }

            m_last_snapshot_idx.store(idx, std::memory_order_release);
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
        std::vector<std::size_t>    m_transforms;
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
