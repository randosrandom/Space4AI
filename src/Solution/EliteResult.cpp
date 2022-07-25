#include "EliteResult.hpp"

namespace Space4AI
{
  void
  EliteResult::print_solution(const System& system, const std::string& path, size_t rank) const
  {
    if(rank > get_size())
    {
      Logger::Warn("rank bigger than available solutions ... printing worst solution of rank: " + std::to_string(get_size()));

      elite_results.back().print_solution(system, path);
    }
    else
    {
      elite_results[rank].print_solution(system, path);
    }
  }

  const Solution&
  EliteResult::get_solution(size_t rank) const
  {
    if(rank > get_size())
    {
      Logger::Warn("rank bigger than available solutions ... returning worse solution of rank: " + std::to_string(get_size()));
      return elite_results.back();
    }
    else
    {
      return elite_results[rank];
    }
  }





}
