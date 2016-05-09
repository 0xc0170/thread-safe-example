#include <mbed.h>
#include <rtos.h>
#include <ITC/TaskQueue.h>
#include <ITC/Expected.h>
#include <ITC/defer.h>

Serial pc(USBTX, USBRX);

TaskQueue<5> queue;

int add(int lhs, int rhs) {
    pc.printf("add %i %i from thread %p\r\n", lhs, rhs, osThreadGetId());
    return lhs + rhs;
}

void thread_function(const void* data) {
    queue.run();
}

int main() {
    pc.baud(115200);
    pc.printf("Starting Example\n");
    int lhs = 1;
    int rhs = 1;

    Thread thread(thread_function);

    while (true) {
        Thread::wait(500);
        pc.printf("posting from thread %p\r\n", osThreadGetId());

        // defer execution of add(lhs,rhs) to the queue.
        itc::Expected<int, bool> res = itc::defer_call(
            queue,
            mbed::util::FunctionPointer2<int,int,int>(add).bind(lhs, rhs)
        );
        if (res.has_value()) {
            printf("from thread %p: result %i\r\n", osThreadGetId(), res.value());
        } else {
            printf("from thread %p: error\r\n", osThreadGetId());
        }
        lhs++;
        rhs++;
    }
    return 0;
}
