// directly use Linux I/O API
#define io_creat        creat
#define io_link         link

int io_open(char*, int, mode_t);
