#include "../include/coasync/execution_context.hpp"
using namespace coasync;
using namespace coasync::detail;
struct sample_service
{
  explicit sample_service(auto&) noexcept {}
  void push_frame(auto&& ...)
  {
    std::puts("push_frame");
  }
  void commit_frame(auto&& ...)
  {
    std::puts("commit_frame");
  }
};
int main()
{
	coasync::execution_context context{};
  sample_service& service = use_service<sample_service>(context);

	return 0;
}
