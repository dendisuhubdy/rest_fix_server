#include <base64.hpp>
#include <filter.hpp>
#include <main.hpp>

#include <pistache/endpoint.h>
#include <pistache/router.h>

#include <memory>
#include <csignal>

using namespace Pistache;
using namespace Pistache::Http;
using namespace Pistache::Rest;
using namespace Core;

namespace Application {


  class RestApplication {
    public:
    RestApplication (Pistache::Address addr): addr_(addr), endpoint_(Pistache::Http::Endpoint(addr)) {

      auto opts = Pistache::Http::Endpoint::options()
        .threads(4)
        .flags(Pistache::Tcp::Options::ReuseAddr)
        .maxPayload( 1e9 );

      for ( auto & handler : routes_ ) {

        std::string p;
        Pistache::Http::Method m;
        Pistache::Rest::Route::Handler h;

        p = handler.first;
        std::tie(m, h) = handler.second;

        router_.addRoute(m,p,h);

      }

      endpoint_.setHandler(router_.handler());
      endpoint_.init(opts);

    }

    RestApplication (): RestApplication( Pistache::Address(Pistache::Ipv4::any(), Pistache::Port(8080)) ) {}
    
    void listen() {
      endpoint_.serve();
    }

    void shutdown() {
      endpoint_.shutdown();
    }

    protected:
    private:

    Pistache::Address addr_;
    Pistache::Http::Endpoint endpoint_;
    Pistache::Rest::Router router_;

    std::unordered_map < std::string, std::tuple< Pistache::Http::Method, Pistache::Rest::Route::Handler > > routes_ = {
      
      {
        "/",
        std::tuple<Pistache::Http::Method, Pistache::Rest::Route::Handler>(
          Pistache::Http::Method::Get,
          
          [this](const Pistache::Http::Request & req, Pistache::Http::ResponseWriter res) -> Pistache::Rest::Route::Result
          {
            std::cout << __PRETTY_FUNCTION__ << std::endl;
            res.send(Pistache::Http::Code::Ok);
          }

        )
      },

      {
        "/insertorder",
        std::tuple<Pistache::Http::Method, Pistache::Rest::Route::Handler>
        (
          Pistache::Http::Method::Post,
          [this](const Pistache::Http::Request & req, Pistache::Http::ResponseWriter res) -> Pistache::Rest::Route::Result
          {
            std::cout << __PRETTY_FUNCTION__ << std::endl;
            auto reqBody = req.body();
            res.send(Pistache::Http::Code::Ok, reqBody);
          }
        )
      }
    };
  };

}

using namespace Application;

void signalHandler(int signal);

static Pistache::Address addr(Pistache::Ipv4::any(), Pistache::Port(8080));
static auto app = Application::RestApplication(addr);

int main(int argc, char ** argv) {

    std::vector<std::string> arguments(argv + 1, argv + argc);
    signal(SIGINT, signalHandler);
    signal(SIGKILL, signalHandler);
    signal(SIGTERM, signalHandler);
    app.listen();
    return 0;
}

void signalHandler(int signal) {
  std::cout << __PRETTY_FUNCTION__ << std::endl;
  app.shutdown();
  exit(signal);
}
