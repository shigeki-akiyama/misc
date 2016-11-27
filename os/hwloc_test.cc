#include <cstdio>
#include <hwloc.h>

int main(int argc, const char ** argv)
{
    hwloc_topology_t topo;

    hwloc_topology_init(&topo);
    hwloc_topology_load(topo);

    auto depth = hwloc_topology_get_depth(topo);

    auto last = depth - 1;
    auto n_objs = hwloc_get_nbobjs_by_depth(topo, last);
    for (int i = 0; i < n_objs; i++) {
        auto obj = hwloc_get_obj_by_depth(topo, last, i);

        char type_str[1024];
        hwloc_obj_type_snprintf(type_str, sizeof(type_str), obj, 0);

        printf("index %u: type = %s, logical_index = %d, os_index = %d\n",
               i, type_str, obj->logical_index, obj->os_index);
    }

    hwloc_topology_destroy(topo);
    return 0;
}

