#include "mbed.h"
#include "rtos.h"

Serial pc(USBTX, USBRX);

rtos::AnalogIn xAxis(A0);
rtos::AnalogIn yAxis(A1);

void thread_function(const void* data)
{
    int x,y;
    while (true) {
        Thread::wait(500);
        x = xAxis.read() * 1000; // float (0->1) to int (0-1000)
        y = yAxis.read() * 1000;
        pc.printf("\rThread 2: X=%3d, Y=%3d\n", x, y);
    }
}

int main() {
    int x,y;
    pc.baud(115200);
    pc.printf("Starting Example\n");
    Thread thread(thread_function);


    while (true) {
        Thread::wait(500);
        x = xAxis.read() * 1000; // float (0->1) to int (0-1000)
        y = yAxis.read() * 1000;
        pc.printf("\rX=%3d, Y=%3d\n", x, y);
    }
}
