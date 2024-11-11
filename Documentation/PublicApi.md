### Interop

To improve interoperability with other programming languages, the following c++ features are not used in the public API:
- std::unique_ptr/std::shared_ptr
- std::vector
- templates
- std::string

These features are still used in implementation.

Instead InteropString, and InteropArray are used.