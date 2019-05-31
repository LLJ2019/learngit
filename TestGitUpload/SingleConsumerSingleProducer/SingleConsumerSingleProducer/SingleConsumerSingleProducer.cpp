// SingleConsumerSingleProducer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//#include <synchapi.h>
#include <iostream>
#include <condition_variable>
#include <mutex>
#include <thread>


static const int REPOSITORY_SIZE = 10;
static const int PRODUCE_SIZE = 1000;

struct Repository
{
    int buffer[REPOSITORY_SIZE];
    size_t read_pos;
    size_t write_pos;
    std::mutex mtx;
    std::condition_variable repo_not_full;
    std::condition_variable repo_not_empty;
}g_repository;

typedef struct Repository REPOSITORY;

void Producer(REPOSITORY *rep, int index)
{
    if (!rep)
        return;

    std::unique_lock<std::mutex> lock(rep->mtx);
    while ((rep->write_pos + 1) % REPOSITORY_SIZE == rep->read_pos)
    {
        std::cout << "Produce is Waiting for empty Repository..." << std::endl;
        rep->repo_not_full.wait(lock);
    }

    rep->buffer[rep->write_pos++] = index;

    if (rep->write_pos == REPOSITORY_SIZE)
        rep->write_pos = 0;

    rep->repo_not_empty.notify_all();
    lock.unlock();
}

int Consumer(REPOSITORY *rep)
{
    if (!rep)
        return -1;

    std::unique_lock<std::mutex> lock(rep->mtx);
    while (rep->read_pos == rep->write_pos)
    {
        std::cout << "Consumer is Waiting for Repository" << std::endl;
        rep->repo_not_empty.wait(lock);
    }

    int data = rep->buffer[rep->read_pos++];

    if (rep->read_pos >= REPOSITORY_SIZE)
        rep->read_pos = 0;

    rep->repo_not_full.notify_all();
    lock.unlock();

    return data;
}


void ProducerTask()
{
    for (size_t i = 1; i < PRODUCE_SIZE; i++)
    {
        //Sleep(1);
        std::cout << "Produce the " << i << " ^th repo..." << std::endl;
        Producer(&g_repository, i);
    }
}

void ConsumerTask()
{
    static int index = 0;
    while (1)
    {
        //Sleep(1);
        std::cout << "Consume the " << Consumer(&g_repository) << "^th repo..." << std::endl;
        if (++index == PRODUCE_SIZE) 
            break;
    }
}

void InitRepository(REPOSITORY* rep)
{
    rep->read_pos = 0;
    rep->write_pos = 0;
}


//Test git upload
int _tmain(int argc, _TCHAR* argv[])
{
    InitRepository(&g_repository);

    std::thread producer(ProducerTask);
    std::thread consumer(ConsumerTask);

    producer.join();
    consumer.join();

    getchar();

	return 0;
}

