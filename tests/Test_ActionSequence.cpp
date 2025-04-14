// file   : Test_ActionSequence.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include <catch2/catch_all.hpp>

#include "../LedEffects/ActionSequence.hpp"

#include <asio.hpp>
#include <chrono>
#include <thread>
#include <vector>
#include <future>
#include <iostream>

using namespace std::chrono_literals;

TEST_CASE("Basic sequence execution", "[ActionSequence]")
{
    asio::io_context io;
    asio::strand<asio::any_io_executor> strand(io.get_executor());
    esp32pp::ActionSequence seq(strand);

    std::vector<int> log;
    std::promise<void> promiseComplete;
    auto fut = promiseComplete.get_future();

    seq.addAction([&] { log.push_back(1); }, 10ms);
    seq.addAction([&] { log.push_back(2); }, 10ms);
    seq.setNoRepeat();
    seq.setOnComplete(
        [&] {
            log.push_back(99);
            promiseComplete.set_value();
        }
    );
    seq.start();

    std::thread t([&] { io.run(); });

    fut.wait();
    io.stop();
    t.join();

    REQUIRE(log.size() == 3);
    REQUIRE(log[0] == 1);
    REQUIRE(log[1] == 2);
    REQUIRE(log[2] == 99);
}

TEST_CASE("Repeat count execution", "[ActionSequence]")
{
    asio::io_context io;
    asio::strand<asio::io_context::executor_type> strand(io.get_executor());
    esp32pp::ActionSequence seq(strand);

    int counter = 0;
    std::promise<void> promiseComplete;
    auto fut = promiseComplete.get_future();

    seq.addAction([&] { counter++; }, 5ms);
    seq.setRepeatCount(3);
    seq.setOnComplete([&] {
        counter += 100;
        promiseComplete.set_value();
    });

    seq.start();

    std::thread t([&] { io.run(); });
    fut.wait();
    io.stop();
    t.join();

    REQUIRE(counter == 103);
}

TEST_CASE("Restart in onComplete", "[ActionSequence]")
{
    asio::io_context io;
    asio::strand<asio::io_context::executor_type> strand(io.get_executor());
    esp32pp::ActionSequence seq(strand);

    auto phases = 0;
    std::promise<void> promiseComplete;
    auto fut = promiseComplete.get_future();

    seq.addAction([&] { phases++; }, 10ms);
    seq.setOnComplete([&] {
        if (phases == 1) {
            seq.start();
        } else {
            promiseComplete.set_value();
        }
    });
    seq.start();

    std::thread t([&] { io.run(); });
    fut.wait();
    io.stop();
    t.join();

    REQUIRE(phases == 2);
}
