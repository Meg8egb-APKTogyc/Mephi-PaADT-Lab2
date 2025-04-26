#include "headers/DynamicArray.hpp"
#include "headers/LinkedList.hpp"
#include "headers/Sequence.hpp"
#include "headers/SequenceTester.hpp"
#include "headers/SegmentedSequence.hpp"

int pow2(int x) {
    return x * x;
}


int main() {
/*
    int* a = new int[3];
    a[0] = 1;
    a[1] = 2;
    a[2] = 3;
    Sequence<int>* immutableSeq = new ImmutableListSequence<int>(a, 3);
    Sequence<int>* immutableSeq3 = immutableSeq->GetSubsequence(2, 0);
    std::cout << immutableSeq3->GetLength() << std::endl;
    std::cout << (*immutableSeq3)[0] << ' ' << (*immutableSeq3)[1] << std::endl;

    std::function<int(int)> f_map = pow2;
    Sequence<int>* result = immutableSeq3->Map(pow2);
    std::cout << result->GetLength() << std::endl;

    for (auto el : *result) {
        std::cout << el << ' ';
    }
    std::cout << std::endl;

    auto* zipped = zip(immutableSeq3, result);

    for (auto el : *zipped) {
        std::cout << "(" << el.first << ", " << el.second << ") ";
    }
    std::cout << std::endl;

    MutableArraySequence<MutableArraySequence<int>*>* obj = new MutableArraySequence<MutableArraySequence<int>*>();
    obj->Append(new MutableArraySequence<int>(a, 3));
    std::cout << obj->Get(0)->Get(1) << std::endl;

    for (auto el : *obj->Get(0)) {
        std::cout << el << ' ';
    }
    std::cout << std::endl;
*/
    ManualSequenceTester tester;
    tester.runInteractiveTest();

    return 0;
}
