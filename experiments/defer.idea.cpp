template <typename TFunc>
struct Deferrer
{
    TFunc func;

    ~Deferrer()
    {
        func();
    }
}

#define defer(name, stuff) Deferrer defer_##name{ [&](){ stuff }}

{
    int * a = new int[3222];
    defer(a, delete[] a;)
}