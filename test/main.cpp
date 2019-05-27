#include <iostream>
#include <random>
#include <thread>

#include "lockfree.h"
#include "common/log.h"
#include "test/test.h"

LockFreeQueue<Test> lf_queue;

int main(int argc, char** argv) {
    std::thread a_thread[20];
    for (int32_t i = 0; i < 20; ++i) {
        a_thread[i] = std::thread([=](int i){
            Test t;
            for (int32_t j = i*10; j < (i+1) * 10; ++j) {
                t.data_ = j;
                lf_queue.push(t);
            }
            Test tmp;
            while (lf_queue.pop(tmp)) {
                char log[128] = {0};
                snprintf(log, sizeof(log), "acquire %d", tmp.data_);
                INFO_LOG(log);
            }
        }, i);
    }

    for (int32_t i = 0; i < 20; ++i) {
        a_thread[i].join();
    }
    std::cout << "execute finished!" << std::endl;
    return 0;
}