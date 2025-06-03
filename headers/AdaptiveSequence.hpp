#pragma once
#include "DynamicArray.hpp"
#include "Sequence.hpp"

template <typename T>
class AdaptiveSequence : public Sequence<T> {
private:
    DynamicArray<T>* buffer;
    int frontIndex;
    int backIndex;
    int size;

    int _getCapacity(int val) const {
        if (val == 0) return 0;
        int ret = 1;
        while (ret < val) ret <<= 1;
        return ret;
    }

    void ensureCapacity() {
        if (backIndex + 1 >= buffer->GetSize()) {
            int newCapacity = buffer->GetSize() == 0 ? 1 : buffer->GetSize() * 2;
            DynamicArray<T>* newBuffer = new DynamicArray<T>(newCapacity);
            
            int newFront = newCapacity / 4;
            for (int i = 0; i < size; ++i) {
                (*newBuffer)[newFront + i] = (*buffer)[frontIndex + i];
            }
                        
            delete buffer;
            buffer = newBuffer;
            frontIndex = newFront;
            backIndex = newFront + size - 1;
        }
    }

    void ensureFrontCapacity() {
        if (frontIndex <= 0) {
            int newCapacity = buffer->GetSize() == 0 ? 1 : buffer->GetSize() * 2;
            DynamicArray<T>* newBuffer = new DynamicArray<T>(newCapacity);
            
            int newFront = newCapacity / 4;
            for (int i = 0; i < size; ++i) {
                (*newBuffer)[newFront + i] = (*buffer)[frontIndex + i];
            }
            
            delete buffer;
            buffer = newBuffer;
            frontIndex = newFront;
            backIndex = newFront + size - 1;
        }
    }

    virtual AdaptiveSequence<T>* Instance() = 0;
    virtual AdaptiveSequence<T>* CreateEmptyAdaptiveSequence() const = 0;

public:
    AdaptiveSequence() : buffer(new DynamicArray<T>()), frontIndex(0), backIndex(-1), size(0) {}

    AdaptiveSequence(const T* items, int count) 
        : buffer(new DynamicArray<T>(_getCapacity(count))), frontIndex(0), backIndex(count-1), size(count) {
        for (int i = 0; i < count; ++i) {
            (*buffer)[i] = items[i];
        }
    }

    AdaptiveSequence(const AdaptiveSequence& other) 
        : buffer(new DynamicArray<T>(*other.buffer)), 
          frontIndex(other.frontIndex), 
          backIndex(other.backIndex), 
          size(other.size) {}

    AdaptiveSequence(AdaptiveSequence&& other) noexcept 
        : buffer(other.buffer), 
          frontIndex(other.frontIndex), 
          backIndex(other.backIndex), 
          size(other.size) {
        other.buffer = nullptr;
    }

    ~AdaptiveSequence() override {
        delete buffer;
    }

    AdaptiveSequence& operator=(const AdaptiveSequence& other) {
        if (this != &other) {
            delete buffer;
            buffer = new DynamicArray<T>(*other.buffer);
            frontIndex = other.frontIndex;
            backIndex = other.backIndex;
            size = other.size;
        }
        return *this;
    }

    AdaptiveSequence& operator=(AdaptiveSequence&& other) noexcept {
        if (this != &other) {
            delete buffer;
            buffer = other.buffer;
            frontIndex = other.frontIndex;
            backIndex = other.backIndex;
            size = other.size;
            other.buffer = nullptr;
        }
        return *this;
    }
    virtual Sequence<T>* CreateEmptySequence() const override {
        return CreateEmptyAdaptiveSequence();
    }
    virtual Sequence<T>* AppendInternal(const T& item) override {
        ensureCapacity();
        if (size == 0) {
            frontIndex = backIndex = buffer->GetSize() == 0 ? 0 : buffer->GetSize() / 2;
        } else {
            if (backIndex + 1 >= buffer->GetSize()) ensureCapacity();
            backIndex++;
        }
        (*buffer)[backIndex] = item;
        size++;
        return this;
    }

    virtual Sequence<T>* PrependInternal(const T& item) override {
        ensureFrontCapacity();
        if (size == 0) {
            frontIndex = backIndex = buffer->GetSize() == 0 ? 0 : buffer->GetSize() / 2;
        } else {
            if (frontIndex - 1 < 0) ensureFrontCapacity();
            frontIndex--;
        }
        (*buffer)[frontIndex] = item;
        size++;
        return this;
    }

    virtual Sequence<T>* InsertAtInternal(const T& item, int index) override {
        if (index < 0 || index > size) throw std::out_of_range("Index out of range");
        
        if (index == 0) return PrependInternal(item);
        if (index == size) return AppendInternal(item);
        
        Sequence<T>* newSeq = CreateEmptySequence();
        for (int i = 0; i < index; ++i) {
            newSeq->AppendInternal((*this)[i]);
        }
        newSeq->AppendInternal(item);
        for (int i = index; i < size; ++i) {
            newSeq->AppendInternal((*this)[i]);
        }
        
        return newSeq;
    }

    virtual Sequence<T>* ConcatInternal(const Sequence<T>* other) override {
        Sequence<T>* newSeq = CreateEmptySequence();
        for (int i = 0; i < this->GetLength(); ++i) {
            newSeq->AppendInternal((*this)[i]);
        }
        for (int i = 0; i < other->GetLength(); ++i) {
            newSeq->AppendInternal(other->Get(i));
        }
        return newSeq;
    }

    int GetLength() const override {
        return size;
    }

    const T& GetFirst() const override {
        if (size == 0) throw std::out_of_range("Sequence is empty");
        return (*buffer)[frontIndex];
    }

    const T& GetLast() const override {
        if (size == 0) throw std::out_of_range("Sequence is empty");
        return (*buffer)[backIndex];
    }

    const T& Get(int index) const override {
        if (index < 0 || index >= size) throw std::out_of_range("Index out of range");
        return (*buffer)[frontIndex + index];
    }

    T& GetFirst() override {
        if (size == 0) throw std::out_of_range("Sequence is empty");
        return (*buffer)[frontIndex];
    }

    T& GetLast() override {
        if (size == 0) throw std::out_of_range("Sequence is empty");
        return (*buffer)[backIndex];
    }

    T& Get(int index) override {
        if (index < 0 || index >= size) throw std::out_of_range("Index out of range");
        return (*buffer)[frontIndex + index];
    }

    Sequence<T>* Append(const T& item) override {
        return Instance()->AppendInternal(item);
    }

    Sequence<T>* Prepend(const T& item) override {
        return Instance()->PrependInternal(item);
    }

    Sequence<T>* InsertAt(const T& item, int index) override {
        if (index < 0 || index > size) throw std::out_of_range("Index out of range");
        return Instance()->InsertAtInternal(item, index);
    }

    Sequence<T>* Concat(const Sequence<T>* other) override {
        return Instance()->ConcatInternal(other);
    }

    T& operator[](int index) override {
        return Get(index);
    }

    Sequence<T>* GetSubsequence(int startIndex, int endIndex) const override {
        if (size == 0 || startIndex < 0 || endIndex >= size || startIndex > endIndex) {
            throw std::out_of_range("Invalid index range");
        }
        
        AdaptiveSequence<T>* subSeq = CreateEmptyAdaptiveSequence();
        for (int i = startIndex; i <= endIndex; ++i) {
            subSeq->AppendInternal(this->Get(i));
        }
        return subSeq;
    }

    class Iterator {
    private:
        AdaptiveSequence<T>* sequence;
        int index;
    
    public:
        Iterator(AdaptiveSequence<T>* seq, int ind) : sequence(seq), index(ind) {}

        T& operator*() {
            return sequence->Get(index);
        }

        Iterator& operator++() {
            ++index;
            return *this;
        }

        bool operator!=(const Iterator& other) const {
            return index != other.index;
        }
    };

    Iterator begin() {
        return Iterator(this, 0);
    }

    Iterator end() {
        return Iterator(this, size);
    }
};


template <typename T>
class MutableAdaptiveSequence : public AdaptiveSequence<T> {
public:
    using tag = MutableSequenceTag;

    MutableAdaptiveSequence() : AdaptiveSequence<T>() {}
    MutableAdaptiveSequence(const T* items, int count) : AdaptiveSequence<T>(items, count) {}
    MutableAdaptiveSequence(const AdaptiveSequence<T>& other) : AdaptiveSequence<T>(other) {}
    MutableAdaptiveSequence(AdaptiveSequence<T>&& other) : AdaptiveSequence<T>(std::move(other)) {}

protected:
    virtual AdaptiveSequence<T>* Instance() override {
        return this;
    }
    
    virtual AdaptiveSequence<T>* CreateEmptyAdaptiveSequence() const override {
        return new MutableAdaptiveSequence<T>();
    }

public:
    virtual Sequence<T>* CreateEmptySequence() const override {
        return new MutableAdaptiveSequence<T>();
    }
};


template <typename T>
class ImmutableAdaptiveSequence : public AdaptiveSequence<T> {
private:
    AdaptiveSequence<T>* Clone() const {
        return new ImmutableAdaptiveSequence<T>(*this);
    }

public:
    using tag = ImmutableSequenceTag;

    ImmutableAdaptiveSequence() : AdaptiveSequence<T>() {}
    ImmutableAdaptiveSequence(const T* items, int count) : AdaptiveSequence<T>(items, count) {}
    ImmutableAdaptiveSequence(const Sequence<T>& other) : AdaptiveSequence<T>(other) {}
    ImmutableAdaptiveSequence(AdaptiveSequence<T>&& other) : AdaptiveSequence<T>(std::move(other)) {}
    ImmutableAdaptiveSequence(const ImmutableAdaptiveSequence<T>& other) : AdaptiveSequence<T>(other) {}

protected:
    virtual AdaptiveSequence<T>* Instance() override {
        return Clone();
    }
    
    virtual AdaptiveSequence<T>* CreateEmptyAdaptiveSequence() const override {
        return new ImmutableAdaptiveSequence<T>();
    }

public:
    virtual Sequence<T>* CreateEmptySequence() const override {
        return new ImmutableAdaptiveSequence<T>();
    }
};