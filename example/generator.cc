#include "../include/coasync/generator.hpp"

#include <iostream>

template<typename T>
struct Tree
{
    T value;
    Tree *left{}, *right{};

    coasync::generator<const T&> traverse_inorder() const
    {
        if (left)
						co_yield coasync::ranges::elements_of(left->traverse_inorder());

        co_yield value;
        if (right)
            for (const T& x : right->traverse_inorder())
                co_yield x;
    }
};

int main()
{
    Tree<char> tree[]
    {
                                    {'D', tree + 1, tree + 2},
        //                            ©¦
        //            ©°©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©Ø©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©´
        //            ©¦                                ©¦
                    {'B', tree + 3, tree + 4},       {'F', tree + 5, tree + 6},
        //            ©¦                                ©¦
        //  ©°©¤©¤©¤©¤©¤©¤©¤©¤©¤©Ø©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©´      ©°©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©Ø©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©´
        //  ©¦                       ©¦      ©¦                         ©¦
          {'A'},                  {'C'}, {'E'},                    {'G'}
    };

    for (char x : tree->traverse_inorder())
        std::cout << x << ' ';
    std::cout << '\n';
}
