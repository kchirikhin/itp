#include <driver.h>

int main(int argc, char *argv[]) {
    Program::Driver d(argc, argv);
    d.run();

    return 0;
}
