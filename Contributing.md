# NOELLE Contribution Guidelines

We are happy to have contributors to our project. 
If you'd like to contribute a feature, bug fix, or other component to Nautilus, we ask that you first read these guidelines.


## Table of Contents
- [Coding style](#coding_style)


## Coding Style
Next is what we would like to do or avoid in our codebase:

1 - No function body in the headers

2 - No use of integer built-in data types (e.g., `int`, `long int`), but use the ones that make the bitwidth explicit (e.g., `uint32_t`, `int32_t`)

3 - The scopes of control structures must be within `{}`
  For example, avoid writing code like
```
while (condition) f();
```

instead, write
```
while (condition) {
  f();
}
```

  Also, avoid
```
if (condition) something();
```
instead, write
```
if (condition) {
  something();
}
```

4 - Always create and name temporary variables.
  Avoid coding like 
```
myF(myG());
```

  Instead, create a temporary variable and name it to help the reader understanding the meaning of that value.
  Something like:
```
auto loopToParallelize = myG();
myF(loopToParallelize);
```

5 - Name your variables in the most meaningful way.
  Variable names are used to read and understand the code. Take advantage of it to make enjoyable reading your code.

6 - Make inputs of a function as explicit as possible. 
  Implicit inputs (e.g., object fields) makes often the code harder to understand.

7 - Name headers with the suffix `.hpp` because NOELLE is written in C++

8 - Add on top of your header file the following:

```
#pragma once
```

This is a replacement of the old style approach:

```
#ifdef MYFILE_H
#define MYFILE_H

#endif
```

9 - Name APIs using the camel case as NOELLE is developed following the object oriented paradigm
  e.g., `myFunction()` rather than `my_function()`

10 - Avoid adding paths (relative or not) in the include command
  E.g., avoid doing the following:
```
#include "../include/MyAPI.h"
```
  
  Instead, write
```
#include "MyAPI.h"
```
and then modify the cmake file to add the include directory in the command line

11 - Include `SystemHeaders.hpp` in your header file. 
`SystemHeaders.hpp` includes the most common C++ and LLVM headers.
Then, include your header file in your `.cpp`.

12 - Avoid using macros to implement functions. Macros make debugging a nightmare.

13 - Use `auto` to make your code less verbose. Modern editors can tell you automatically the type when your cursor goes on top of the variable (if you use `vim`, take a look at our configuration at 
```
https://github.com/scampanoni/vim_configuration
```
