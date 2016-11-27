int main()
{
    asm volatile ("### begin __sync_syncronize ###\n\t":::);
    __sync_synchronize();
    asm volatile ("### end__sync_syncronize ###\n\t":::);
    return 0;
}
