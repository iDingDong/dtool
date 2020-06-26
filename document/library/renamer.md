# Renamer APIs

For examples, see `source/drename.cpp`.

## Class `dtool::renamer::ItemIndex`

Defined in header `dtool/renamer.hpp`.

This class behaves like a pointer when performing arithmetic operations. It can be initialized with an integer. The range of its values shall be [1, std::numeric_limits<std::ptrdiff_t>::max()]. Creation of an out-of-range value throws an exception of type `dtool::renamer::BadItemIndex` derrived from `std::runtime_errror`.

### Member function `dtool::renamer::ItemIndex::underlyingIndex`

```cpp
auto underlyingIndex() const noexcept -> std::size_t
```

Returns the corresponding index(offset to the first element) used in a sequence container.

### Member function `dtool::renamer::ItemIndex::fromUnderlyingIndex`

```cpp
auto fromUnderlyingIndex(std::size_t index) const noexcept -> std::dtool::renamer::ItemIndex
```

Make a item index from an index(offset to the first element) used in a sequence container.

#### Parameters of `dtool::renamer::ItemIndex::fromUnderlyingIndex`

`index`: The index(offset to the first element) used in a sequence container.

## Class `dtool::renamer::Pattern`

Defined in header `dtool/renamer.hpp`.

This class is copy constructible.

### Constructors of `dtool::renamer::Pattern`

```cpp
Pattern(std::string_view rawPattern);
```

Construct a pattern. If the `rawPattern` is not a valid pattern, throw an exception of type `dtool::renamer::BadPattern` derrived from `std::runtime_errror`.

#### Parameters of constructors of `dtool::renamer::Pattern`

- `rawPattern`: The user-defined pattern. For more details about pattern, please run `drename -h`.

### Member function `dtool::renamer::Pattern::raw`

```cpp
auto raw() const -> std::string;
```

Returns the pattern in string form. It should be equivalent to the user's input.

## Class `dtool::renamer::Core`

Defined in header `dtool/renamer.hpp`.

### Member type `dtool::renamer::Core::Paths`

```cpp
using Paths = /* unspecified */;
```

An objects of dtool::renamer::Core::Paths represents a set of paths.

Guaranteed to be default constructible and copy constructible. Pass an object of `std::filesystem::path` to its non-static member function `insert` as the only argument to insert a path.

Contains

### Member type `dtool::renamer::Core::Preview`

```cpp
struct Preview {
	Core::Paths::iterator origin;
	std::string newName;
};
```

Each object of `dtool::renamer::Core::Preview` demonstrates how a file(represented by `origin`) will be renamed(represented by `newName`).

### Member type `dtool::renamer::Core::Previews`

```cpp
using Previews = /* unspecified */;
```

A randomly indexable sequence container of objects `dtool::renamer::Core::Preview`. It might be `std::vector<std::filesystem::path>` or `std::deque<std::filesystem::path>`, or another type that is compatible to their common operations.

### Member type `dtool::renamer::Core::Action`

```cpp
using Action = /* unspecified */;
```

The return type of `dtool::renamer::Core::ActionHandler`

### Member type `dtool::renamer::Core::ActionHandler`

```cpp
std::function<auto (dtool::renamer::Pattern const& currentPattern, dtool::renamer::Core::Previews const& currentPreview) -> dtool::renamer::Core::Action>;
```

The type of an action handler of drename service core.

When an action handler returns when called during an interaction(see `dtool::renamer::Core::interact`), a corresponding action is taken according to the value it stores. The possible values that can be returned are:

- A copy of `dtool::renamer::Core::NO_OP`: no action will be taken.
- An object of `dtool::renamer::Core::DoneChoice`: if the value is `dtool::renamer::Core::DoneChoice::CONFIRM`, apply the current pattern and exit interaction; if the value is `dtool::renamer::Core::DoneChoice::ABORT`, do nothing but exit interaction.
- An object of `dtool::renamer::Pattern`: change the current pattern to this value.
- An object of `dtool::renamer::Core::SwapInfo`: swap the two files at index `dtool::renamer::Core::SwapInfo::left` and `dtool::renamer::Core::SwapInfo::right` of `currentPreview`.
- An object of `dtool::renamer::Core::ReorderMethod`: reorder the files in `currentPreview` according to the value.
- An object of `dtool::renamer::Core::AddInfo`: add a file specified by `dtool::renamer::Core::AddInfo::path`.
- An object of `dtool::renamer::Core::RemoveInfo`: remove a file at index `dtool::renamer::Core::RemoveInfo::index` of `currentPreview`.

### Static member object `dtool::renamer::Core::NO_OP`

```cpp
static /* unspecified */ constexpr NO_OP = /* unspecified */;
```

See `dtool::renamer::Core::ActionHandler` for more.

### Member type `dtool::renamer::Core::DoneChoice`

```cpp
enum class DoneChoice {
	CONFIRM,
	ABORT
};
```

See `dtool::renamer::Core::ActionHandler` for more.

### Member type `dtool::renamer::Core::SwapInfo`

```cpp
struct SwapInfo {
	public: dtool::renamer::ItemIndex left;
	public: dtool::renamer::ItemIndex right;
};
```

See `dtool::renamer::Core::ActionHandler` for more.

### Member type `dtool::renamer::Core::ReorderMethod`

```cpp
enum class ReorderMethod {
	SORT_BY_NAME,
	SORT_BY_MODIFIED_TIME,
	REVERSE
};
```

Each value represent a reorder method:

- `dtool::renamer::Core::ReorderMethod::SORT_BY_NAME`: Sort by file name.
- `dtool::renamer::Core::ReorderMethod::SORT_BY_MODIFIED_TIME`: Sort by file name.
- `dtool::renamer::Core::ReorderMethod::REVERSE`: Reverse.

All sort methods sort ascending if not otherwise specified.

See `dtool::renamer::Core::ActionHandler` for more.

### Member type `dtool::renamer::Core::AddInfo`

```cpp
struct AddInfo {
	std::filesystem::path path;
};
```

See `dtool::renamer::Core::ActionHandler` for more.

### Member type `dtool::renamer::Core::RemoveInfo`

```cpp
struct RemoveInfo {
	dtool::renamer::ItemIndex index;
};
```

See `dtool::renamer::Core::ActionHandler` for more.

### Constructors of `dtool::renamer::Core`

```cpp
Core(dtool::renamer::Core::ActionHandler actionHandler);
```

Constructs a drename service core and register an action handler to it.

#### Parameters of constructors of `dtool::renamer::Core`

- `actionHandler`: the action handler to register.

### Member function `dtool::renamer::Core::interact`

```cpp
auto interact(dtool::renamer::Core::Paths const& inputPaths) -> void;
auto interact(
	dtool::renamer::Pattern pattern = Pattern("{o}"),
	dtool::renamer::Core::Paths const& inputPaths = dtool::renamer::Core::Paths()
) -> void;
```

Starts interaction which repeatedly calls the registered action handler and execute actions according to its return value.

A pattern and file list will make the interaction stateful. Read-only references of thier current value will be passed to the action handler.

#### Parameters of `dtool::renamer::Core::interact`

- `inputPaths`: The initial paths to files to be processed.
- `pattern`: The initial pattern.
