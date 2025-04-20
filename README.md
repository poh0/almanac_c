# almanac_c
 Simple command line calendar utility. Written in C.
- Keep in mind that this is my first actual C program.
- Help, suggestions, criticism and pull requests are welcome.

## TODO
- Add notes to arbitrary year/month
- Edit notes
- Add multiple notes to a date
- Show multiple months at once

## Demo
Green color indicates current date and yellow is a date with note (significant date)

<p align="center">
<img src="images/demo.gif">
</p>

## Usage

Print calendar of current month
```sh
$ alm
```

Print the note of a date (if there is one)
```sh
$ alm <date_num>
```

Add a note to a date
```sh
$ alm sig <date_num>
```

Remove note of a date
```sh
$ alm rm <date_num>
```

Where ```date_num``` is the day of the current month.

Notes are saved to ``~/.alma.txt``
or in the executable directory if using the debug build.

## Building
just run ``$ make`` or ``$ make debug`` for debug build.
Tested with clang and gcc


## Installation
```sh
$ git clone https://github.com/poh0/almanac_c.git
$ make && make install
```

## License

The license for this project can be found in the [LICENSE](./LICENSE) file.
