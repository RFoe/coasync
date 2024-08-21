<!-- Improved compatibility of back to top link: See: https://github.com/RFoe/coasync -->
<a id="readme-top"></a>


<h3 align="center">coasync</h3>

  <p align="center">
    Asynchronous network library that supports coroutines
    <br />
    <a href="https://github.com/RFoe/coasync"><strong>Explore the docs »</strong></a>
    <br />
    <br />
    <a href="https://github.com/RFoe/coasync/tree/master/test">View Demo</a>
    ·
    <a href="https://github.com/RFoe/coasync/issues/new?labels=bug&template=bug-report---.md">Report Bug</a>
    ·
    <a href="https://github.com/RFoe/coasync/issues/new?labels=enhancement&template=feature-request---.md">Request Feature</a>
  </p>
</div>



<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#usage">Usage</a></li>
    <li><a href="#roadmap">Roadmap</a></li>
    <li><a href="#contributing">Contributing</a></li>
    <li><a href="#license">License</a></li>
    <li><a href="#contact">Contact</a></li>
    <li><a href="#acknowledgments">Acknowledgments</a></li>
  </ol>
</details>



<!-- ABOUT THE PROJECT -->
## About The Project

Coasync is a cross-platform C++ library for network and low-level I/O programming that provides developers with a consistent asynchronous model using a modern C++ 20 approach.

`RFoer`, `coasync`, `wangxlang3@mail2.sysu.edu.com`, `C++/coasync`


<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- GETTING STARTED -->
## Getting Started

header only, does not rely on any third party library, does not need to compile separately, add the include/coasync path in the Include directory of the project, in the source code #include <coasync/...> can be used;

### Prerequisites
* **C++20:** Robust support for most language features in Cpp20[ranges, concepts, coroutines].
* **Winsock:** If the project is built on the windows platform, you need to dynamically link the winsock2 network library
* compile command
  ```sh
  -std=c++20
  ```
* linkage command
  ```sh
  -lws2_32
  ```

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- USAGE EXAMPLES -->
## Usage

#### when_any/when_any algorithm
``` cpp

#include "../include/coasync/when_all.hpp"
#include "../include/coasync/when_any.hpp"
#include "../include/coasync/execution_context.hpp"
#include "../include/coasync/functional.hpp"
#include "../include/coasync/this_coro.hpp"

using namespace coasync;
using std::chrono::operator""s;

awaitable<int> delay(int seconds) /// sleep some seconds
{
  co_await sleep_for(std::chrono::seconds(seconds));
  std::puts("sleep awaiken");
  co_return co_await this_coro::id; /// return coroutine-id
}
awaitable<void> test()
{
  for(unsigned int i {}; i < 10; i ++)
    {
      auto [a, b, c] = co_await when_all(delay(1), delay(2), delay(3));
      std::printf("when_all results: [%d, %d, %d]\n", a, b, c);
	  
      auto result = co_await when_any(delay(1), delay(2), delay(3));
      std::printf("when_any: index: %llu\n", result.index());
      std::visit([](int value)
      {
        std::printf("when_any: result: %d\n", value);
      }, result);
    }
  co_return;
}
int main()
{
  execution_context context{3};
  /// Initiate three child threads
  co_spawn(context, test(), use_detach);
  context.loop();
}
``` 

#### rpc_client

``` cpp
#include "../include/coasync/execution_context.hpp"
#include "../include/coasync/this_coro.hpp"
#include "../include/coasync/co_spawn.hpp"
#include "../include/coasync/net/socket.hpp"
#include "../include/coasync/net/protocol.hpp"
#include "../include/coasync/net/endpoint.hpp"
#include "../include/coasync/net/rpc/rpc_client.hpp"
using namespace coasync;

awaitable<void> test() noexcept
{
  net::tcp::socket socket { co_await this_coro::context, net::tcp::v4() };
  co_await socket.connect(net::tcp::endpoint{net::address_v4::loopback(), 10086});
  net::rpc::rpc_client s { std::move(socket) };
  std::printf("[888 + 999 = %d]\n", co_await s.call<int>("add", 888, 999));
}

int main()
{
  execution_context ctx;
  co_spawn(ctx, test(), use_detach);
  ctx.loop();
}

```
#### rpc_server

``` cpp
#include "../include/coasync/execution_context.hpp"
#include "../include/coasync/this_coro.hpp"
#include "../include/coasync/net/endpoint.hpp"
#include "../include/coasync/net/protocol.hpp"
#include "../include/coasync/net/acceptor.hpp"
#include "../include/coasync/net/rpc/rpc_server.hpp"
using namespace coasync;
awaitable<void> test() noexcept {
  net::tcp::acceptor acceptor
  {
    co_await this_coro::context,
    net::tcp::endpoint(net::address_v4::loopback(), 10086)
  };
  net::rpc::rpc_server server(std::move(acceptor));
  server.bind("add", [](int a, int b) -> int { return a + b; });
  co_await server.start();
}
int main() {
 	execution_context ctx;
	co_spawn(ctx, test(), use_detach);
	ctx.loop();
}

```
#### channel

``` cpp
#include "../include/coasync/execution_context.hpp"
#include "../include/coasync/functional.hpp"
#include "../include/coasync/channel.hpp"
#include "../include/coasync/co_spawn.hpp"
using namespace coasync;
using std::chrono::operator""s;
awaitable<void> do_send(channel<int, 8>& channel) {
	for(unsigned int count {}; count < 8; count ++) {
		co_await sleep_for(1s);
		co_await channel.send(std::rand());
	}
}
awaitable<void> do_receive(channel<int, 8>& channel) {
	for(unsigned int count {}; count < 8; count ++)
		std::printf("receive: %d\n", co_await channel.receive());
}
int main() {
 execution_context context{2};
 channel<int, 8> channel;
 co_spawn(context, do_send(channel), use_detach);
 co_spawn(context, do_send(channel), use_detach);
 co_spawn(context, do_send(channel), use_detach);
 co_spawn(context, do_receive(channel), use_detach);
 co_spawn(context, do_receive(channel), use_detach);
 co_spawn(context, do_receive(channel), use_detach);
 context.loop();
}

```
#### serialize/deserialize

``` cpp
#include "../include/coasync/execution_context.hpp"
#include "../include/coasync/co_spawn.hpp"
#include "../include/coasync/this_coro.hpp"
#include "../include/coasync/net/endpoint.hpp"
#include "../include/coasync/net/acceptor.hpp"
#include "../include/coasync/net/protocol.hpp"
#include "../include/coasync/net/serde_stream.hpp"

using namespace coasync;

awaitable<void> deserial(net::tcp::socket socket) {
	net::serde_stream s { std::move(socket) };
	std::vector<int> vec;
	std::queue<int>  que;
	co_await s.deserialize(vec);
	for(auto value: vec)
		std::printf("vector %d\n", value);
	co_await s.deserialize(que);
	while(not que.empty()) {
		printf("queue: %d\n", que.front());
		que.pop();
	}
}

awaitable<void> acceptance() noexcept
{
  net::tcp::acceptor acceptor {
    co_await this_coro::context,
    net::tcp::endpoint(net::address_v4::loopback(), 10086)
  };
  while(true)
    co_spawn(co_await this_coro::context, deserial(co_await acceptor.accept()), use_detach);
}

int main() {
	  execution_context context {0};
	  co_spawn(context, acceptance(), use_detach);
	  context.loop();
}

```


### For more examples, please refer to the test set in directory[coasync/test]


<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- ROADMAP -->
## Roadmap

- [ ] Threading and coroutines work together
- [ ] Many provided asynchronous algorithms
- [ ] Asynchronous schedulers are easy to extend
- [ ] High performance and low latency

See the [open issues](https://github.com/RFoe/coasync/issues) for a full list of proposed features (and known issues).

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- CONTRIBUTING -->
## Contributing

Any contributions you make are **greatly appreciated**.

If you have a suggestion that would make this better, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement".
Don't forget to give the project a star! Thanks again!

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/feature-dev`)
3. Commit your Changes (`git commit -m 'Add some features'`)
4. Push to the Branch (`git push origin feature/feature-dev`)
5. Open a Pull Request

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Top contributors:

<a href="https://github.com/RFoe/graphs/contributors">
  <img src="https://github.com/RFoe/coasync" alt="contrib.rocks image" />
</a>



<!-- LICENSE -->
## License

Distributed under the MIT License. See `LICENSE.txt` for more information.

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- CONTACT -->
## Contact

My email - wangxlang3.mail2.sysu.edn.com

Project Link: [https://github.com/RFoe/coasync](https://github.com/RFoe/coasync)

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- ACKNOWLEDGMENTS -->
## Acknowledgments

* []()
* []()
* []()

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[contributors-shield]: https://img.shields.io/github/contributors/github_username/repo_name.svg?style=for-the-badge
[contributors-url]: https://github.com/github_username/repo_name/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/github_username/repo_name.svg?style=for-the-badge
[forks-url]: https://github.com/github_username/repo_name/network/members
[stars-shield]: https://img.shields.io/github/stars/github_username/repo_name.svg?style=for-the-badge
[stars-url]: https://github.com/github_username/repo_name/stargazers
[issues-shield]: https://img.shields.io/github/issues/github_username/repo_name.svg?style=for-the-badge
[issues-url]: https://github.com/github_username/repo_name/issues
[license-shield]: https://img.shields.io/github/license/github_username/repo_name.svg?style=for-the-badge
[license-url]: https://github.com/github_username/repo_name/blob/master/LICENSE.txt
[linkedin-shield]: https://img.shields.io/badge/-LinkedIn-black.svg?style=for-the-badge&logo=linkedin&colorB=555
[linkedin-url]: https://linkedin.com/in/linkedin_username
[product-screenshot]: images/screenshot.png
[Next.js]: https://img.shields.io/badge/next.js-000000?style=for-the-badge&logo=nextdotjs&logoColor=white
[Next-url]: https://nextjs.org/
[React.js]: https://img.shields.io/badge/React-20232A?style=for-the-badge&logo=react&logoColor=61DAFB
[React-url]: https://reactjs.org/
[Vue.js]: https://img.shields.io/badge/Vue.js-35495E?style=for-the-badge&logo=vuedotjs&logoColor=4FC08D
[Vue-url]: https://vuejs.org/
[Angular.io]: https://img.shields.io/badge/Angular-DD0031?style=for-the-badge&logo=angular&logoColor=white
[Angular-url]: https://angular.io/
[Svelte.dev]: https://img.shields.io/badge/Svelte-4A4A55?style=for-the-badge&logo=svelte&logoColor=FF3E00
[Svelte-url]: https://svelte.dev/
[Laravel.com]: https://img.shields.io/badge/Laravel-FF2D20?style=for-the-badge&logo=laravel&logoColor=white
[Laravel-url]: https://laravel.com
[Bootstrap.com]: https://img.shields.io/badge/Bootstrap-563D7C?style=for-the-badge&logo=bootstrap&logoColor=white
[Bootstrap-url]: https://getbootstrap.com
[JQuery.com]: https://img.shields.io/badge/jQuery-0769AD?style=for-the-badge&logo=jquery&logoColor=white
[JQuery-url]: https://jquery.com 
