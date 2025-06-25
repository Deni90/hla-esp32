#ifndef circular_deque_h
#define circular_deque_h

#include <optional>

namespace hla {
template <typename T> class CircularDeque {
  private:
    struct Node {
        T value;
        Node* next;
        Node* prev;
        Node(T value) : value(value), next(nullptr), prev(nullptr) {}
    };

  public:
    class Cursor {
      public:
        /**
         * @brief Constructor
         *
         * @param[in] start Pointer to a node
         */
        Cursor(Node* start) : mCurrent(start) {}

        /**
         * @brief Return the value that the cursor holds
         * @return the value
         */
        T& value() { return mCurrent->value; }

        /**
         * @brief Move cursor to the next element
         */
        Cursor next() const {
            return mCurrent ? Cursor(mCurrent->next) : Cursor(nullptr);
        }

        /**
         * @brief Move cursor to the previous element
         */
        Cursor prev() const {
            return mCurrent ? Cursor(mCurrent->prev) : Cursor(nullptr);
        }

        /**
         * @brief Check if the cursor is valud, not null
         * @return True if valid, false if invalid
         */
        bool isValid() const { return mCurrent != nullptr; }

        /**
         * @brief Reset cursor
         */
        void reset() { mCurrent = nullptr; }

      private:
        Node* mCurrent;
    };

    /**
     * @brief Constructor
     */
    CircularDeque() : mHead(nullptr), mTail(nullptr), mSize(0) {}

    /**
     * @brief Destructor
     */
    ~CircularDeque() {
        if (!mHead) {
            return;
        }
        Node* ptr = mHead->next;
        while (ptr != mHead) {
            Node* tmp = ptr;
            ptr = ptr->next;
            delete tmp;
        }
        if (mHead) {
            delete mHead;
        }
    }

    /**
     * @brief Copy constructor
     */
    CircularDeque(const CircularDeque& other) {
        if (!other.mHead || !other.mTail) {
            mHead = nullptr;
            mTail = nullptr;
            return;
        }
        mHead = new Node(other.mHead->value);
        Node* src = other.mHead->next;
        Node* dst = mHead;
        while (src != other.mHead) {
            Node* tmp = new Node(src->value);
            dst->next = tmp;
            tmp->prev = dst;
            dst = dst->next;
            src = src->next;
        }
        dst->next = mHead;
        mHead->prev = dst;
        mTail = mHead->prev;
        mSize = other.mSize;
    }

    /**
     * @brief Copy assignment operator
     */
    CircularDeque& operator=(const CircularDeque& other) {
        if (this == other) {
            return *this;
        }

        if (mHead) {
            Node* ptr = mHead->next;
            while (ptr != mHead) {
                Node* tmp = ptr;
                ptr = ptr->next;
                delete tmp;
            }
            if (mHead) {
                delete mHead;
                mHead = nullptr;
            }
            mTail = nullptr;
        }

        mHead = new Node(other.mHead->value);
        Node* src = other.mHead->next;
        Node* dst = mHead;
        while (src != other.mHead) {
            Node* tmp = new Node(src->value);
            dst->next = tmp;
            tmp->prev = dst;
            dst = dst->next;
            src = src->next;
        }
        dst->next = mHead;
        mHead->prev = dst;
        mTail = mHead->prev;
        mSize = other.mSize;
        return *this;
    }

    /**
     * @brief Add a new node to the beginning of the container
     * @param[in] value Value
     */
    void pushFront(T value) {
        if (!mHead && !mTail) {
            mHead = new Node(value);
            mHead->next = mHead;
            mHead->prev = mHead;
            mTail = mHead;
        } else {
            Node* tmp = new Node(value);
            tmp->next = mHead;
            tmp->prev = mTail;
            mHead->prev = tmp;
            mHead = tmp;
            mTail->next = mHead;
        }
        ++mSize;
    }

    /**
     * @brief Add a new node to the end of the container
     * @param[in] value Value
     */
    void pushBack(T value) {
        if (!mHead && !mTail) {
            mHead = new Node(value);
            mHead->next = mHead;
            mHead->prev = mHead;
            mTail = mHead;
        } else {
            Node* tmp = new Node(value);
            tmp->next = mHead;
            tmp->prev = mTail;
            mTail->next = tmp;
            mTail = tmp;
            mHead->prev = mTail;
        }
        ++mSize;
    }

    /**
     * @brief Get a value from the begining of the container
     * @return Value
     */
    std::optional<T> front() const {
        if (!mHead) {
            return std::nullopt;
        }
        return mHead->value;
    }

    /**
     * @brief Get a value from the end of the container
     * @return Value
     */
    std::optional<T> back() const {
        if (!mTail) {
            return std::nullopt;
        }
        return mTail->value;
    }

    /**
     * @brief Remove a node from the begining of the container
     */
    void popFront() {
        if (!mHead || !mTail) {
            return;
        }
        if (mHead == mHead->next && mHead == mHead->prev) {
            delete mHead;
            mHead = nullptr;
            mTail = nullptr;
            return;
        }
        Node* tmp = mHead;
        mHead = mHead->next;
        mHead->prev = mTail;
        mTail->next = mHead;
        delete tmp;
        --mSize;
    }

    /**
     * @brief Remove a node from the end of the container
     */
    void popBack() {
        if (!mHead || !mTail) {
            return;
        }
        if (mHead == mHead->next && mHead == mHead->prev) {
            delete mHead;
            mHead = nullptr;
            mTail = nullptr;
            return;
        }
        Node* tmp = mTail;
        mTail = mTail->prev;
        mTail->next = mHead;
        mHead->prev = mTail;
        delete tmp;
        --mSize;
    }

    /**
     * @brief Get a cursor that points to the beginning of the container
     * @return cursor
     */
    Cursor frontCursor() const { return Cursor(mHead); }

    /**
     * @brief Get a cursor that points to the end of the container
     * @return cursor
     */
    Cursor backCursor() const { return Cursor(mTail); }

    /**
     * @brief Get size of the container
     * @return size
     */
    int length() const { return mSize; }

    /**
     * @brief Clear the container
     */
    void empty() {
        if (!mHead) {
            return;
        }
        Node* ptr = mHead->next;
        while (ptr != mHead) {
            Node* tmp = ptr;
            ptr = ptr->next;
            delete tmp;
        }
        if (mHead) {
            delete mHead;
        }
        mHead = nullptr;
        mTail = nullptr;
        mSize = 0;
    }

  private:
    Node* mHead;
    Node* mTail;
    int mSize;
};
}   // namespace hla
#endif   // circular_deque_h