#pragma once
#include "Sequence.hpp"
#include "SegmentedSequence.hpp"
#include <functional>


class ManualSequenceTester {
public:
    void runInteractiveTest() {
        while(true) {
            printMainMenu();
            int typeChoice;
            std::cin >> typeChoice;
    
            if(typeChoice == 4) break;
    
            switch(typeChoice) {
                case 1: testWithType<int>(); break;
                case 2: testWithType<double>(); break;
                case 3: testWithType<std::string>(); break;
                default: std::cout << "Invalid choice!\n";
            }
        }
    }

private:
    template <typename T>
    std::string printSequence(Sequence<T>* seq) {
        if (auto* segSeq = dynamic_cast<SegmentedSequence<T>*>(seq)) {
            std::string ret = "[ ";
            for (int i = 0; i < segSeq->GetSegmentsLength(); ++i) {
                if (i != 0) ret += ", ";
                ret += printSequence(segSeq->GetSegment(i));
            }
            ret += " ]";
            return ret;
        }
        
        std::string ret = "[";
        for (int i = 0; i < seq->GetLength(); ++i) {
            if (i != 0) ret += ", ";
            ret += std::to_string(seq->Get(i));
        }
        ret += "]";
        return ret;
    }

    template <typename T>
    void testWithType() {
        std::cout << "\n=== Testing ===\n";
        printImplementationMenu();
        
        int implChoice;
        std::cin >> implChoice;
    
        if(implChoice == 7) {
            testNestedSequences<T>();
            return;
        }
    
        Sequence<T>* seq = createSequence<T>(implChoice);
        if(seq) {
            testImplementation(seq, implChoice);
            delete seq;
        } else {
            std::cout << "Invalid implementation choice!\n";
        }
    }

    template <typename T>
    void testImplementation(Sequence<T>* seq, int choice) {
        std::cout << "\n=== Testing ===\n";
    
        printTestResult(seq->GetLength() == 0, "Initial length is 0");

        T sample1 = getSampleValue<T>(0);
        seq = seq->Append(sample1);
        printTestResult(seq->GetLength() == 1, "Length after append");
        printTestResult(seq->GetLast() == sample1, "GetLast() after append");

        T sample2 = getSampleValue<T>(1);
        seq = seq->Prepend(sample2);
        printTestResult(seq->GetFirst() == sample2, "GetFirst() after prepend");

        T sample3 = getSampleValue<T>(2);
        seq = seq->InsertAt(sample3, 1);
        seq = seq->InsertAt(sample3, 1);
        printTestResult(seq->Get(1) == sample3, "InsertAt middle position");

        Sequence<T>* mapped = seq->Map(getMapFunction<T>());
        printTestResult(mapped->GetLength() == seq->GetLength(), "Applied map function: " + printSequence(seq) + " -> " + printSequence(mapped));
        delete mapped;

        Sequence<T>* whereed = seq->Where(getWhereFunction<T>());
        printTestResult(whereed->GetLength() < seq->GetLength(), "Applied where function: " + printSequence(seq) + " -> " + printSequence(whereed));
        delete whereed;

        try {
            seq->Get(-1);
            printTestResult(0, "Get(-1) does not throw exception");
        } catch (const std::out_of_range& e) {
            printTestResult(1, "Get(-1) throws exception. Message: " + std::string(e.what()));
        } catch (...) {
            printTestResult(0, "Get(-1) throws unexpected exception");
        }

        T sample4 = getSampleValue<T>(1);
        int len = seq->GetLength();
        seq->Append(sample4);
        printTestResult(seq->GetLength() == len + choice % 2, "Mutable/Immutable test");

        T* arr = new T[3];
        arr[0] = sample1;
        arr[1] = sample2;
        arr[2] = sample3;
        Sequence<T>* seq2 = new MutableArraySequence<T>(arr, 3);
        printTestResult(seq2->GetLength() == 3, "Created MutableArraySequence from array");
        delete[] arr;
        delete seq2;
    }

    template <typename T>
    void testNestedSequences() {
        std::cout << "\n=== Testing Nested Sequences ===\n";
        std::cout << "Choose outer sequence implementation:\n";
        printImplementation2Menu();
        
        int outerChoice;
        std::cin >> outerChoice;
        
        std::cout << "Choose inner sequence implementation:\n";
        printImplementation2Menu();
        
        int innerChoice;
        std::cin >> innerChoice;
    
        using InnerSeq = Sequence<T>*;
        using NestedSeq = Sequence<InnerSeq>*;
        
        NestedSeq outerSeq = createSequence<InnerSeq>(outerChoice);
        
        for(int i = 0; i < 3; i++) {
            InnerSeq innerSeq = createSequence<T>(innerChoice);
            for(int j = 0; j < 3; j++) {
                innerSeq = innerSeq->Append(getSampleValue<T>(i*3 + j));
            }
            outerSeq = outerSeq->Append(innerSeq);
        }
        
        std::cout << "Nested sequence structure:\n";
        for(int i = 0; i < outerSeq->GetLength(); i++) {
            InnerSeq inner = outerSeq->Get(i);
            std::cout << "Sequence " << i << ": [";
            for(int j = 0; j < inner->GetLength(); j++) {
                std::cout << inner->Get(j);
                if(j < inner->GetLength() - 1) std::cout << ", ";
            }
            std::cout << "]\n";
        }
        
        for(int i = 0; i < outerSeq->GetLength(); i++) {
            delete outerSeq->Get(i);
        }
        delete outerSeq;
    }

    template <typename T>
    T getSampleValue(size_t index) {
        static_assert(sizeof(T) == 0, "Not implemented for this type");
        return T();
    }

    template <typename T>
    std::function<T(T)> getMapFunction() {
        static_assert(sizeof(T) == 0, "Not implemented for this type");
        return T();
    }

    template <typename T>
    std::function<bool(T)> getWhereFunction() {
        static_assert(sizeof(T) == 0, "Not implemented for this type");
        return T();
    }

    void printMainMenu() {
        std::cout << "\n=== Main Menu ===\n"
        << "Choose data type:\n"
        << "1. int\n"
        << "2. double\n"
        << "3. std::string\n"
        << "4. Exit\n"
        << "Your choice: ";
    }
    void printImplementationMenu() {
        std::cout << "Choose implementation:\n"
        << "1. MutableArraySequence\n"
        << "2. ImmutableArraySequence\n"
        << "3. MutableListSequence\n"
        << "4. ImmutableListSequence\n"
        << "5. MutableSegmentedSequence\n"
        << "6. ImmutableSegmentedSequence\n"
        << "7. Test nested sequences\n"
        << "Your choice: ";
    }
    void printImplementation2Menu() {
        std::cout << "Choose implementation:\n"
        << "1. MutableArraySequence\n"
        << "2. ImmutableArraySequence\n"
        << "3. MutableListSequence\n"
        << "4. ImmutableListSequence\n"
        << "5. MutableSegmentedSequence\n"
        << "6. ImmutableSegmentedSequence\n"
        << "Your choice: ";
    }
    void printTestMenu() {
        
    }
    void printTestResult(bool success, const std::string& message) {
        std::cout << (success ? "[PASS] " : "[FAIL] ") << message << "\n";
    }
    
    template <typename T>
    Sequence<T>* createSequence(int choice) {
        switch(choice) {
            case 1: return new MutableArraySequence<T>();
            case 2: return new ImmutableArraySequence<T>();
            case 3: return new MutableListSequence<T>();
            case 4: return new ImmutableListSequence<T>();
            case 5: return new MutableSegmentedSequence<T>(3);
            case 6: return new ImmutableSegmentedSequence<T>(3);
            default: return nullptr;
        }
    }
};


template <>
inline std::string ManualSequenceTester::printSequence(Sequence<std::string>* seq) {
    if (auto* segSeq = dynamic_cast<SegmentedSequence<std::string>*>(seq)) {
        std::string ret = "[ ";
        for (int i = 0; i < segSeq->GetSegmentsLength(); ++i) {
            if (i != 0) ret += ", ";
            ret += printSequence(segSeq->GetSegment(i));
        }
        ret += " ]";
        return ret;
    }
    
    std::string ret = "[";
    for (int i = 0; i < seq->GetLength(); ++i) {
        if (i != 0) ret += ", ";
        ret += "\"" + seq->Get(i) + "\"";
    }
    ret += "]";
    return ret;
}


template <>
inline int ManualSequenceTester::getSampleValue<int>(size_t index) {
    static int samples[] = {10, 20, 30};
    return samples[index % 3];
}

template <>
inline double ManualSequenceTester::getSampleValue<double>(size_t index) {
    static double samples[] = {1.1, 2.2, 3.3};
    return samples[index % 3];
}

template <>
inline std::string ManualSequenceTester::getSampleValue<std::string>(size_t index) {
    static std::string samples[] = {"apple", "banana", "cherry"};
    return samples[index % 3];
}

template <>
inline std::function<int(int)> ManualSequenceTester::getMapFunction<int>() {
    return [](int x) {
        return x + 1;
    };
}

template <>
inline std::function<double(double)> ManualSequenceTester::getMapFunction<double>() {
    return [](double x) {
        return x + 2;
    };
}

template <>
inline std::function<std::string(std::string)> ManualSequenceTester::getMapFunction<std::string>() {
    return [](std::string str) {
        return str + "!";
    };
}

template <>
inline std::function<bool(int)> ManualSequenceTester::getWhereFunction<int>() {
    return [](int x) {
        return x > 10;
    };
}

template <>
inline std::function<bool(double)> ManualSequenceTester::getWhereFunction<double>() {
    return [](double x) {
        return x > 2;
    };
}

template <>
inline std::function<bool(std::string)> ManualSequenceTester::getWhereFunction<std::string>() {
    return [](std::string str) {
        return str.size() > 5;
    };
}