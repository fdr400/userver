#pragma once

#include <algorithm>
#include <chrono>
#include <unordered_map>
#include <vector>

#include <server/handlers/http_handler_base.hpp>
#include <server/http/handler_methods.hpp>
#include <utils/statistics.hpp>
#include <utils/statistics/http_codes.hpp>
#include <utils/statistics/percentile.hpp>
#include <utils/statistics/recentperiod.hpp>

namespace server {
namespace handlers {

class HttpHandlerMethodStatistics {
 public:
  void Account(unsigned int code, size_t ms) {
    reply_codes_.Account(code);
    timings_.GetCurrentCounter().Account(ms);
  }

  formats::json::Value FormatReplyCodes() const {
    return reply_codes_.FormatReplyCodes();
  }

  using Percentile = utils::statistics::Percentile<2048, unsigned int, 120>;

  Percentile GetTimings() const { return timings_.GetStatsForPeriod(); }

  size_t GetInFlight() const { return in_flight_; }

  void IncrementInFlight() { in_flight_++; }

  void DecrementInFlight() { in_flight_--; }

 private:
  utils::statistics::RecentPeriod<Percentile, Percentile,
                                  utils::datetime::SteadyClock>
      timings_;
  utils::statistics::HttpCodes reply_codes_{400, 401, 500};
  std::atomic<size_t> in_flight_{0};
};

class HttpHandlerStatistics {
 public:
  HttpHandlerMethodStatistics& GetStatisticByMethod(http::HttpMethod method);

  const HttpHandlerMethodStatistics& GetStatisticByMethod(
      http::HttpMethod method) const;

  HttpHandlerMethodStatistics& GetTotalStatistics();

  const HttpHandlerMethodStatistics& GetTotalStatistics() const;

  void Account(http::HttpMethod method, unsigned int code,
               std::chrono::milliseconds ms);

  bool IsOkMethod(http::HttpMethod method) const;

 private:
  HttpHandlerMethodStatistics stats_;
  std::array<HttpHandlerMethodStatistics, http::kHandlerMethodsMax + 1>
      stats_by_method_;
};

class HttpHandlerStatisticsScope {
 public:
  HttpHandlerStatisticsScope(HttpHandlerStatistics& stats,
                             http::HttpMethod method);

  void Account(unsigned int code, std::chrono::milliseconds ms);

 private:
  HttpHandlerStatistics& stats_;
  const http::HttpMethod method_;
};

}  // namespace handlers
}  // namespace server
