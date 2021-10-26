#ifndef ENTITY_ENTITY_API_TYPES
#define ENTITY_ENTITY_API_TYPES

#include <foundation/api_types.h>

// Represents an entity.
typedef union tm_entity_t
{
    // Internal -- represents an entity with a slot index and a generation counter that is incremented
    // when the slot is reused, allowing us to use the entity as a weak reference.
    struct
    {
        uint32_t index;
        uint32_t generation;
    };

    // Entity as uint64_t for simple comparison and assignment.
    uint64_t u64;
} tm_entity_t;

// [[tm_entity_t]] that means "no entity".
#define TM_NO_ENTITY ((tm_entity_t){ 0 })

// Represents a component type.
typedef struct tm_component_type_t
{
    // The index of the component type in the entity context.
    uint32_t index;
} tm_component_type_t;

// [[tm_component_type_t]] that means "no component".
#define TM_NO_COMPONENT_TYPE ((tm_component_type_t){ 0 })

struct tm_set_entity_t;

#endif