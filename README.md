# almanac_c
 Simple command line calendar utility. Written in C.
- Keep in mind that this is my first actual C program.
- Help, suggestions, criticism and pull requests are welcome.

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

Where ```date_num``` is the day of the actual month.

## Installation
```sh
$ git clone https://github.com/poh0/almanac_c.git
$ make && make install
```

## License

The license for this project can be found in the [LICENSE](./LICENSE) file.
