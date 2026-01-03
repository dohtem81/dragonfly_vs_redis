#include <hiredis/hiredis.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <atomic>

std::string get_env(const char* name, const std::string& default_value) {
    const char* value = std::getenv(name);
    return value ? std::string(value) : default_value;
}

int global_max_messages = -1;

std::string get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

void subscribe_to_channel(const std::string& name, const std::string& host, 
                         int port, const std::string& channel) {
    std::cout << "[" << name << "] Subscriber starting..." << std::endl;
    std::cout << "[" << name << "] Host: " << host << ":" << port << std::endl;
    std::cout << "[" << name << "] Channel: " << channel << std::endl;

    // received message count
    int message_count = 0;
    std::atomic<bool> running(true);
    std::atomic<int> total_messages_received(0);


    // timestamp of start set as minimum
    auto start_time = std::chrono::system_clock::time_point::min();
    auto finish_time = std::chrono::system_clock::time_point::min();
    
    // Connect
    redisContext* context = redisConnect(host.c_str(), port);
    if (context == nullptr || context->err) {
        if (context) {
            std::cerr << "[" << name << "] Connection error: " << context->errstr << std::endl;
            redisFree(context);
        } else {
            std::cerr << "[" << name << "] Connection error: can't allocate redis context" << std::endl;
        }
        return;
    }
    
    std::cout << "[" << name << "] Connected successfully!" << std::endl;
    
    // Subscribe to channel
    redisReply* reply = (redisReply*)redisCommand(context, "SUBSCRIBE %s", channel.c_str());
    if (reply == nullptr) {
        std::cerr << "[" << name << "] Subscribe error: " << context->errstr << std::endl;
        redisFree(context);
        return;
    }
    freeReplyObject(reply);
    
    std::cout << "[" << name << "] Subscribed to channel: " << channel << std::endl;
    
    // Listen for messages
    while (running) {       
        redisReply* message_reply;
        if (redisGetReply(context, (void**)&message_reply) != REDIS_OK) {
            std::cerr << "[" << name << "] Error getting reply: " << context->errstr << std::endl;
            break;
        }
        
        if (message_reply->type == REDIS_REPLY_ARRAY && message_reply->elements == 3) {
            if (std::string(message_reply->element[0]->str) == "message") {
                std::string received_channel = message_reply->element[1]->str;
                std::string message = message_reply->element[2]->str;
                std::string timestamp = get_timestamp();
                if (start_time == std::chrono::system_clock::time_point::min()) {
                    start_time = std::chrono::system_clock::now();
                }
                finish_time = std::chrono::system_clock::now();
                total_messages_received++;
            }
        }
        
        freeReplyObject(message_reply);

        // Check if we've reached the max message limit
        if (total_messages_received >= global_max_messages) {
            std::cout << "[" << name << "] Reached max messages limit" << std::endl;
            break;
        }        
    }
    
    redisFree(context);
    std::cout << "[" << name << "] Subscriber stopped. Completed in " 
        << std::chrono::duration_cast<std::chrono::milliseconds>(finish_time - start_time).count() << " ms." 
        << std::endl;
}

int main(int argc, char* argv[]) {

    int max_messages = std::stoi(get_env("MESSAGE_COUNT", "1000"));
    int thread_count = std::stoi(get_env("THREAD_COUNT", "50"));
    global_max_messages = max_messages * thread_count;
    
    // Get Dragonfly configuration
    std::string dragonfly_host = get_env("DRAGONFLY_HOST", "localhost");
    int dragonfly_port = std::stoi(get_env("DRAGONFLY_PORT", "6379"));
    std::string dragonfly_channel = get_env("TEST_CHANNEL", "testchannel");
    
    // Get Redis configuration
    std::string redis_host = get_env("REDIS_HOST", "localhost");
    int redis_port = std::stoi(get_env("REDIS_PORT", "6379"));
    std::string redis_channel = get_env("TEST_CHANNEL", "testchannel");

    std::cout << "Global max messages for test: " << global_max_messages << std::endl;
    std::cout << "Multi-Subscriber starting..." << std::endl;
    std::cout << "Will subscribe to both Dragonfly and Redis channels" << std::endl;
    if (global_max_messages > 0) {
        std::cout << "Will receive up to " << global_max_messages << " total messages" << std::endl;
    } else {
        std::cout << "Will receive messages indefinitely" << std::endl;
    }
    std::cout << std::endl;
    
    // Create threads for each subscription
    std::thread dragonfly_thread(subscribe_to_channel, "DRAGONFLY", 
                                 dragonfly_host, dragonfly_port, dragonfly_channel);
    std::thread redis_thread(subscribe_to_channel, "REDIS", 
                            redis_host, redis_port, redis_channel);
    
    // Wait for threads to complete
    dragonfly_thread.join();
    redis_thread.join();
    
    std::cout << std::endl;
    std::cout << "Multi-Subscriber finished."  << std::endl;
    
    return 0;
}
