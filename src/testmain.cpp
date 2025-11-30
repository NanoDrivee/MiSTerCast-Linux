#include <iostream>

#include "WaylandVideoCapture.h"

int main() {
    std::cout << "INIT START\n";

    WaylandVideoCapture capture = WaylandVideoCapture();

    // pw_thread_loop_wait(capture.getPwLoop());

    std::cout << "INIT COMPLETE\n";
}