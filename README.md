<!-- Improved compatibility of back to top link: See: https://github.com/RFoe/coasync -->
<a id="readme-top"></a>

<h3 align="center">coasync</h3>

  <p align="center">
    Asynchronous network library that supports coroutines
    <br />
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

Coasync is a cross-platform C++ library for network and low-level I/O programming that provides developers with a consistent asynchronous model using a modern C++ 20 approach. Coasync provides an efficient and flexible way to handle network communications, file I/O, and other asynchronous tasks, making it easy for developers to build high performance, scalable network applications. Coasync can handle a large number of I/O operations without blocking the main thread. It supports multiple operating systems, including Windows and Linux, and provides a consistent API to ensure cross-platf-orm compatibility of code. It can be easily integrated into existing projects to support common network protocols such as TCP and UDP.
`Username: RFoer`, `Project-name: coasync`, `Email: wangxlang3@mail2.sysu.edu.com`


<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- GETTING STARTED -->
## Getting Started
Coasync has a lightweight compilation process, so its integrated compilation is very simple, just specify the path of the project, and include the required header file in the C++ source file. Header-only makes compiler optimization can do better. Because inline functions and all implementations are visible at compile time, considerable optimizations can be made, including some global optimizations. Coasync library does not rely on any third party library, and does not need to compile separately. Just add the include/coasync path in the Include directory of the project, and in the source code #include <coasync/...>, then we can get started.

### Prerequisites
* **C++20:** Robust support for most language features in Cpp20{[ranges](https://en.cppreference.com/w/cpp/header/ranges), [concepts](https://en.cppreference.com/w/cpp/header/concepts), [coroutines](https://en.cppreference.com/w/cpp/header/coroutine), [memory_resource](https://en.cppreference.com/w/cpp/header/memory_resource)}.
* **Winsock:** If the project is built on the windows platform, you need to dynamically link the winsock2 network library
* compile command
  ```sh
  ##Gnuc/clang
  -std=c++20
  ##MSVC
  /std:c++20
  ```
* linkage command
  ```sh
  -lws2_32
  ##VS
  #pragma comment(lib, "Ws2_32.lib")
  ```

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- USAGE EXAMPLES -->
## Usage

### when_any/when_any algorithm
 ** when_all is a awaitable generator that returns a awaitable that completes when the last of the input awaitables completes. It sends a pack of values, where the elements of said pack are the values sent by the input awaitables, in order.
 
 ** when_any is a awaitable generator that returns a awaitable that completes when the first of the input awaitables completes[or throws]. It sends a variant of values, where the element of that are the value sent by the first awaitable. Coasync supports for cancelling an operation, by send stop_request using std::stop_token/std::stop_source
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
rpc allows a client to invoke an object that exists on a remote computer as if it were an object in a local application, without knowing the details of the call.
Coasync providing both a client and server implementation for rpc. It does not matter to the caller how the received parameters are used inside the callable object on the server side and how the result is calculated. And for a remote call, these parameters are passed to another computer on the network in some form of information that the caller does not care about as well.
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
The channel can be used to send messages between different parts of the same application. The set of messages and the capacity of messages supported by a channel is specified by its template parameters. Messages can be sent and received using asynchronous and non-blocking operations in C++20 coroutine syntax. 

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
When you want to save the state of an object in memory to a file or database, or want to use a socket to transfer an object over the network, you need to convert an object to a sequence of bytes, and the process of restoring a sequence of bytes to an object is called an object, this process is called serialization/deserialization.

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


### For more examples, please refer to the test set in project directory[coasync/test]. 
### There are many code demos that cover all the current features of the project


<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- ROADMAP -->
## Roadmap

- [ ] Threading and coroutines work together: All nodes in the coroutine code that cede control are known, and there are no problems associated with multithreaded synchronization. On the other hand, the overhead of coroutines is very small, and tens of thousands of coroutines concurrent is perfectly fine.
- [ ] Many provided asynchronous algorithms: When launching a asynchronous work, and it won't block the calling execution agent even if the work is not completed. and we can specify where asynchronous work is executed.And also the asynchronous algorithm is composable and can be customized

- [ ] Asynchronous schedulers are easy to extend:The structure of the execution agent and scheduling server is very clear and simple, so users can easily customize new asynchronous callable objects according to the agreed rules
- [ ] High performance and low latency: The underlying event loop and scheduling algorithm are optimized to achieve high performance and low latency concurrency and response

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
