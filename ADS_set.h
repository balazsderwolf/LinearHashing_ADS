//
// Created by Balázs Farkas on 2021. 04. 23..
//

#ifndef LINEAR_HASHING_ADS_SET_H
#define LINEAR_HASHING_ADS_SET_H

template<
        typename Key,
        size_t N = 4
>
class ADS_set {
public:
    class Iterator;

    using value_type = Key;
    using key_type = Key;
    using reference = value_type &;
    using const_reference = const value_type &;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;
    using const_iterator = Iterator;
    using iterator = Iterator;
    using key_equal = std::equal_to<key_type>; // Hashing
    using hasher = std::hash<key_type>;        // Hashing
private:
    struct Bucket {
        size_t nextEmpty;
        key_type *bucketValues;
        Bucket *nextBucket;

        Bucket() {
            nextBucket = nullptr;
            bucketValues = new key_type[N];
            nextEmpty = 0;


        }

        bool isFull() {
            if (nextEmpty == N)
                return true;
            return false;
        }

        bool eraseFromBucket(key_type key) {
            size_t i{0};
            for (; i < nextEmpty; i++) {
                if (key_equal{}(bucketValues[i], key)) {
                    break;
                }
            }
            if (i != nextEmpty) {
                //its in there ->deletable
                for (; i < nextEmpty - 1; ++i)
                    bucketValues[i] = bucketValues[i + 1];
                --nextEmpty;
                return true; //torolve
            }
            return false; //nem talalta meg-->nem torolte ki

        }

        void insertElementToBucket(const key_type &data) {
            this->bucketValues[nextEmpty] = data;
            nextEmpty++;

        }

        size_type findInBucket(const key_type &data) {
            size_t i{0};
            while (i < nextEmpty) {
                if (key_equal{}(bucketValues[i], data)) {
                    return i;
                }
                i++;
            }
            return i;
        }

    };

    size_type tableSize;
    size_type nextEmpty;
    Bucket **bucketArray;

    size_type countElements;  //helper variable, count the size of the hashtable is easier
    size_type nextToSplit;    // pointer to the next Split
    size_type lastIndexInRound; // last index before increase round
    size_type round;          //round used for hashfunction 1,2
public:

    ADS_set() {
        countElements = 0;
        tableSize = 8;
        bucketArray = new Bucket *[tableSize];
        nextEmpty = 1;
        bucketArray[0] = new Bucket();

        // empty table with one bucket
        round = 0;
        lastIndexInRound = 0;
        nextToSplit = 0;

    }

    ADS_set(std::initializer_list<key_type> ilist) : ADS_set() {
        for (const auto &g : ilist) {
            insert(g);
        }
    }

    ADS_set(const ADS_set &other) : ADS_set() {
        for (auto i = other.begin(); i != other.end(); i++) {
            insert(*i);
        }
    } //PH2
    template<typename InputIt>
    ADS_set(InputIt first, InputIt last): ADS_set() {
        for (; first != last; first++) {
            this->insert(*first);
        }
    }

    ADS_set &operator=(const ADS_set &other) {
        this->clear();
        for (auto i = other.begin(); i != other.end(); i++) {
            insert(*i);
        }
        return *this;
    } //PH2
    ADS_set &operator=(std::initializer_list<key_type> ilist) {
        this->clear();
        for (const auto &g : ilist) {
            insert(g);
        }
        return *this;

    } //PH2


    const_iterator begin() const {
        for (size_type i{0}; i < nextEmpty; i++) {
            for (Bucket *j = bucketArray[i]; j != nullptr; j = j->nextBucket) {
                if (j->nextEmpty != 0)
                    return const_iterator(nextEmpty, bucketArray, i, j, 0);
            }
        }
        return this->end();
    }//PH2
    const_iterator end() const {
        return const_iterator(nullptr);
    } //PH2


    ~ADS_set() {
        for (size_t i = 0; i < nextEmpty; i++) {
            deleteRow(bucketArray[i]);
        }
        delete[] bucketArray;
    } //PH2

    bool isFull() {
        if (nextEmpty == tableSize)
            return true;
        return false;
    }

    size_type hashAlgorithmOne(const size_type &key) const {           // Basic Hashfunction
        const int pw = 1 << round;
        return key % pw;
    }

    size_type hashAlgorithmTwo(const size_type &key) const {          // Owerflow(Split) Hashfunction
        const int pw = 1 << (round + 1);
        return key % pw;
    }

    std::pair<Bucket *, bool> firstNotFullBucketInRow(Bucket *b) {
        //find the first not full bucket (for inserting) or return with nullptr->all full
        for (; b != nullptr; b = b->nextBucket) {
            if (!b->isFull())
                return std::make_pair(b, true);
            else if (b->nextBucket == nullptr)
                return std::make_pair(b, false);
        }
        return std::make_pair(nullptr, false); //error if this happens
    }

    std::pair<const_iterator, bool> insert(const key_type &key) {
        /* size_type  row = hashAlgorithmOne(hasher{}(key));      //Megtalalja a sort ahova insertel
         if (row < nextToSplit) {
             row = hashAlgorithmTwo(hasher{}(key));
         }*/
        std::pair<const_iterator, size_type> r = findHelp(key);
        //const_iterator r = find(key);
        //if bool == true found a not full bucket (ptr)

        if (r.first == this->end()) {
            auto firstNotFull = firstNotFullBucketInRow(*(bucketArray +
                                                          r.second)); //pair<ptr,bool> if bool == false, the ptr is the last in the row and its full
            if (firstNotFull.second == false) {
                //Create Overflow Bucket, insert Key
                firstNotFull.first->nextBucket = new Bucket();
                firstNotFull.first->nextBucket->insertElementToBucket(key);
                countElements++;
                //SPLITTING
                // Create new row with 1 Empty Bucket
                createNewRowToSplit();
                //replace elements
                replaceElements(nextToSplit);
                //Before next element- search for empty overflowbuckets
                deleteOverflowBucket(nextToSplit); // maximum 1 empty bucket, every other will be deleted
                //get an Iterator

                increaseRound();
                return std::make_pair(find(key), true);
            } else {
                firstNotFull.first->insertElementToBucket(key);

                countElements++;
                //return std::make_pair(find(key),true);-----i think its slower->dont have to do find() we know do location of the key in this fall
                return std::make_pair(const_iterator(nextEmpty, bucketArray, r.second, firstNotFull.first,
                                                     firstNotFull.first->nextEmpty - 1), true);

            }
            //nem volt benne, bool = true, iterator mutat arra ahova beraktuk
        }
        return std::make_pair(r.first, false);

        //benne van mar, bool = false, iterator mutat arra
    } //PH2
    void insert(std::initializer_list<key_type> ilist) {
        for (const auto &i: ilist) {
            insert(i);
        }
    }

    template<typename InputIt>
    void insert(InputIt first, InputIt last) {
        for (; first != last; first++) {
            insert(*first);
        }
    }

    void deleteOverflowBucket(size_type row) {
        bool oneOverflow = false; //if we found 1 empty overflow bucket, the others need to be deleted

        for (Bucket *i = bucketArray[row]; i != nullptr;) {
            if (i->nextEmpty == 0 && !oneOverflow) {
                oneOverflow = true;
            }

            if (i->nextBucket != nullptr && i->nextBucket->nextEmpty == 0 &&
                oneOverflow) { // uncouple the empty bucket, couple the emptybucket-1 with the emptybucket+1
                auto ptr = i->nextBucket->nextBucket;
                //delete bucket
                delete[] i->nextBucket->bucketValues;

                delete i->nextBucket;
                i->nextBucket = nullptr;
                i->nextBucket = ptr;
            } else {
                i = i->nextBucket;
            }
        }
    }

    size_type count(const key_type &key) const {
        size_type row = hashAlgorithmOne(hasher{}(key));
        if (row < nextToSplit) {
            row = hashAlgorithmTwo(hasher{}(key));
        }
        for (Bucket *i = bucketArray[row]; i != nullptr; i = i->nextBucket) {
            if (i->findInBucket(key) != i->nextEmpty)
                return 1;
        }
        return 0;
    }

    const_iterator find(const key_type &key) const {
        size_type row = hashAlgorithmOne(hasher{}(key));
        if (row < nextToSplit) {
            row = hashAlgorithmTwo(hasher{}(key));
        }
        for (Bucket *i = bucketArray[row]; i != nullptr; i = i->nextBucket) {
            size_type index = i->findInBucket(key);
            if (index != i->nextEmpty)
                return iterator(nextEmpty, bucketArray, row, i, index);
        }
        return this->end();
    }

    std::pair<const_iterator, size_type> findHelp(const key_type &key) const {
        size_type row = hashAlgorithmOne(hasher{}(key));
        if (row < nextToSplit) {
            row = hashAlgorithmTwo(hasher{}(key));
        }
        for (Bucket *i = bucketArray[row]; i != nullptr; i = i->nextBucket) {
            size_type index = i->findInBucket(key);
            if (index != i->nextEmpty)
                return std::make_pair(const_iterator(nextEmpty, bucketArray, row, i, index), row);
        }
        return std::make_pair(this->end(), row);
    }

    void dump(std::ostream &o = std::cerr) const {
        for (size_type i = 0; i < nextEmpty; i++) {
            //adott sor elso bucketjere mutat majd nextBuckettel mindig a kovire
            o << i << ". index: ";
            for (Bucket *j = bucketArray[i]; j != nullptr; j = j->nextBucket) {
                for (size_type key = 0; key < j->nextEmpty; key++) {

                    if (key == j->nextEmpty - 1)
                        o << j->bucketValues[key];
                    else
                        o << j->bucketValues[key] << " ";
                }
                if (j->nextBucket != nullptr) {
                    o << "-->";
                }

            }
            o << std::endl;
        }

        o << "End of Hashtable" << std::endl;
        o << "Next to Split: " << nextToSplit << std::endl;
        o << "Last index before new round: " << lastIndexInRound << std::endl;
        o << "Current Round: " << round << std::endl;
        o << "Begin: ";
        if (this->begin() == nullptr) {
            o << " nullptr.";
        } else {
            o << *this->begin();
        }
        o << std::endl;
        o << "End: ";
        if (this->end() == nullptr) {
            o << " nullptr.";
        } else {
            o << "hiba";
        }
        o << std::endl;
        o << size() << std::endl;
    }

    void reserve(const size_type &n) {
        Bucket **newArray = new Bucket *[n];
        for (size_type i = 0; i < nextEmpty; ++i) {
            newArray[i] = bucketArray[i];
        }
        delete[] bucketArray;
        bucketArray = newArray;
        tableSize = n;


    }

    void deleteRow(Bucket *b) {
        if (b == nullptr) {
            return;
        }
        deleteRow(b->nextBucket);
        delete[] b->bucketValues;
        b->nextBucket = nullptr;
        b->nextEmpty = 0;
        delete b;
        b = NULL;

    }

    size_type erase(const key_type &key) {
        size_type row = hashAlgorithmOne(hasher{}(key));
        if (row < nextToSplit) {
            row = hashAlgorithmTwo(hasher{}(key));
        }
        for (Bucket *i = bucketArray[row]; i != nullptr; i = i->nextBucket) {
            bool deleted = i->eraseFromBucket(key);
            if (deleted) {
                countElements--;
                deleteOverflowBucket(row);
                return 1;
            }
        }
        deleteOverflowBucket(row);
        return 0;
    }//PH2

    void createNewRowToSplit() {                 //Created a new row with 1 Empty Bucket
        if (isFull()) {
            reserve(tableSize * 2);
        }
        bucketArray[nextEmpty] = new Bucket();
        nextEmpty++;
    }

    void replaceElements(const size_type &index) {
        //dupla for ciklus (sor(.at elem(vector)))
        //minden elemre -> hashfunctionTwo-> marad/megy
        for (Bucket *i = *(bucketArray + index); i != nullptr; i = i->nextBucket) {
            //adott sor elso bucketjere mutat majd nextBuckettel mindig a kovire
            for (size_type key = 0; key < i->nextEmpty; key++) {
                //az adott bucket 0 elemetol- a bucket maxSizedik elemig
                size_type otherIndex = hashAlgorithmTwo(hasher{}(i->bucketValues[key]));
                if (otherIndex != index) { // Ha a 2. hash alapjan mashova kerul az adott key
                    auto firstNotFull = firstNotFullBucketInRow(
                            *(bucketArray + otherIndex)); //visszakap egy pointert amelyik Bucketbe fer meg
                    if (firstNotFull.second == false) {
                        //Create Overflow Bucket, insert Key
                        firstNotFull.first->nextBucket = new Bucket();
                        firstNotFull.first->nextBucket->insertElementToBucket(i->bucketValues[key]);
                    } else {
                        firstNotFull.first->insertElementToBucket(i->bucketValues[key]);
                    }
                    i->eraseFromBucket(i->bucketValues[key]);
                    if (i->nextEmpty == 0) {
                        break;
                    } else {
                        key--;
                    }
                }

            }
        }
    }

    void increaseRound() {
        if (nextToSplit == lastIndexInRound) {
            round++;
            nextToSplit = 0;
            lastIndexInRound = (1 << round) - 1;
        } else {
            nextToSplit++;
        }

    }

    size_type size() const {
        return countElements;
    }

    bool empty() const {
        if (size() == 0)
            return true;
        return false;
    }

    void clear() {
        //in Bucket realSize = 0, otherBuckets delete, Index = 0; otherIndex delete-> modified destructor
        deleteRow(bucketArray[0]->nextBucket);
        bucketArray[0]->nextEmpty = 0;
        bucketArray[0]->nextBucket = NULL;
        for (size_t i = 1; i < nextEmpty; i++) {
            deleteRow(bucketArray[i]);
        }
        //modified deletebucket: first index: delete all bucket but the first!
        countElements = 0;
        nextEmpty = 1;
        // empty table with one bucket
        round = 0;
        lastIndexInRound = 0;
        nextToSplit = 0;


    } //PH2
    void swap(ADS_set &other) {
        using std::swap;
        //round lastIndexInRound nextToSplit countElements tablesize nextEmpty
        swap(this->bucketArray, other.bucketArray);
        swap(round, other.round);
        swap(lastIndexInRound, other.lastIndexInRound);
        swap(nextToSplit, other.nextToSplit);
        swap(countElements, other.countElements);
        swap(tableSize, other.tableSize);
        swap(nextEmpty, other.nextEmpty);

        //std::swap(*this,other);
    }//PH2
    friend bool operator==(const ADS_set<Key, N> &lhs, const ADS_set<Key, N> &rhs) {
        if (lhs.size() != rhs.size())
            return false;
        for (auto i = rhs.begin(); i != rhs.end(); i++) {
            if (lhs.count(*i) == 0)
                return false;
        }
        return true;
    }

    friend bool operator!=(const ADS_set<Key, N> &lhs, const ADS_set<Key, N> &rhs) {
        return !(lhs == rhs);
    }
};

template<typename Key, size_t N>
class ADS_set<Key, N>::Iterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = ADS_set::value_type;
    using difference_type = ADS_set::difference_type;
    using pointer = const ADS_set::value_type *;
    using reference = ADS_set::const_reference;

private:
    size_type lastIndex;
    Bucket **bucketArray;
    size_type index;
    Bucket *bucketIndex;
    size_type indexInBucket;
    pointer ptr;
public:
    Iterator() {
        ptr = nullptr;
        bucketArray = nullptr;
        bucketIndex = nullptr;
        index = 0;
        lastIndex = 0;
        indexInBucket = 0;

    }

    Iterator(size_type lastIndex, Bucket **bucketArray, size_type index, Bucket *bucketIndex, size_type indexInBucket)
            : lastIndex(lastIndex), bucketArray(bucketArray), index(index), bucketIndex(bucketIndex),
              indexInBucket(indexInBucket) {
        ptr = bucketIndex->bucketValues + indexInBucket;
    }

    Iterator(pointer ptr) : ptr(ptr) {
        bucketArray = nullptr;
        bucketIndex = nullptr;
        index = 0;
        lastIndex = 0;
        indexInBucket = 0;
    }

    reference operator*() const {
        return *(ptr);
    }

    pointer operator->() const {
        return ptr;
    }

    bool operator==(const iterator &it) const {
        if (it.ptr == this->ptr)
            return true;
        return false;
    }

    bool operator!=(const iterator &it) const {
        if (*this == it)
            return false;
        return true;
    }

    Iterator &operator++() {
        indexInBucket++;
        while (true) {
            if (index == lastIndex) {
                ptr = nullptr;
                return *this;
            }
            if (bucketIndex == nullptr) {
                if (index == lastIndex) {
                    ptr = nullptr;
                    return *this;
                }
                index++;
                bucketIndex = bucketArray[index];
                indexInBucket = 0;
            } else if (indexInBucket < bucketIndex->nextEmpty) {
                ptr = bucketIndex->bucketValues + indexInBucket;

                return *this;
            } else if (indexInBucket == bucketIndex->nextEmpty) {
                bucketIndex = bucketIndex->nextBucket;
                indexInBucket = 0;
            }
        }
    }

    Iterator operator++(int) {
        ADS_set::const_iterator temp = *this;
        ++*this; //TODO test
        return temp;
    }

}; //PH2

//ALL PH2
template<typename Key, size_t N>
bool operator==(const ADS_set<Key, N> &lhs, const ADS_set<Key, N> &rhs);

template<typename Key, size_t N>
bool operator!=(const ADS_set<Key, N> &lhs, const ADS_set<Key, N> &rhs);

template<typename Key, size_t N>
void swap(ADS_set<Key, N> &lhs, ADS_set<Key, N> &rhs) {
    lhs.swap(rhs);
}


#endif //LINEAR_HASHING_ADS_SET_H
