#ifndef ENTITY_H
#define ENTITY_H

#include <cstdint>

using EntityID = uint32_t;

class EntityManager {
    public:
        EntityID create() {
            return s_entity_counter++;
        }
        
    private:
        static EntityID s_entity_counter;
};

#endif
