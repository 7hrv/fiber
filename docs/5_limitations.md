# Limitations

## Cooperative Yields

The only way context switching occurs between fibers is if one fiber explicitly yields
control. If a fiber fails to yield control, all other fibers are starved.


