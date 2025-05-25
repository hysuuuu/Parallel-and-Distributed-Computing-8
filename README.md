`balance.c` include two types of load balancing strategies: `strict_balance`(default) and `relaxed_balance`.

To change to `relaxed_balance`, replace the call to `strict_balance` in `main()` with the commented part.


# Usage

```
$ make
$ ./balance <num_processors = {5, 10, 100}> 
```