#ifndef JUMP_LIST_H
#define JUMP_LIST_H

#include <memory>
#include <random>
#include <iterator>
#include <functional>
#include <initializer_list>
#include <stdexcept>
#include <concepts>
#include <vector> // Required for std::vector in Node

/**
 * @file jump_list.h
 * @brief Defines the jump_list container, a Skip List based associative container.
 */

/**
 * @class jump_list
 * @brief A templated associative container implementing a Skip List data structure.
 *
 * This class provides a container that stores elements in a sorted order,
 * allowing for efficient insertion, deletion, and lookup operations.
 * It offers similar functionality to `std::set` or `std::map` (when `T` is a `std::pair`),
 * but is based on a Skip List for its underlying structure.
 *
 * @tparam T The type of elements stored in the jump_list.
 * @tparam Compare A comparison function object type, which defaults to `std::less<T>`.
 *                 It defines the ordering criterion for elements.
 * @tparam Allocator An allocator type, which defaults to `std::allocator<T>`.
 *                   It defines the memory allocation strategy.
 */
template<typename T, typename Compare = std::less<T>, typename Allocator = std::allocator<T>>
requires std::predicate<Compare, const T&, const T&>
class jump_list {
private:
    /**
     * @brief The maximum allowed level for any node in the Skip List.
     * @details This constant determines the maximum height of the Skip List.
     */
    static constexpr int MAX_LEVEL = 32;

    /**
     * @brief The probability factor used in generating random levels for new nodes.
     * @details A value of 0.5 means there's a 50% chance to increase the level.
     */
    static constexpr double P = 0.5;

    /**
     * @struct Node
     * @brief Represents a node in the Skip List.
     *
     * Each node contains the actual data, a vector of forward pointers
     * (one for each level it participates in), and a backward pointer
     * for bidirectional iteration.
     */
    struct Node {
        T data;                          /**< The data stored in the node. */
        std::vector<Node*> forward;      /**< Vector of forward pointers for each level. */
        Node* backward;                  /**< Backward pointer for bidirectional iteration. */
        
        /**
         * @brief Constructor for a data node.
         * @param value The value to store in the node.
         * @param level The number of levels this node will span (0-indexed).
         */
        Node(const T& value, int level) : data(value), forward(level + 1, nullptr), backward(nullptr) {}

        /**
         * @brief Move constructor for a data node.
         * @param value The value to store in the node (moved).
         * @param level The number of levels this node will span (0-indexed).
         */
        Node(T&& value, int level) : data(std::move(value)), forward(level + 1, nullptr), backward(nullptr) {}

        /**
         * @brief Constructor for the header node.
         * @param level The maximum level the header node can span.
         */
        Node(int level) : forward(level + 1, nullptr), backward(nullptr) {} // Header node
    };

    /**
     * @brief Type alias for the rebinded allocator for Node objects.
     */
    using AllocatorType = typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;

    /**
     * @brief Type alias for allocator traits for Node objects.
     */
    using AllocatorTraits = std::allocator_traits<AllocatorType>;

    Node* header;                      /**< Pointer to the header node of the Skip List. */
    int level;                         /**< The current maximum level of the Skip List. */
    size_t size_;                      /**< The number of elements in the list. */
    Compare comp;                      /**< The comparison object used for ordering elements. */
    AllocatorType alloc;               /**< The allocator object used for memory management. */
    mutable std::mt19937 rng;          /**< Random number generator for determining node levels. */

    /**
     * @brief Generates a random level for a new node.
     * @details The level is determined probabilistically, with a P chance of increasing the level,
     *          up to MAX_LEVEL.
     * @return The randomly generated level.
     */
    int randomLevel() {
        int lvl = 0;
        while (std::uniform_real_distribution<double>(0.0, 1.0)(rng) < P && lvl < MAX_LEVEL) {
            lvl++;
        }
        return lvl;
    }

    /**
     * @brief Allocates and constructs a new Node with a given value and level.
     * @param value The value to be copied into the new node.
     * @param level The level of the new node.
     * @return A pointer to the newly created node.
     * @throws std::bad_alloc if allocation fails.
     */
    Node* createNode(const T& value, int level) {
        Node* node = AllocatorTraits::allocate(alloc, 1);
        try {
            AllocatorTraits::construct(alloc, node, value, level);
        } catch (...) {
            AllocatorTraits::deallocate(alloc, node, 1);
            throw;
        }
        return node;
    }

    /**
     * @brief Allocates and constructs a new Node with a given value (moved) and level.
     * @param value The value to be moved into the new node.
     * @param level The level of the new node.
     * @return A pointer to the newly created node.
     * @throws std::bad_alloc if allocation fails.
     */
    Node* createNode(T&& value, int level) {
        Node* node = AllocatorTraits::allocate(alloc, 1);
        try {
            AllocatorTraits::construct(alloc, node, std::move(value), level);
        } catch (...) {
            AllocatorTraits::deallocate(alloc, node, 1);
            throw;
        }
        return node;
    }

    /**
     * @brief Destroys and deallocates a given node.
     * @param node A pointer to the node to be destroyed and deallocated.
     * @note The header node is not destroyed by this function.
     */
    void destroyNode(Node* node) {
        if (node && node != header) {
            AllocatorTraits::destroy(alloc, node);
            AllocatorTraits::deallocate(alloc, node, 1);
        }
    }

public:
    // Member types
    using value_type = T;                          /**< @brief The type of elements. */
    using size_type = std::size_t;                 /**< @brief The type for sizes. */
    using difference_type = std::ptrdiff_t;        /**< @brief The type for differences between pointers. */
    using reference = value_type&;                 /**< @brief Reference to a value_type. */
    using const_reference = const value_type&;     /**< @brief Const reference to a value_type. */
    using pointer = typename std::allocator_traits<Allocator>::pointer; /**< @brief Pointer to a value_type. */
    using const_pointer = typename std::allocator_traits<Allocator>::const_pointer; /**< @brief Const pointer to a value_type. */
    using key_compare = Compare;                   /**< @brief The comparison type used for keys. */
    using value_compare = Compare;                 /**< @brief The comparison type used for values. (Same as key_compare for set-like containers) */
    using allocator_type = Allocator;              /**< @brief The allocator type. */

    /**
     * @class iterator
     * @brief Bidirectional iterator for the jump_list.
     * @details This iterator allows traversing the jump_list in both forward and backward directions.
     */
    class iterator {
    private:
        Node* node;    /**< Pointer to the current node. */
        Node* header;  /**< Pointer to the jump_list's header node. */
        friend class jump_list; /**< Allows jump_list to access private members. */

    public:
        using iterator_category = std::bidirectional_iterator_tag; /**< @brief The iterator category. */
        using value_type = T;                                      /**< @brief The type of elements. */
        using difference_type = std::ptrdiff_t;                    /**< @brief The type for differences between pointers. */
        using pointer = T*;                                        /**< @brief Pointer to a value_type. */
        using reference = T&;                                      /**< @brief Reference to a value_type. */

        /**
         * @brief Default constructor. Creates a null iterator.
         */
        iterator() : node(nullptr), header(nullptr) {}

        /**
         * @brief Constructor from a Node pointer.
         * @param n Pointer to the node this iterator will point to.
         * @param h Pointer to the jump_list's header node.
         */
        explicit iterator(Node* n, Node* h) : node(n), header(h) {}

        /**
         * @brief Dereference operator.
         * @return A reference to the data of the current node.
         */
        reference operator*() const { return node->data; }

        /**
         * @brief Member access operator.
         * @return A pointer to the data of the current node.
         */
        pointer operator->() const { return &(node->data); }

        /**
         * @brief Pre-increment operator. Moves the iterator to the next element.
         * @return A reference to the incremented iterator.
         */
        iterator& operator++() {
            node = node->forward[0];
            return *this;
        }

        /**
         * @brief Post-increment operator. Moves the iterator to the next element.
         * @return A copy of the iterator before incrementing.
         */
        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        /**
         * @brief Pre-decrement operator. Moves the iterator to the previous element.
         * @return A reference to the decremented iterator.
         */
        iterator& operator--() {
            if (node) {
                node = node->backward;
            } else if (header) { // If at end(), move to last element
                node = header->backward;
            }
            return *this;
        }

        /**
         * @brief Post-decrement operator. Moves the iterator to the previous element.
         * @return A copy of the iterator before decrementing.
         */
        iterator operator--(int) {
            iterator tmp = *this;
            --(*this);
            return tmp;
        }

        /**
         * @brief Equality comparison operator.
         * @param other The other iterator to compare with.
         * @return True if both iterators point to the same node, false otherwise.
         */
        bool operator==(const iterator& other) const { return node == other.node; }

        /**
         * @brief Inequality comparison operator.
         * @param other The other iterator to compare with.
         * @return True if iterators point to different nodes, false otherwise.
         */
        bool operator!=(const iterator& other) const { return node != other.node; }
    };

    /**
     * @class const_iterator
     * @brief Bidirectional const iterator for the jump_list.
     * @details This iterator allows traversing the jump_list in both forward and backward directions,
     *          but does not allow modification of the elements.
     */
    class const_iterator {
    private:
        const Node* node; /**< Pointer to the current const node. */
        const Node* header; /**< Pointer to the jump_list's const header node. */
        friend class jump_list; /**< Allows jump_list to access private members. */

    public:
        using iterator_category = std::bidirectional_iterator_tag; /**< @brief The iterator category. */
        using value_type = T;                                      /**< @brief The type of elements. */
        using difference_type = std::ptrdiff_t;                    /**< @brief The type for differences between pointers. */
        using pointer = const T*;                                  /**< @brief Const pointer to a value_type. */
        using reference = const T&;                                /**< @brief Const reference to a value_type. */

        /**
         * @brief Default constructor. Creates a null const iterator.
         */
        const_iterator() : node(nullptr), header(nullptr) {}

        /**
         * @brief Constructor from a const Node pointer.
         * @param n Pointer to the const node this iterator will point to.
         * @param h Pointer to the jump_list's const header node.
         */
        explicit const_iterator(const Node* n, const Node* h) : node(n), header(h) {}

        /**
         * @brief Conversion constructor from a non-const iterator.
         * @param it The non-const iterator to convert.
         */
        const_iterator(const iterator& it) : node(it.node), header(it.header) {}

        /**
         * @brief Dereference operator.
         * @return A const reference to the data of the current node.
         */
        reference operator*() const { return node->data; }

        /**
         * @brief Member access operator.
         * @return A const pointer to the data of the current node.
         */
        pointer operator->() const { return &(node->data); }

        /**
         * @brief Pre-increment operator. Moves the iterator to the next element.
         * @return A reference to the incremented const iterator.
         */
        const_iterator& operator++() {
            node = node->forward[0];
            return *this;
        }

        /**
         * @brief Post-increment operator. Moves the iterator to the next element.
         * @return A copy of the const iterator before incrementing.
         */
        const_iterator operator++(int) {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        /**
         * @brief Pre-decrement operator. Moves the iterator to the previous element.
         * @return A reference to the decremented const iterator.
         */
        const_iterator& operator--() {
            if (node) {
                node = node->backward;
            } else if (header) { // If at end(), move to last element
                node = header->backward;
            }
            return *this;
        }

        /**
         * @brief Post-decrement operator. Moves the iterator to the previous element.
         * @return A copy of the const iterator before decrementing.
         */
        const_iterator operator--(int) {
            const_iterator tmp = *this;
            --(*this);
            return tmp;
        }

        /**
         * @brief Equality comparison operator.
         * @param other The other const iterator to compare with.
         * @return True if both iterators point to the same node, false otherwise.
         */
        bool operator==(const const_iterator& other) const { return node == other.node; }

        /**
         * @brief Inequality comparison operator.
         * @param other The other const iterator to compare with.
         * @return True if iterators point to different nodes, false otherwise.
         */
        bool operator!=(const const_iterator& other) const { return node != other.node; }
    };

    /**
     * @brief Default constructor.
     * @details Constructs an empty jump_list. Initializes the header node and random number generator.
     */
    jump_list() : header(nullptr), level(0), size_(0), comp(Compare()), alloc(AllocatorType()), rng(std::random_device{}()) {
        header = AllocatorTraits::allocate(alloc, 1);
        AllocatorTraits::construct(alloc, header, MAX_LEVEL);
    }

    /**
     * @brief Constructor with a custom comparison object and allocator.
     * @param c The comparison object.
     * @param a The allocator object.
     */
    explicit jump_list(const Compare& c, const Allocator& a = Allocator())
        : header(nullptr), level(0), size_(0), comp(c), alloc(a), rng(std::random_device{}()) {
        header = AllocatorTraits::allocate(alloc, 1);
        AllocatorTraits::construct(alloc, header, MAX_LEVEL);
    }

    /**
     * @brief Constructor with a custom allocator.
     * @param a The allocator object.
     */
    explicit jump_list(const Allocator& a)
        : header(nullptr), level(0), size_(0), comp(Compare()), alloc(a), rng(std::random_device{}()) {
        header = AllocatorTraits::allocate(alloc, 1);
        AllocatorTraits::construct(alloc, header, MAX_LEVEL);
    }

    /**
     * @brief Range constructor.
     * @details Constructs a jump_list with elements from the range [first, last).
     * @tparam InputIt The input iterator type.
     * @param first An input iterator to the beginning of the range.
     * @param last An input iterator to the end of the range.
     * @param c The comparison object (defaults to `Compare()`).
     * @param a The allocator object (defaults to `Allocator()`).
     */
    template<class InputIt>
    jump_list(InputIt first, InputIt last, const Compare& c = Compare(), const Allocator& a = Allocator())
        : jump_list(c, a) {
        insert(first, last);
    }

    /**
     * @brief Copy constructor.
     * @param other The jump_list to copy elements from.
     * @details Performs a deep copy of all elements from `other`.
     */
    jump_list(const jump_list& other)
        : header(nullptr), level(0), size_(0), comp(other.comp), alloc(other.alloc), rng(std::random_device{}()) {
        header = AllocatorTraits::allocate(alloc, 1);
        AllocatorTraits::construct(alloc, header, MAX_LEVEL);
        insert(other.begin(), other.end());
    }

    /**
     * @brief Move constructor.
     * @param other The jump_list to move resources from.
     * @details Efficiently transfers ownership of resources from `other` to this object.
     *          `other` is left in a valid, but unspecified, state (typically empty).
     */
    jump_list(jump_list&& other) noexcept
        : header(other.header), level(other.level), size_(other.size_), 
          comp(std::move(other.comp)), alloc(std::move(other.alloc)), rng(std::move(other.rng)) {
        // Create a new empty header for the moved-from object
        other.header = AllocatorTraits::allocate(other.alloc, 1);
        AllocatorTraits::construct(other.alloc, other.header, MAX_LEVEL);
        other.level = 0;
        other.size_ = 0;
    }

    /**
     * @brief Initializer list constructor.
     * @param init An initializer list of elements to insert.
     * @param c The comparison object (defaults to `Compare()`).
     * @param a The allocator object (defaults to `Allocator()`).
     */
    jump_list(std::initializer_list<T> init, const Compare& c = Compare(), const Allocator& a = Allocator())
        : jump_list(c, a) {
        insert(init.begin(), init.end());
    }

    /**
     * @brief Destructor.
     * @details Cleans up all allocated nodes and the header node.
     */
    ~jump_list() {
        clear();
        if (header) {
            AllocatorTraits::destroy(alloc, header);
            AllocatorTraits::deallocate(alloc, header, 1);
        }
    }

    /**
     * @brief Copy assignment operator.
     * @param other The jump_list to copy elements from.
     * @return A reference to `*this`.
     * @details Clears the current list and copies all elements from `other`.
     */
    jump_list& operator=(const jump_list& other) {
        if (this != &other) {
            clear();
            comp = other.comp; // Copy comparator
            // Allocator might also be copied if Propagate_on_container_copy_assignment is true
            // For simplicity, assuming default allocator or handling it externally if needed.
            insert(other.begin(), other.end());
        }
        return *this;
    }

    /**
     * @brief Move assignment operator.
     * @param other The jump_list to move resources from.
     * @return A reference to `*this`.
     * @details Clears the current list, deallocates its resources, and then
     *          efficiently transfers ownership of resources from `other`.
     *          `other` is left in a valid, but unspecified, state (typically empty).
     */
    jump_list& operator=(jump_list&& other) noexcept {
        if (this != &other) {
            clear(); // Clear current content
            if (header) { // Destroy and deallocate current header
                AllocatorTraits::destroy(alloc, header);
                AllocatorTraits::deallocate(alloc, header, 1);
            }
            
            // Transfer resources
            header = other.header;
            level = other.level;
            size_ = other.size_;
            comp = std::move(other.comp);
            alloc = std::move(other.alloc);
            rng = std::move(other.rng);
            
            // Leave other in a valid, empty state
            other.header = AllocatorTraits::allocate(other.alloc, 1);
            AllocatorTraits::construct(other.alloc, other.header, MAX_LEVEL);
            other.level = 0;
            other.size_ = 0;
        }
        return *this;
    }

    /**
     * @brief Initializer list assignment operator.
     * @param init An initializer list of elements to insert.
     * @return A reference to `*this`.
     * @details Clears the current list and inserts elements from the initializer list.
     */
    jump_list& operator=(std::initializer_list<T> init) {
        clear();
        insert(init.begin(), init.end());
        return *this;
    }

    // Iterators

    /**
     * @brief Returns an iterator to the beginning of the jump_list.
     * @return An iterator pointing to the first element. If the list is empty, it returns `end()`.
     */
    iterator begin() { return iterator(header->forward[0], header); }

    /**
     * @brief Returns a const iterator to the beginning of the jump_list.
     * @return A const iterator pointing to the first element. If the list is empty, it returns `end()`.
     */
    const_iterator begin() const { return const_iterator(header->forward[0], header); }

    /**
     * @brief Returns a const iterator to the beginning of the jump_list.
     * @details Same as `begin() const`, useful for explicit const-correctness.
     * @return A const iterator pointing to the first element.
     */
    const_iterator cbegin() const { return const_iterator(header->forward[0], header); }
    
    /**
     * @brief Returns an iterator to the end of the jump_list.
     * @return An iterator pointing one past the last element. This is a sentinel value.
     */
    iterator end() { return iterator(nullptr, header); }

    /**
     * @brief Returns a const iterator to the end of the jump_list.
     * @return A const iterator pointing one past the last element. This is a sentinel value.
     */
    const_iterator end() const { return const_iterator(nullptr, header); }

    /**
     * @brief Returns a const iterator to the end of the jump_list.
     * @details Same as `end() const`, useful for explicit const-correctness.
     * @return A const iterator pointing one past the last element.
     */
    const_iterator cend() const { return const_iterator(nullptr, header); }

    // Capacity

    /**
     * @brief Checks if the jump_list is empty.
     * @return True if the list contains no elements, false otherwise.
     */
    bool empty() const { return size_ == 0; }

    /**
     * @brief Returns the number of elements in the jump_list.
     * @return The number of elements.
     */
    size_type size() const { return size_; }

    /**
     * @brief Returns the maximum number of elements the jump_list can hold.
     * @return The maximum number of elements. This is limited by the allocator.
     */
    size_type max_size() const { return AllocatorTraits::max_size(alloc); }

    // Modifiers

    /**
     * @brief Clears the contents of the jump_list.
     * @details Removes all elements from the jump_list, leaving it empty.
     *          The header node itself is not destroyed, only its forward pointers are reset.
     */
    void clear() {
        Node* current = header->forward[0];
        while (current) {
            Node* next = current->forward[0];
            destroyNode(current);
            current = next;
        }
        for (int i = 0; i <= MAX_LEVEL; ++i) {
            header->forward[i] = nullptr;
        }
        header->backward = nullptr; // Reset backward pointer for header
        level = 0;
        size_ = 0;
    }

    /**
     * @brief Inserts a new element into the jump_list.
     * @param value The value to be inserted.
     * @return A `std::pair` consisting of an iterator to the newly inserted element
     *         (or to the existing element if a duplicate) and a boolean indicating
     *         whether the insertion took place (true if inserted, false if already exists).
     */
    std::pair<iterator, bool> insert(const T& value) {
        std::vector<Node*> update(MAX_LEVEL + 1);
        Node* current = header;

        // Find position and fill update array
        for (int i = level; i >= 0; --i) {
            while (current->forward[i] && comp(current->forward[i]->data, value)) {
                current = current->forward[i];
            }
            update[i] = current;
        }

        current = current->forward[0]; // Move to data level

        // Check if value already exists
        if (current && !comp(current->data, value) && !comp(value, current->data)) {
            return std::make_pair(iterator(current, header), false); // Element already exists
        }

        // Insert new node
        int newLevel = randomLevel();
        if (newLevel > level) {
            for (int i = level + 1; i <= newLevel; ++i) {
                update[i] = header; // New levels point from header
            }
            level = newLevel; // Update max level of the list
        }

        Node* newNode = createNode(value, newLevel);
        for (int i = 0; i <= newLevel; ++i) {
            newNode->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = newNode;
        }
        
        // Update backward pointers
        newNode->backward = update[0]; // Previous node at level 0
        if (newNode->forward[0]) {
            newNode->forward[0]->backward = newNode; // Next node at level 0 points back to newNode
        } else {
            header->backward = newNode; // If newNode is the new last element, header's backward points to it
        }

        ++size_;
        return std::make_pair(iterator(newNode, header), true);
    }

    /**
     * @brief Inserts a new element into the jump_list using move semantics.
     * @param value The value to be inserted (moved).
     * @return A `std::pair` consisting of an iterator to the newly inserted element
     *         (or to the existing element if a duplicate) and a boolean indicating
     *         whether the insertion took place (true if inserted, false if already exists).
     */
    std::pair<iterator, bool> insert(T&& value) {
        std::vector<Node*> update(MAX_LEVEL + 1);
        Node* current = header;

        // Find position and fill update array
        for (int i = level; i >= 0; --i) {
            while (current->forward[i] && comp(current->forward[i]->data, value)) {
                current = current->forward[i];
            }
            update[i] = current;
        }

        current = current->forward[0]; // Move to data level

        // Check if value already exists
        if (current && !comp(current->data, value) && !comp(value, current->data)) {
            return std::make_pair(iterator(current, header), false); // Element already exists
        }

        // Insert new node
        int newLevel = randomLevel();
        if (newLevel > level) {
            for (int i = level + 1; i <= newLevel; ++i) {
                update[i] = header; // New levels point from header
            }
            level = newLevel; // Update max level of the list
        }

        Node* newNode = createNode(std::move(value), newLevel);
        for (int i = 0; i <= newLevel; ++i) {
            newNode->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = newNode;
        }
        
        // Update backward pointers
        newNode->backward = update[0]; // Previous node at level 0
        if (newNode->forward[0]) {
            newNode->forward[0]->backward = newNode; // Next node at level 0 points back to newNode
        } else {
            header->backward = newNode; // If newNode is the new last element, header's backward points to it
        }

        ++size_;
        return std::make_pair(iterator(newNode, header), true);
    }

    /**
     * @brief Inserts a new element into the jump_list, with a hint for insertion position.
     * @details For a Skip List, the hint is typically ignored as insertion always involves
     *          traversing from the header. This implementation simply calls the regular insert.
     * @param hint An iterator suggesting where the insertion might take place (ignored).
     * @param value The value to be inserted.
     * @return An iterator to the newly inserted element, or to the existing element if a duplicate.
     */
    iterator insert(const_iterator hint, const T& value) {
        // For simplicity, ignore hint and use regular insert
        return insert(value).first;
    }

    /**
     * @brief Inserts a new element into the jump_list using move semantics, with a hint.
     * @details For a Skip List, the hint is typically ignored. This implementation simply calls the regular insert.
     * @param hint An iterator suggesting where the insertion might take place (ignored).
     * @param value The value to be inserted (moved).
     * @return An iterator to the newly inserted element, or to the existing element if a duplicate.
     */
    iterator insert(const_iterator hint, T&& value) {
        // For simplicity, ignore hint and use regular insert
        return insert(std::move(value)).first;
    }

    /**
     * @brief Inserts a range of elements into the jump_list.
     * @tparam InputIt The input iterator type.
     * @param first An input iterator to the beginning of the range.
     * @param last An input iterator to the end of the range.
     */
    template<class InputIt>
    void insert(InputIt first, InputIt last) {
        for (auto it = first; it != last; ++it) {
            insert(*it);
        }
    }

    /**
     * @brief Inserts elements from an initializer list into the jump_list.
     * @param init An initializer list of elements to insert.
     */
    void insert(std::initializer_list<T> init) {
        insert(init.begin(), init.end());
    }

    /**
     * @brief Constructs and inserts an element in place.
     * @tparam Args Variadic template arguments for the constructor of `T`.
     * @param args Arguments to forward to the constructor of `T`.
     * @return A `std::pair` consisting of an iterator to the newly inserted element
     *         (or to the existing element if a duplicate) and a boolean indicating
     *         whether the insertion took place.
     */
    template<class... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        return insert(T(std::forward<Args>(args)...));
    }

    /**
     * @brief Constructs and inserts an element in place, with a hint.
     * @tparam Args Variadic template arguments for the constructor of `T`.
     * @param hint An iterator suggesting where the insertion might take place (ignored).
     * @param args Arguments to forward to the constructor of `T`.
     * @return An iterator to the newly inserted element, or to the existing element if a duplicate.
     */
    template<class... Args>
    iterator emplace_hint(const_iterator hint, Args&&... args) {
        return insert(hint, T(std::forward<Args>(args)...));
    }

    /**
     * @brief Erases the element at a specific position.
     * @param pos A const_iterator pointing to the element to be erased.
     * @return An iterator pointing to the element immediately following the erased element,
     *         or `end()` if the erased element was the last one.
     */
    iterator erase(const_iterator pos) {
        if (pos == end()) {
             // throw std::out_of_range("Cannot erase end() iterator."); // Or handle as desired
             return end();
        }

        const T& value = *pos; // Get value to search for deletion

        std::vector<Node*> update(MAX_LEVEL + 1);
        Node* current = header;

        // Find the node to delete and update pointers
        for (int i = level; i >= 0; --i) {
            while (current->forward[i] && comp(current->forward[i]->data, value)) {
                current = current->forward[i];
            }
            update[i] = current;
        }

        current = current->forward[0]; // Node at level 0

        // Check if the found node matches the value from the iterator (handles potential duplicates or not found if iterator is invalid)
        if (!current || comp(current->data, value) || comp(value, current->data)) {
            // This case should ideally not happen if 'pos' is a valid iterator to an element
            // that hasn't been modified or deleted by other means.
            return end(); // Element not found or iterator was invalid for erase by value.
        }

        Node* next_node = current->forward[0]; // Node immediately following 'current'

        // Remove node from all levels
        for (int i = 0; i <= level; ++i) {
            if (update[i]->forward[i] == current) { // Only update if current is actually in this level
                update[i]->forward[i] = current->forward[i];
            }
        }
        
        // Update backward pointers
        Node* prev = current->backward;
        if (next_node) {
            next_node->backward = prev;
        } else {
            header->backward = prev; // If current was the last element, header's backward points to its previous.
        }

        destroyNode(current); // Deallocate node

        // Adjust the list's max_level if necessary
        while (level > 0 && !header->forward[level]) {
            --level;
        }

        --size_;
        return iterator(next_node, header);
    }

    /**
     * @brief Erases a range of elements from the jump_list.
     * @param first A const_iterator to the beginning of the range to erase (inclusive).
     * @param last A const_iterator to the end of the range to erase (exclusive).
     * @return An iterator pointing to the element immediately following the last erased element.
     */
    iterator erase(const_iterator first, const_iterator last) {
        iterator result = end();
        while (first != last) {
            auto next = first;
            ++next;
            result = erase(first); // erase returns an iterator to the next element
            first = next;
        }
        return result;
    }

    /**
     * @brief Erases an element with a specific key.
     * @param key The key of the element to be erased.
     * @return The number of elements erased (0 or 1, as jump_list stores unique elements).
     */
    size_type erase(const T& key) {
        auto it = find(key);
        if (it != end()) {
            erase(it);
            return 1;
        }
        return 0;
    }

    /**
     * @brief Swaps the contents of this jump_list with another jump_list.
     * @param other The jump_list to swap with.
     * @details This operation is efficient as it only swaps internal pointers and states.
     */
    void swap(jump_list& other) noexcept {
        std::swap(header, other.header);
        std::swap(level, other.level);
        std::swap(size_, other.size_);
        std::swap(comp, other.comp);
        std::swap(alloc, other.alloc);
        std::swap(rng, other.rng);
    }

    // Lookup

    /**
     * @brief Returns the number of elements with a specific key.
     * @param key The key to count.
     * @return 1 if an element with the key exists, 0 otherwise (since jump_list stores unique elements).
     */
    size_type count(const T& key) const {
        return find(key) != end() ? 1 : 0;
    }

    /**
     * @brief Finds an element with a specific key.
     * @param key The key to find.
     * @return An iterator to the found element, or `end()` if the element is not found.
     */
    iterator find(const T& key) {
        Node* current = header;
        for (int i = level; i >= 0; --i) {
            while (current->forward[i] && comp(current->forward[i]->data, key)) {
                current = current->forward[i];
            }
        }
        current = current->forward[0]; // Move to the data level
        
        if (current && !comp(current->data, key) && !comp(key, current->data)) {
            return iterator(current, header); // Found
        }
        return end(); // Not found
    }

    /**
     * @brief Finds an element with a specific key (const version).
     * @param key The key to find.
     * @return A const_iterator to the found element, or `end()` if the element is not found.
     */
    const_iterator find(const T& key) const {
        Node* current = header;
        for (int i = level; i >= 0; --i) {
            while (current->forward[i] && comp(current->forward[i]->data, key)) {
                current = current->forward[i];
            }
        }
        current = current->forward[0]; // Move to the data level
        
        if (current && !comp(current->data, key) && !comp(key, current->data)) {
            return const_iterator(current, header); // Found
        }
        return end(); // Not found
    }

    /**
     * @brief Checks if the jump_list contains an element with a specific key.
     * @param key The key to check for.
     * @return True if an element with the key exists, false otherwise.
     */
    bool contains(const T& key) const {
        return find(key) != end();
    }

    /**
     * @brief Returns a pair of iterators defining the range of elements with a specific key.
     * @param key The key to find the range for.
     * @return A `std::pair` where the first element is an iterator to the first element
     *         with the key, and the second element is an iterator to the element
     *         immediately following it. If the key is not found, both iterators are `end()`.
     */
    std::pair<iterator, iterator> equal_range(const T& key) {
        auto it = find(key);
        if (it == end()) {
            return std::make_pair(it, it);
        }
        auto next = it;
        ++next;
        return std::make_pair(it, next);
    }

    /**
     * @brief Returns a pair of const iterators defining the range of elements with a specific key (const version).
     * @param key The key to find the range for.
     * @return A `std::pair` where the first element is a const_iterator to the first element
     *         with the key, and the second element is a const_iterator to the element
     *         immediately following it. If the key is not found, both iterators are `end()`.
     */
    std::pair<const_iterator, const_iterator> equal_range(const T& key) const {
        auto it = find(key);
        if (it == end()) {
            return std::make_pair(it, it);
        }
        auto next = it;
        ++next;
        return std::make_pair(it, next);
    }

    /**
     * @brief Returns an iterator to the first element not less than a given key.
     * @details This corresponds to the first element whose key is not ordered before `key`
     *          (i.e., greater than or equal to `key`).
     * @param key The key to search for.
     * @return An iterator to the lower bound of `key`.
     */
    iterator lower_bound(const T& key) {
        Node* current = header;
        for (int i = level; i >= 0; --i) {
            while (current->forward[i] && comp(current->forward[i]->data, key)) {
                current = current->forward[i];
            }
        }
        return iterator(current->forward[0], header);
    }

    /**
     * @brief Returns a const iterator to the first element not less than a given key (const version).
     * @param key The key to search for.
     * @return A const iterator to the lower bound of `key`.
     */
    const_iterator lower_bound(const T& key) const {
        Node* current = header;
        for (int i = level; i >= 0; --i) {
            while (current->forward[i] && comp(current->forward[i]->data, key)) {
                current = current->forward[i];
            }
        }
        return const_iterator(current->forward[0], header);
    }

    /**
     * @brief Returns an iterator to the first element greater than a given key.
     * @details This corresponds to the first element whose key is ordered after `key`.
     * @param key The key to search for.
     * @return An iterator to the upper bound of `key`.
     */
    iterator upper_bound(const T& key) {
        Node* current = header;
        for (int i = level; i >= 0; --i) {
            while (current->forward[i] && !comp(key, current->forward[i]->data)) { // current->forward[i]->data <= key
                current = current->forward[i];
            }
        }
        return iterator(current->forward[0], header);
    }

    /**
     * @brief Returns a const iterator to the first element greater than a given key (const version).
     * @param key The key to search for.
     * @return A const iterator to the upper bound of `key`.
     */
    const_iterator upper_bound(const T& key) const {
        Node* current = header;
        for (int i = level; i >= 0; --i) {
            while (current->forward[i] && !comp(key, current->forward[i]->data)) { // current->forward[i]->data <= key
                current = current->forward[i];
            }
        }
        return const_iterator(current->forward[0], header);
    }

    // Observers

    /**
     * @brief Returns the comparison object used by the jump_list.
     * @return A copy of the comparison object.
     */
    key_compare key_comp() const { return comp; }

    /**
     * @brief Returns the value comparison object used by the jump_list.
     * @details For set-like containers, this is typically the same as `key_comp()`.
     * @return A copy of the value comparison object.
     */
    value_compare value_comp() const { return comp; }

    /**
     * @brief Returns the allocator object used by the jump_list.
     * @return A copy of the allocator object.
     */
    allocator_type get_allocator() const { return alloc; }

    // Reverse iterators

    using reverse_iterator = std::reverse_iterator<iterator>;               /**< @brief Reverse iterator type. */
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;   /**< @brief Const reverse iterator type. */

    /**
     * @brief Returns a reverse iterator to the last element of the jump_list.
     * @return A reverse iterator pointing to the last element.
     */
    reverse_iterator rbegin() { return reverse_iterator(end()); }

    /**
     * @brief Returns a const reverse iterator to the last element of the jump_list.
     * @return A const reverse iterator pointing to the last element.
     */
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }

    /**
     * @brief Returns a const reverse iterator to the last element of the jump_list.
     * @return A const reverse iterator pointing to the last element.
     */
    const_reverse_iterator crbegin() const { return const_reverse_iterator(cend()); }

    /**
     * @brief Returns a reverse iterator to the theoretical element before the first element.
     * @return A reverse iterator pointing one before the first element.
     */
    reverse_iterator rend() { return reverse_iterator(begin()); }

    /**
     * @brief Returns a const reverse iterator to the theoretical element before the first element.
     * @return A const reverse iterator pointing one before the first element.
     */
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    /**
     * @brief Returns a const reverse iterator to the theoretical element before the first element.
     * @return A const reverse iterator pointing one before the first element.
     */
    const_reverse_iterator crend() const { return const_reverse_iterator(cbegin()); }

    static_assert(std::predicate<Compare, const T&, const T&>, "Compare must be invocable with T");
    static_assert(std::bidirectional_iterator<iterator>, "iterator must be bidirectional");
    static_assert(std::bidirectional_iterator<const_iterator>, "const_iterator must be bidirectional");
};

// Non-member functions

/**
 * @brief Checks if two jump_list containers are equal.
 * @param lhs The left-hand side jump_list.
 * @param rhs The right-hand side jump_list.
 * @return True if both jump_lists have the same size and contain the same elements in the same order, false otherwise.
 */
template<typename T, typename Compare, typename Allocator>
bool operator==(const jump_list<T, Compare, Allocator>& lhs,
                 const jump_list<T, Compare, Allocator>& rhs) {
    return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

/**
 * @brief Checks if two jump_list containers are not equal.
 * @param lhs The left-hand side jump_list.
 * @param rhs The right-hand side jump_list.
 * @return True if the jump_lists are not equal, false otherwise.
 */
template<typename T, typename Compare, typename Allocator>
bool operator!=(const jump_list<T, Compare, Allocator>& lhs,
                 const jump_list<T, Compare, Allocator>& rhs) {
    return !(lhs == rhs);
}

/**
 * @brief Compares two jump_list containers lexicographically.
 * @param lhs The left-hand side jump_list.
 * @param rhs The right-hand side jump_list.
 * @return True if `lhs` is lexicographically less than `rhs`, false otherwise.
 */
template<typename T, typename Compare, typename Allocator>
bool operator<(const jump_list<T, Compare, Allocator>& lhs,
               const jump_list<T, Compare, Allocator>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

/**
 * @brief Compares two jump_list containers lexicographically.
 * @param lhs The left-hand side jump_list.
 * @param rhs The right-hand side jump_list.
 * @return True if `lhs` is lexicographically less than or equal to `rhs`, false otherwise.
 */
template<typename T, typename Compare, typename Allocator>
bool operator<=(const jump_list<T, Compare, Allocator>& lhs,
                const jump_list<T, Compare, Allocator>& rhs) {
    return !(rhs < lhs);
}

/**
 * @brief Compares two jump_list containers lexicographically.
 * @param lhs The left-hand side jump_list.
 * @param rhs The right-hand side jump_list.
 * @return True if `lhs` is lexicographically greater than `rhs`, false otherwise.
 */
template<typename T, typename Compare, typename Allocator>
bool operator>(const jump_list<T, Compare, Allocator>& lhs,
               const jump_list<T, Compare, Allocator>& rhs) {
    return rhs < lhs;
}

/**
 * @brief Compares two jump_list containers lexicographically.
 * @param lhs The left-hand side jump_list.
 * @param rhs The right-hand side jump_list.
 * @return True if `lhs` is lexicographically greater than or equal to `rhs`, false otherwise.
 */
template<typename T, typename Compare, typename Allocator>
bool operator>=(const jump_list<T, Compare, Allocator>& lhs,
                const jump_list<T, Compare, Allocator>& rhs) {
    return !(lhs < rhs);
}

/**
 * @brief Specializes the `std::swap` algorithm for `jump_list`.
 * @param lhs The first jump_list to swap.
 * @param rhs The second jump_list to swap.
 * @details This non-member `swap` function calls the member `swap` function.
 */
template<typename T, typename Compare, typename Allocator>
void swap(jump_list<T, Compare, Allocator>& lhs, jump_list<T, Compare, Allocator>& rhs) noexcept {
    lhs.swap(rhs);
}

#endif // JUMP_LIST_H