#include "wait_list.hpp"

#include <algorithm>
#include <cassert>

#include "task/task_context.hpp"

namespace engine {
namespace impl {

void WaitList::Append([[maybe_unused]] WaitListBase::Lock& lock,
                      boost::intrusive_ptr<impl::TaskContext> context) {
  assert(lock);
  waiting_contexts_.push_back(std::move(context));
}

void WaitList::WakeupOne([[maybe_unused]] WaitListBase::Lock& lock) {
  assert(lock);
  while (!waiting_contexts_.empty()) {
    auto next_context = std::move(waiting_contexts_.front());
    waiting_contexts_.pop_front();
    if (next_context) {
      next_context->Wakeup(impl::TaskContext::WakeupSource::kWaitList);
      break;
    }
  }
}

void WaitList::WakeupAll([[maybe_unused]] WaitListBase::Lock& lock) {
  assert(lock);
  for (auto& context : waiting_contexts_) {
    if (context) {
      context->Wakeup(impl::TaskContext::WakeupSource::kWaitList);
    }
  }
  waiting_contexts_.clear();
}

void WaitList::Remove(const boost::intrusive_ptr<impl::TaskContext>& context) {
  Lock lock(*this);

  auto it =
      std::find(waiting_contexts_.begin(), waiting_contexts_.end(), context);
  if (it == waiting_contexts_.end()) return;

  it->reset();
  assert(std::find(std::next(it), waiting_contexts_.end(), context) ==
         waiting_contexts_.end());
}

}  // namespace impl
}  // namespace engine
