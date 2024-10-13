### Interop

To improve interoperability with other programming languages, the following c++ features are not used in the public API:
- std::unique_ptr/std::shared_ptr
- std::vector
- templates

These features are still used in implementation.

A common pattern to replace vector in the public API is simply a structure like so:

```cpp
#define DZ_MAX_SEMAPHORES 16
    struct Semaphores
    {
        size_t      NumElements;
        ISemaphore *Array[ DZ_MAX_SEMAPHORES ];
    };
```

`struct <Structure with 'I' + plural(s)>`