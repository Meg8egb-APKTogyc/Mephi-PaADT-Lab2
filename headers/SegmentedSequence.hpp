#pragma once
#include <stdexcept>
#include "Sequence.hpp"


template <typename T, 
    template<typename> class SegmentSequence = MutableArraySequence,
    template<typename> class ContainerSequence = MutableArraySequence>
class SegmentedSequence : public Sequence<T> {
private:
    static_assert(std::is_base_of_v<MutableSequenceTag, typename SegmentSequence<T>::tag>,
        "SegmentSequence must be a mutable sequence type");

    static_assert(std::is_base_of_v<MutableSequenceTag, typename ContainerSequence<Sequence<T>*>::tag>,
        "ContainerSequence must be a mutable sequence type");

    ContainerSequence<SegmentSequence<T>*>* segments;
    int segmentSize;
    int totalSize;

    std::tuple<SegmentSequence<T>*, int, int> getSegmentAndOffset(int index, bool isAdded=0) const {
        if (index < 0 || index >= totalSize || segments->GetLength() == 0) {
            throw std::out_of_range("Index out of range");
        }
    
        int ind = 0;
        while (ind < segments->GetLength() && index >= segments->Get(ind)->GetLength() + isAdded) {
            index -= segments->Get(ind)->GetLength();
            ++ind;
        }
    
        if (ind >= segments->GetLength()) {
            throw std::out_of_range("Index out of range");
        }
    
        return std::make_tuple(segments->Get(ind), ind, index);
    }

    void ensureCapacity(int requiredCapacity) {
        int requiredSegments = (requiredCapacity + segmentSize - 1) / segmentSize;
        while (this->segments->GetLength() < requiredSegments) {
            this->segments->Append(ContainerSequence<T>::CreateSegment());
        }
    }

    SegmentSequence<T>* createSegment() {
        return new SegmentSequence<T>();
    }

    void splitSegment(int segmentIndex, int splitPos = -1) {
        if (segmentIndex < 0 || segmentIndex >= segments->GetLength()) {
            throw std::out_of_range("Invalid segment index");
        }
    
        Sequence<T>* oldSegment = segments->Get(segmentIndex);
        int segmentLength = oldSegment->GetLength();
    
        if (splitPos == -1) {
            splitPos = segmentLength / 2;
        } else if (splitPos <= 0 || splitPos >= segmentLength) {
            throw std::invalid_argument("Invalid split position");
        }
    
        SegmentSequence<T>* newSegment = createSegment();
        SegmentSequence<T>* firstPart = static_cast<SegmentSequence<T>*>(oldSegment->GetSubsequence(0, splitPos-1));

        for (int i = splitPos; i < oldSegment->GetLength(); ++i) {
            newSegment->Append(oldSegment->Get(i));
        }

        segments->Get(segmentIndex) = firstPart;
        if (segmentIndex + 1 == segments->GetLength()) {
            segments->Append(newSegment);
        } else {
            segments->InsertAt(newSegment, segmentIndex + 1);
        }
        
        delete oldSegment;
    }

    virtual Sequence<T>* AppendInternal(const T& item) override {
        if (this->segments->GetLength() == 0) {
            this->segments->Append(createSegment());
        }

        if (this->segments->GetLast()->GetLength() >= segmentSize) {
            splitSegment(this->GetSegmentsLength() - 1);
            return AppendInternal(item);
        }

        this->segments->GetLast()->Append(item);
        totalSize++;
        return this;
    }

    virtual Sequence<T>* PrependInternal(const T& item) override {
        if (this->segments->GetLength() == 0) {
            segments->Prepend(createSegment());
        }

        if (this->segments->GetFirst()->GetLength() >= segmentSize) {
            splitSegment(0);
            return PrependInternal(item);
        }

        this->segments->GetFirst()->Prepend(item);
        totalSize++;
        return this;
    }

    virtual Sequence<T>* InsertAtInternal(const T& item, int globalIndex) override {
        if (globalIndex < 0 || globalIndex > totalSize) {
            throw std::out_of_range("Global index out of range");
        }
    
        if (globalIndex == totalSize) {
            return AppendInternal(item);
        }
        if (globalIndex == 0) {
            return PrependInternal(item);
        }
    
        auto [segment, segmentIndex, localIndex] = getSegmentAndOffset(globalIndex, 1);
        if (segment->GetLength() >= segmentSize) {
            splitSegment(segmentIndex);
            return InsertAtInternal(item, globalIndex);
        }
        
        if (localIndex == segment->GetLength()) {
            segment->Append(item);
        } else if (localIndex == 0) {
            segment->Prepend(item);
        } else {
            segment->InsertAt(item, localIndex);
        }

        totalSize++;
        return this;
    }

    virtual Sequence<T>*ConcatInternal(const Sequence<T>* other) override {
        for (int i = 0; i < other->GetLength(); ++i) {
            this->Append(other->Get(i));
        }
        return this;
    }

protected:
    virtual Sequence<T>* Instance() = 0;
    virtual SegmentedSequence<T, SegmentSequence, ContainerSequence>* CreateEmptySegSequence() const = 0;

public:
    explicit SegmentedSequence(int segmentSize_) :
        segments(new ContainerSequence<SegmentSequence<T>*>()),
        segmentSize(segmentSize_),
        totalSize(0) {
        if (segmentSize_ <= 0) {
            throw std::invalid_argument("Segment size must be positive");
        }
    }

    SegmentedSequence(const T* items, int count, int segmentSize_ = 10) : 
    segments(new ContainerSequence<SegmentSequence<T>*>()),
    segmentSize(segmentSize_),
    totalSize(0)
    {
        if (segmentSize_ <= 0) throw std::invalid_argument("Segment size must be positive");

        for (int i = 0; i < count; ++i) {
            AppendInternal(items[i]);
        }
    }

    SegmentedSequence(const Sequence<T>& other, int segmentSize_ = 10) : 
    segments(new ContainerSequence<Sequence<T>*>()),
    segmentSize(segmentSize_),
    totalSize(0)
    {
        if (segmentSize_ <= 0) throw std::invalid_argument("Segment size must be positive");

        for (int i = 0; i < other.GetLength(); ++i) {
            AppendInternal(other.Get(i));
        }
    }

    SegmentedSequence(const SegmentedSequence& other) : 
    segments(new ContainerSequence<SegmentSequence<T>*>()),
    segmentSize(other.segmentSize),
    totalSize(0)
    {
        for (int i = 0; i < other.segments->GetLength(); ++i) {
            SegmentSequence<T>* newSegment = static_cast<SegmentSequence<T>*>(other.segments->Get(i)->GetSubsequence(0, other.segments->Get(i)->GetLength()-1));
            segments->Append(newSegment);
        }
        totalSize = other.totalSize;
    }

    SegmentedSequence(SegmentedSequence&& other) noexcept :
    segments(other.segments),
    segmentSize(other.segmentSize),
    totalSize(other.totalSize)
    {
        other.segments = nullptr;
        other.totalSize = 0;
    }

    SegmentedSequence& operator=(const SegmentedSequence& other) {
        if (this != &other) {
            for (int i = 0; i < segments->GetLength(); ++i) {
                delete segments->Get(i);
            }
            delete segments;

            segments = new ContainerSequence<Sequence<T>*>();
            segmentSize = other.segmentSize;
            totalSize = 0;

            for (int i = 0; i < other.segments->GetLength(); ++i) {
                Sequence<T>* newSegment = other.segments->Get(i)->GetSubsequence(0, other.segments->Get(i)->GetLength()-1);
                segments->Append(newSegment);
            }
            totalSize = other.totalSize;
        }
        return *this;
    }

    SegmentedSequence& operator=(SegmentedSequence&& other) noexcept {
        if (this != &other) {
            for (int i = 0; i < segments->GetLength(); ++i) {
                delete segments->Get(i);
            }
            delete segments;

            segments = other.segments;
            segmentSize = other.segmentSize;
            totalSize = other.totalSize;

            other.segments = nullptr;
            other.totalSize = 0;
        }
        return *this;
    }

    ~SegmentedSequence() override {
        for (int i = 0; i < this->segments->GetLength(); ++i) {
            delete this->segments->Get(i);
        }
        delete this->segments;
    }

    virtual Sequence<T>* GetSubsequence(int startIndex, int endIndex) const override {
        if (std::min(startIndex, endIndex) < 0 || std::max(startIndex, endIndex) >= totalSize) {
            throw std::out_of_range("Invalid subsequence range");
        }

        auto* result = this->CreateEmptySegSequence();
        for (int i = std::min(startIndex, endIndex); i <= std::max(startIndex, endIndex); ++i) {
            if (startIndex < endIndex) {
                result->Append(this->Get(i));
            } else {
                result->Prepend(this->Get(i));
            }
        }

        return result;
    }

    virtual T& operator[] (int index) override {
        return this->Get(index);
    }

    virtual const T& GetFirst() const override {
        if (totalSize == 0) throw std::out_of_range("Sequence is empty");

        return this->segments->GetFirst()->GetFirst();
    }

    virtual const T& GetLast() const override {
        if (totalSize == 0) throw std::out_of_range("Sequence is empty");

        return this->segments->GetLast()->GetLast();
    }

    virtual const T& Get(int index) const override {
        if (index < 0 || index >= totalSize) {
            throw std::out_of_range("Index out of range");
        }
        
        auto [segment, segmentIndex, localIndex] = getSegmentAndOffset(index);
        return segment->Get(localIndex);
    }

    virtual T& GetFirst() override {
        if (totalSize == 0) throw std::out_of_range("Sequence is empty");

        return this->segments->GetFirst()->GetFirst();
    }

    virtual T& GetLast() override {
        if (totalSize == 0) throw std::out_of_range("Sequence is empty");

        return this->segments->GetLast()->GetLast();
    }

    virtual T& Get(int index) override {
        auto [segment, segmentIndex, localIndex] = getSegmentAndOffset(index);
        return segment->Get(localIndex);
    }

    virtual int GetLength() const override {
        return totalSize;
    }

    virtual Sequence<T>* Append(const T& item) override {
        return this->Instance()->AppendInternal(item);
    }

    virtual Sequence<T>* Prepend(const T& item) override {
        return this->Instance()->PrependInternal(item);
    }

    virtual Sequence<T>* InsertAt(const T& item, int index) override {
        return this->Instance()->InsertAtInternal(item, index);
    }

    virtual Sequence<T>* Concat(const Sequence<T>* other) override {
        return this->Instance()->ConcatInternal(other);
    }

    Sequence<T>* GetSegment(int idx) {
        return this->segments->Get(idx);
    }

    int GetSegmentsLength() const {
        return this->segments->GetLength();
    }

    int GetSegmentSize() const {
        return this->segmentSize;
    }
};

template <typename T, 
          template<typename> class SegmentSequence = MutableArraySequence,
          template<typename> class ContainerSequence = MutableArraySequence>
class MutableSegmentedSequence : public SegmentedSequence<T, SegmentSequence, ContainerSequence> {
public:
    using tag = MutableSequenceTag;
    explicit MutableSegmentedSequence(int segmentSize) : 
        SegmentedSequence<T, SegmentSequence, ContainerSequence>(segmentSize) {}

    MutableSegmentedSequence(const T* items, int count, int segmentSize) : 
        SegmentedSequence<T, SegmentSequence, ContainerSequence>(items, count, segmentSize) {}

    MutableSegmentedSequence(const Sequence<T>& other, int segmentSize) : 
        SegmentedSequence<T, SegmentSequence, ContainerSequence>(other, segmentSize) {}

    MutableSegmentedSequence(MutableSegmentedSequence&& other) noexcept : 
        SegmentedSequence<T, SegmentSequence, ContainerSequence>(std::move(other)) {}

    MutableSegmentedSequence(const MutableSegmentedSequence& other) : 
        SegmentedSequence<T, SegmentSequence, ContainerSequence>(other) {}

    virtual SegmentedSequence<T, SegmentSequence, ContainerSequence>* Instance() override {
        return this;
    }

    virtual Sequence<T>* CreateEmptySequence() const override {
        return new MutableSegmentedSequence<T, SegmentSequence, ContainerSequence>(this->GetSegmentSize());
    }
    
    virtual SegmentedSequence<T, SegmentSequence, ContainerSequence>* CreateEmptySegSequence() const override {
        return new MutableSegmentedSequence<T, SegmentSequence, ContainerSequence>(this->GetSegmentSize());
    }
};


template <typename T, 
          template<typename> class SegmentSequence = MutableArraySequence,
          template<typename> class ContainerSequence = MutableArraySequence>
class ImmutableSegmentedSequence : public SegmentedSequence<T, SegmentSequence, ContainerSequence> {
private:
    SegmentedSequence<T, SegmentSequence, ContainerSequence>* Clone() const {
        return new ImmutableSegmentedSequence<T, SegmentSequence, ContainerSequence>(*this);
    }

public:
    using tag = ImmutableSequenceTag;
    explicit ImmutableSegmentedSequence(int segmentSize) : 
        SegmentedSequence<T, SegmentSequence, ContainerSequence>(segmentSize) {}
    
    ImmutableSegmentedSequence(const T* items, int count, int segmentSize) : 
        SegmentedSequence<T, SegmentSequence, ContainerSequence>(items, count, segmentSize) {}

    ImmutableSegmentedSequence(const Sequence<T>& other, int segmentSize) : 
        SegmentedSequence<T, SegmentSequence, ContainerSequence>(other, segmentSize) {}

    ImmutableSegmentedSequence(ImmutableSegmentedSequence&& other) noexcept : 
        SegmentedSequence<T, SegmentSequence, ContainerSequence>(std::move(other)) {}

    ImmutableSegmentedSequence(const ImmutableSegmentedSequence& other) : 
        SegmentedSequence<T, SegmentSequence, ContainerSequence>(other) {}

    virtual SegmentedSequence<T, SegmentSequence, ContainerSequence>* Instance() override {
        return Clone();
    }

    virtual Sequence<T>* CreateEmptySequence() const override {
        return new ImmutableSegmentedSequence<T, SegmentSequence, ContainerSequence>(this->GetSegmentSize());
    }

    virtual SegmentedSequence<T, SegmentSequence, ContainerSequence>* CreateEmptySegSequence() const override {
        return new ImmutableSegmentedSequence<T, SegmentSequence, ContainerSequence>(this->GetSegmentSize());
    }
};