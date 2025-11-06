#ifndef MPSC_H
#define MPSC_H

#include <atomic>

template<typename T>
class MPSCQueue {
    public:
        MPSCQueue() {
            Node* stub = new Node(T{});
            m_head.store(stub, std::memory_order_relaxed);
            m_tail = stub;
        }

        void enqueue(T& v) noexcept {
            Node* stub = new Node(v);
            stub->next.store(nullptr, std::memory_order_relaxed);

            Node* prev = m_head.exchange(stub, std::memory_order_acq_rel);
            prev->next.store(stub, std::memory_order_release);
        }

        T* dequeue() noexcept {
            
        }

    private:
        struct Node {
            T                   value;
            std::atomic<Node*>  next;

            Node(const T& v) : value (v) {}
        };
        
    private:
        std::atomic<Node*>  m_head;
        Node*               m_tail;
};

#endif
