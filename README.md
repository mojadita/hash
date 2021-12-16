## HASH: a simple implementation of a hash table.

This implementation allows to handle hash tables of string based keys (you
can provide a hash function and a comparison function to the constructor)
to references (the keys are strdup()ed, but the pointed references not, so
be careful)

The keys can be strings, null terminated, or not, as you need to provide a
function to calculate the key size (to allocate memory for the copy)  I'm
still deciding if it should be better to provide a complete copy allocator
for the stored keys.  For now the keys are duplicated by calling the
`sizeof_f`  function and making a copy (with `malloc()` and `memcpy()`)
but a complete allocator would be better.

The implementation doesn't resize itself, so no provision is made to
implement a resizable hash table (if you want to resize it you must
allocate a new hash table and copy all the values to the new one)

I plan to make it more generic, with `void *` keys, instead of `char *`,
and full copy allocators
