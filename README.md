# 13# (13 Sharp)

> **ਪੰਜਾਬੀ ਭਾਸ਼ਾ ਵਿੱਚ ਕੋਡਿੰਗ** - A Punjabi-inspired programming language

13# (pronounced *"13 Sharp"*) is a programming language where the keywords are Punjabi words. Write real programs i.e. variables, loops, functions, arrays - all in Punjabi.

```
hukam
    likh "Sat Sri Akal, Duniya!"
samapt
```

> ⚠️ This is a fun/educational project. Not intended for production use.

---

## Table of Contents

- [Features](#features)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [Language Reference](#language-reference)
  - [Program Structure](#program-structure)
  - [Variables](#variables)
  - [Constants](#constants)
  - [Printing](#printing)
  - [Input](#input)
  - [Operators](#operators)
  - [Conditionals](#conditionals)
  - [Loops](#loops)
  - [Functions](#functions)
  - [Arrays](#arrays)
  - [Built-in Functions](#built-in-functions)
  - [Comments](#comments)
- [Interactive Shell](#interactive-shell)
- [Examples](#examples)
- [Keyword Reference](#keyword-reference)
- [Contributing](#contributing)
- [License](#license)

---

## Features

- **Punjabi keywords** : `likh`, `suno`, `je`, `nhite`, `dohrao`, `jad tak`, `seva` and more
- **Interactive shell** : just double-click the `.exe`, works like Python's REPL
- **File mode** : run `.13` files from the command line
- **Dynamic typing** : numbers, strings, booleans, arrays
- **Functions** : define and call with `seva` / `wapas`
- **Loops** : for loops (`dohrao`) and while loops (`jad tak`)
- **Arrays** : create, index and update with `sangat`
- **Constants** : declare with `pakka`
- **Logic operators** : `te` (and), `ya` (or), `na` (not)
- **String concatenation** : `"Sat Sri Akal " + naam`
- **Built-in functions** : math, string, array utilities
- **Single-file C source** : compile anywhere with GCC

---

## Installation

### Option 1: Compile from Source (Recommended)

You need [GCC](https://gcc.gnu.org/) installed.

```bash
# Clone the repo
git clone https://github.com/eishnoordhanjal/13-sharp.git
cd 13sharp

# Compile
gcc -O2 -o 13sharp 13sharp.c -lm

# Linux/macOS: run
./13sharp myprogram.13

# Windows: compile
gcc -O2 -o 13sharp.exe 13sharp.c -lm
13sharp.exe myprogram.13
```

### Option 2: Windows - Double-Click

Download `13sharp.exe` from [Releases](https://github.com/eishnoordhanjal/13-sharp/releases) and double-click it. The interactive shell opens automatically.

### Option 3: MinGW / TDM-GCC on Windows

```cmd
gcc -O2 -o 13sharp.exe 13sharp.c -lm
```

---

## Quick Start

### Hello World

Create a file `hello.13`:

```
hukam
    likh "Sat Sri Akal, Duniya!"
samapt
```

Run it:

```bash
./13sharp hello.13
```

Output:
```
Sat Sri Akal, Duniya!
```

### Interactive Shell

Just run `13sharp` with no arguments (or double-click the `.exe`):

```
$ ./13sharp

╔══════════════════════════════════════════════╗
║   13# (13 Sharp) - Punjabi Language v3.0     ║
║   Interactive Shell  |  'madad' for help     ║
║   'band' ya Ctrl+C to exit                   ║
╚══════════════════════════════════════════════╝

>>> ank x = 10
>>> likh x * x
100
>>> cheez naam = "Taranpreet"
>>> likh "Sat Sri Akal " + naam
Sat Sri Akal Taranpreet
>>> band
Sat Sri Akal! 🙏
```

---

## Language Reference

### Program Structure

A 13# program can be written in two styles:

**With `hukam` block** (recommended for files):
```
#wjkkwjkf

hukam
    likh "Mera pehla program!"
samapt
```

**Without `hukam`** (works too, especially in the shell):
```
ank x = 5
likh x
```

`hukam` marks the main entry point. `samapt` closes every block (like `}` in C or `end` in Ruby). `#wjkkwjkf` is the optional program header, a blessing.

---

### Variables

| Keyword  | Type    | Example                        |
|----------|---------|--------------------------------|
| `ank`    | Number  | `ank umar = 25`                |
| `cheez`  | String  | `cheez naam = "Gurpreet"`      |
| `halat`  | Boolean | `halat active = sach`          |
| `sangat` | Array   | `sangat scores = [10, 20, 30]` |

```
ank umar = 25
ank price = 99.99
cheez naam = "Taranpreet Singh"
halat active = sach
sangat scores = [85, 90, 78, 100]
```

Variables are dynamically typed, you can reassign them to any type:
```
ank x = 5
x = "hun main string haan"
likh x
```

---

### Constants

Use `pakka` to declare a constant that cannot be changed:

```
pakka ank PI = 3.14159
pakka cheez SITE = "punjab.dev"

likh PI
likh SITE
```

Trying to change a `pakka` variable prints an error:
```
PI = 3    → Error: 'PI' pakka hai, badal nahi sakde
```

---

### Printing

`likh` prints to the console. It supports multiple values separated by commas:

```
likh "Sat Sri Akal!"
likh 42
likh "Naam:", naam, "Umar:", umar
likh x + y
likh               ← prints a blank line
```

Numbers print without unnecessary decimals:
```
likh 10        → 10
likh 3.14      → 3.14
likh 10 / 3    → 3.33333
```

Booleans print as `sach` or `jhooth`.
Nil prints as `khaali`.

---

### Input

`suno` reads a line from the user. It automatically converts to a number if the input looks like one, otherwise stores as a string.

```
suno naam                           ← no prompt, just wait
suno "Apna naam likho: " naam       ← with a prompt
suno "Umar dasao: " umar

likh "Sat Sri Akal " + naam
likh "Tusi " + cheez(umar) + " saal de ho"
```

---

### Operators

**Arithmetic:**

| Operator | Meaning        | Example       |
|----------|----------------|---------------|
| `+`      | Add / Concat   | `x + y`       |
| `-`      | Subtract       | `x - y`       |
| `*`      | Multiply       | `x * y`       |
| `/`      | Divide         | `x / y`       |
| `%`      | Modulo         | `x % y`       |
| `^`      | Power          | `x ^ 2`       |

**Comparison:**

| Operator | Meaning               |
|----------|-----------------------|
| `==`     | Equal to              |
| `!=`     | Not equal to          |
| `>`      | Greater than          |
| `<`      | Less than             |
| `>=`     | Greater than or equal |
| `<=`     | Less than or equal    |

**Logical:**

| Keyword | Meaning | Example                    |
|---------|---------|----------------------------|
| `te`    | and     | `x > 5 te x < 10`         |
| `ya`    | or      | `x < 0 ya x > 100`        |
| `na`    | not     | `na active`                |

**Compound Assignment:**

```
x += 5     ← x = x + 5
x -= 3     ← x = x - 3
x *= 2     ← x = x * 2
x /= 4     ← x = x / 4
```

**String Concatenation:**

The `+` operator joins strings. Numbers are auto-converted when mixed:
```
cheez s = "Score: " + 95       → "Score: 95"
cheez msg = naam + " ji!"
```

---

### Conditionals

```
je condition
    ...
samapt
```

With else:
```
je umar >= 18
    likh "Tuci Vadde ho"
nhite
    likh "Tuci Bache ho"
samapt
```

Nested:
```
je marks >= 90
    likh "A Grade"
nhite
    je marks >= 75
        likh "B Grade"
    nhite
        je marks >= 60
            likh "C Grade"
        nhite
            likh "Fail"
        samapt
    samapt
samapt
```

Logical operators in conditions:
```
je x > 5 te x < 10
    likh "x panj te das de vich hai"
samapt

je naam == "Gurpreet" ya naam == "Taranpreet"
    likh "Jaanpehchan naam!"
samapt

je na active
    likh "Active nahi"
samapt
```

---

### Loops

**For Loop — `dohrao`**

```
dohrao i ton 1 to 10
    likh i
samapt
```

With a custom step using `kadam`:
```
dohrao i ton 0 to 20 kadam 5
    likh i
samapt
```

Count down (negative step):
```
dohrao i ton 10 to 1 kadam -1
    likh i
samapt
```

**While Loop — `jad tak`**

```
ank count = 1
jad tak count <= 5
    likh count
    count += 1
samapt
```

**Loop Control:**

| Keyword | Meaning               |
|---------|-----------------------|
| `rok`   | Break out of the loop |
| `agle`  | Skip to next iteration (continue) |

```
dohrao i ton 1 to 10
    je i == 5
        rok
    samapt
    likh i
samapt
```

```
dohrao i ton 1 to 10
    je i % 2 == 0
        agle
    samapt
    likh i          ← prints only odd numbers
samapt
```

---

### Functions

Define with `seva`, return with `wapas`, close with `samapt`:

```
seva jodhna(a, b)
    wapas a + b
samapt

ank result = jodhna(10, 20)
likh result           → 30
```

Functions can call themselves recursively:
```
seva factorial(n)
    je n <= 1
        wapas 1
    samapt
    wapas n * factorial(n - 1)
samapt

likh factorial(5)     → 120
likh factorial(10)    → 3628800
```

Multiple parameters:
```
seva greet(naam, umar)
    cheez msg = "Sat Sri Akal " + naam + "! Tusi " + cheez(umar) + " saal de ho."
    likh msg
samapt

greet("Gurpreet", 25)
```

Functions can be defined before or after `hukam` — the interpreter registers them all first.

---

### Arrays

Declare with `sangat`:

```
sangat scores = [85, 90, 78, 100, 92]
sangat names  = ["Gurpreet", "Taranpreet", "Harpreet"]
sangat mixed  = [1, "do", sach, 4.5]
```

**Access by index** (0-based):
```
likh scores[0]    → 85
likh names[1]     → Taranpreet
```

**Update an element:**
```
scores[2] = 99
likh scores[2]    → 99
```

**Get length:**
```
likh lambayi(scores)    → 5
```

**Loop through an array:**
```
dohrao i ton 0 to lambayi(scores) - 1
    likh "Score", i + 1, ":", scores[i]
samapt
```

---

### Built-in Functions

**Math:**

| Function         | Description              | Example                   |
|------------------|--------------------------|---------------------------|
| `gol(x)`         | Round to nearest integer | `gol(3.7)` → `4`          |
| `ghat(x)`        | Floor (round down)       | `ghat(3.9)` → `3`         |
| `vadh(x)`        | Ceil (round up)          | `vadh(3.1)` → `4`         |
| `mutlaq(x)`      | Absolute value           | `mutlaq(-5)` → `5`        |
| `varg(x)`        | Square root              | `varg(16)` → `4`          |
| `taaqat(x, y)`   | Power (x to the y)       | `taaqat(2, 8)` → `256`    |
| `bakiya(x, y)`   | Modulo (remainder)       | `bakiya(10, 3)` → `1`     |
| `vadda(a, b, ...)` | Maximum value          | `vadda(3, 9, 5)` → `9`    |
| `chhota(a, b, ...)` | Minimum value         | `chhota(3, 9, 5)` → `3`   |

**String:**

| Function       | Description                | Example                        |
|----------------|----------------------------|--------------------------------|
| `lambayi(s)`   | Length of string or array  | `lambayi("hello")` → `5`       |
| `upar(s)`      | Convert to uppercase       | `upar("hello")` → `HELLO`      |
| `thhalle(s)`   | Convert to lowercase       | `thhalle("HELLO")` → `hello`   |

**Type Conversion:**

| Function    | Description             | Example                    |
|-------------|-------------------------|----------------------------|
| `ank(x)`    | Convert to number       | `ank("42")` → `42`         |
| `cheez(x)`  | Convert to string       | `cheez(99)` → `"99"`       |
| `halat(x)`  | Convert to boolean      | `halat(0)` → `jhooth`      |

---

### Comments

Single-line comment with `waak`:
```
waak Ik line da comment
ank x = 5    waak ← this is also valid after code
```

Multi-line comment with `waak[` ... `]`:
```
waak[
    Ik vada comment
    jo kai lines te failaya hua hai
    koi bhi gall likho ithey
]
```

---

## Interactive Shell

Run `13sharp` with no arguments to open the interactive shell:

```
$ ./13sharp
>>> ank x = 5
>>> ank y = 3
>>> likh x + y
8
>>> seva square(n)
...     wapas n * n
... samapt
(seva nondhi ho gayi)
>>> likh square(7)
49
>>> likh square(x)
25
```

**Shell Prompts:**
- `>>>` - ready for a new statement
- `...` - inside a multi-line block (je / dohrao / jad tak / seva), keep typing

**Shell Commands:**

| Command         | Action                         |
|-----------------|--------------------------------|
| `madad`         | Show help and keyword list     |
| `saaf`          | Clear all variables            |
| `band` / `exit` | Exit the shell                 |
| `Ctrl+C`        | Force exit                     |

**Shell Features:**
- Variables and functions **persist** across entries in the same session
- Define a function once, call it as many times as you want
- Multi-line blocks work naturally - keep typing until `samapt`

---

## Examples

### Calculator
```
hukam
    likh "=== Simple Calculator ==="
    suno "Pehla number: " a
    suno "Dooja number: " b

    likh "Jod:    ", a + b
    likh "Ghatao: ", a - b
    likh "Guna:   ", a * b
    je b != 0
        likh "Bhag:   ", a / b
    nhite
        likh "Bhag: zero nahi ho sakda!"
    samapt
samapt
```

### Multiplication Table
```
hukam
    suno "Kis number di table chahidi? " num
    likh "=== " + cheez(num) + " di table ==="
    dohrao i ton 1 to 10
        likh num, "x", i, "=", num * i
    samapt
samapt
```

### Fibonacci
```
seva fibonacci(n)
    je n <= 1
        wapas n
    samapt
    wapas fibonacci(n - 1) + fibonacci(n - 2)
samapt

hukam
    likh "Fibonacci Sequence (pehle 10):"
    dohrao i ton 0 to 9
        likh fibonacci(i)
    samapt
samapt
```

### Grade Checker
```
hukam
    suno "Apne marks dasao (0-100): " marks

    je marks >= 90
        likh "Grade: A — Shabash!"
    nhite
        je marks >= 75
            likh "Grade: B — Bahut Vadiya!"
        nhite
            je marks >= 60
                likh "Grade: C — Theek Hai"
            nhite
                je marks >= 40
                    likh "Grade: D — Aur Mehnat Karo"
                nhite
                    likh "Grade: F — Himmat rakho!"
                samapt
            samapt
        samapt
    samapt
samapt
```

### Array Average
```
seva average(arr)
    ank total = 0
    ank n = lambayi(arr)
    dohrao i ton 0 to n - 1
        total += arr[i]
    samapt
    wapas total / n
samapt

hukam
    sangat scores = [85, 92, 78, 96, 88, 74, 91]
    likh "Scores:", scores
    likh "Average:", average(scores)
    likh "Highest:", vadda(85, 92, 78, 96, 88, 74, 91)
    likh "Lowest:",  chhota(85, 92, 78, 96, 88, 74, 91)
samapt
```

### FizzBuzz
```
hukam
    dohrao i ton 1 to 30
        je i % 15 == 0
            likh "FizzBuzz"
        nhite
            je i % 3 == 0
                likh "Fizz"
            nhite
                je i % 5 == 0
                    likh "Buzz"
                nhite
                    likh i
                samapt
            samapt
        samapt
    samapt
samapt
```

### Number Guessing Game
```
hukam
    pakka ank SECRET = 42
    ank tries = 0
    halat guessed = jhooth

    likh "=== Andaaza Lagao! (1-100) ==="

    jad tak na guessed
        suno "Tera andaaza: " guess
        tries += 1

        je guess == SECRET
            likh "Bilkul sahi! " + cheez(tries) + " tries vich!"
            guessed = sach
        nhite
            je guess < SECRET
                likh "Vadda karo!"
            nhite
                likh "Chhota karo!"
            samapt
        samapt
    samapt
samapt
```

---

## Keyword Reference

| 13# Keyword | English Equivalent | Example                        |
|-------------|-------------------|--------------------------------|
| `ank`       | number            | `ank x = 5`                   |
| `cheez`     | string            | `cheez s = "hi"`              |
| `halat`     | boolean           | `halat b = sach`              |
| `sangat`    | array             | `sangat a = [1,2,3]`          |
| `pakka`     | constant          | `pakka ank MAX = 100`         |
| `likh`      | print             | `likh "hello"`                |
| `suno`      | input             | `suno "Enter: " x`            |
| `je`        | if                | `je x > 5`                    |
| `nhite`     | else              | `nhite`                       |
| `samapt`    | end               | `samapt`                      |
| `dohrao`    | for               | `dohrao i ton 1 to 10`        |
| `ton`       | from              | (used with `dohrao`)          |
| `to`        | to                | (used with `dohrao`)          |
| `kadam`     | step              | `dohrao i ton 0 to 10 kadam 2`|
| `jad tak`   | while             | `jad tak x > 0`               |
| `rok`       | break             | `rok`                         |
| `agle`      | continue          | `agle`                        |
| `seva`      | function          | `seva jodhna(a, b)`           |
| `wapas`     | return            | `wapas a + b`                 |
| `sach`      | true              | `halat b = sach`              |
| `jhooth`    | false             | `halat b = jhooth`            |
| `khaali`    | nil/null          | `likh khaali`                 |
| `te`        | and               | `x > 0 te x < 10`            |
| `ya`        | or                | `x < 0 ya x > 100`           |
| `na`        | not               | `na active`                   |
| `waak`      | comment           | `waak This is a comment`      |
| `hukam`     | main block        | `hukam ... samapt`            |
| `band`      | exit (shell)      | `band`                        |
| `madad`     | help (shell)      | `madad`                       |
| `saaf`      | clear (shell)     | `saaf`                        |
| `#wjkkwjkf` | program start     | (optional file header)        |

---

## Contributing

Pull requests are welcome! Ideas for contribution:

- More built-in functions (string split, file I/O, etc.)
- Better error messages with line numbers
- A web-based playground / IDE
- Syntax highlighting for VS Code
- More example programs

Please open an issue before starting large changes.

---

## License

MIT License. See [LICENSE](LICENSE) for details.

---
