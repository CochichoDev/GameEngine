#ifndef TRIPLE_BUFFER_H
#define TRIPLE_BUFFER_H

#include <array>
#include <utility>
#include <vector>
#include <atomic>

#include <assert.h>

template<typename T>
class TripleBuffer {
    public:
        TripleBuffer() = default;
        ~TripleBuffer() = default;

        void produce(const std::vector<T>& data) {
            uint8_t idx = spare_index();
            m_data[idx] = data;

            m_last_written.store(idx, std::memory_order_release);
        }
        void produce(std::vector<T>&& data) {
            uint8_t idx = spare_index();
            m_data[idx] = std::move(data);

            m_last_written.store(idx, std::memory_order_release);
        }

        /* Returns true if there is a new frame, false if the frame is the same as before */
        std::pair<const std::vector<T>&, bool> consume() {
            uint8_t idx = m_last_written.load(std::memory_order_acquire);

            if (idx == m_reading.load(std::memory_order_acquire)) {
                return std::make_pair(m_data[idx], false);
            }

            m_reading.store(idx, std::memory_order_release);
            return std::make_pair(m_data[idx], true);
        }

    private:
        uint8_t spare_index() const {
            uint8_t last = m_last_written.load(std::memory_order_acquire);
            uint8_t reading = m_reading.load(std::memory_order_acquire);

            for (uint8_t i = 0; i < 3; ++i) {
                if (i != last && i != reading) return i;
            }
            assert(false && "No spare buffer found"); 
            return 0; // unreachable
        }


    private:
        std::array<std::vector<T>, 3> m_data;

        std::atomic<uint8_t> m_last_written{0};
        std::atomic<uint8_t> m_reading{0};
};

#endif
