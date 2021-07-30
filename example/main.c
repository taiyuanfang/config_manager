#include <stdio.h>
#include <stdlib.h>
#include <config_manager.h>
//------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    system("echo '{}' > test.json");
    cmc_set("test.json", "/A/B/C", "ABC");
    cmc_set("test.json", "/a/b/c", "abc");
    cmc_set("test.json", "/1/2/3", "123");
    cmc_set("test.json", "/array[0]", "0");
    cmc_set_i("test.json", "/array[1]", 1);
    cmc_set_f("test.json", "/array[2]", 2.3456789);
    cmc_save("test.json");
    system("cat test.json");
    return 0;
}
//------------------------------------------------------------------------------
