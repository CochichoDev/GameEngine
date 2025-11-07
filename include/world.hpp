#ifndef WORLD_H
#define WORLD_H

#include <thread>

class World {
    public:
        World() = default;
        ~World() = default;



    private:
        std::thread m_world_thread;
};

#endif
