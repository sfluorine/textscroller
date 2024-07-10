# textscroller

This is a simple text scroller program written in C. Simulating a scrolling effect.

## Requirements

- A C compiler (e.g., `gcc`)
- [tup](https://gittup.org/tup) build system

## Installation

1. Clone the repository or download the source code.
2. Open a terminal and navigate to the directory containing the source code.
3. Compile the program using the following command:
    ```sh
    tup init
    tup
    ```

## Usage

Run the compiled program from the terminal:
```sh
./bin/textscroller
```

You can customize the text and the number of shifts directly in the source code.

## Example

    ```sh
    ./bin/textscroller -d 700 "hello, this is a quite long text."
    ```

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.
