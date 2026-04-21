```markdown
# 13# Programming Language

![13# Logo](https://github.com/eishnoordhanjal/13-sharp/blob/main/13sharplogo.png)

**13#** (pronounced "13 Sharp") is a Punjabi inspired programming language interpreter with simple syntax, dynamic typing and automatic memory management. This is a fun project created to bring programming closer to Punjabi speakers and celebrate Punjabi culture through code.

> ⚠️ Note: This is a FUN project created for educational and entertainment purposes. It's not intended for production use.

---

## 📜 Table of Contents
- [Features](#features)
- [Installation](#installation)
- [Language Basics](#language-basics)
- [Commands Reference](#commands-reference)
- [Examples](#examples)
- [Running Programs](#running-programs)
- [Contributing](#contributing)
- [License](#license)

---

## Features

-  **Punjabi-based keywords** - Write code using Punjabi vocabulary
-  **Simple syntax** - Easy to read and write, minimal punctuation
-  **Dynamic typing** - Variables can hold different types automatically
-  **Automatic memory management** - No manual memory allocation needed
-  **Array support** - Built in array data structure
-  **Functions** - Create reusable code blocks
-  **Loops** - For loops (`dohrao`) and While loops (`jad tak`)
-  **Conditionals** - If else statements (`je`/`nhite`)
-  **Input/Output** - Print (`likh`) and Input (`suno`) operations
-  **Comments** - Single and multi-line comment support

---

## Installation

### Method 1: Web IDE (Easiest)
Simply open `ide.html` in any modern web browser & no installation required!

### Method 2: Windows Executable
Double-click `13sharp.exe` and use the interactive REPL mode.

---

## Language Basics

### Program Structure
Every 13# program starts with `#wjkkwjkf` and contains a `hukam` (command) block:

```13
#wjkkwjkf

hukam
    // Your code here
samapt
```

### Hello World
```13
#wjkkwjkf

hukam
    likh "Sat Sri Akal!"
    likh "Welcome to 13# Programming Language"
samapt
```

---

## 📚 Commands Reference

### 1. Variables & Data Types

| Keyword | Type | Description |
|---------|------|-------------|
| `ank` | Number | Integer or floating point number |
| `cheez` | String | Text string |
| `halat` | Boolean | True/False value |
| `sangat` | Array | Collection of values |

#### Examples:
```13
ank age = 25
ank price = 99.99
cheez name = "Taranpreet Singh Makkar"
halat isActive = sach
sangat numbers = [1, 2, 3, 4, 5]
```

### 2. Constants
Use `pakka` to create constants (cannot be changed):

```13
pakka ank PI = 3.14159
pakka cheez GREETING = "Hello, Taranpreet Singh"
```

### 3. Input/Output

| Command | Description |
|---------|-------------|
| `likh` | Print to console |
| `suno` | Read input from user |

#### Examples:
```13
likh "What is your name?"
suno name
likh "Hello"
likh name
```

### 4. Arithmetic Operators

| Operator | Operation |
|----------|-----------|
| `+` | Addition |
| `-` | Subtraction |
| `*` | Multiplication |
| `/` | Division |
| `%` | Modulo |
| `^` | Power |

#### Example:
```13
ank a = 10
ank b = 3
ank sum = a + b
ank product = a * b
ank power = a ^ 2
```

### 5. Comparison Operators

| Operator | Meaning |
|----------|---------|
| `=` | Equal to |
| `!=` | Not equal to |
| `>` | Greater than |
| `<` | Less than |
| `>=` | Greater than or equal |
| `<=` | Less than or equal |

### 6. Logical Operators

| Keyword | English | Operation |
|---------|---------|-----------|
| `te` | and | Logical AND |
| `ya` | or | Logical OR |
| `na` | not | Logical NOT |

#### Example:
```13
je x > 5 te x < 10
    likh "x is between 5 and 10"
samapt
```

### 7. Conditional Statements

```13
je age >= 18
    likh "Tuci Bache ho"
nhite
    likh "Tuci Vadde ho"
samapt
```

### 8. Loops

#### For Loop (`dohrao`):
```13
dohrao i ton 1 to 10
    likh i
samapt
```

#### While Loop (`jad tak`):
```13
ank count = 1
jad tak count <= 5
    likh count
    count = count + 1
samapt
```

### 9. Functions

```13
seva addNumbers(a, b)
    wapas a + b
samapt

ank result = addNumbers(10, 20)
likh result  // Output: 30
```

### 10. Arrays

```13
sangat teachings = ["Naam Japo", "Vand Chakko", "Kirat Karo"]
likh teachings[0]  // Output: Naam Japo

ank scores = [85, 90, 78, 92]
ank total = scores[0] + scores[1]
```

### 11. Comments

```13
waak This is a single-line comment

waak[
    This is a
    multi-line comment
    for longer explanations
]
```

---

## 💡 Examples

### Example 1: Simple Calculator
```13
#wjkkwjkf

hukam
    likh "Simple Calculator"
    likh "Enter first number:"
    suno num1
    likh "Enter second number:"
    suno num2
    
    ank sum = num1 + num2
    ank diff = num1 - num2
    ank product = num1 * num2
    
    likh "Sum:"
    likh sum
    likh "Difference:"
    likh diff
    likh "Product:"
    likh product
samapt
```


### Example 2: Multiplication Table
```13
#wjkkwjkf

hukam
    likh "Enter a number:"
    suno num
    
    likh "Multiplication Table for"
    likh num
    
    dohrao i ton 1 to 10
        ank result = num * i
        likh num
        likh " x "
        likh i
        likh " = "
        likh result
    samapt
samapt
```
---

## Running Programs

### Web IDE
1. Open `13sharp_ide.html` in your browser
2. Write or paste your code
3. Click "Run Code" button
4. View output in the console panel

### Windows
```bash
# Double-click 13sharp.exe for REPL mode
# Or run from command prompt:
13sharp.exe myprogram.13
```

---

## Keywords Quick Reference

| 13# Keyword | English Equivalent |
|-------------|-------------------|
| `ank` | number |
| `cheez` | string |
| `halat` | boolean |
| `sangat` | array |
| `likh` | print |
| `suno` | input |
| `je` | if |
| `nhite` | else |
| `samapt` | end/close |
| `dohrao` | for |
| `ton` | from |
| `to` | to |
| `jad tak` | while |
| `seva` | function |
| `wapas` | return |
| `pakka` | constant |
| `sach` | true |
| `jhooth` | false |
| `te` | and |
| `ya` | or |
| `na` | not |
| `waak` | comment |
| `hukam` | command block |
| `wjkkwjkf` | program start |

---

## 🤝 Contributing

Contributions are welcome! Since this is a fun project, feel free to:

- Report bugs
- Suggest new features
- Improve documentation
- Add more examples
- Translate documentation to Punjabi

### How to Contribute
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

---

## License

This project is licensed under the GNU License.

---

## Acknowledgments

- Inspired by Punjabi culture and language
- Thanks to the Punjabi computing community
- Built with ❤️ for fun and learning

---

## Connect

- **GitHub**: [@eishnoordhanjal](https://github.com/eishnoordhanjal)
- **Project Repository**: [13-sharp](https://github.com/eishnoordhanjal/13-sharp)

---

## Show Your Support

If you find this project interesting or fun, please give it a ⭐ on GitHub!

---

**Sat Sri Akal!** 🙏  
*Happy Coding in 13#!*

