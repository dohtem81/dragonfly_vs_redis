#include <hiredis/hiredis.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <omp.h>

std::string get_env(const char *name, const std::string &default_value)
{
    const char *value = std::getenv(name);
    return value ? std::string(value) : default_value;
}

void print_usage(const char *program_name)
{
    std::cerr << "Usage: " << program_name << " <host> <port> <channel> [name] [max_messages]" << std::endl;
    std::cerr << "  host         : Server hostname (e.g., dragonfly, redis, localhost)" << std::endl;
    std::cerr << "  port         : Server port (e.g., 6379)" << std::endl;
    std::cerr << "  channel      : Channel name to publish to" << std::endl;
    std::cerr << "  name         : Optional display name (default: Publisher)" << std::endl;
    std::cerr << "  max_messages : Optional max number of messages to send (default: 5000)" << std::endl;
    std::cerr << "  thread_count : Optional number of threads (default: 50)" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Example: " << program_name << " dragonfly 6379 my-channel Dragonfly-Pub 100" << std::endl;
}

int runTestPublish(const std::string &host, int port, const std::string &channel,
                   const std::string &name = "Publisher", int max_messages = 5000)
{
    int errorCode = 0;
    
    // Connect to server
    redisContext *context = redisConnect(host.c_str(), port);
    if (context == nullptr || context->err)
    {
        if (context)
        {
            std::cerr << "[" << name << "] Connection error within task: " << context->errstr << std::endl;
            redisFree(context);
        }
    }
    else
    {
        int message_count = 0;
        // snapshot timestamp, store in local variable
        auto start_time = std::chrono::system_clock::now();
        std::cout << "[" << name << "] Publisher connected successfully!" << std::endl;
        while (max_messages < 0 || message_count < max_messages)
        {
            message_count++;

            std::string message(std::stoi(get_env("MESSAGE_SIZE", "1000")), '\0');
            redisReply *reply = (redisReply *)redisCommand(context,
                                                           "PUBLISH %s %s", channel.c_str(), message.c_str());

            if (reply == nullptr)
            {
                std::cerr << "[" << name << "] Publish error: " << context->errstr << std::endl;
                break;
            }

            freeReplyObject(reply);
        }
        auto finish_time = std::chrono::system_clock::now();
        std::cout << "[" << name << "] Published " << message_count << " messages in "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(finish_time - start_time).count()
                  << " ms." << std::endl;
    }

    redisFree(context);
    return 0;
}

int main(int argc, char *argv[])
{
    std::string host;
    std::string name = "Publisher";
    int port;
    std::string channel;
    int max_messages = 5000;
    int thread_count = 50;

    // Parse command line arguments
    if (argc >= 4)
    {
        host = argv[1];
        port = std::stoi(argv[2]);
        channel = argv[3];
        if (argc >= 5)
        {
            name = argv[4];
        }
        if (argc >= 6)
        {
            max_messages = std::stoi(argv[5]);
        }
        if (argc >= 7)
        {
            thread_count = std::stoi(argv[6]);
        }
    }

    std::cout << "[" << name << "] Starting..." << std::endl;
    std::cout << "[" << name << "] Host: " << host << ":" << port << std::endl;
    std::cout << "[" << name << "] Channel: " << channel << std::endl;

    // Connect to server as a test publisher
    redisContext *context = redisConnect(host.c_str(), port);
    if (context == nullptr || context->err)
    {
        if (context)
        {
            std::cerr << "[" << name << "] Connection error: " << context->errstr << std::endl;
            redisFree(context);

            return 1;
        }
    }
    // clean up after test
    redisFree(context);

    // start threads to publish messages - each thread will create its own connection
    std::cout << "[" << name << "] Starting " << thread_count << " publisher threads" << std::endl;
    // each thread will run runtestpublish
    #pragma omp parallel for num_threads(thread_count)
    for (int i = 0; i < thread_count; i++)
    {
        std::string thread_name = name + "-Thread-" + std::to_string(i);
        runTestPublish(host, port, channel, thread_name, max_messages);
    }   
    std::cout << "[" << name << "] All publisher threads completed." << std::endl;

    
    return 0;
}