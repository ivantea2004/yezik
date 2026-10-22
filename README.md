# Yezik programming language

## Examples
### Hello, world!
```ts
    import 'std.yezik'
    
    const main = (): c_int => {
        print('Hello, world!\n')
        return 0
    }
```
### Adding numbers
```js
    import 'std.yezik'

    const add_numbers = (a: s32, b: s32): void => {
        print_ddd('{} + {} = {}\n', a, b, a + b)
    }

    const main = (): c_int => {
        add_numbers(2, 3)
        add_number(42, 25)
        return 0
    }
```

## How to build program

### Windows (gcc)
    cmd /c build.bat main.yezik build\out.exe gcc "-std=c89 -pedantic -g -O0 -Wall -Wextra -Werror -D_CRT_SECURE_NO_WARNINGS"

### Windows (msvc)
Run using [MVSC Build Tools from command line](https://learn.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=msvc-170)

    cmd /c build.bat main.yezik build\out.exe cl "/Zi /Ob0 /Od /RTC1 /W4 /WX /D_CRT_SECURE_NO_WARNINGS"

## Linux
    sh build.sh main.yezik build/out gcc "-std=c89 -pedantic -g -O0 -Wall -Wextra -Werror"

## Editors support

[Visual Studio Code extension](vscode-extension/README.md)
