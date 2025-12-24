#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <algorithm>

// See also this example lib: https://github.com/OSSIA/libossia/blob/v3/OSSIA/ossia/detail/algorithms.hpp

// Extended std
namespace Estd
{

using namespace std;

template<typename T>
class Vec : public std::vector<T> {
public:
    using vector<T>::vector;  // use the construcors from vector (under the name Vec)

    T& operator[](int i) {return vector<T>::at(i);}
    const T& operator[](int i) const {return vector<T>::at(i);}
};

// Sort full container
template<typename C>
void sort(C& c){sort(c.begin(),c.end());}
template<typename C, typename Pred>
void sort(C& c, Pred p){sort(c.begin(),c.end(),p);}

// Check if v is in c, when we don't need to retrieve an iterator to v
template<typename T, typename U>
bool contains(const T& c, const U v)
{return find(c,v) != c.end();}

// Check if p() for any element in c
template <typename C, typename Pred>
bool any_of(C& c, Pred p){ return std::any_of(std::begin(c), std::end(c), p); }

// Check if p() for all elements in c
template <typename C, typename Pred>
auto all_of(C& v, Pred p)
{return std::all_of(std::begin(v), std::end(v), p);}

// Check if !p() for all elements in c
template <typename C, typename Pred>
bool none_of(C& v, Pred p)
{return std::none_of(std::begin(v), std::end(v), p);}

// find val in c, return iterator to val
template <typename C, typename U>
auto find(C& c, const U& val){return std::find(std::begin(c), std::end(c), val);}
// find first element in c where p()
template <typename C, typename Pred>
auto find_if(C& c, Pred p){return std::find_if(std::begin(c), std::end(c), p);}

}  // namespace Estd

#endif // UTILS_H
