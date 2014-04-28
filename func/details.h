#ifndef FUNC_INCLUDE_PRIVATE_H_
#define FUNC_INCLUDE_PRIVATE_H_

#include <memory>

namespace func {

enum class FuncType {
  FILTER,
  FOLD_LEFT,
  KEEP,
  MAP,
  SKIP,
  ZIP,
};

class Private;

}  // namespace func

#endif  // FUNC_INCLUDE_PRIVATE_H_

