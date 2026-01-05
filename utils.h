#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <algorithm>
#include <numeric>  // iota
#include <string>
#include <cctype>  // toupper

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

// Return vector of sorted indices based on `v`. Does not modify v.
template<typename T>
std::vector<size_t> argsort(const std::vector<T>& v)
{
    using std::vector;
    // Initialize with original indices
    vector<size_t> idx(v.size());
    std::iota(idx.begin(),idx.end(),0);

    // Sort indices
    std::stable_sort(idx.begin(),idx.end(),
                [&v](size_t i1,size_t i2) {return v[i1] < v[i2];}
                );

    return idx;
}

inline void to_upper(std::string& str)
{
    std::transform(str.begin(),str.end(),str.begin(),::toupper);
}
inline void to_lower(std::string& str)
{
    std::transform(str.begin(),str.end(),str.begin(),::tolower);
}

}  // namespace Estd


#endif // UTILS_H
