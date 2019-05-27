#include <algorithm>
#include <cstdlib>
#include <deque>
#include <array>
#include <thread>
#include <iostream>
#include <cstring>
#include <iterator>
#include <random>
#include <algorithm>
#include <chrono>
#include <cstdlib>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>

// JSON
#include <nlohmann/json.hpp>

// C++ kafka
#include <cppkafka/cppkafka.h>

// HF FIX
#include <hffix.hpp>

// Pistache
#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>

using namespace Pistache;
using namespace boost::posix_time;
using namespace boost::gregorian;
using namespace cppkafka;

using json = nlohmann::json;


void printCookies(const Http::Request& req) {
    auto cookies = req.cookies();
    std::cout << "Cookies: [" << std::endl;
    const std::string indent(4, ' ');
    for (const auto& c: cookies) {
        std::cout << indent << c.name << " = " << c.value << std::endl;
    }
    std::cout << "]" << std::endl;
}

namespace Generic {

	void handleReady(const Rest::Request&, Http::ResponseWriter response) {
		response.send(Http::Code::Ok, "1");
	}

}

class StatsEndpoint {
public:
    StatsEndpoint(Address addr)
        : httpEndpoint(std::make_shared<Http::Endpoint>(addr))
    { }

    void init(size_t thr = 2) {
        auto opts = Http::Endpoint::options()
            .threads(thr)
            .flags(Tcp::Options::InstallSignalHandler);
        httpEndpoint->init(opts);
        setupRoutes();
    }

    void start() {
        httpEndpoint->setHandler(router.handler());
        httpEndpoint->serve();
    }

    void shutdown() {
        httpEndpoint->shutdown();
    }

private:
    void setupRoutes() {
        using namespace Rest;

        Routes::Post(router, "/insertorder", Routes::bind(&StatsEndpoint::doInsertOrder, this));
        Routes::Post(router, "/cancelorder", Routes::bind(&StatsEndpoint::doCancelOrder, this));
        Routes::Get(router, "/ready", Routes::bind(&Generic::handleReady));
        Routes::Get(router, "/auth", Routes::bind(&StatsEndpoint::doAuth, this));

    }

    void doInsertOrder(const Rest::Request& request, Http::ResponseWriter response) {
        auto order = request.body();
        Guard guard(OrderLock);
        
        std::string order_json_data = order.c_str();
        auto json_parsed_data = json::parse(order_json_data);
        std::cout << json_parsed_data.dump(4) << std::endl;
        std::string symbol = json_parsed_data.at("symbol");
        int quantity = json_parsed_data.at("quantity");
        int price = json_parsed_data.at("price");

        char buffer[1 << 13];
        
        // build fix message
        int seq_send(1);

        // TODO: change date to now
        ptime tsend(date(2019,5,17), time_duration(12,12,12));

        // We'll put a FIX Logon message in the buffer.
        hffix::message_writer logon(buffer, buffer + sizeof(buffer));

        logon.push_back_header("FIX.4.2"); // Write BeginString and BodyLength.

        // Logon MsgType.
        logon.push_back_string    (hffix::tag::MsgType, "A");
        logon.push_back_string    (hffix::tag::SenderCompID, "AAAA");
        logon.push_back_string    (hffix::tag::TargetCompID, "BBBB");
        logon.push_back_int       (hffix::tag::MsgSeqNum, seq_send++);
        logon.push_back_timestamp (hffix::tag::SendingTime, tsend);
        // No encryption.
        logon.push_back_int       (hffix::tag::EncryptMethod, 0);
        // 10 second heartbeat interval.
        logon.push_back_int       (hffix::tag::HeartBtInt, 10);

        logon.push_back_trailer(); // write CheckSum.

        // Now the Logon message is written to the buffer.

        // Add a FIX New Order - Single message to the buffer, after the Logon
        // message.
        hffix::message_writer new_order(logon.message_end(), buffer + sizeof(buffer));

        new_order.push_back_header("FIX.4.2");

        // New Order - Single
        new_order.push_back_string    (hffix::tag::MsgType, "D");
        // Required Standard Header field.
        new_order.push_back_string    (hffix::tag::SenderCompID, "AAAA");
        new_order.push_back_string    (hffix::tag::TargetCompID, "BBBB");
        new_order.push_back_int       (hffix::tag::MsgSeqNum, seq_send++);
        new_order.push_back_timestamp (hffix::tag::SendingTime, tsend);
        new_order.push_back_string    (hffix::tag::ClOrdID, "A1");
        // Automated execution.
        new_order.push_back_char      (hffix::tag::HandlInst, '1');
        // Ticker symbol OIH.
        new_order.push_back_string    (hffix::tag::Symbol, symbol);
        // Buy side.
        new_order.push_back_char      (hffix::tag::Side, '1');
        new_order.push_back_timestamp (hffix::tag::TransactTime, tsend);
        // 100 shares.
        new_order.push_back_int       (hffix::tag::OrderQty, quantity);
        // Limit order.
        new_order.push_back_char      (hffix::tag::OrdType, '2');
        // Limit price $500.01 = 50001*(10^-2). The push_back_decimal() method
        // takes a decimal floating point number of the form mantissa*(10^exponent).
        new_order.push_back_decimal   (hffix::tag::Price, price, -2);
        // Good Till Cancel.
        new_order.push_back_char      (hffix::tag::TimeInForce, '1');

        new_order.push_back_trailer(); // write CheckSum.
        
        std::string message(buffer);
        producer.produce(MessageBuilder("fix_message_queue").partition(0).payload(message));
        producer.flush();
            
        std::cout << buffer << std::endl;

		response.send(Http::Code::Created, order);
    }
    
	void doCancelOrder(const Rest::Request& request, Http::ResponseWriter response) {
        auto order = request.body();
        Guard guard(OrderLock);
        
        std::string order_json_data = order.c_str();
        auto json_parsed_data = json::parse(order_json_data);
        std::cout << json_parsed_data.dump(4) << std::endl;
        std::string symbol = json_parsed_data.at("symbol");
        int quantity = json_parsed_data.at("quantity");
        int price = json_parsed_data.at("price");
        std::cout << symbol << std::endl;
        char buffer[1 << 13];

        // build fix message
        int seq_send(1);

        // TODO: change date to now
        ptime tsend(date(2019,5,17), time_duration(12,12,12));

        // We'll put a FIX Logon message in the buffer.
        hffix::message_writer logon(buffer, buffer + sizeof(buffer));

        logon.push_back_header("FIX.4.2"); // Write BeginString and BodyLength.

        // Logon MsgType.
        logon.push_back_string    (hffix::tag::MsgType, "A");
        logon.push_back_string    (hffix::tag::SenderCompID, "AAAA");
        logon.push_back_string    (hffix::tag::TargetCompID, "BBBB");
        logon.push_back_int       (hffix::tag::MsgSeqNum, seq_send++);
        logon.push_back_timestamp (hffix::tag::SendingTime, tsend);
        // No encryption.
        logon.push_back_int       (hffix::tag::EncryptMethod, 0);
        // 10 second heartbeat interval.
        logon.push_back_int       (hffix::tag::HeartBtInt, 10);

        logon.push_back_trailer(); // write CheckSum.

        // Now the Logon message is written to the buffer.

        // Add a FIX New Order - Single message to the buffer, after the Logon
        // message.
        hffix::message_writer new_order(logon.message_end(), buffer + sizeof(buffer));

        new_order.push_back_header("FIX.4.2");

        // New Order - Single
        new_order.push_back_string    (hffix::tag::MsgType, "D");
        // Required Standard Header field.
        new_order.push_back_string    (hffix::tag::SenderCompID, "AAAA");
        new_order.push_back_string    (hffix::tag::TargetCompID, "BBBB");
        new_order.push_back_int       (hffix::tag::MsgSeqNum, seq_send++);
        new_order.push_back_timestamp (hffix::tag::SendingTime, tsend);
        new_order.push_back_string    (hffix::tag::ClOrdID, "A1");
        // Automated execution.
        new_order.push_back_char      (hffix::tag::HandlInst, '1');
        // Ticker symbol OIH.
        new_order.push_back_string    (hffix::tag::Symbol, symbol);
        // Buy side.
        new_order.push_back_char      (hffix::tag::Side, '1');
        new_order.push_back_timestamp (hffix::tag::TransactTime, tsend);
        // 100 shares.
        new_order.push_back_int       (hffix::tag::OrderQty, quantity);
        // Limit order.
        new_order.push_back_char      (hffix::tag::OrdType, '2');
        // Limit price $500.01 = 50001*(10^-2). The push_back_decimal() method
        // takes a decimal floating point number of the form mantissa*(10^exponent).
        new_order.push_back_decimal   (hffix::tag::Price, price, -2);
        // Good Till Cancel.
        new_order.push_back_char      (hffix::tag::TimeInForce, '1');

        new_order.push_back_trailer(); // write CheckSum.
        
        std::string message(buffer);
        producer.produce(MessageBuilder("fix_message_queue").partition(0).payload(message));
        producer.flush();
            
        std::cout << buffer << std::endl;

		response.send(Http::Code::Created, order);
    }

    void doAuth(const Rest::Request& request, Http::ResponseWriter response) {
        printCookies(request);
        response.cookies()
            .add(Http::Cookie("lang", "en-US"));
        response.send(Http::Code::Ok);
    }

    typedef std::mutex Lock;
    typedef std::lock_guard<Lock> Guard;
    Lock OrderLock;

    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router router;
    // Create the config
    Configuration config = {
        { "metadata.broker.list", "127.0.0.1:9092" }
    };
    // Create the producer
    Producer producer(config);
};

int main(int argc, char *argv[]) {
    Port port(8080);

    int thr = 2;

    if (argc >= 2) {
        port = std::stol(argv[1]);

        if (argc == 3)
            thr = std::stol(argv[2]);
    }

    Address addr(Ipv4::any(), port);

    std::cout << "Cores = " << hardware_concurrency() << std::endl;
    std::cout << "Using " << thr << " threads" << std::endl;

    StatsEndpoint stats(addr);

    stats.init(thr);
    stats.start();

    stats.shutdown();
}
