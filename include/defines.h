#pragma once
#include <memory>
#include <vector>
#include <string>

namespace kal
{
  using String = std::string;

  template<typename _Type>
  using Vector = std::vector<_Type>;

  template<typename _Type>
  using UPtr = std::vector<_Type>;

}