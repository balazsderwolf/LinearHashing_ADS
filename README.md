# LinearHashing_ADS
ADS course Uni Vienna-semester_project
Dynamic Linear Hashing without using STL, STD methods, datasctructures<br>
Creating the datastructure: include "ADS_set.h" in main<br>
```cpp
ADS_set<int,4> table;
//type,default blocksize
table.dump(); //visualisation

METHODS
void insert(std::initializer_list<key_type> ilist);

std::pair<iterator,bool> insert(const key_type& key);

template<typename InputIt> void insert(InputIt first, InputIt last);

void clear();

size_type erase(const key_type& key);

void swap(ADS_set& other);

iterator find(const key_type& key) const;

const_iterator begin() const;

const_iterator end() const;

+Operator: == != =



CONSTRUCTORS
ADS_set();

ADS_set(std::initializer_list<key_type> ilist);

template<typename InputIt> ADS_set(InputIt first, InputIt last);

ADS_set(const ADS_set& other);


```
