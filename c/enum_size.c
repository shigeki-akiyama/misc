
#include <stdio.h>

enum int8 { INT8VALUE = 0xF };
enum int16 { INT16VALUE = 0xFFFF };
enum int32 { INT32VALUE = 0xFFFFFFFF };
enum int64 { INT64VALUE = 0xFFFFFFFFFFFFFFFF };

// cannot compile
//enum int128 { INT128VALUE = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF };

int main(void) {
    printf("sizeof(enum int8) = %zd\n", sizeof(enum int8));
    printf("sizeof(enum int16) = %zd\n", sizeof(enum int16));
    printf("sizeof(enum int32) = %zd\n", sizeof(enum int32));
    printf("sizeof(enum int64) = %zd\n", sizeof(enum int64));
//    printf("sizeof(enum int128) = %zd\n", sizeof(enum int128));
    return 0;
}
