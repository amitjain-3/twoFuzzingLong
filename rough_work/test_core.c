#include <stdio.h>
#include <libcpuid.h>

int main(void)
{
    struct cpu_raw_data_t raw;
    struct cpu_id_t data;

    cpuid_get_raw_data(&raw);
    cpu_identify(&raw, &data);
    printf("No. of Physical Core(s) : %d\n", data.num_cores);
    return 0;
}