
#ifndef SRC_STD_H_
#define SRC_STD_H_

#include <type_traits>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <functional>

namespace wasm {

template <typename T>
using Vector = std::vector<T>;

template <typename T>
using Function = std::function<T>;

template <typename K, typename V>
using Map = std::map<K, V, std::less<>>;

using String = std::string;
using StringStream = std::ostringstream;

}

#endif /* SRC_STD_H_ */
