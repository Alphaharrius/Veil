#include <iostream>
#include <thread>
#include <string>
#include <utility>

#include "queue.h"

using namespace veil::threading;

Queue queue_0;
Queue queue_1;
Queue queue_2;

struct Params {
    std::string index;
    QueueClient *client;
    uint32 *count;
    uint32 iteration_count;

    Params(std::string index,
           QueueClient *client,
           uint32 *count,
           uint32 iteration_count) : index(std::move(index)),
                                     client(client),
                                     count(count),
                                     iteration_count(iteration_count) {}
};

void nested_with_count(Params *params) {
    for (int i = 0; i < params->iteration_count; i++) {
        params->client->wait(queue_0);
        params->client->wait(queue_1);
        params->client->wait(queue_2);
        (*params->count)++;
        params->client->exit(queue_0);
        params->client->exit(queue_1);
        params->client->exit(queue_2);
    }
}

void nested_with_count_sleep_1s(Params *params) {
    for (int i = 0; i < params->iteration_count; i++) {
        params->client->wait(queue_0);
        params->client->wait(queue_1);
        params->client->wait(queue_2);
        (*params->count)++;
        std::cout << "\tClient: " << params->index << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        params->client->exit(queue_0);
        params->client->exit(queue_1);
        params->client->exit(queue_2);
    }
}

void nested_reentrance_with_count(Params *params) {
    for (int i = 0; i < params->iteration_count; i++) {
        params->client->wait(queue_0);
        params->client->wait(queue_1);
        params->client->wait(queue_0);
        params->client->wait(queue_2);
        params->client->wait(queue_0);
        params->client->wait(queue_1);
        (*params->count)++;
        params->client->exit(queue_0);
        params->client->exit(queue_1);
        params->client->exit(queue_0);
        params->client->exit(queue_0);
        params->client->exit(queue_2);
        params->client->exit(queue_1);
    }
}

void nested_reentrance_with_count_sleep_1s(Params *params) {
    for (int i = 0; i < params->iteration_count; i++) {
        params->client->wait(queue_0);
        params->client->wait(queue_1);
        params->client->wait(queue_0);
        params->client->wait(queue_2);
        params->client->wait(queue_0);
        params->client->wait(queue_1);
        (*params->count)++;
        std::cout << "\tClient: " << params->index << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        params->client->exit(queue_0);
        params->client->exit(queue_1);
        params->client->exit(queue_0);
        params->client->exit(queue_0);
        params->client->exit(queue_2);
        params->client->exit(queue_1);
    }
}

int main() {
    QueueClient client_0;
    QueueClient client_1;
    QueueClient client_2;

    uint32 count = 0;
    uint32 iteration_count = 4096;
    std::cout << "Begin test on single client nested with count, expects: count = " << iteration_count << std::endl;
    {
        Params p("", &client_0, &count, iteration_count);
        std::thread thread(nested_with_count, &p);
        thread.join();
    }
    std::cout << "Test result: count = " << count << std::endl;

    count = 0;

    std::cout << "Begin test on multiple client nested with count, expects: count = " << iteration_count * 3
              << std::endl;
    {
        Params p_0("red", &client_0, &count, iteration_count);
        Params p_1("green", &client_1, &count, iteration_count);
        Params p_2("blue", &client_2, &count, iteration_count);
        std::thread thread_0(nested_with_count, &p_0);
        std::thread thread_1(nested_with_count, &p_1);
        std::thread thread_2(nested_with_count, &p_2);
        thread_0.join();
        thread_1.join();
        thread_2.join();
    }
    std::cout << "Test result: count = " << count << std::endl;

    count = 0;

    std::cout << "Begin reentrance test on multiple client nested with count, expects: count = " << iteration_count * 3
              << std::endl;
    {
        Params p_0("red", &client_0, &count, iteration_count);
        Params p_1("green", &client_1, &count, iteration_count);
        Params p_2("blue", &client_2, &count, iteration_count);
        std::thread thread_0(nested_reentrance_with_count, &p_0);
        std::thread thread_1(nested_reentrance_with_count, &p_1);
        std::thread thread_2(nested_reentrance_with_count, &p_2);
        thread_0.join();
        thread_1.join();
        thread_2.join();
    }
    std::cout << "Test result: count = " << count << std::endl;

    count = 0;
    iteration_count = 3;

    std::cout << "Begin test on multiple client nested with count and log, expects: count = " << iteration_count * 3
              << std::endl;
    {
        Params p_0("red", &client_0, &count, iteration_count);
        Params p_1("green", &client_1, &count, iteration_count);
        Params p_2("blue", &client_2, &count, iteration_count);
        std::thread thread_0(nested_with_count_sleep_1s, &p_0);
        std::thread thread_1(nested_with_count_sleep_1s, &p_1);
        std::thread thread_2(nested_with_count_sleep_1s, &p_2);
        thread_0.join();
        thread_1.join();
        thread_2.join();
    }
    std::cout << "Test result: count = " << count << std::endl;

    count = 0;

    std::cout << "Begin reentrance test on multiple client nested with count and log, expects: count = "
              << iteration_count * 3
              << std::endl;
    {
        Params p_0("red", &client_0, &count, iteration_count);
        Params p_1("green", &client_1, &count, iteration_count);
        Params p_2("blue", &client_2, &count, iteration_count);
        std::thread thread_0(nested_reentrance_with_count_sleep_1s, &p_0);
        std::thread thread_1(nested_reentrance_with_count_sleep_1s, &p_1);
        std::thread thread_2(nested_reentrance_with_count_sleep_1s, &p_2);
        thread_0.join();
        thread_1.join();
        thread_2.join();
    }
    std::cout << "Test result: count = " << count << std::endl;

    return 0;
}
