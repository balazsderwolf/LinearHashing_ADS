# LinearHashing_ADS
ADS course Uni Vienna-semester_project<br>
Dynamic Linear Hashing without using STL, STD methods, data structures <br>
Works with user defined Objects (you need to implement your own hashcode)<br/>
<img width="796" alt="Képernyőfotó 2021-06-10 - 20 12 41" src="https://user-images.githubusercontent.com/72218495/121575998-43cdde80-ca28-11eb-85c0-960ce878c1ad.png">
<br/>With integer
<br/><img width="816" alt="Képernyőfotó 2021-06-10 - 20 14 44" src="https://user-images.githubusercontent.com/72218495/121576298-84c5f300-ca28-11eb-9064-2daaaa3f8c44.png">

To create the data structure: include "ADS_set.h" <br>
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
