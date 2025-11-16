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

        ~MPSCQueue() {
            Node* curr = m_tail;

            while (curr != nullptr) {
                Node* next = curr->next.load(std::memory_order_relaxed);
                delete curr;
                curr = next;
            }
        }

        /* Thread safe for multiple producers */
        void enqueue(const T& v) noexcept {
            Node* stub = new Node(v);
            stub->next.store(nullptr, std::memory_order_relaxed);

            Node* prev = m_head.exchange(stub, std::memory_order_acq_rel);
            prev->next.store(stub, std::memory_order_release);
        }

        bool dequeue(T& v) noexcept {
            Node* next = m_tail->next.load(std::memory_order_acquire);
            if (!next) return false;

            v = std::move(next->value);

            delete m_tail;
            m_tail = next;

            return true;
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
