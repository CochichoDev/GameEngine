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
            while (true) {
                uint32_t old = m_state.load(std::memory_order_relaxed);

                uint16_t last = unpack_last(old);
                uint16_t reading = unpack_reading(old);

                uint16_t idx = 0;
                for (; idx < 3; ++idx) {
                    if (idx != last && idx != reading) break;
                }

                m_data[idx] = data;
                uint32_t desired = pack(idx, reading);
                if (m_state.compare_exchange_weak(old, desired, 
                            std::memory_order_release, 
                            std::memory_order_relaxed)) 
                {
                    return;
                }
            }
        }

        /* Returns true if there is a new frame, false if the frame is the same as before */
        std::pair<const std::vector<T>&, bool> consume() {
            while (true) {
                uint32_t old = m_state.load(std::memory_order_relaxed);

                uint16_t last = unpack_last(old);
                uint16_t reading = unpack_reading(old);
                
                if (last == reading) {
                    return std::pair<const std::vector<T>&, bool>(m_data[last], false);
                }

                uint32_t desired = pack(last, last);
                if (m_state.compare_exchange_weak(old, desired, 
                            std::memory_order_acquire, 
                            std::memory_order_relaxed)) {

                    return std::pair<const std::vector<T>&, bool>(m_data[last], true);

                }

            }
        }
    
    private:

    private:
        uint32_t pack(uint16_t last, uint16_t reading) {
            return ((static_cast<uint32_t>(last) << 0x10) | static_cast<uint32_t>(reading));
        }
        uint16_t unpack_reading(uint32_t state) {
            return (state & 0xFFFF);
        }
        uint16_t unpack_last(uint32_t state) {
            return (state >> 0x10);
        }


    private:
        std::array<std::vector<T>, 3> m_data;

        std::atomic<uint32_t> m_state = 0;
};

#endif
