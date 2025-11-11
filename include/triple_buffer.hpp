#ifndef TRIPLE_BUFFER_H
#define TRIPLE_BUFFER_H

#include <array>
#include <vector>
#include <atomic>

#include <assert.h>

template<typename T>
class TripleBuffer {

    uint8_t spare_index() const {
        uint8_t last = m_last_written.load(std::memory_order_acquire);
        uint8_t reading = m_reading.load(std::memory_order_acquire);

        for (uint8_t i = 0; i < 3; ++i) {
            if (i != last && i != reading) return i;
        }
        assert(false && "No spare buffer found"); 
        return 0; // unreachable
    }

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
    bool consume(std::vector<T>& out) {
        uint8_t idx = m_last_written.load(std::memory_order_acquire);

        out = m_data[idx];
        if (idx == m_reading.load(std::memory_order_acquire)) {
            return false;
        }

        m_reading.store(idx, std::memory_order_release);
        return true;
    }

    private:
        std::array<std::vector<T>, 3> m_data;

        std::atomic<uint8_t> m_last_written{0};
        std::atomic<uint8_t> m_reading{0};
};

#endif
