# MacOS USB

Using `ioreg`

```sh
ioreg -p IOUSB
```

To show details add `-l`
```sh
ioreg -p IOUSB -l
```

Only the device names
```sh
ioreg -p IOUSB -w0 | sed 's/[^o]*o //; s/@.*$//' | grep -v '^Root.*'
```
